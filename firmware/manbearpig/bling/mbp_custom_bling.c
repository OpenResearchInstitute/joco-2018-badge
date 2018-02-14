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

#define ANIMATION_MAX	70	//Save the heap space!
#define LED_FPS			20

APP_TIMER_DEF(m_timer);
UTIL_LED_ANIM_INIT(m_anim);

//static void __led_chase_cw_callback(void *p_data) {
//	uint8_t *p_index = (uint8_t *) p_data;
//	uint8_t order[] = { 0, 1, 2, 3, 7, 11, 10, 9, 8, 4 };
//
//	util_led_set(order[*p_index], 255, 0, 0);
//	util_led_show();
//	util_led_set(order[*p_index], 0, 0, 0);
//
//	(*p_index)++;
//	if (*p_index > 10) {
//		*p_index = 0;
//	}
//}

//static void __led_glow_callback(void *p_data) {
//	//Unpack the data
//	uint8_t *data = (uint8_t *) p_data;
//	float value = (float) data[0] / 100.0;
//	float value_delta = (data[1] == 1) ? 0.015 : -0.015;
//	float hue = (float) data[2] / 100.0;
//
//	//Current RGB value
//	uint32_t rgb = util_led_hsv_to_rgb(hue, 1.0, value);
//
//	util_led_set_all_rgb(rgb);
//	util_led_show();
//
//	value += value_delta;
//
//	//Reverse
//	if (value > 1.0) {
//		value = 1.0;
//		value_delta = -0.015;
//	}
//	if (value < 0) {
//		value = 0;
//		value_delta = 0.015;
//	}
//
//	//Change hue on action button click
//	if (util_button_action() > 0) {
//		util_button_clear();
//		hue += 0.1;
//		while (hue >= 1.0) {
//			hue -= 1.0;
//		}
//	}
//
//	//Pack all the data
//	data[0] = (uint8_t) (value * 100.0);
//	if (value_delta > 0) {
//		data[1] = 1;
//	} else {
//		data[1] = 0;
//	}
//	data[2] = (uint8_t) (hue * 100.0);
//}

//static void __led_hue_cycle_callback(void *p_data) {
//	uint8_t *p_hue = (uint8_t *) p_data;
//	float hue = (float) (*p_hue) / 100.0;
//	uint32_t color = util_led_hsv_to_rgb(hue, .6, .6);
//	util_led_set_all_rgb(color);
//	util_led_show();
//
//	hue += .015;
//	if (hue >= 1) {
//		hue = 0;
//	}
//	*p_hue = (hue * 100.0);
//}

static void __led_rgb_scheduler_handler(void * p_event_data, uint16_t event_size) {
	util_led_play_rgb_frame(&m_anim);
}

static void __led_rgb_callback(void *p_data) {
	app_sched_event_put(NULL, 0, __led_rgb_scheduler_handler);
}

/**
 Per-frame callback for rainbow
 */
