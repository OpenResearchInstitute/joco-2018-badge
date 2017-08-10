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

#ifndef BLE_ONLY_BUILD

#define BOTNET_UI_MARGIN			3
#define BOTNET_UI_MARGIN_RIGHT		12

//Experience Parameters
#define EXPERIENCE_BASE				100
#define EXPERIENCE_DELTA			50
#define EXPERIENCE_FACTOR			0.2
#define EXPERIENCE_LEVEL_OFFSET		4
#define EXPERIENCE_DEFEND_SUCCESS	10

#define ENABLE_SERVICE_PORT_MS		15 * 60 * 1000			/** 15 min **/
#define EXPLOIT_DEGRADE_MS			5 * 60 * 1000		/** 5 min **/
#define POINTS_GAINED_MS			2 * 60 * 1000		/** 2 min **/
#define SERVICES_DEGRADE_MS			5 * 60 * 1000		/** 5 min **/

//Research max
#define RESEARCH_MAX				40

//Indices of specific services
#define SERVICE_INDEX_SSH			0
#define SERVICE_INDEX_HTTPS			1
#define SERVICE_INDEX_SMTP			2
#define SERVICE_INDEX_POP3			3
#define SERVICE_INDEX_FTP			4
#define SERVICE_INDEX_TELNET		5
#define SERVICE_INDEX_VNC			6
#define SERVICE_INDEX_NC			7

//Exploits
#define EXPLOIT_MAX_UPGRADES		4

//Immunity
#define IMMUNITY_TIME_MS			(5 * 60 * 1000)	//5 min

//Action costs
#define COST_ATTACK					10
#define COST_PATCH					40
#define COST_RESEARCH				10
#define COST_UPGRADE				10

#define FIREWALL_NORMAL_STATE		0b00000011			/** SSH and HTTPS allowed **/

//Avatars
#define AVATARS						{"BOTNET/BNBENDER.RAW","BOTNET/BNZAPP.RAW","BOTNET/BNAMY.RAW", "BOTNET/BNFRY.RAW", "BOTNET/BNKIFF.RAW", "BOTNET/BNLEELA.RAW", "BOTNET/BNAYB.RAW"}
#define AVATAR_NAMES				{"Bender", "Zapp", "Amy", "Fry", "Kiff", "Leela","Zero Wing"}
#define AVATAR_COUNT				8

APP_TIMER_DEF(m_botnet_timer_enable_service_port);
APP_TIMER_DEF(m_botnet_timer_exploit);
APP_TIMER_DEF(m_botnet_timer_points);
APP_TIMER_DEF(m_botnet_timer_services);

static const uint8_t max_difficulties[] = { 90, 90, 80, 70, 60, 50, 25, 10 };
static void __attack_result(ble_badge_t *p_victim, botnet_attack_t *p_attack);
static uint16_t __experience_gained(uint8_t target_level);
static uint16_t __experience_needed();
static int8_t __menu_pick_exploit();
static int8_t __menu_pick_payload();
static void __patch();
static bool __screen_attack(ble_badge_t *p_victim);
static void __screen_exploits();
static void __screen_firewall();
static void __screen_services();

static uint32_t m_immune_end_time = 0;

