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
	mbp_ui_popup("AND!XOR", about);
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
	mbp_ui_input("Code", "Enter Code:", code, 8);

	//Master mode
	if (strcmp(code, "BD88ED") == 0) {
		mbp_state_master_set(true);
		mbp_state_save();
		mbp_ui_popup("Master", "Master Mode Engaged.");
	}
	//Badge activation
	else if (strcmp(code, "RHAC6+7X") == 0) {
		mbp_state_activated_set(true);
		botnet_state_t *p_state = mbp_state_botnet_state_get();
		p_state->points = 0;
		mbp_state_save();
		mbp_ui_popup("Activated", "Badge Activated!");
	}
	//TCL
	else if (strcmp(code, "DAMON") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_DAMON);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Matt Damon mode enabled.");
	}
	//Business Card
	else if (strcmp(code, "YJ9XZHFA") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_CARD);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "New Bender bling unlocked.");
	}
	//Chip 8 #2
	else if (strcmp(code, "7CA3FF1D") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_CHIP8);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Fry avatar unlocked.");
	}
	//Scroll unlock
	else if (strcmp(code, "VN2D85FF") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_SCROLL);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "New Party Modes!");
	}
	//TCL
	else if (strcmp(code, "Y_AN:LRN") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_TCL);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Owl mode unlocked.");
	}
	//Twitter
	else if (strcmp(code, "3TNXFBBN") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_TWITTER);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Illusion Bling unlocked.");
	}
	//Whiskey Pirates
	else if (strcmp(code, "XXXXXXXX") == 0) {
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_WHISKEY_PIRATES);
		mbp_state_save();
		mbp_ui_popup("Unlocked", "Zero Wing avatar unlocked.");
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
	mbp_ui_input("Name", "Enter Name:", name, SETTING_NAME_LENGTH - 1);

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
			"BENDER", "LEELA", "ZOIDBERG", "FRY", "AMY",
			"SCRUFFY", "CLAMPS", "KIFF", "ZAPP", "NIXON"
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
		mbp_state_new();
		mbp_state_save();

		mbp_ui_popup("Reset", "Badge is a n00b again.");
	}

	mbp_system_name_select();
	botnet_screen_pick_avatar();
}

bool mbp_system_seekrit_get() {
	nrf_gpio_cfg_input(PIN_SEEKRIT, NRF_GPIO_PIN_PULLUP);
	return nrf_gpio_pin_read(PIN_SEEKRIT) == 0;
}

void mbp_system_shouts() {
	mbp_ui_popup(
			"Shouts",
			"Spouses\n"
					"Children\n"
					"Fur Children (Dogs & Cats)\n"
					"DC24 Fans\n"
					"Crowd Funders\n"
					"Phobos\n"
					"bg3t\n"
					"g8\n"
					"manchmod\n"
					"Rigado\n"
					"Macrofab\n"
					"OSH Park\n"
					"#badgelife\n"
					"#pirateirc\n"
					"\n\n\n\n\n\n\n\n\n\n\n"
					"Nothing down here"
					"\n\n\n\n\n\n\n\n\n\n\n"
					"VN2D85FF");
}

void mbp_system_test() {
	mbp_cigar_eyes_stop();
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
			util_gfx_print("Nearby");

			util_gfx_set_cursor(0, 36);
			util_gfx_print("Up");

			util_gfx_set_cursor(0, 48);
			util_gfx_print("Down");

			util_gfx_set_cursor(0, 60);
			util_gfx_print("Left");

			util_gfx_set_cursor(0, 72);
			util_gfx_print("Right");

			util_gfx_set_cursor(0, 84);
			util_gfx_print("Action");

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
			util_gfx_print("Yes");
		} else {
			util_gfx_print("No");
		}

		//Test tilt
		util_gfx_set_cursor(90, 12);
		if (util_tilt_inverted()) {
			util_gfx_print("Inv");
		} else {
			util_gfx_print("Norm");
		}

		//Show badge db count
		util_gfx_set_cursor(90, 24);
		sprintf(buffer, "%d", util_ble_badge_db_get()->badge_count);
		util_gfx_print(buffer);

		//up button
		util_gfx_set_cursor(90, 36);
		if (util_button_up()) {
			util_gfx_print("true");
		} else {
			util_gfx_print("false");
		}

		//down button
		util_gfx_set_cursor(90, 48);
		if (util_button_down()) {
			util_gfx_print("true");
		} else {
			util_gfx_print("false");
		}

		//left button
		util_gfx_set_cursor(90, 60);
		if (util_button_left()) {
			util_gfx_print("true");
		} else {
			util_gfx_print("false");
		}

		//right button
		util_gfx_set_cursor(90, 72);
		if (util_button_right()) {
			util_gfx_print("true");
		} else {
			util_gfx_print("false");
		}

		//action button
		util_gfx_set_cursor(90, 84);
		if (util_button_action()) {
			util_gfx_print("true");
		} else {
			util_gfx_print("false");
		}

		//Current time
		util_gfx_set_cursor(90, 96);
		sprintf(buffer, "%lu", util_millis() / 1000);
		util_gfx_print(buffer);

		//FW Version
		util_gfx_set_cursor(90, 108);
		util_gfx_print(VERSION);

		util_gfx_validate();

		//Test LEDs
		switch (color) {
		case 0:
			util_led_set_all(255, 0, 0);
			break;
		case 1:
			util_led_set_all(0, 255, 0);
			break;
		case 2:
			util_led_set_all(0, 0, 255);
			break;
		case 3:
			util_led_set_all(255, 255, 0);
			break;
		case 4:
			util_led_set_all(255, 255, 255);
			break;
		}
		util_led_show();
		color = (color + 1) % 5;

		//Allow user to quit
		if (util_button_left() > 0) {
			break;
		}

		app_sched_execute();
		nrf_delay_ms(300);
	}

	util_led_clear();
	util_button_clear();
	mbp_cigar_eyes_start();
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

	mbp_cigar_eyes_stop();
	//Clear out scheduler of any cig events
	app_sched_execute();

	util_gfx_draw_raw_file("BENDERL.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);
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
	util_button_wait();
	util_button_clear();
	util_led_clear();

	mbp_cigar_eyes_start();
}
