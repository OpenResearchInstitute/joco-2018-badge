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

#define PIN_SEEKRIT		24

static bool m_interuptable = true;

void mbp_system_about() {
	char version[32] = VERSION;
	char about[64];
	uint16_t serial = util_get_device_id();
	ble_version_t ble;
	sd_ble_version_get(&ble);
	memset(about, '\0', 64);
	sprintf(about, "Firmware:\n%s\nSoft Device:\n%#X\nDevice ID:\n%#X", version, ble.subversion_number, serial);
	mbp_ui_popup("JOCO2018", about);
}

void mbp_system_airplane_mode_select() {
	if (mbp_state_airplane_mode_get()) {
		bool enabled = mbp_ui_toggle_popup("Airplne Mode", 0, "Enable", "Disable", "Currently: ENABLED") == 0;
		mbp_state_airplane_mode_set(enabled);
		mbp_state_save();

		//Turn off
		if (!enabled) {
			util_ble_on();
		}
	} else {
		bool enabled = mbp_ui_toggle_popup("Airplne Mode", 1, "Enable", "Disable", "Currently: DISABLED") == 0;
		mbp_state_airplane_mode_set(enabled);
		mbp_state_save();

		//Turn on
		if (enabled) {
			util_ble_off();
		}
	}
}

void mbp_system_code() {
	char code[9];
	memset(code, 0, 9);
	mbp_ui_input("Code", "Enter Code:", code, 8, false);

	//Master mode
	if (strcmp(code, "UP2RIGHT") == 0) {
		mbp_state_master_set(true);
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_MASTER_1 | UNLOCK_MASK_MASTER_2 | UNLOCK_MASK_MASTER_3 | UNLOCK_MASK_MASTER_4);
		mbp_state_save();
		mbp_ui_popup("Master", "Master Mode Engaged.");
	}
	//DAMON
	else if (strcmp(code, "DAMON") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_DAMON);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Matt Damon mode enabled.");
	}
	//Wheaton
	else if (strcmp(code, "TABLETOP") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_WHEATON);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Wil Wheaton mode enabled.");
	}
	//Illusion
	else if (strcmp(code, "NUKACOLA") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_TWITTER);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Illusion Bling unlocked.");
	}

	//Hack Time
	else if (strcmp(code, "HAXXOR") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_DATE_TIME);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Hack Time Bling unlocked.");
	}

	//Defrag
	else if (strcmp(code, "DISKFULL") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_DEFRAG);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Defrag Bling unlocked.");
	}

	//He Man
	else if (strcmp(code, "WHATSUP") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_WHATS_UP);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "He Man Bling unlocked.");
	}

	//Everything else
	else {
		mbp_ui_error(":(");
	}
}

uint16_t mbp_system_color_selection_menu(uint16_t current_color) {
	//first color will get replaced
	uint16_t colors[] = {
	COLOR_BLACK,
	COLOR_BLACK, COLOR_BROWN, COLOR_NAVY, COLOR_DARKBLUE, COLOR_DARKGREEN,
	COLOR_DARKCYAN, COLOR_MAROON, COLOR_PURPLE, COLOR_OLIVE, COLOR_LIGHTGREY,
	COLOR_DARKGREY, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_RED,
	COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE, COLOR_ORANGE, COLOR_GREENYELLOW,
	COLOR_PINK, COLOR_NEONPURPLE };
	char *color_names[] = {
			"Black",
			"Black", "Brown", "Navy", "Dark Blue", "Dark Green",
			"Dark Cyan", "Maroon", "Purple", "Olive", "Light Grey",
			"Dark Grey", "Blue", "Green", "Cyan", "Red",
			"Magenta", "Yellow", "White", "Orange", "Green Yellow",
			"Pink", "Neon Purple" };

	menu_t menu;
	menu.count = 0;
	menu.title = "Pick Color";
	menu_item_t items[23];
	menu.items = items;

	//First menu item should be the name of the current color
	for (uint8_t i = 0; i < 23; i++) {
		if (current_color == colors[i]) {
			items[menu.count++] = (menu_item_t ) { color_names[i], NULL, NULL, NULL, NULL };
			break;
		}
	}

	//All other colors add after that
	for (uint8_t i = 1; i < 23; i++) {
		items[menu.count++] = (menu_item_t ) { color_names[i], NULL, NULL, NULL, NULL };
	}

	//If they quit, return the current color
	if (mbp_submenu(&menu) == MENU_QUIT) {
		return current_color;
	}

	//If they picked something return the selected color
	return colors[menu.selected];
}