static void __attack() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();

	char *payload_names[] = BOTNET_PAYLOAD_NAMES;
	char *service_names[] = BOTNET_SERVICE_NAMES;

	ble_badge_db_t *p_db = util_ble_badge_db_get();
	menu_item_t items[p_db->badge_count];

	//Make sure they have exploits
	if (p_state->exploit_count == 0) {
		mbp_ui_error("No exploits. Research at least one.");
		return;
	}

	//Make sure there are neighbors
	if (p_db->badge_count == 0) {
		mbp_ui_error("No badges nearby. Find some friends.");
		return;
	}

	//Ensure they have enough points
	if (p_state->points < COST_ATTACK) {
		char message[32];
		sprintf(message, "You need %d points to attack.", COST_ATTACK);
		mbp_ui_error(message);
		return;
	}

	//Nearby badge menu
	menu_t menu;
	menu.items = items;
	menu.count = 0;
	menu.selected = 0;
	menu.title = "Victims";
	menu.top = 0;

	//Generate menu of nearby badges
	for (uint16_t i = 0; i < p_db->badge_count; i++) {
		menu_item_t item;
		item.text = (char *) malloc(SETTING_NAME_LENGTH);
		item.callback = NULL;
		sprintf(item.text, "%s[%d]", p_db->badges[i].name, p_db->badges[i].level);
		items[i] = item;
		menu.count++;
	}

	//Allow user to quit
	if (mbp_submenu(&menu) == MENU_QUIT) {
		return;
	}

	//Retrieve the victim badge
	ble_badge_t *p_victim = &p_db->badges[menu.selected];

	//Present victim to user and ask to attack
	if (!__screen_attack(p_victim)) {
		return;
	}

	//Connect to the selected badge
	mbp_ui_cls();
	util_gfx_set_cursor(0, 0);
	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_color(COLOR_WHITE);
	util_gfx_print("bendmap v1.1\nScanning...\n");

	//Cleanup the menu text
	for (uint8_t i = 0; i < p_db->badge_count; i++) {
		free(items[i].text);
	}

	//Connect
	if (!botnet_ble_connect_blocking(p_victim)) {
		mbp_ui_error("Connection failed");
		return;
	}

	//"scan" the badge
	uint8_t scan_results = botnet_ble_scan_get(p_victim);
	for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
		if ((scan_results & (1 << i)) > 0) {
			nrf_delay_ms(200);
			util_gfx_print("  ");
			util_gfx_print(service_names[i]);
			util_gfx_print("\n");
		}
	}

	nrf_delay_ms(200);
	util_gfx_print("Complete.\nAny key to continue.");
	util_button_wait();
	util_button_clear();

	//Build an attack
	botnet_attack_t attack;
	attack.level = p_state->level;
	attack.result = BOTNET_ATTACK_IN_PROGRESS;
	mbp_state_name_get(attack.name);

	//Pick a compatible exploit
	int8_t exploit_index = __menu_pick_exploit();
	//Quit?
	if (exploit_index < 0) {
		util_ble_disconnect();
		return;
	}

	//Copy the exploits
	memcpy(&(attack.exploit), &p_state->exploits[exploit_index], sizeof(botnet_exploit_t));

	//Pick a payload
	int8_t payload_index = __menu_pick_payload();
	if (payload_index == BOTNET_PAYLOAD_GOATSE) {
		util_ble_disconnect();
		util_led_clear();
		mbp_bling_rickroll();
		util_led_clear();
		return;
	}

	//Quit?
	if (payload_index < 0) {
		util_ble_disconnect();
		return;
	}
	attack.payload = payload_index;

	//Launch?
	char note[64];
	sprintf(note, "Attack '%s'?\n  %s[%d]\n  %s payload",
			p_db->badges[menu.selected].name,
			service_names[attack.exploit.target_index],
			attack.exploit.sophistication,
			payload_names[attack.payload]);

	if (mbp_ui_toggle_popup("Exploit", 0, "ABORT", "ATTACK", note) == 1) {
		botnet_ble_attack(&(p_db->badges[menu.selected]), &attack);

		//Remove the exploit from the user by shuffling exploits after it in the list on top of it
//		for (uint8_t i = exploit_index; i < p_state->exploit_count - 1; i++) {
//			memcpy(&p_state->exploits[i], &p_state->exploits[i + 1], sizeof(botnet_exploit_t));
//		}
//		p_state->exploit_count--;

		util_ble_disconnect();

		//Remove immunity since they attacked
		m_immune_end_time = 0;

		//Remove points
		p_state->points -= COST_ATTACK;
		mbp_state_save();

		//Process the result
		__attack_result(p_victim, &attack);
	} else {
		util_ble_disconnect();
	}
}

static void __attack_result(ble_badge_t *p_victim, botnet_attack_t *p_attack) {
	botnet_state_t *p_state = mbp_state_botnet_state_get();

	uint8_t old_level = p_state->level;
	char message[32];
	if (p_attack->result == BOTNET_ATTACK_SUCCESS) {
		uint16_t xp = __experience_gained(p_victim->level);
		p_state->experience += xp;

		while (p_state->experience > __experience_needed()) {
			p_state->experience -= __experience_needed();
			if (p_state->level < 255) {
				p_state->level++;
			}
		}

		//Tell the user
		sprintf(message, "Attack successful! %d XP gained.", xp);
		mbp_ui_popup("Attack", message);

		//Save state
		mbp_state_save();
	} else {
		sprintf(message, "Attack failed :(");
		mbp_ui_popup("Attack", message);
	}

	if (p_state->level > old_level) {
		util_ble_level_set(p_state->level);
		sprintf(message, "LEVEL UP! %d!", p_state->level);
		mbp_ui_popup("Level", message);
	}
}

static void __draw_avatar(uint8_t avatar) {
	char *avatars[] = AVATARS;
	if (avatar >= AVATAR_COUNT) {
		avatar = 0;
	}
	util_gfx_draw_raw_file(avatars[avatar], 0, 63, 33, 65, NULL, false, NULL);
}

/**
 * Special debug mode that eases botnet development
 */
#ifdef BOTNET_DEBUG_MODE
static void __debug_mode() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();

	//Generate 0 days for all services
	p_state->exploit_count = BOTNET_SERVICE_COUNT;
	for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
		p_state->exploits[i].sophistication = BOTNET_EXPLOIT_MAX_SOPHISTICATION;
		p_state->exploits[i].target_index = i;
	}
}
#endif

/**
 * Scheduler handler for executing a botnet payload
 */
