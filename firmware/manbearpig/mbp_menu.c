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

//New menu defines
#define MAX_ITEMS				4
#define MENU_COLOR				COLOR_WHITE
#define MENU_COLOR_BG			COLOR_BLACK
#define MENU_ICON_SIZE			20
#define MENU_INDICATOR_BG		COLOR_BLACK
#define MENU_INDICATOR_FG		COLOR_GREEN
#define MENU_INDICATOR_H		16
#define MENU_PADDING			2
#define MENU_SCROLL_DELAY		100
#define MENU_SELECTED_COLOR		COLOR_WHITE
#define MENU_SIZE				27
#define MENU_SELECTED_SIZE		114

#define SUBMENU_SELECTED_COLOR	COLOR_CYAN
#define SUBMENU_PADDING			2
#define SUBMENU_TITLE_BG		COLOR_DARKCYAN
#define SUBMENU_TITLE_FG		COLOR_WHITE
#define SUBMENU_TITLE_SIZE		15

uint8_t mbp_menu(menu_t *p_menu) {
	p_menu->selected = 0;
	p_menu->top = 0;
	util_gfx_set_font(FONT_LARGE);
	int16_t selected_button = 0;
	uint8_t max_visible_buttons = MIN(p_menu->count, MAX_ITEMS);

	uint32_t size = MENU_ICON_SIZE * MENU_ICON_SIZE * 2;
	uint8_t text_offset = 11;
	uint8_t icons[p_menu->count][size];

	//Load icons
	for (uint8_t i = 0; i < p_menu->count; i++) {
		if (p_menu->items[i].icon != NULL) {
			util_sd_load_file(p_menu->items[i].icon, icons[i], size);
		}
	}

	while (1) {
		menu_item_t selected = p_menu->items[p_menu->selected];
		util_gfx_cursor_area_reset();

		//Clear the screen
		mbp_ui_cls();

		//Draw preview
		if (selected.preview == NULL) {
			if (mbp_master_c2_infected()) {
				util_gfx_draw_raw_file("BOTNET/WORM.PRV", MENU_SIZE, MENU_INDICATOR_H, GFX_WIDTH - MENU_SIZE, GFX_HEIGHT - MENU_INDICATOR_H, NULL, false, NULL);
			} else {
				util_gfx_draw_raw_file("MENU/DEFAULT.PRV", MENU_SIZE, MENU_INDICATOR_H, GFX_WIDTH - MENU_SIZE, GFX_HEIGHT - MENU_INDICATOR_H, NULL, false,
				NULL);
			}
		} else {
			util_gfx_draw_raw_file(selected.preview, MENU_SIZE, MENU_INDICATOR_H, GFX_WIDTH - MENU_SIZE, GFX_HEIGHT - MENU_INDICATOR_H, NULL,
			false, NULL);
		}

		//Draw indicator section
		char name[SETTING_NAME_LENGTH];
		mbp_state_name_get(name);
		util_gfx_set_font(FONT_LARGE);
		util_gfx_set_color(MENU_INDICATOR_FG);
		util_gfx_fill_rect(0, 0, GFX_WIDTH, MENU_INDICATOR_H, MENU_INDICATOR_BG);
		util_gfx_set_cursor(0, 5);
		util_gfx_print(name);

		if (mbp_state_airplane_mode_get()) {
			util_gfx_draw_raw_file("MENU/INDPLANE.RAW", 113, 1, 14, 14, NULL, false, NULL);
		}

		if (botnet_immune()) {
			util_gfx_draw_raw_file("MENU/INDSHLD.RAW", 98, 1, 14, 14, NULL, false, NULL);
		}

		//Setup font again to ensure something else hasn't changed it to small
		util_gfx_set_font(FONT_LARGE);
		util_gfx_set_color(COLOR_WHITE);

		//Draws the menu
		for (uint8_t i = 0; i < max_visible_buttons; i++) {

			uint16_t x = 0;
			uint16_t y = MENU_INDICATOR_H + (i * MENU_SIZE) + (i);

			if (i == selected_button) {
				util_gfx_fill_rect(x, y, MENU_SELECTED_SIZE, MENU_SIZE, MENU_COLOR_BG);
				util_gfx_draw_rect(x, y, MENU_SELECTED_SIZE, MENU_SIZE, MENU_SELECTED_COLOR);
				util_gfx_set_cursor(x + (MENU_SIZE - 2) + MENU_PADDING, y + text_offset);
				util_gfx_print(selected.text);
			} else {
				util_gfx_fill_rect(x, y, MENU_SIZE + 1, MENU_SIZE, MENU_COLOR_BG);
				util_gfx_draw_rect(x, y, MENU_SIZE + 1, MENU_SIZE, MENU_COLOR);
			}

			if (p_menu->items[p_menu->top + i].icon != NULL) {
				util_gfx_draw_raw(x + MENU_PADDING, y + MENU_PADDING, MENU_ICON_SIZE, MENU_ICON_SIZE, icons[p_menu->top + i]);
			}
		}

		//Track that screen is in a valid state
		util_gfx_validate();

//		util_button_clear();
		util_button_wait();

		if (util_button_down()) {
			//Move selected menu item down one if able
			if (p_menu->selected < p_menu->count - 1) {
				p_menu->selected++;

				//Move selected button down one
				if (selected_button < MAX_ITEMS - 1) {
					selected_button++;
				}
				//If already at bottom, scroll
				else {
					p_menu->top++;
				}

				nrf_delay_ms(MENU_SCROLL_DELAY);
			}
		} else if (util_button_up()) {
			if (p_menu->selected > 0) {
				p_menu->selected--;

				//Move selection up one
				if (selected_button > 0) {
					selected_button--;
				}
				//If already at top scroll
				else {
					p_menu->top--;
				}

				nrf_delay_ms(MENU_SCROLL_DELAY);
			}
		} else if (util_button_left()) {
			util_button_clear();
			return MENU_QUIT;
		} else if (util_button_action()) {
			util_button_clear();
			menu_item_t item = p_menu->items[p_menu->selected];
			if (item.callback != NULL) {
				item.callback(item.data);
				util_led_clear();
			}
		}
	}
}