bool mbp_system_interuptable_get() {
	return m_interuptable;
}

//void mbp_system_interuptable_set(bool interuptable) {
//	m_interuptable = interuptable;
//}

void mbp_system_game_menu() {
	menu_item_t items[] = {
			{ "Game Exit Hint", NULL, NULL, NULL, NULL },
			{ "Foreground Color", NULL, NULL, NULL, NULL },
			{ "Background Color", NULL, NULL, NULL, NULL },
			{ "LED Beep Toggle", NULL, NULL, NULL, NULL },
	};
	menu_t menu;
	menu.count = 4;
	menu.items = items;
	menu.selected = 0;
	menu.title = "Game CFG";
	menu.top = 0;

	if (mbp_submenu(&menu) == MENU_QUIT) {
		return;
	}

	switch (menu.selected) {
	case 0:
		; //Toggle the Exit Game Hint
		if (mbp_state_game_exit_pop_up_get()) {
			mbp_state_game_exit_pop_up_set(mbp_ui_toggle_popup("Hint Pop Up", 0, "Enable", "Disable", "Currently: ENABLED") == 0);
			mbp_state_save();
		}
		else {
			mbp_state_game_exit_pop_up_set(mbp_ui_toggle_popup("Hint Pop Up", 1, "Enable", "Disable", "Currently: DISABLED") == 0);
			mbp_state_save();
		}
		break;

	case 1:
		; //Change the CHIP8 Foreground Color
		mbp_state_chip8_fg_color_set(mbp_system_color_selection_menu(mbp_state_chip8_fg_color_get()));
		mbp_state_save();
		break;

	case 2:
		; //Change the CHIP8 Background Color
		mbp_state_chip8_bg_color_set(mbp_system_color_selection_menu(mbp_state_chip8_bg_color_get()));
		mbp_state_save();
		break;

	case 3:
		; //Toggle the LED Blinks for Sound
		if (mbp_state_game_led_sound_get()) {
			mbp_state_game_led_sound_set(mbp_ui_toggle_popup("LED Beeps", 0, "Enable", "Disable", "Currently: ENABLED") == 0);
			mbp_state_save();
		}
		else {
			mbp_state_game_led_sound_set(mbp_ui_toggle_popup("LED Beeps", 1, "Enable", "Disable", "Currently: DISABLED") == 0);
			mbp_state_save();
		}
		break;
	}

}

#define SPECIAL_STRING_LEN 4
void mbp_system_special_edit() {
    uint8_t special;
    special = mbp_state_special_get();
    char special_buf[SPECIAL_STRING_LEN];

//Ask if they want to clear
    char message[64];
    sprintf(message, "Value is '%03d' (MAX 255), edit existing or clear?", special);
    if (mbp_ui_toggle_popup("Value", 0, "Edit", "Clear", message) == 1) {
	special = 0;
    }

//Edit the special value
    sprintf(special_buf, "%03d", special);
    mbp_ui_input("Value", "Enter Value:", special_buf, SPECIAL_STRING_LEN - 1, true);
    special = atoi(special_buf) % 256;

    sprintf(message, "Change value to: '%03d'?", special);
    if (mbp_ui_toggle_popup("Name", 0, "No", "Yes", message) == 1) {
	mbp_state_special_set(special);
	mbp_state_save();

	sprintf(message, "Value changed to '%03d'.", special);
	mbp_ui_popup("Value", message);
    }
    //Aborted
    else {
	mbp_ui_popup("Value", "Value not changed.");
    }
}