static void __execute_payload_sched_handler(void *p_data, uint16_t length) {
	app_sched_pause();
	bool cigar_running = mbp_cigar_eyes_running();
	mbp_cigar_eyes_stop();

	botnet_attack_t *p_attack = (botnet_attack_t *) p_data;

	m_immune_end_time = util_millis() + IMMUNITY_TIME_MS;

	switch (p_attack->payload) {
	case BOTNET_PAYLOAD_BSOD:
		util_gfx_draw_raw_file("/BOTNET/BSOD.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, mbp_bling_led_botnet, true, NULL);
		break;
	case BOTNET_PAYLOAD_CLIPPY:
		util_gfx_draw_raw_file("/BLING/CLIPPY.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, mbp_bling_led_botnet, true, NULL);
		break;
	case BOTNET_PAYLOAD_RICKROLL:
		util_gfx_draw_raw_file("/BLING/AND!XOR/RICKROLL.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, mbp_bling_led_botnet, true, NULL);
		break;
	case BOTNET_PAYLOAD_DAMON:
		util_gfx_draw_raw_file("/BLING/AND!XOR/DAMON1.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, mbp_bling_led_botnet, true, NULL);
		break;
	case BOTNET_PAYLOAD_WINDOWSXP:
		util_gfx_draw_raw_file("/BLING/WINXP.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, mbp_bling_led_botnet, true, NULL);
		break;
	case BOTNET_PAYLOAD_WANNACRY:
		util_gfx_draw_raw_file("/BOTNET/WANNACRY.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, mbp_bling_led_botnet, true, NULL);
		break;
	case BOTNET_PAYLOAD_TROLL:
		util_gfx_draw_raw_file("/BLING/AND!XOR/TROLOLOL.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, mbp_bling_led_botnet, true, NULL);
		break;
	case BOTNET_PAYLOAD_ROOT:
		mbp_term_user_role_set(2);
		break;
	case BOTNET_PAYLOAD_CLONE:
		util_ble_name_set(p_attack->name);
		break;
	}

	//Popup who hacked them
	char message[64];
	sprintf(message, "Hacked by %s!", p_attack->name);
	mbp_ui_popup("Oh no!", message);

	//Give them full 5 min immunity after dismissing payload
	m_immune_end_time = util_millis() + IMMUNITY_TIME_MS;

	util_gfx_invalidate();
	util_led_clear();

	//Only start if cigar was running when we started
	if (cigar_running) {
		mbp_cigar_eyes_start();
	}

	app_sched_resume();
}

static uint16_t __experience_gained(uint8_t target_level) {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	uint16_t xp;

	//If we're attacking a much lower player
	if (p_state->level >= (target_level + EXPERIENCE_LEVEL_OFFSET)) {
		xp = EXPERIENCE_BASE * EXPERIENCE_FACTOR * (float) ((float) target_level / (float) p_state->level);
	}
	//Attacking a player in our range or higher
	else {
		xp = ((target_level - p_state->level + EXPERIENCE_LEVEL_OFFSET) * EXPERIENCE_BASE) * EXPERIENCE_FACTOR;
	}

	return xp;
}

static uint16_t __experience_needed() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	return EXPERIENCE_BASE + ((p_state->level - 1) * EXPERIENCE_DELTA);
}

static int8_t __menu_pick_exploit() {
	char *service_names[] = BOTNET_SERVICE_NAMES;
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	menu_t menu;
	menu.count = p_state->exploit_count;
	menu.selected = 0;
	menu.title = "Exploit";
	menu.top = 0;

	menu_item_t items[p_state->exploit_count];
	for (uint8_t i = 0; i < p_state->exploit_count; i++) {
		items[i].callback = NULL;
		items[i].text = (char *) malloc(20);
		sprintf(items[i].text, "%s [%d]", service_names[p_state->exploits[i].target_index], p_state->exploits[i].sophistication);
	}

	menu.items = items;

	uint8_t result = mbp_submenu(&menu);

	//Cleanup
	for (uint8_t i = 0; i < p_state->exploit_count; i++) {
		free(items[i].text);
	}

	//Quit?
	if (result == MENU_QUIT) {
		return -1;
	}

	return menu.selected;
}

/**
 * Allow the user to pick a payload to drop into the attack.
 *
 * @return -1 if the user quit
 */
static int8_t __menu_pick_payload() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();

	char *payload_names[] = BOTNET_PAYLOAD_NAMES;
	menu_t menu;
	menu.selected = 0;
	menu.title = "Payload";
	menu.top = 0;
	uint8_t max = 4;
	if (p_state->level >= 5) {
		max = 6;
	}
	if (p_state->level >= 10) {
		max = 7;
	}
	if (p_state->level >= 15) {
		max = 8;
	}
	if (p_state->level >= 20) {
		max = 9;
	}
	if (p_state->level >= 25) {
		max = 10;
	}
	if (mbp_state_master_get()) {
		max = 10;
	}

	menu.count = max;
	menu_item_t items[max];
	for (uint8_t i = 0; i < max; i++) {
		items[i].callback = NULL;
		items[i].text = payload_names[i];
	}

	menu.items = items;

	uint8_t result = mbp_submenu(&menu);

	//Quit?
	if (result == MENU_QUIT) {
		return -1;
	}

	return menu.selected;
}

static void __patch() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	char *message = (char *) malloc(32);

//	if (p_state->points < COST_PATCH) {
//		char message[50];
//		sprintf(message, "Not enough points! You need at least %d, you have %d.", COST_PATCH, p_state->points);
//		mbp_ui_error(message);
//		free(message);
//		return;
//	}

//Confirm
	sprintf(message, "Patch all services for %d points?", COST_PATCH);
	if (mbp_ui_toggle_popup("Patch", 0, "Cancel", "Yes", message) == 1) {
		//Spend their points
		p_state->points -= COST_PATCH;

		//Patch each service up to their max difficulty
		uint8_t i = 0;
		for (i = 0; i < 8; i++) {
			p_state->services[i].difficulty = max_difficulties[i];
		}

		//Oh yeah, save their state too!
		mbp_state_save();

		//Be nice to the user
		mbp_ui_popup("Patch", "All Services Up-to-Date!");
	}

	free(message);
}

static void __research() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	char *service_names[] = BOTNET_SERVICE_NAMES;

	//Make sure they have room
	if (p_state->exploit_count >= BOTNET_MAX_EXPLOITS) {
		mbp_ui_error("Exploit cache is full.");
		return;
	}

	//Make sure they enough points
	if (p_state->points < COST_RESEARCH) {
		char message[50];
		sprintf(message, "Not enough points! You need at least %d, you have %d.", COST_RESEARCH, p_state->points);
		mbp_ui_error(message);
		return;
	}

	//Spend their points
	p_state->points -= COST_RESEARCH;

	botnet_exploit_t exploit;
	exploit.upgrade_count = 0;
	exploit.target_index = util_math_rand8_max(BOTNET_SERVICE_COUNT);

	//Pick random sophistication
	exploit.sophistication = util_math_rand8_max(RESEARCH_MAX);

	//Add new exploit to the end
	p_state->exploits[p_state->exploit_count++] = exploit;

	mbp_state_save();

	//Tell the user
	char message[32];
	sprintf(message, "%s exploit found!\n%d strength.", service_names[exploit.target_index], (int) exploit.sophistication);
	mbp_ui_popup("Exploit", message);
}

static bool __screen_attack(ble_badge_t *p_victim) {
	uint8_t selected = 0;
	bool attack = false;
	util_gfx_invalidate();

	while (1) {

		if (!util_gfx_is_valid_state()) {
			//Draw background
			util_gfx_draw_raw_file("BOTNET/BOTNETBG.RAW", 0, 0, 128, 128, NULL, false, NULL);

			util_gfx_set_font(FONT_LARGE);

			//Victim name centered and with a white shadow
			uint16_t w = 0;
			uint16_t h = 0;
			util_gfx_set_color(COLOR_WHITE);
			util_gfx_get_text_bounds(p_victim->name, 0, 0, &w, &h);
			util_gfx_set_cursor(((GFX_WIDTH - w) / 2) + 1, 4);
			util_gfx_print(p_victim->name);
			util_gfx_set_color(COLOR_RED);
			util_gfx_set_cursor((GFX_WIDTH - w) / 2, 3);
			util_gfx_print(p_victim->name);

			//Attack?
			util_gfx_set_color(COLOR_WHITE);
			util_gfx_set_cursor(40, 63);
			util_gfx_print("Attack?");

			//Level
			char level_str[32];
			sprintf(level_str, "Level %d", p_victim->level);
			util_gfx_get_text_bounds(level_str, 0, 0, &w, &h);
			util_gfx_set_cursor((GFX_WIDTH - w) / 2, 32);	//center
			util_gfx_set_color(COLOR_YELLOW);
			util_gfx_print(level_str);

			//Avatar
			__draw_avatar(p_victim->avatar);

		}

		//Cancel button
		mbp_ui_button(40, 82, 76, 19, "Cancel", selected == 0);
		//Attack button
		mbp_ui_button(40, 104, 76, 19, "Attack", selected == 1);

		//Flag drawing complete
		util_gfx_validate();

		util_button_wait();
		if (util_button_down()) {
			if (selected == 0) {
				selected = 1;
			}
		} else if (util_button_up()) {
			if (selected == 1) {
				selected = 0;
			}
		} else if (util_button_action()) {
			attack = (selected == 1);
			break;
		}
		util_button_clear();
	}

	return attack;
}

static void __screen_exploits() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	char *service_names[] = BOTNET_SERVICE_NAMES;

	if (p_state->exploit_count == 0) {
		mbp_ui_error("No exploits.");
		return;
	}

	menu_t menu;
	menu.count = 0;
	menu.title = "Exploits";
	menu_item_t items[BOTNET_MAX_EXPLOITS];
	menu.items = items;

	for (uint8_t i = 0; i < p_state->exploit_count; i++) {
		char zday[5];
		if (p_state->exploits[i].sophistication > max_difficulties[p_state->exploits[i].target_index]) {
			sprintf(zday, "0day");
		} else {
			sprintf(zday, " ");
		}
		char *p_name = (char *) malloc(32);
		sprintf(p_name, "%s[%d] %s", service_names[p_state->exploits[i].target_index], p_state->exploits[i].sophistication, zday);
		menu.items[menu.count++] = (menu_item_t ) { p_name, NULL, NULL, NULL, NULL };
	}

	//If something was selected present a menu of options to do to the exploit
	if (mbp_submenu(&menu) != MENU_QUIT) {
		botnet_exploit_t *p_exploit = &(p_state->exploits[menu.selected]);
		char title[20];
		sprintf(title, "%s %d", service_names[p_exploit->target_index], p_exploit->sophistication);
		menu_t exploit_menu;
		exploit_menu.count = 2;
		exploit_menu.title = title;
		menu_item_t exploit_menu_items[2];
		exploit_menu.items = exploit_menu_items;
		exploit_menu_items[0] = (menu_item_t ) { "Upgrade", NULL, NULL, NULL, NULL };
		exploit_menu_items[1] = (menu_item_t ) { "Delete", NULL, NULL, NULL, NULL };

		if (mbp_submenu(&exploit_menu) != MENU_QUIT) {
			char message[32];
			switch (exploit_menu.selected) {
			case 0:
				//upgrade
				;
				uint8_t cost = COST_UPGRADE;

				//Ensure they have enough points
				if (p_state->points < COST_UPGRADE) {
					sprintf(message, "You need %d points.", cost);
					mbp_ui_error(message);
					break;
				}

				//Ensure they havent reached max upgrades
				if (p_exploit->upgrade_count >= EXPLOIT_MAX_UPGRADES) {
					mbp_ui_error("Exploit fully upgraded.");
					break;
				}

				//Confirm

				sprintf(message, "Upgrade %s for %d points?", items[menu.selected].text, cost);
				if (mbp_ui_toggle_popup("Upgrade", 0, "No", "Yes", message) == 1) {
					p_state->points -= 10;
					p_exploit->sophistication += util_math_rand8_max(20) + 10;
					if (p_exploit->sophistication > 100) {
						p_exploit->sophistication = 100;
					}

					p_exploit->upgrade_count++;
					mbp_state_save();

					sprintf(message, "%s upgraded to %d.", service_names[p_exploit->target_index], p_exploit->sophistication);
					mbp_ui_popup("Upgrade", message);
				}
				break;
			case 1:
				//delete
				sprintf(message, "Delete %s?", items[menu.selected].text);
				if (mbp_ui_toggle_popup("Delete", 0, "No", "Yes", message) == 1) {

					for (uint8_t i = menu.selected; i < p_state->exploit_count - 1; i++) {
						memcpy(&(p_state->exploits[i]), &(p_state->exploits[i + 1]), sizeof(botnet_exploit_t));
					}
					p_state->exploit_count--;
					mbp_state_save();

					mbp_ui_popup("Delete", "Deleted");
				}
				break;
			}
		}
	}

	//cleanup mallocs
	for (uint8_t i = 0; i < menu.count; i++) {
		free(menu.items[i].text);
	}
}

static void __screen_firewall() {
	char *service_names[] = BOTNET_SERVICE_NAMES;
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	uint8_t selected = 0;
	char temp[32];
	uint16_t w;
	uint16_t h;

	while (1) {
		util_gfx_cursor_area_reset();
		//Draw background
		util_gfx_draw_raw_file("BOTNET/BOTNETBG.RAW", 0, 0, 128, 128, NULL, false, NULL);

		//Print the title
		util_gfx_set_font(FONT_LARGE);
		util_gfx_set_color(COLOR_WHITE);
		util_gfx_set_cursor(0, BOTNET_UI_MARGIN);
		util_gfx_print("Firewall");

		util_gfx_set_font(FONT_SMALL);
		for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
			util_gfx_set_color(COLOR_WHITE);
			util_gfx_set_cursor(32, 16 + (i * 13));
			util_gfx_print(service_names[i]);

			util_gfx_set_cursor(73, 16 + (i * 13));
			if ((p_state->firewall_state & (1 << i)) > 0) {
				util_gfx_set_color(COLOR_GREEN);
				util_gfx_print("Allow");
			} else {
				util_gfx_set_color(COLOR_RED);
				util_gfx_print("Deny");
			}
		}

		//Highlight selected
		util_gfx_draw_rect(69, 13 + (selected * 13), 57, 12, COLOR_YELLOW);

		//Draw points
		sprintf(temp, "%d points", p_state->points);
		util_gfx_get_text_bounds(temp, 0, 0, &w, &h);
		util_gfx_set_cursor(GFX_WIDTH - w - 4, GFX_HEIGHT - h);
		util_gfx_set_color(COLOR_WHITE);
		util_gfx_print(temp);

		//graphics state is valid
		util_gfx_validate();

		util_button_wait();
		if (util_button_down()) {
			if (selected < BOTNET_SERVICE_COUNT - 1) {
				selected++;
			}
		} else if (util_button_up()) {
			if (selected > 0) {
				selected--;
			}
		} else if (util_button_left()) {
			break;
		} else if (util_button_action()) {
			//Free to turn open firewall
			if ((p_state->firewall_state & (1 << selected)) == 0) {
				p_state->firewall_state |= 1 << selected;
				botnet_update_service_status();
				mbp_state_save();
			}
			//Not free to enable the firewall
			else {
				if (p_state->points >= COST_FIREWALL) {
					p_state->firewall_state &= ~(1 << selected);
					p_state->points -= COST_FIREWALL;
					botnet_update_service_status();
					mbp_state_save();
				} else {
					sprintf(temp, "Not enough points, you need %d.", COST_FIREWALL);
					mbp_ui_error(temp);
				}
			}
		}
		util_button_clear();
	}
}

static void __screen_more() {

	uint8_t selected = 0;

	//Modal popup for more menu items
	while (1) {
		//Draw background
		util_gfx_draw_raw_file("BOTNET/BOTNETBG.RAW", 0, 0, 128, 128, NULL, false, NULL);

		mbp_ui_button(20, 20, 88, 20, "Services", selected == 0);
		mbp_ui_button(20, 42, 88, 20, "Firewall", selected == 1);
		mbp_ui_button(20, 64, 88, 20, "Patch", selected == 2);
		mbp_ui_button(20, 86, 88, 20, "Exploits", selected == 3);

		util_gfx_validate();

		util_button_wait();

		if (util_button_down()) {
			selected++;
			if (selected > 3)
				selected = 3;
		} else if (util_button_up()) {
			if (selected > 0) {
				selected--;
			}
		} else if (util_button_left()) {
			util_button_clear();
			break;
		} else if (util_button_action()) {
			util_button_clear();
			switch (selected) {
			case 0:
				__screen_services();
				return;
				break;
			case 1:
				__screen_firewall();
				return;
				break;
			case 2:
				__patch();
				return;
				break;
			case 3:
				__screen_exploits();
				break;
			}
		}

		util_button_clear();
	}
}

static void __screen_services() {
	char *service_names[] = BOTNET_SERVICE_NAMES;
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	uint8_t selected = 0;
	char temp[32];
	uint16_t w;
	uint16_t h;

	while (1) {
		util_gfx_cursor_area_reset();
		mbp_ui_cls();

		//Print the title
		util_gfx_set_font(FONT_LARGE);
		util_gfx_set_color(COLOR_WHITE);
		util_gfx_set_cursor(0, BOTNET_UI_MARGIN);
		util_gfx_print("Services");

		util_gfx_set_font(FONT_SMALL);
		for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
			util_gfx_set_color(COLOR_WHITE);
			util_gfx_set_cursor(10, 16 + (i * 13));
			util_gfx_print(service_names[i]);

			//Toggle buttons + status
			util_gfx_set_cursor(51, 16 + (i * 13));
			if (p_state->services[i].enabled) {
				util_gfx_set_color(COLOR_GREEN);
				util_gfx_print("UP");
			} else {
				util_gfx_set_color(COLOR_RED);
				util_gfx_print("DOWN");
			}

			//Difficult levels
			util_gfx_draw_rect(81, 13 + (i * 13), 45, 12, COLOR_WHITE);
			w = util_math_map(p_state->services[i].difficulty, 0, max_difficulties[i], 0, 43);
			util_gfx_fill_rect(82, 14 + (i * 13), w, 10, COLOR_YELLOW);
		}

		//Highlight selected
		util_gfx_draw_rect(48, 13 + (selected * 13), 30, 12, COLOR_YELLOW);

		//Draw points
		sprintf(temp, "%d points", p_state->points);
		util_gfx_get_text_bounds(temp, 0, 0, &w, &h);
		util_gfx_set_cursor(GFX_WIDTH - w - 4, GFX_HEIGHT - h);
		util_gfx_set_color(COLOR_WHITE);
		util_gfx_print(temp);

		//graphics state is valid
		util_gfx_validate();

		util_button_wait();
		if (util_button_down()) {
			if (selected < BOTNET_SERVICE_COUNT - 1) {
				selected++;
			}
		} else if (util_button_up()) {
			if (selected > 0) {
				selected--;
			}
		} else if (util_button_left()) {
			break;
		} else if (util_button_action()) {

			//Free to turn on services
			if (!p_state->services[selected].enabled) {
				p_state->services[selected].enabled = true;
				botnet_update_service_status();
				mbp_state_save();
			}
			//Not free to turn off services
			else {
				if (p_state->points >= COST_SERVICE) {
					p_state->points -= COST_SERVICE;
					p_state->services[selected].enabled = false;
					botnet_update_service_status();
					mbp_state_save();
				} else {
					sprintf(temp, "Not enough points, you need %d.", COST_SERVICE);
					mbp_ui_error(temp);
				}
			}
		}
		util_button_clear();
	}
}