/**
 * Sort menu items using bubble sort.
 * There are better algorithms but this is good enough
 */
void mbp_sort_menu(menu_t *p_menu) {
	for (uint8_t i = 0; i < p_menu->count; i++) {
		for (uint8_t j = 0; j < p_menu->count; j++) {
			if (strcasecmp(p_menu->items[i].text, p_menu->items[j].text) < 0) {
				//swap
				menu_item_t temp = p_menu->items[i];
				p_menu->items[i] = p_menu->items[j];
				p_menu->items[j] = temp;

			}
		}
	}
}

uint8_t mbp_submenu(menu_t *p_menu) {
	p_menu->selected = 0;
	p_menu->top = 0;

	//Setup font again to ensure something else hasn't changed it to small
	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_color(COLOR_WHITE);

	uint8_t w = GFX_WIDTH;
	uint8_t font_height = util_gfx_font_height();
	uint8_t max_visible_items = MIN(p_menu->count, (GFX_HEIGHT - SUBMENU_TITLE_SIZE) / font_height);
	uint8_t selected_button = 0;
	bool draw_title = true;
	bool draw_menu_valid[max_visible_items];
	for (uint8_t i = 0; i < max_visible_items; i++) {
		draw_menu_valid[i] = false;
	}

	util_gfx_invalidate();

	while (1) {
		if (!util_gfx_is_valid_state()) {
			draw_title = true;
			mbp_ui_cls();
			draw_title = true;
			for (uint8_t i = 0; i < max_visible_items; i++) {
				draw_menu_valid[i] = false;
			}
		}

		util_gfx_cursor_area_reset();

		if (draw_title) {
			util_gfx_set_font(FONT_LARGE);
			util_gfx_set_cursor(0, 5);
			util_gfx_set_color(COLOR_WHITE);

			util_gfx_fill_rect(0, 0, GFX_WIDTH, SUBMENU_TITLE_SIZE, SUBMENU_TITLE_BG);
			util_gfx_draw_line(0, SUBMENU_TITLE_SIZE, GFX_WIDTH, SUBMENU_TITLE_SIZE, SUBMENU_TITLE_FG);
			util_gfx_print(p_menu->title);

			draw_title = false;
		}

		util_gfx_set_font(FONT_SMALL);

		//Draws the menu
		for (uint8_t i = 0; i < max_visible_items; i++) {
			if (!draw_menu_valid[i]) {
				uint16_t y = (i * font_height) + SUBMENU_PADDING;

				util_gfx_fill_rect(SUBMENU_PADDING - 1, SUBMENU_TITLE_SIZE + y - 1, w - SUBMENU_PADDING, font_height, COLOR_BLACK);

				if (i == selected_button) {
					util_gfx_draw_rect(SUBMENU_PADDING - 1, SUBMENU_TITLE_SIZE + y - 1, w - SUBMENU_PADDING, font_height, SUBMENU_SELECTED_COLOR);
				}

				util_gfx_set_cursor(SUBMENU_PADDING, SUBMENU_TITLE_SIZE + y);
				char title[32];
				sprintf(title, "%d) %s", p_menu->top + i + 1, p_menu->items[p_menu->top + i].text);
				util_gfx_print(title);

				draw_menu_valid[i] = true;
			}
		}

		//Highlight selected
		util_gfx_draw_rect(
		SUBMENU_PADDING - 1,
		SUBMENU_TITLE_SIZE + (selected_button * font_height) + SUBMENU_PADDING - 1,
				w - SUBMENU_PADDING,
				font_height,
				SUBMENU_SELECTED_COLOR);

		//Validate the gfx state since we're done drawing
		util_gfx_validate();

		//Wait for user interaction
		util_button_wait();

		if (util_button_down()) {
			//Move selected menu item down one if able
			if (p_menu->selected < p_menu->count - 1) {
				p_menu->selected++;

				//Move selected button down one
				if (selected_button < max_visible_items - 1) {
					draw_menu_valid[selected_button] = false;
					selected_button++;
					draw_menu_valid[selected_button] = false;
				}
				//If already at bottom, scroll
				else {
					p_menu->top++;

					for (uint8_t i = 0; i < max_visible_items; i++) {
						draw_menu_valid[i] = false;
					}
				}

				nrf_delay_ms(MENU_SCROLL_DELAY);
			}
		} else if (util_button_up()) {
			if (p_menu->selected > 0) {
				p_menu->selected--;

				//Move selection up one
				if (selected_button > 0) {
					draw_menu_valid[selected_button] = false;
					selected_button--;
					draw_menu_valid[selected_button] = false;
				}
				//If already at top scroll
				else {
					p_menu->top--;

					for (uint8_t i = 0; i < max_visible_items; i++) {
						draw_menu_valid[i] = false;
					}
				}

				nrf_delay_ms(MENU_SCROLL_DELAY);
			}
		} else if (util_button_left()) {
			util_button_clear();
			return MENU_QUIT;
		} else if (util_button_action()) {
			util_button_clear();
			menu_item_t item = p_menu->items[p_menu->selected];
			if (item.callback != NULL) {
				item.callback(item.data);
			} else {
				return MENU_OK;
			}

			util_gfx_invalidate();
		}
	}
}

