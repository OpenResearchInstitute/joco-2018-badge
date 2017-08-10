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
 *****************************************************************************/
#include "../system.h"

uint32_t m_millis_offset = 0;

/**
 Get index of char c in text
 */
int16_t util_index_of(char *p_text, char c) {
	//Treat null like a space
	if (c == 0) {
		c = ' ';
	}

	//Walk through the char array looking for the character
	for (uint8_t i = 0; i < strlen(p_text); i++) {
		if (p_text[i] == c) {
			return i;
		}
	}
	return -1;
}

uint16_t util_get_device_id() {
	return (uint16_t) (NRF_FICR->DEVICEID[0]);
}

/**
 * Returns approximation since LFCLK was started
 */
uint32_t util_millis() {
	//LFCLK runs at 32khz
	return m_millis_offset + (app_timer_cnt_get() / 32);
}

void util_millis_offset_set(uint32_t offset) {
	//Don't jump ahead more than 3 days
	if (offset < (1000 * 60 * 60 * 24 * 3)) {
		m_millis_offset += offset;
	}
}

void util_timers_start() {
//	uint32_t err_code;
	APP_TIMER_INIT(UTIL_TIMER_PRESCALER, UTIL_TIMER_OP_QUEUE_SIZE, false);
//	err_code = app_timer_create(&m_timer_1, APP_TIMER_MODE_REPEATED, __timer_handler);
//	APP_ERROR_CHECK(err_code);
//
//	err_code = app_timer_start(m_timer_1, APP_TIMER_TICKS(1, UTIL_TIMER_PRESCALER), NULL);
//	APP_ERROR_CHECK(err_code);
}