/**
 * Periodically enable a firewall or service randomly
 */
static void __timer_enable_service_port_handler(void *p_context) {
	botnet_state_t *p_state = mbp_state_botnet_state_get();

//Pick a service index
	uint8_t index = util_math_rand8_max(BOTNET_SERVICE_COUNT);
	uint8_t selector = util_math_rand8_max(2);

//Pick firewall or service randomly
	if (selector == 0) {
		//Enable a firewall
		p_state->firewall_state |= (1 << index);
	} else {
		//Enable a service
		p_state->services[index].enabled = true;
	}

	botnet_update_service_status();
}

/**
 * Degrade all exploits by one point never going below 0.
 */
static void __timer_exploit_handler(void *p_context) {
	uint8_t degrade = util_math_rand8_max(1) + 5;
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	for (uint8_t i = 0; i < p_state->exploit_count; i++) {

		if (p_state->exploits[i].sophistication >= degrade) {
			p_state->exploits[i].sophistication -= degrade;
		} else {
			p_state->exploits[i].sophistication = 0;
		}
	}
}

static void __timer_points_handler(void *p_context) {
	//Don't earn points in airplane mode
	if (mbp_state_airplane_mode_get()) {
		return;
	}

	botnet_state_t *p_state = mbp_state_botnet_state_get();

	//Ensure points are negative
	if (p_state->points < 0) {
		p_state->points = 0;
	} else if (p_state->points > BOTNET_POINTS_MAX) {
		p_state->points = BOTNET_POINTS_MAX;
	}

	for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
		if ((p_state->service_state & (1 << i)) > 0) {
			if (p_state->points < UINT16_MAX) {
				p_state->points++;
			}
		}
	}

	mbp_state_save();
}

