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
#include "system.h"

#define APP_SCHED_MAX_EVENT_SIZE    48   	/**< Maximum size of scheduler events. */
#define APP_SCHED_QUEUE_SIZE        32   	/**< Maximum number of events in the scheduler queue. */

static uint8_t boot_frame;
static void __boot_frame_callback(uint8_t frame, void *p_data) {
	if (boot_frame < 128) {
            util_led_set_all(0, 0, boot_frame * 2);
	} else if (boot_frame < 200) {
		util_led_set(util_math_rand8_max(LED_COUNT), 255, 255, 255);
	} else {
		uint8_t b = (222 - boot_frame) * 11;
		util_led_set_all(b, b, b);
	}

	util_led_show();

	//This is a bit of a hack but it only allows animation to play once
	if (boot_frame > 220) {
		util_gfx_draw_raw_file_stop();
                boot_frame = 1;
	}
        boot_frame++;
}

void __boot() {
	mbp_ui_cls();
	util_led_clear();

	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_cursor(0, 4);
	util_gfx_set_color(COLOR_WHITE);
	util_gfx_print("JOCO Cruise 2018\n");
	nrf_delay_ms(400);
	util_gfx_print("64K RAM SYSTEM 38911 BYTES FREE\n");
	nrf_delay_ms(1000);

	util_gfx_print("READY\n");
	nrf_delay_ms(400);
	util_gfx_print("# /bin/joco --gui\n");
	nrf_delay_ms(400);
	util_gfx_print("Launching...");
	nrf_delay_ms(1000);

	//Loop the intro animation. Loop=true allows user to quit
	//The callback will manually force the raw to stop
	util_gfx_draw_raw_file("JINTRO.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, __boot_frame_callback, true, NULL);

	util_led_clear();
}

int main(void) {
#ifndef BLE_ONLY_BUILD
	//Start up the timers
	util_timers_start();

	//Enable terminal
	mbp_term_init();

	hello_init();

	// set up the lists we use to track badges we hear
	ble_lists_init();

	//Start up BLE, this starts the LFCLK needs to happen first
	//If airplane mode we'll disable later
	util_ble_init();

	APP_SCHED_INIT(APP_SCHED_MAX_EVENT_SIZE, APP_SCHED_QUEUE_SIZE);

	//Fire up the HW RNG
	util_math_rand_start();

	//Init ili9163 LCD
	st7735_init();
	st7735_start();
	util_gfx_init();

	//Init buttons - this needs to happen in case an error is popped up
	util_button_init();

	//Init crypto utilities. This should happen before state loading.
	util_crypto_init();

	//Init the SD
	bool sd_available = util_sd_init();

	//Init the LEDs
	util_led_init();
	util_led_clear();

	//Init tilt sensor, this should happen before SD so it's available in self test
	util_tilt_start();

	//Test for SD, if not, then run POST
	if (!sd_available) {
		mbp_system_test();
	}

	if (!mbp_state_load()) {
		mbp_ui_popup("JOCO2018", "Welcome to the JoCo Cruise 2018 Badge! Select a name. You can change your name later from settings.");
		mbp_state_new();
		mbp_system_name_select();
		mbp_state_save();
	}

	// Near-Field Comms setup
	util_nfc_init();
	util_nfc_start();

	//Airplane mode?
	if (mbp_state_airplane_mode_get()) {
		util_ble_off();
	} else {
		util_ble_on();
	}

	//Startup game
	// TBD
	score_ble_init();

	//Start terminal
	mbp_term_start();

	//Clear LEDs and LCD
	util_led_clear();
	mbp_ui_cls();

	//Read seekrit resistor and set unlocked state appropriately
        //State persists across boot
	if (mbp_system_seekrit_get()) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_SEEKRIT);
	}

	__boot();

	util_gfx_set_font(FONT_SMALL);

//	mbp_system_test();

#ifdef UNLOCK_ALL_BLING
        mbp_state_unlock_set(0xFFFF);
#endif

	while (1) {
		app_sched_execute();
		mbp_menu_main();

		mbp_system_unlock_state();
	}
#else
	util_ble_init();
	util_ble_advertising_start();
	while (1) {
		//do nothing
	}
#endif
}

/**@brief Function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing ASSERT call.
 * @param[in] p_file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name) {
	uint8_t test = 100;
	UNUSED_VARIABLE(test);
	app_error_handler(0xDEADBEEF, line_num, p_file_name);
}

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(pc);

	error_info_t *error_info = (error_info_t *) info;
	UNUSED_VARIABLE(error_info);
}

void HardFault_Handler(void) {
	uint32_t *sp = (uint32_t *) __get_MSP(); // Get stack pointer
	uint32_t ia = sp[24 / 4]; // Get instruction address from stack

	printf("Hard Fault at address: 0x%08x\r\n", (unsigned int) ia);
	while (1)
		;
}