static void mbp_menu_bling_ks() {
	char *backer_images[] = {
			"BLING/BACKERS/APRIL.RAW",
			"BLING/BACKERS/BTCCTF.RAW",
			"BLING/BACKERS/ABRAXAS.RAW",
			"BLING/BACKERS/CYBERS.RAW",
			"BLING/BACKERS/SOL.RAW",
			"BLING/BACKERS/BACKERSM.RAW"
	};

	menu_callback_t backer_callbacks[] = {
			&mbp_bling_backer_april,
			&mbp_bling_backer_btcctf,
			&mbp_bling_backer_abraxas3d,
			&mbp_bling_backer_cybersulu,
			&mbp_bling_backer_sol,
			&mbp_bling_backer_credits
	};

	uint8_t selected = 0;
	uint8_t count = 6;
	util_gfx_invalidate();

	while (1) {
		if (!util_gfx_is_valid_state()) {
			util_led_clear();
			util_gfx_draw_raw_file(backer_images[selected], 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);
		}
		util_gfx_validate();

		util_button_wait();
		if (util_button_down() > 0) {
			selected = (selected + 1) % count;
			util_gfx_invalidate();
		} else if (util_button_up() > 0) {
			if (selected == 0) {
				selected = count - 1;
			} else {
				selected--;
			}
			util_gfx_invalidate();
		} else if (util_button_action()) {
			util_button_clear();
			backer_callbacks[selected](NULL);
			util_gfx_invalidate();
		} else if (util_button_left()) {
			util_button_clear();
			return;
		}

		util_button_clear();
	}
}