static void __timer_screen_refresh(void *p_context) {
	util_gfx_invalidate();
}

static void __timer_services_handler(void *p_context) {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
		if (p_state->services[i].difficulty > 0) {
			p_state->services[i].difficulty--;
		}
	}
}

void botnet_eval_incoming_attacking(botnet_attack_t *p_attack) {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	bool accessible = botnet_service_accessible(p_attack->exploit.target_index);
	uint16_t attacker_strength = (2 * p_attack->level) + p_attack->exploit.sophistication;
	uint16_t defender_strength = (2 * p_state->level) + p_state->services[p_attack->exploit.target_index].difficulty;

	if (accessible && (attacker_strength > defender_strength) && !botnet_immune()) {
		p_attack->result = BOTNET_ATTACK_SUCCESS;

		//Queue up payload execution
		app_sched_event_put(p_attack, sizeof(botnet_attack_t), __execute_payload_sched_handler);
	}
	//Failed attack, reward experience
	else {
		p_attack->result = BOTNET_ATTACK_FAILED;

		//Give player experience
		p_state->experience += EXPERIENCE_DEFEND_SUCCESS;

		//Level up if we can
		if (p_state->experience > __experience_needed()) {
			p_state->experience -= __experience_needed();
			p_state->level++;
		}

		mbp_state_save();
	}
}

