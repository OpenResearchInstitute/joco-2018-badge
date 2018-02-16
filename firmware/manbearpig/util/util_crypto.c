/*****************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 * 	@sconklin
 * 	@mustbeart
 * 	@abraxas3d
 *****************************************************************************/
#include "../system.h"

#define	UTIL_CRYPTO_KEY_LEN			16
#define UTIL_CRYPTO_DEVICEID_OFFSET	14	// offset in the key
#define UTIL_CRYPTO_COUNTER_LEN		4

// This can stay in ROM.
static const uint8_t	key[UTIL_CRYPTO_KEY_LEN] = {
	0x4d, 0x92, 0x91, 0x63,
	0x8b, 0x4b, 0x46, 0xd0,
	0x67, 0x8d, 0x67, 0x53,
	0xa3, 0xc8, 0xf0, 0x01
};


/**********************************
 * Interface to the crypto library
 **********************************/
static nrf_ecb_hal_data_t m_ecb_data;

static uint32_t __crypt(util_crypto_cryptable_t *cryptable) {
	uint32_t err_code;
	uint8_t block_size;
	uint16_t offset = 0;

	// The message will be changed in-place, encrypted or decrypted.
	uint8_t	*buf = cryptable->message;
	uint16_t 	length;

	if (cryptable->data_len > UTIL_CRYPTO_CRYPTINFO_LENGTH) {
		length = cryptable->data_len - UTIL_CRYPTO_CRYPTINFO_LENGTH;
	} else {
	return NRF_ERROR_DATA_SIZE;
	}

	// The library doesn't touch our message text. It just encrypts,
	// electronic-codebook style, the counter and nonce (that is, the
	// cryptinfo). We then XOR the resulting data with our message to
	// encrypt OR decrypt. Start by loading up the cryptinfo into the
	// library's data structure.

	// Copy the initial counter value into the library's data structure
 	(*((uint32_t*) m_ecb_data.cleartext)) = cryptable->cryptinfo.counter;

 	// Copy the nonce into the library's data structure
 	memcpy(&m_ecb_data.cleartext[UTIL_CRYPTO_COUNTER_LEN],
			cryptable->cryptinfo.nonce, UTIL_CRYPTO_NONCE_LEN);


	//Iterate through all blocks of data, XORing with encrypted ctr+nonce.
	while (length > 0) {

		//Determine how big of a block to do, limited by ECB_KEY_LEN
		//This ensures we don't run off the end of the memory and start encrypting stuff we shouldnt
		block_size = MIN(length, UTIL_CRYPTO_KEY_LEN);

		//Do the library encryption by generating a block of ciphertext;
		// quit with any failure
		err_code = sd_ecb_block_encrypt(&m_ecb_data);
		if (NRF_SUCCESS != err_code) {
			return err_code;
		}

		//XOR the private data with the chunk of ciphertext
		for (uint8_t i = 0; i < block_size; i++) {
			uint8_t v = buf[offset] ^ m_ecb_data.ciphertext[i];
			buf[offset] = v;
			offset++;
		}

		// Increment the counter in the library's data structure
		(*((uint32_t*) m_ecb_data.cleartext))++;

		//Less work to do
		length -= block_size;
	}

	return NRF_SUCCESS;
}


/**
 * @brief Initialize crypto utilities.
 * @details Modifies the hard-coded key with our device_id, so that each
 * device is using a different key.
 *
 * @param[inout]	key		the encryption key for all operations
 */
extern void util_crypto_init(void) {
	uint16_t device_id = util_get_device_id();

	// Save the customized key into the library's data structure.
	memcpy(m_ecb_data.key, key, UTIL_CRYPTO_KEY_LEN);
	m_ecb_data.key[UTIL_CRYPTO_DEVICEID_OFFSET] = (device_id >> 8) & 0xFF;
	m_ecb_data.key[UTIL_CRYPTO_DEVICEID_OFFSET+1] = device_id & 0xFF;
}


/**
 * @brief Uses the RNG to write a nonce to a buffer.
 * @details Size of the nonce in bytes is NONCE_RAND_BYTE_LEN.
 *	The argument is a pointer into the buffer where the nonce goes.
 *
 * @param[in]    p_buf    A buffer of length at least NONCE_RAND_BYTE_LEN
 */
static void __generate_nonce(uint8_t * p_buf) {
	uint8_t i = 0;
	uint8_t remaining = UTIL_CRYPTO_NONCE_LEN;

// The random number pool may not contain enough bytes at the moment so
// a busy wait may be necessary.
	while (remaining > 0) {
		p_buf[i] = util_math_rand8();
		i++;
		remaining--;
	}
}


/**
 * @brief Interface function for encryption.
 * @details See util_crypto.h for usage instructions.
 * The counter value in cryptinfo is left at 0; we want to store the
 * counter value for the first block of the message.
 * NOTE that counter handling is different from the AND!XOR code.
 * We reset counter to 0 on every encrypt, which is OK because we
 * (and they) always generate a new nonce on every encrypt.
 *
 * @param[inout]	cryptable	pointer to a cryptable
 */
uint32_t util_crypto_encrypt(util_crypto_cryptable_t *cryptable){
	uint32_t err_code;

	cryptable->cryptinfo.counter = 0;
	__generate_nonce(cryptable->cryptinfo.nonce);
	err_code = __crypt(cryptable);
	cryptable->cryptinfo.counter = 0;

	return err_code;
}

/**
 * @brief Interface function for decryption.
 * @detail See util_crypto.h for usage instructions.
 *
 * @param[inout]	cryptable	pointer to a cryptable
 */
uint32_t util_crypto_decrypt(util_crypto_cryptable_t *cryptable){
	uint32_t	err_code;

	err_code = __crypt(cryptable);

	return err_code;
}