static void mbp_menu_bling() {
	uint16_t unlock = mbp_state_unlock_get();

	menu_t menu;
	menu_item_t items[32];
	menu.items = items;
	menu.selected = 0;
	menu.top = 0;
	menu.title = "Bling";
	menu.count = 0;

	items[menu.count++] = (menu_item_t ) { "Backers", "MENU/KSLOGO.ICO", "MENU/KSLOGO.PRV", &mbp_menu_bling_ks, NULL };
	items[menu.count++] = (menu_item_t ) { "Bender", "MENU/MASTER.ICO", "MENU/BENDER.PRV", &mbp_bling_bender, NULL };
	items[menu.count++] = (menu_item_t ) { "Custom", "MENU/WRENCH.ICO", NULL, &mbp_bling_menu_custom, NULL };
	items[menu.count++] = (menu_item_t ) { "Badger", "MENU/BADGERS.ICO", "MENU/BADGERS.PRV", &mbp_bling_badgers, NULL };
	items[menu.count++] = (menu_item_t ) { "Flames", "MENU/FLAMES.ICO", "MENU/FLAMES.PRV", &mbp_bling_flames, NULL };
	items[menu.count++] = (menu_item_t ) { "Party", "MENU/PARTY.ICO", "MENU/PARTY.PRV", &mbp_bling_party, NULL };

	if ((unlock & UNLOCK_MASK_MASTER) > 0) {
		items[menu.count++] = (menu_item_t ) { "Rager", "MENU/PARTY.ICO", "MENU/PARTY.PRV", &mbp_bling_rager, NULL };
	}

	items[menu.count++] = (menu_item_t ) { "Toad", "MENU/TOAD.ICO", "MENU/TOAD.PRV", &mbp_bling_toad, NULL };
#ifndef OPSEC
	items[menu.count++] = (menu_item_t ) { "Twitter", "MENU/TWITTER.ICO", "MENU/TWITTER.PRV", &mbp_bling_twitter, NULL };

	//Add whats up bling
	if ((unlock & UNLOCK_MASK_WHATS_UP) > 0) {
		items[menu.count++] = (menu_item_t ) { "WhatsUp", "MENU/HEYYEYA.ICO", "MENU/HEYYEYA.PRV", &mbp_bling_whats_up, NULL };
	}

	//Add major lazer bling if they've unlocked CPV
	if ((unlock & UNLOCK_MASK_CPV) > 0) {
		items[menu.count++] = (menu_item_t ) { "Lazer", "MENU/MAJORL.ICO", "MENU/MAJORL.PRV", &mbp_bling_major_lazer, NULL };
	}
#endif
	items[menu.count++] = (menu_item_t ) { "Matrix", "MENU/MATRIX.ICO", "MENU/MATRIX.PRV", &mbp_bling_matrix, NULL };
	items[menu.count++] = (menu_item_t ) { "Meatspn", "MENU/MEATSPIN.ICO", "MENU/MEATSPIN.PRV", &mbp_bling_meatspin, NULL };
	items[menu.count++] = (menu_item_t ) { "Nyan", "MENU/NYAN.ICO", "MENU/NYAN.PRV", &mbp_bling_nyan, NULL };
	items[menu.count++] = (menu_item_t ) { "Scroll", "MENU/SCROLL.ICO", "MENU/SCROLL.PRV", &mbp_bling_scroll_cycle, NULL };
	items[menu.count++] = (menu_item_t ) { "Pirate", "MENU/PIRATES.ICO", "MENU/PIRATES.PRV", &mbp_bling_pirate, NULL };

	//Add illusion bling
	if ((unlock & UNLOCK_MASK_TWITTER) > 0) {
		items[menu.count++] = (menu_item_t ) { "Illusn", "MENU/ILLUSION.ICO", "MENU/ILLUSION.PRV", &mbp_bling_illusion, NULL };
	}

	//Add Owl bling
	if ((unlock & UNLOCK_MASK_TCL) > 0) {
		items[menu.count++] = (menu_item_t ) { "Owls", "MENU/OWL.ICO", "MENU/OWL.PRV", &mbp_bling_owl, NULL };
	}

	//Add matt damon bling
	if ((unlock & UNLOCK_MASK_DAMON) > 0) {
		items[menu.count++] = (menu_item_t ) { "Damon", "MENU/DAMON.ICO", "MENU/DAMON.PRV", &mbp_bling_damon, NULL };
	}

	//Defrag bling
	if ((unlock & UNLOCK_MASK_DEFRAG) > 0) {
		items[menu.count++] = (menu_item_t ) { "Defrag", "MENU/DEFRAG.ICO", "MENU/DEFRAG.PRV", &mbp_bling_defrag, NULL };
	}

	//Hack time bling
	if ((unlock & UNLOCK_MASK_DATE_TIME) > 0) {
		items[menu.count++] = (menu_item_t ) { "Time", "MENU/HACKTIME.ICO", "MENU/HACKTIME.PRV", &mbp_bling_hack_time, NULL };
	}

	if ((unlock & UNLOCK_MASK_SEEKRIT) > 0) {
		items[menu.count++] = (menu_item_t ) { "Trolol", "MENU/TROLOLOL.ICO", "MENU/TROLOLOL.PRV", &mbp_bling_trololol, NULL };
	}

	mbp_cigar_eyes_stop();
	//clear out app_scheduler
	app_sched_execute();
	mbp_menu(&menu);
	mbp_cigar_eyes_start();
}