bool botnet_immune() {
	return util_millis() <= m_immune_end_time;
}

uint8_t botnet_level_get() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	return p_state->level;
}

void botnet_main_screen() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	uint16_t w, h;
	APP_TIMER_DEF(invalidate_timer);
	APP_ERROR_CHECK(app_timer_create(&invalidate_timer, APP_TIMER_MODE_REPEATED, __timer_screen_refresh));
	APP_ERROR_CHECK(app_timer_start(invalidate_timer, APP_TIMER_TICKS(1000*60, UTIL_TIMER_PRESCALER), NULL));

#ifdef BOTNET_DEBUG_MODE
	__debug_mode();
#endif

	//buffer for formatting text
	char temp[32];

	//Selected button index
	uint8_t selected = 0;
	bool redraw = true;

	while (1) {

		if (redraw || !util_gfx_is_valid_state()) {
			//Make sure there's no clipping
			util_gfx_cursor_area_reset();

			//Draw background
			util_gfx_draw_raw_file("BOTNET/BOTNETBG.RAW", 0, 0, 128, 128, NULL, false, NULL);

			//Print their name
			util_gfx_set_font(FONT_LARGE);
			util_gfx_set_color(COLOR_WHITE);
			util_gfx_set_cursor(0, BOTNET_UI_MARGIN);
			mbp_state_name_get(temp);
			util_gfx_print(temp);

			//Print their level
			util_gfx_set_color(COLOR_YELLOW);
			sprintf(temp, "L%d", p_state->level);
			util_gfx_get_text_bounds(temp, 0, 0, &w, &h);
			int16_t offset = (48 - BOTNET_UI_MARGIN - w) / 2;
			util_gfx_set_cursor(offset, 26);
			util_gfx_print(temp);

			//Print points
			util_gfx_set_color(COLOR_WHITE);
			util_gfx_set_font(FONT_SMALL);
			sprintf(temp, "%d pt\n", p_state->points);
			util_gfx_set_cursor(BOTNET_UI_MARGIN, 45);
			util_gfx_print(temp);

			//Print exploits
			sprintf(temp, "%d expl\n", p_state->exploit_count);
			util_gfx_get_text_bounds(temp, 0, 0, &w, &h);
			util_gfx_set_cursor(GFX_WIDTH - BOTNET_UI_MARGIN_RIGHT - w, 45);
			util_gfx_print(temp);

			//Experience bar
			util_gfx_draw_rect(48, 18, 70, 20, COLOR_WHITE);
			w = util_math_map(p_state->experience, 0, __experience_needed(), 0, 70);
			util_gfx_fill_rect(49, 19, w, 18, COLOR_YELLOW);

			//Draw avatar
			__draw_avatar(p_state->avatar);

			//Service and firewall state
			uint16_t service_h = GFX_HEIGHT / 8;
			for (uint8_t i = 0; i < 8; i++) {

				//Firewall state
				uint16_t color = COLOR_RED;
				if ((p_state->firewall_state & (1 << i)) > 0) {
					color = COLOR_GREEN;
				}
				util_gfx_fill_rect(GFX_WIDTH - 3, service_h * i + 1, 1, service_h - 1, color);

				//Service state
				color = COLOR_RED;
				if (p_state->services[i].enabled) {
					color = COLOR_GREEN;
				}
				util_gfx_fill_rect(GFX_WIDTH - 6, service_h * i + 1, 1, service_h - 1, color);
			}

			redraw = false;
		}

		//Indicate neighbors
		uint8_t neighbors = 0;
		ble_badge_db_t *p_badge_db = util_ble_badge_db_get();
		for (uint8_t i = 0; i < p_badge_db->badge_count; i++) {
			if (p_badge_db->badges[i].last_seen >= (util_millis() - 30000)) {
				neighbors++;
			}
		}

		char btn_attack_text[16];
		sprintf(btn_attack_text, "Attack [%d]", neighbors);

		//Always draw the buttons
		mbp_ui_button(40, 60, 76, 19, "Research", selected == 0);
		mbp_ui_button(40, 82, 76, 19, btn_attack_text, selected == 1);
		mbp_ui_button(40, 104, 76, 19, "More", selected == 2);

		//validate screen state
		util_gfx_validate();

		util_button_wait();

		if (util_button_action()) {
			util_button_clear();
			switch (selected) {
			case 0:
				__research();
				break;
			case 1:
				__attack();
				break;
			case 2:
				__screen_more();
				break;
			}

			redraw = true;
		} else if (util_button_left()) {
			util_button_clear();
			app_timer_stop(invalidate_timer);
			return;
		} else if (util_button_down()) {
			selected = (selected + 1) % 3;
		} else if (util_button_up()) {
			if (selected > 0) {
				selected--;
			}
		}

		util_button_clear();
	}
}
void botnet_new() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	p_state->avatar = 0;
	p_state->experience = 0;
	p_state->level = 1;
	p_state->points = 60;
	p_state->exploit_count = 0;
	p_state->service_state = 0;
	p_state->firewall_state = FIREWALL_NORMAL_STATE;

	//Init services
	for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
		p_state->services[i].enabled = (i < 2);		//Only enabled first two services (SSH and HTTPS)
		p_state->services[i].difficulty = max_difficulties[i];
	}

	botnet_update_service_status();
}

