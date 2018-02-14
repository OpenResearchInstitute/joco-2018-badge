/*****************************************************************************
 * (C) Copyright 2017 AND!XOR LLC (http://andnxor.com/).
 *
 * PROPRIETARY AND CONFIDENTIAL UNTIL AUGUST 1ST, 2017 then,
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
 * 	@andnxor
 * 	@zappbrandnxor
 * 	@hyr0n1
 * 	@andrewnriley
 * 	@lacosteaef
 * 	@bitstr3m
 * 
 * Further modifications made by
 *      @sconklin
 *      @mustbeart
 *
 *****************************************************************************/
#include "../system.h"

uint32_t util_math_map(uint32_t x, uint32_t in_min, uint32_t in_max,
		uint32_t out_min, uint32_t out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * @brief Function that returns a random float between 0 and 1
 */
float util_math_rand_float() {
	uint32_t r = 0;
	nrf_drv_rng_block_rand((uint8_t *) &r, sizeof(r));
	return (float) r / (float) UINT32_MAX;
}

void util_math_rand_start() {
	uint32_t err_code;
	err_code = nrf_drv_rng_init(NULL);
	APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for generating an 8 bit random number using the internal random generator.
 */
uint8_t util_math_rand8(void) {
	uint32_t err_code;
	uint8_t available = 0;
	uint8_t r;

	while (available == 0) {
		nrf_drv_rng_bytes_available(&available);
	}

	err_code = nrf_drv_rng_rand(&r, 1);
	APP_ERROR_CHECK(err_code);

	return r;
}

uint8_t util_math_rand8_max(uint8_t max) {
	float scalar = (float) max / (float) 256;
	return (uint8_t) (scalar * (float) util_math_rand8());
}

uint16_t util_math_rand16_max(uint16_t max) {
	float scalar = (float) max / (float)UINT16_MAX;
	uint16_t r = 0;
	nrf_drv_rng_block_rand((uint8_t *) &r, sizeof(r));
	return (uint16_t) (scalar * (float) r);
}

uint32_t util_math_rand32_max(uint32_t max) {
	float scalar = (float) max / (float)UINT32_MAX;
	uint32_t r = 0;
	nrf_drv_rng_block_rand((uint8_t *) &r, sizeof(r));
	return (uint32_t) (scalar * (float) r);
}

//uint8_t util_math_min(uint8_t a, uint8_t b) {
//	if (a < b) {
//		return a;
//	} else {
//		return b;
//	}
//}