void mbp_system_name_edit() {
	char name[SETTING_NAME_LENGTH];
	mbp_state_name_get(name);

//Ask if they want to clear
	char message[64];
	sprintf(message, "Name is '%s', edit existing or clear?", name);
	if (mbp_ui_toggle_popup("Name", 0, "Edit", "Clear", message) == 1) {
		memset(name, 0, SETTING_NAME_LENGTH);
	}

//Edit the name
	mbp_ui_input("Name", "Enter Name:", name, SETTING_NAME_LENGTH - 1, false);

	sprintf(message, "Change name to: '%s'?", name);
	if (mbp_ui_toggle_popup("Name", 0, "No", "Yes", message) == 1) {
		mbp_state_name_set(name);
		mbp_state_save();

		sprintf(message, "Name changed to '%s'.", name);
		mbp_ui_popup("Name", message);
	}
	//Aborted
	else {
		mbp_ui_popup("Name", "Name not changed.");
	}

}

void mbp_system_name_select() {
	char *names[] = {
			"MONKEY", "REDSHIRT", "GLADOS", "KVOTHE", "DENNA",
			"AURI", "WESLEY", "GEORDI", "KINGA", "MAX"
	};
	uint8_t name_count = 10;

	menu_item_t items[name_count];
	menu_t menu;
	menu.count = name_count;
	menu.items = items;
	menu.title = "Name";

	for (uint8_t i = 0; i < name_count; i++) {
		items[i] = (menu_item_t ) { names[i], NULL, NULL, NULL, NULL };
	}

	while (mbp_submenu(&menu) != MENU_OK)
		;

	mbp_state_name_set(names[menu.selected]);

	char message[32];
	sprintf(message, "Name set to '%s'", names[menu.selected]);
	mbp_ui_popup("Name", message);
}

void mbp_system_reset() {
	if (mbp_ui_toggle_popup("Reset", 1, "Yes", "Cancel", "Progress will be lost. Are you sure?") == 0) {
		uint16_t contacted_count;
		mbp_state_new();
		// Give credit for all contacts in the db
		contacted_count = count_db_entries();
		mbp_state_score_set(contacted_count * POINTS_4_VISIT);
		mbp_state_save();
		mbp_ui_popup("Reset", "Badge is a n00b again.");
		mbp_system_name_select();
	}
}

bool mbp_system_seekrit_get() {
	nrf_gpio_cfg_input(PIN_SEEKRIT, NRF_GPIO_PIN_PULLUP);
	return nrf_gpio_pin_read(PIN_SEEKRIT) == 0;
}

void mbp_system_shouts() {
	mbp_ui_popup(
			"Shouts",
                        "@ANDnXOR\n"
			"Partners\n"
			"JOCO Fans\n"
			"#badgelife\n");
}