//static void __led_rainbow_callback(void *p_data) {
//	mbp_bling_led_rainbow_callback(0, p_data);
//}
///**
// * Per-frame callback for animating the LEDs randomly
// */
//static void __led_random_callback(void *p_data) {
//	uint8_t r = util_math_rand8_max(100);
//	float hue = (float) r / 100.0;
//	uint32_t rgb = util_led_hsv_to_rgb(hue, 1, 1);
//	for (uint8_t i = 0; i <= LED_MATRIX_LAST_INDEX; i++) {
//		util_led_set_rgb(i, rgb);
//	}
//	util_led_show();
//}
//
///**
// * Snake around the mouth of bender!
// */
//static void __led_snake_callback(void *p_data) {
//	uint8_t *p_head = (uint8_t *) p_data;
//	uint8_t head = *p_head;
//	uint8_t step = 255 / 4;
//	uint8_t led_order_cw[] = LED_ORDER_CW;
//
//	//Clear all
//	util_led_set_all(0, 0, 0);
//
//	for (uint8_t i = 0; i < 3; i++) {
//		uint8_t ii = (head + i) % LED_ORDER_CW_COUNT;
//		int16_t brightness = 255 - (step * i);
//		if (brightness < 0) {
//			brightness = 0;
//		}
//		util_led_set(led_order_cw[ii], 0, brightness, 0);
//	}
//	util_led_show();
//
//	//wrap around
//	*p_head = (head + 1) % LED_ORDER_CW_COUNT;
//}
static void __menu_custom_anim_callback(void *p_data) {
	uint32_t err_code;

	menu_t menu;
	menu_item_t items[100];
	menu.items = items;
	menu.title = "LED Mode";
	menu.count = 0;
	menu.selected = 0;
	menu.top = 0;

	FRESULT result;
	DIR dir;
	static FILINFO fno;

	result = f_opendir(&dir, "BLING"); /* Open the directory */
	if (result == FR_OK) {
		for (;;) {
			result = f_readdir(&dir, &fno); /* Read a directory item */
			if (result != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR) { /* It is a directory */
				//ignore
			} else { /* It is a file. */
				char *ext = strrchr(fno.fname, '.') + 1;

				//Look for RGB LED files
				if (strcmp(ext, "RGB") == 0) {

					menu_item_t item;
					item.callback = NULL;
					item.icon = NULL;
					item.preview = NULL;
					item.text = (char *) malloc(16);
					item.data = (char *) malloc(20);

					snprintf(item.text, ext - fno.fname, "%s", fno.fname);
					sprintf(item.data, "BLING/%s", fno.fname);
					items[menu.count++] = item;
				}
			}

			if (menu.count >= 100) {
				break;
			}
		}
		f_closedir(&dir);
	}

	//Sort LED menu
	mbp_sort_menu(&menu);

	menu.items[menu.count++] = (menu_item_t ) { "<None>", NULL, NULL, NULL, NULL };

	//Show the menu
	if (mbp_submenu(&menu) == MENU_QUIT) {
		return;
	}

	util_button_clear();

	//Setup for the LED animation now that the menu has returned
	void *p_timer_data = NULL;
	app_timer_timeout_handler_t led_callback = NULL;

	//Get the RGB file
	if (menu.selected < (menu.count - 1)) {
		char * filename = (char *) menu.items[menu.selected].data;
		util_led_load_rgb_file(filename, &m_anim);
		p_timer_data = &m_anim;
		led_callback = __led_rgb_callback;
	}

	//Start the LED timer
	if (led_callback != NULL) {
		uint32_t ticks = APP_TIMER_TICKS(1000 / LED_FPS, UTIL_TIMER_PRESCALER);
		err_code = app_timer_create(&m_timer, APP_TIMER_MODE_REPEATED, led_callback);
		APP_ERROR_CHECK(err_code);
		err_code = app_timer_start(m_timer, ticks, p_timer_data);
		APP_ERROR_CHECK(err_code);
	}

	if (p_data != NULL) {
		//Clear the LEDs
		mbp_tooth_eye_stop();
		util_led_clear();
		char *filename = (char *) p_data;
		util_gfx_draw_raw_file(filename, 0, 0, 128, 128, NULL, true, NULL);
	} else {
		mbp_ui_cls();
		util_button_wait();
	}

	if (led_callback != NULL) {
		app_timer_stop(m_timer);
	}

	//Cleanup
	for (uint16_t i = 0; i < (menu.count - 1); i++) {
		free((items[i].data));
		free((items[i].text));
	}

	//Clear out scheduler queue
	app_sched_execute();

	//Cleanup
	util_led_clear();
	mbp_tooth_eye_start();
}

static void __animation_menu(void *p_data) {
	menu_t menu;
	menu_item_t items[ANIMATION_MAX];
	menu.items = items;
	menu.title = "Animation";
	menu.count = 0;
	menu.selected = 0;
	menu.top = 0;

	uint8_t raw_count = 0;

	//Split up the animations
	int8_t split = *((int8_t *) p_data);

	FRESULT result;
	DIR dir;
	static FILINFO fno;

	result = f_opendir(&dir, "BLING"); /* Open the directory */
	if (result == FR_OK) {
		for (;;) {
			result = f_readdir(&dir, &fno); /* Read a directory item */
			if (result != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR) { /* It is a directory */
				//ignore
			} else { /* It is a file. */
				char *ext = strrchr(fno.fname, '.') + 1;
				if ((split == 0 && fno.fname[0] <= 'F') ||
						(split == 1 && fno.fname[0] > 'F' && fno.fname[0] <= 'P') ||
						(split == 2 && fno.fname[0] > 'P')) {

					//Look for RGB LED files
					if (strcmp(ext, "RAW") == 0) {
						raw_count++;

						menu_item_t item;
						item.callback = &__menu_custom_anim_callback;
						item.icon = NULL;
						item.preview = NULL;
						item.text = (char *) malloc(16);
						item.data = (char *) malloc(20);

						snprintf(item.text, ext - fno.fname, "%s", fno.fname);
						sprintf(item.data, "BLING/%s", fno.fname);

						items[menu.count++] = item;
					}
				}
			}

			if (menu.count >= ANIMATION_MAX - 1) {
				break;
			}
		}
		f_closedir(&dir);
	}

	mbp_sort_menu(&menu);

	menu.items[menu.count].text = "<None>";
	menu.items[menu.count].callback = &__menu_custom_anim_callback;
	menu.items[menu.count].data = NULL;
	menu.count++;

	mbp_submenu(&menu);

	for (uint8_t i = 0; i < (menu.count - 1); i++) {
		free(menu.items[i].text);
		free(menu.items[i].data);
	}
}

void mbp_bling_menu_custom() {
	menu_t menu;
	menu_item_t items[3];
	menu.items = items;
	menu.title = "Animation";
	menu.count = 0;
	menu.selected = 0;
	menu.top = 0;

	int8_t a_to_f = 0;
	int8_t g_to_p = 1;
	int8_t q_to_z = 2;
	items[menu.count++] = (menu_item_t ) { "A-F", NULL, NULL, __animation_menu, &a_to_f };
	items[menu.count++] = (menu_item_t ) { "G-P", NULL, NULL, __animation_menu, &g_to_p };
	items[menu.count++] = (menu_item_t ) { "Q-Z", NULL, NULL, __animation_menu, &q_to_z };

	mbp_submenu(&menu);
}