static void mbp_menu_games() {
	menu_item_t items[] = {
			{ "Flappy DEF CON", NULL, NULL, &flappy, NULL },
			{ "Ski Free", NULL, NULL, &ski, NULL },
			{ "CHIP-8", NULL, NULL, &chip8_menu, NULL },
	};

	menu_t menu;
	menu.items = items;
	menu.count = 3;
	menu.selected = 0;
	menu.title = "Games";
	menu.top = 0;
	mbp_submenu(&menu);
}

static void mbp_menu_nearby() {
	if (util_ble_badge_db_get()->badge_count == 0 && mbp_medea_bottle_count() == 0) {
		mbp_ui_popup("Nearby", "Sorry no neighbors :(");
		return;
	}

	menu_t menu;
	menu_item_t items[BADGE_DB_SIZE + MEDEA_DB_SIZE];
	menu.items = items;
	menu.count = 0;
	menu.title = "Nearby";

	//Add nearby badges
	ble_badge_db_t *p_badges = util_ble_badge_db_get();
	for (uint8_t i = 0; i < p_badges->badge_count; i++) {
		ble_badge_t *p_badge = &(p_badges->badges[i]);
		char *badge_text = (char *) malloc(32);
		sprintf(badge_text, "%s %ddB", p_badge->name, p_badge->rssi);
		items[menu.count++] = (menu_item_t ) { badge_text, NULL, NULL, NULL, NULL };
	}

	//Add bottles
	medea_bottle_t *bottles = mbp_medea_bottle_db_get();
	for (uint8_t i = 0; i < mbp_medea_bottle_count(); i++) {
		uint8_t *a = bottles[i].address.addr;
		char *addr_str = (char *) malloc(32);
		sprintf(addr_str, "MEDEA%02X%02X%02X%02X%02X%02X", a[0], a[1], a[2], a[3], a[4], a[5]);
		items[menu.count++] = (menu_item_t ) { addr_str, NULL, NULL, &mbp_medea_hack, &(bottles[i].address) };
	}

	mbp_submenu(&menu);

	for (uint8_t i = 0; i < menu.count; i++) {
		free(items[i].text);
	}
}

