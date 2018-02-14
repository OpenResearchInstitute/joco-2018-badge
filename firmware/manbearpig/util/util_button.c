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

#define BUTTON_COUNT				6		//This must be less than or equal to GPIOTE low power even channel count
#define BUTTON_UP					25
#define BUTTON_DOWN					28
#define BUTTON_LEFT					26
#define BUTTON_RIGHT				27
#define BUTTON_ACTION				20
#define TICKS_PER_100MS				APP_TIMER_TICKS(100, 0)

static volatile uint8_t button_state = 0;

void __gpiote_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

	uint8_t mask = 0;
	uint8_t state = nrf_gpio_pin_read(pin);

	switch (pin) {
	case BUTTON_UP:
		mask = util_tilt_inverted() ? BUTTON_MASK_DOWN : BUTTON_MASK_UP;
		break;
	case BUTTON_DOWN:
		mask = util_tilt_inverted() ? BUTTON_MASK_UP : BUTTON_MASK_DOWN;
		break;
	case BUTTON_LEFT:
		mask = util_tilt_inverted() ? BUTTON_MASK_RIGHT : BUTTON_MASK_LEFT;
		break;
	case BUTTON_RIGHT:
		mask = util_tilt_inverted() ? BUTTON_MASK_LEFT : BUTTON_MASK_RIGHT;
		break;
	case BUTTON_ACTION:
		mask = BUTTON_MASK_ACTION;
		break;
	}

	if (state == 0) {
		button_state |= mask;
	} else {
		button_state &= ~mask;
	}
}

void util_button_clear() {
	button_state = 0;
}

void util_button_init() {
	uint32_t err_code;

	nrf_gpio_cfg_input(BUTTON_DOWN, NRF_GPIO_PIN_PULLDOWN);

	err_code = nrf_drv_gpiote_init();
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
	in_config.pull = NRF_GPIO_PIN_PULLUP;

	err_code = nrf_drv_gpiote_in_init(BUTTON_UP, &in_config, __gpiote_handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_gpiote_in_init(BUTTON_DOWN, &in_config, __gpiote_handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_gpiote_in_init(BUTTON_LEFT, &in_config, __gpiote_handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_gpiote_in_init(BUTTON_RIGHT, &in_config, __gpiote_handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_gpiote_in_init(BUTTON_ACTION, &in_config, __gpiote_handler);
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_event_enable(BUTTON_UP, true);
	nrf_drv_gpiote_in_event_enable(BUTTON_DOWN, true);
	nrf_drv_gpiote_in_event_enable(BUTTON_LEFT, true);
	nrf_drv_gpiote_in_event_enable(BUTTON_RIGHT, true);
	nrf_drv_gpiote_in_event_enable(BUTTON_ACTION, true);
}

inline uint8_t util_button_state() {
	return button_state;
}

/**
 * Block until button press
 */
uint8_t util_button_wait() {
	uint32_t err_code;
	uint8_t button = util_button_state();
	while (button == 0 && util_gfx_is_valid_state()) {
		err_code = sd_app_evt_wait();
		APP_ERROR_CHECK(err_code);

		button = util_button_state();

		//Work on anything in the scheduler queue
		app_sched_execute();
	}
	return button;
}

uint8_t util_button_down() {
	return button_state & BUTTON_MASK_DOWN;
}
uint8_t util_button_up() {
	return button_state & BUTTON_MASK_UP;
}
uint8_t util_button_left() {
	return button_state & BUTTON_MASK_LEFT;
}
uint8_t util_button_right() {
	return button_state & BUTTON_MASK_RIGHT;
}
uint8_t util_button_action() {
	return button_state & BUTTON_MASK_ACTION;
}