void botnet_screen_pick_avatar() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	char *avatar_names_all[] = AVATAR_NAMES;	//All avatar names
	char *avatar_names[AVATAR_COUNT];
	uint8_t avatar_numbers[AVATAR_COUNT];	//Map to actual avatar numbers from local
	uint8_t selected = 0;
	uint8_t count = 0;
	uint16_t unlock = mbp_state_unlock_get();
	util_gfx_invalidate();

	//build local list of avatars based on unlock status
	avatar_names[count] = avatar_names_all[0];
	avatar_numbers[count++] = 0;

	//Zapp
	if ((unlock & UNLOCK_MASK_C2) > 0) {
		avatar_names[count] = avatar_names_all[1];
		avatar_numbers[count++] = 1;
	}

	avatar_names[count] = avatar_names_all[2];
	avatar_numbers[count++] = 2;

	//Fry
	if ((unlock & UNLOCK_MASK_CHIP8) > 0) {
		avatar_names[count] = avatar_names_all[3];
		avatar_numbers[count++] = 3;
	}
	avatar_names[count] = avatar_names_all[4];
	avatar_numbers[count++] = 4;
	avatar_names[count] = avatar_names_all[5];
	avatar_numbers[count++] = 5;

	//Zero Wing
	if ((unlock & UNLOCK_MASK_WHISKEY_PIRATES) > 0) {
		avatar_names[count] = avatar_names_all[6];
		avatar_numbers[count++] = 6;
	}

	while (1) {
		//Draw background
		util_gfx_draw_raw_file("BOTNET/BOTNETBG.RAW", 0, 0, 128, 128, NULL, false, NULL);

		util_gfx_set_font(FONT_LARGE);
		util_gfx_set_color(COLOR_WHITE);
		util_gfx_set_cursor(0, 3);
		util_gfx_print("Avatar");

		__draw_avatar(avatar_numbers[selected]);

		//Draw buttons
		for (uint8_t i = 0; i < count; i++) {
			mbp_ui_button(48, (i * 16) + 16, 76, 14, avatar_names[i], i == selected);
		}

		//Flag drawing complete
		util_gfx_validate();

		util_button_wait();
		if (util_button_down()) {
			if (selected < count - 1) {
				selected++;
			}
		} else if (util_button_up()) {
			if (selected > 0) {
				selected--;
			}
		} else if (util_button_action()) {
			p_state->avatar = avatar_numbers[selected];
			mbp_state_save();
			util_ble_avatar_update();
			char message[32];
			sprintf(message, "Avatar set to %s", avatar_names[selected]);
			mbp_ui_popup("Avatar", message);
			break;
		}
		util_button_clear();
	}
}