static void mbp_menu_system() {
	menu_t menu;
	menu_item_t items[10];
	menu.items = items;
	menu.count = 0;

	items[menu.count++] = (menu_item_t ) { "Name", "MENU/NAME.ICO", NULL, &mbp_system_name_edit, NULL };
	items[menu.count++] = (menu_item_t ) { "Avatar", "MENU/MASTER.ICO", NULL, &botnet_screen_pick_avatar, NULL };
	items[menu.count++] = (menu_item_t ) { "About", "MENU/ABOUT.ICO", NULL, &mbp_system_about, NULL };
	items[menu.count++] = (menu_item_t ) { "Shouts", "MENU/SHOUTS.ICO", NULL, &mbp_system_shouts, NULL };
	items[menu.count++] = (menu_item_t ) { "Games", "MENU/CONTROL.ICO", NULL, &mbp_system_game_menu, NULL };
	items[menu.count++] = (menu_item_t ) { "Plane", "MENU/AIRPLANE.ICO", NULL, &mbp_system_airplane_mode_select, NULL };
	items[menu.count++] = (menu_item_t ) { "Test", "MENU/TEST.ICO", NULL, &mbp_system_test, NULL };
	items[menu.count++] = (menu_item_t ) { "Tilt", "MENU/TILT.ICO", NULL, &mbp_system_tilt_mode_select, NULL };
	items[menu.count++] = (menu_item_t ) { "Reset", "MENU/RESET.ICO", NULL, &mbp_system_reset, NULL };

	menu.selected = 0;
	menu.title = "System";
	menu.top = 0;
	mbp_menu(&menu);
}

void mbp_menu_main() {
	menu_t menu;
	menu_item_t items[10];
	menu.count = 0;
	items[menu.count++] = (menu_item_t ) { "Bling!", "MENU/BLING.ICO", NULL, &mbp_menu_bling, NULL };
	items[menu.count++] = (menu_item_t ) { "Games", "MENU/CONTROL.ICO", NULL, &mbp_menu_games, NULL };
	items[menu.count++] = (menu_item_t ) { "Nearby", "MENU/NEARBY.ICO", NULL, &mbp_menu_nearby, NULL };

#ifndef OPSEC
	if (mbp_state_activated_get()) {
		items[menu.count++] = (menu_item_t ) { "Botnet", "MENU/BOTNET.ICO", NULL, &botnet_main_screen, NULL };
		items[menu.count++] = (menu_item_t ) { "TCL", "MENU/TCL.ICO", NULL, &mbp_tcl_menu, NULL };
	}
#endif

	items[menu.count++] = (menu_item_t ) { "Code", "MENU/CODE.ICO", NULL, &mbp_system_code, NULL };
	items[menu.count++] = (menu_item_t ) { "System", "MENU/GEAR.ICO", NULL, &mbp_menu_system, NULL };

	if (mbp_state_master_get()) {
		items[menu.count++] = (menu_item_t ) { "Master", "MENU/MASTER.ICO", NULL, &mbp_master_menu_main, NULL };
	}

	menu.items = items;
	menu.title = "AND!XOR";

	mbp_cigar_eyes_start();
	mbp_menu(&menu);
}
