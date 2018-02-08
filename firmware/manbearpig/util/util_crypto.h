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

 #ifndef UTIL_CRYPTO_H_
 #define UTIL_CRYPTO_H_

#define	UTIL_CRYPTO_CRYPTINFO_LENGTH	16
#define UTIL_CRYPTO_NONCE_LEN			12

#define	UTIL_CRYPTO_STORAGE(x)	(x.cryptable.cryptinfo)
#define	UTIL_CRYPTO_STORAGE_LEN(x)	(sizeof(x) - sizeof(x.cryptable.data_len))

// Treat this struct as opaque. You don't need to know!
typedef	struct {
	uint32_t	counter;
	uint8_t		nonce[UTIL_CRYPTO_NONCE_LEN];
} __attribute__ ((packed)) util_crypto_cryptinfo_t;
STATIC_ASSERT(sizeof(util_crypto_cryptinfo_t) == UTIL_CRYPTO_CRYPTINFO_LENGTH);

// See below for instructions on using this struct.
typedef	struct {
	uint16_t				data_len;	// bytes in the cryptinfo+message fields
	util_crypto_cryptinfo_t	cryptinfo;
	uint8_t					message[];
} __attribute__ ((packed)) util_crypto_cryptable_t;
STATIC_ASSERT(sizeof(util_crypto_cryptable_t) == UTIL_CRYPTO_CRYPTINFO_LENGTH+2);


// Initialize crypto utilities. Call once before any other functions.
extern void util_crypto_init(void);

// Encrypt any in-memory data.
// To use, place your cleartext into message[], and initialize
// data_len = CRYPTINFO_LENGTH + the size of your cleartext in bytes.
// Call the function.
// Then store or transmit data_len bytes starting at cryptinfo.
// Returns NRF_SUCCESS on success, something else on failure.
extern uint32_t util_crypto_encrypt(util_crypto_cryptable_t *cryptable);

// Decrypt any in-memory data.
// To use, receive or read from storage your encrypted data,
// write it to memory starting at cryptinfo, and initialize
// data_len = the number of bytes received or read.
// Call the function.
// Your cleartext will be in message[], replacing the cipher_text
// you put there. data_len will be unchanged.
// Returns NRF_SUCCESS on success, something else on failure.
extern uint32_t util_crypto_decrypt(util_crypto_cryptable_t *cryptable);

// You may re-use a cryptable struct.

// util_crypto has a single encryption/decryption key hard-coded,
// so you don't have to do any key management.

// util_crypto is compatible with SHADOW.DAT files written by AND!XOR
// DC25 code, and writes files it can read, but has a different policy
// for updating the counter.

 #endif /* UTIL_CRYPTO_H_ */