inline bool botnet_service_accessible(uint8_t index) {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	return p_state->services[index].enabled && (p_state->firewall_state & (1 << index));
}

void botnet_start() {
	uint32_t err_code;

//Start up the points timer
	err_code = app_timer_create(&m_botnet_timer_points, APP_TIMER_MODE_REPEATED, __timer_points_handler);
	APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(m_botnet_timer_points, APP_TIMER_TICKS(POINTS_GAINED_MS, UTIL_TIMER_PRESCALER), NULL);
	APP_ERROR_CHECK(err_code);

//Start up the exploit degredation timer
	err_code = app_timer_create(&m_botnet_timer_exploit, APP_TIMER_MODE_REPEATED, __timer_exploit_handler);
	APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(m_botnet_timer_exploit, APP_TIMER_TICKS(EXPLOIT_DEGRADE_MS, UTIL_TIMER_PRESCALER), NULL);
	APP_ERROR_CHECK(err_code);

//Start up the service degredation timer
	err_code = app_timer_create(&m_botnet_timer_services, APP_TIMER_MODE_REPEATED, __timer_services_handler);
	APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(m_botnet_timer_services, APP_TIMER_TICKS(SERVICES_DEGRADE_MS, UTIL_TIMER_PRESCALER), NULL);
	APP_ERROR_CHECK(err_code);

//Start up the service and firewall enable timer
	err_code = app_timer_create(&m_botnet_timer_enable_service_port, APP_TIMER_MODE_REPEATED, __timer_enable_service_port_handler);
	APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(m_botnet_timer_enable_service_port, APP_TIMER_TICKS(ENABLE_SERVICE_PORT_MS, UTIL_TIMER_PRESCALER), NULL);
	APP_ERROR_CHECK(err_code);

	botnet_update_service_status();
}

/**
 * Update the service state mask based on firewall and service availability
 */
void botnet_update_service_status() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();

//Build service state
	p_state->service_state = 0;
	for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
		if (p_state->services[i].enabled) {
			p_state->service_state |= (p_state->firewall_state & (1 << i));
		}
	}

	botnet_ble_update_service_status();
}

#endif
