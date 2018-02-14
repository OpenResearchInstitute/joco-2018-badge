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

#define TILT_CHECK_TIME_MS			20
#define TILT_PIN					19
#define TILT_DEBOUNCE_TIME_MS		500

APP_TIMER_DEF(m_tilt_timer);

static bool m_inverted = false;
static uint32_t m_debounce_time = 0xFFFFFFFF;

static void __tilt_timer_handler(void *p_data) {
	bool state = (nrf_gpio_pin_read(TILT_PIN) == 0);

	//If there's a change in state
	if (state != m_inverted) {

		//If we've passed the debounce time, then invert
		if (util_millis() > m_debounce_time) {
			//Only continue if the user allows screen tilt and the SD card is in
			if (util_sd_available() && !mbp_state_tilt_get()) {
				return;
			}

			//Invert!
			app_sched_event_put(&state, sizeof(state), util_gfx_inverted_schedule_handler);

			m_debounce_time = 0xFFFFFFFF;
		} else if (m_debounce_time == 0xFFFFFFFF) {
			m_debounce_time = util_millis() + TILT_DEBOUNCE_TIME_MS;
		} else {
			//end up here if we're waiting out debounce time
		}
	}
}

bool util_tilt_inverted() {
	return m_inverted;
}

void util_tilt_inverted_set(bool inverted) {
	m_inverted = inverted;
}

void util_tilt_start() {
	nrf_gpio_cfg_input(TILT_PIN, NRF_GPIO_PIN_PULLUP);

	APP_ERROR_CHECK(app_timer_create(&m_tilt_timer, APP_TIMER_MODE_REPEATED, __tilt_timer_handler));
	APP_ERROR_CHECK(app_timer_start(m_tilt_timer, APP_TIMER_TICKS(TILT_CHECK_TIME_MS, UTIL_TIMER_PRESCALER), NULL));
}