void mbp_system_test() {
	mbp_tooth_eye_stop();
	//clear out app_scheduler
	app_sched_execute();
	util_gfx_set_font(FONT_SMALL);
	char buffer[32];

	util_gfx_invalidate();
	uint8_t color = 0;

	while (1) {
		//Full redraw if invalidated
		if (!util_gfx_is_valid_state()) {
			mbp_ui_cls();
			util_gfx_set_font(FONT_SMALL);
			util_gfx_set_color(COLOR_WHITE);

			util_gfx_set_cursor(0, 0);
			util_gfx_print("Micro SD");

			util_gfx_set_cursor(0, 12);
			util_gfx_print("Tilt");

			util_gfx_set_cursor(0, 24);
			util_gfx_print("Up");

			util_gfx_set_cursor(0, 36);
			util_gfx_print("Down");

			util_gfx_set_cursor(0, 48);
			util_gfx_print("Left");

			util_gfx_set_cursor(0, 60);
			util_gfx_print("Right");

			util_gfx_set_cursor(0, 72);
			util_gfx_print("Action");

			util_gfx_set_cursor(0, 84);
			util_gfx_print("Nearby");

			util_gfx_set_cursor(0, 96);
			util_gfx_print("Time");

			util_gfx_set_cursor(0, 108);
			util_gfx_print("Version");
		}

		//Clear values
		util_gfx_fill_rect(90, 0, 38, GFX_HEIGHT, COLOR_BLACK);

		//Test for microsd
		util_gfx_set_cursor(90, 0);
		if (util_sd_file_size("BG.RAW") > 0) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("Yes");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("No");
		}

		//Test tilt
		util_gfx_set_cursor(90, 12);
		if (util_tilt_inverted()) {
			util_gfx_set_color(COLOR_BLUE);
			util_gfx_print("Inv");
		} else {
			util_gfx_set_color(COLOR_YELLOW);
			util_gfx_print("Norm");
		}

		//up button
		util_gfx_set_cursor(90, 24);
		if (util_button_up()) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("true");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("false");
		}

		//down button
		util_gfx_set_cursor(90, 36);
		if (util_button_down()) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("true");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("false");
		}

		//left button
		util_gfx_set_cursor(90, 48);
		if (util_button_left()) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("true");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("false");
		}

		//right button
		util_gfx_set_cursor(90, 60);
		if (util_button_right()) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("true");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("false");
		}

		//action button
		util_gfx_set_cursor(90, 72);
		if (util_button_action()) {
			util_gfx_set_color(COLOR_GREEN);
			util_gfx_print("true");
		} else {
			util_gfx_set_color(COLOR_RED);
			util_gfx_print("false");
		}

                util_gfx_set_color(COLOR_WHITE);

		//Show badge db count
                util_gfx_set_color(COLOR_WHITE);
		util_gfx_set_cursor(90, 84);
		sprintf(buffer, "%d", get_nearby_badge_count());
		util_gfx_print(buffer);

		//Current time
		util_gfx_set_cursor(90, 96);
		sprintf(buffer, "%lu", util_millis() / 1000);
		util_gfx_print(buffer);

		//FW Version
		util_gfx_set_cursor(90, 108);
		util_gfx_print(VERSION);

		util_gfx_validate();

		if (util_tilt_inverted()) {
                    util_led_set_all(100, 100, 100);
                    util_led_show();
                } else {
                    //Test LEDs
                    switch (color) {
                    case 0:
                            util_led_set_all(100, 0, 0);
                            break;
                    case 1:
                            util_led_set_all(0, 100, 0);
                            break;
                    case 2:
                            util_led_set_all(0, 0, 100);
                            break;
                    }
                    util_led_show();
                    color = (color + 1) % 3;
                }

                util_gfx_set_color(COLOR_WHITE);
		//Allow user to quit
		if (util_button_left() > 0) {
			break;
		}

		app_sched_execute();
		nrf_delay_ms(300);
	}

	util_led_clear();
	util_button_clear();
	mbp_tooth_eye_start();
}

void mbp_system_tilt_mode_select() {
	if (mbp_state_tilt_get()) {
		mbp_state_tilt_set(mbp_ui_toggle_popup("Tilt", 0, "Enable", "Disable", "Tilt sensor is enabled.") == 0);
	} else {
		mbp_state_tilt_set(mbp_ui_toggle_popup("Tilt", 1, "Enable", "Disable", "Tilt sensor is disabled.") == 0);
	}
	mbp_state_save();
}

void mbp_system_unlock_state() {
	uint16_t unlock = mbp_state_unlock_get();

	mbp_tooth_eye_stop();
	//Clear out scheduler of any eye/tooth events
	app_sched_execute();

	util_led_clear();

	//If they've unlocked all 16 set all LEDs to blue
	if (unlock == 0xFFFF) {
		util_led_set_all(0, 0, 255);
	} else {
		for (uint8_t i = 0; i <= LED_COUNT; i++) {
			if ((unlock & (1 << i)) > 0) {
				util_led_set_rgb(i, LED_COLOR_ORANGE);
			}
		}
	}

	util_led_show();
	util_gfx_draw_raw_file("BLING/JOCO/SKLCROSS.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, true, NULL);
	util_led_clear();

	mbp_tooth_eye_start();
}
