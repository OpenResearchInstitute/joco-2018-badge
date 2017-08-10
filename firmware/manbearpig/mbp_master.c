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

#define C2_STOP_TIME_MS		(10 * 60 * 1000)		/** 10 min **/

//Master C2
#define C2_CMD_NAMES		{"Play Bling", "Give Points", "Level Up", "Unlock Zapp", "Enable Services", "Shutdown"}
#define C2_CMD_COUNT		6
#define C2_CMDS				{MASTER_C2_CMD_PLAY, MASTER_C2_CMD_POINTS, MASTER_C2_CMD_LEVEL, MASTER_C2_CMD_UNLOCK, MASTER_C2_CMD_SERVICES, MASTER_C2_CMD_STOP}

//Master C2 play
#define C2_PLAY_NAMES		{"Rick Roll", "What's Up", "Nyan", "Major Lazer", "Hypnotoad", \
							"Matt Damon", "Bender"	\
							}
#define C2_PLAY_COUNT		7
#define C2_PLAY_MODES		{MASTER_C2_PLAY_RICKROLL, MASTER_C2_PLAY_WHATS_UP, MASTER_C2_PLAY_NYAN, MASTER_C2_PLAY_MAJOR_LAZER, MASTER_C2_PLAY_TOAD, \
							MASTER_C2_PLAY_DAMON, MASTER_C2_PLAY_BENDER	\
							}

//Master to human actions
#define ACTION_NAMES		{"Give 0day", "Give Points", "Level Up", "Unlock"}
#define ACTION_COUNT		4
#define ACTION_DATA			{MASTER_ACTION_FREE_0DAY, MASTER_ACTION_FREE_POINTS, MASTER_ACTION_LEVEL_UP, MASTER_ACTION_UNLOCK}

master_c2_t m_c2;

//static inline uint8_t __gen_special_byte(int8_t animation, int8_t time) {
//	uint8_t byte = (animation << 5) | time;
//	byte ^= (util_get_device_id() & 0xFF);
//	return byte;
//}

typedef struct {
	uint8_t animation;
	uint8_t time;
} troll_sync_t;

static int8_t __menu_pick_human_action() {
	char *action_names[] = ACTION_NAMES;

	menu_item_t items[10];
	menu_t menu;
	menu.count = 0;
	menu.selected = 0;
	menu.top = 0;
	menu.title = "Action";
	menu.items = items;
	for (uint8_t i = 0; i < ACTION_COUNT; i++) {
		menu.items[menu.count++] = (menu_item_t ) { action_names[i], NULL, NULL, NULL, NULL };
	}

	if (mbp_submenu(&menu) == MENU_QUIT) {
		return -1;
	}

	return menu.selected;
}

static int8_t __menu_pick_c2_cmd() {
	char *cmd_names[C2_CMD_COUNT] = C2_CMD_NAMES;
	uint8_t cmds[] = C2_CMDS;

	menu_item_t items[C2_CMD_COUNT];
	menu_t menu;
	menu.count = 0;
	menu.selected = 0;
	menu.top = 0;
	menu.title = "C2 Command";
	menu.items = items;

	for (uint8_t i = 0; i < C2_CMD_COUNT; i++) {
		items[menu.count++] = (menu_item_t ) { cmd_names[i], NULL, NULL, NULL, NULL };
	}

	if (mbp_submenu(&menu) == MENU_QUIT) {
		return -1;
	}

	return cmds[menu.selected];
}

static int8_t __menu_pick_c2_play_animation() {
	char *play_names[C2_PLAY_COUNT] = C2_PLAY_NAMES;
	uint8_t modes[] = C2_PLAY_MODES;

	menu_item_t items[C2_PLAY_COUNT];
	menu_t menu;
	menu.count = 0;
	menu.selected = 0;
	menu.top = 0;
	menu.title = "C2 Command";
	menu.items = items;

	for (uint8_t i = 0; i < C2_PLAY_COUNT; i++) {
		items[menu.count++] = (menu_item_t ) { play_names[i], NULL, NULL, NULL, NULL };
	}

	if (mbp_submenu(&menu) == MENU_QUIT) {
		return -1;
	}

	return modes[menu.selected];
}

static void __cheat_0days() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();

//Generate 0 days for all services
	p_state->exploit_count = BOTNET_SERVICE_COUNT;
	for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
		p_state->exploits[i].sophistication = 100;
		p_state->exploits[i].target_index = i;
	}

	mbp_state_save();

	mbp_ui_popup("CHEAT", "0 Days generated for all services");
}

static void __cheat_level_up() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	if (p_state->level < UINT8_MAX - 1) {
		p_state->level++;
	}
	mbp_state_save();

	char message[32];
	sprintf(message, "Leveled up to %d!", p_state->level);
	mbp_ui_popup("CHEAT", message);
}

static void __cheat_points() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	p_state->points = BOTNET_POINTS_MAX;
	mbp_state_save();

	mbp_ui_popup("CHEAT", "Points set to MAX");
}

static void __cheat_unlock_all() {
	mbp_state_unlock_set(0x7FFF);
	mbp_state_save();

	mbp_ui_popup("CHEAT", "Everything unlocked. Have fun!");
}

static void __c2_play_schedule_handler(void *p_data, uint16_t length) {
	app_sched_pause();
	bool cigar_running = mbp_cigar_eyes_running();
	mbp_cigar_eyes_stop();

	uint32_t endtime = util_millis() + (1 * 60 * 1000); //1min minimum run time

	while (util_millis() < endtime) {
		util_button_clear();
		uint8_t mode = *((uint8_t *) p_data);
		switch (mode) {
		case MASTER_C2_PLAY_BENDER:
			mbp_bling_bender();
			break;
		case MASTER_C2_PLAY_DAMON:
			mbp_bling_damon();
			break;
		case MASTER_C2_PLAY_WHATS_UP:
			mbp_bling_whats_up();
			break;
		case MASTER_C2_PLAY_MAJOR_LAZER:
			mbp_bling_major_lazer(NULL);
			break;
		case MASTER_C2_PLAY_NYAN:
			mbp_bling_nyan();
			break;
		case MASTER_C2_PLAY_RICKROLL:
			mbp_bling_rickroll();
			break;
		case MASTER_C2_PLAY_TOAD:
			mbp_bling_toad();
			break;
		}

		if (util_millis() < endtime) {
			uint32_t sec = (endtime - util_millis()) / 1000;
			char message[32];
			sprintf(message, "You were p0wn3d - wait %d seconds.", (int) sec);
			mbp_ui_popup("p0wn3d", message);
		} else {
			mbp_ui_popup("p0wn3d", "You were p0wn3d.");
		}
	}

	util_led_clear();
	util_gfx_invalidate();

	if (cigar_running) {
		mbp_cigar_eyes_start();
	}
	app_sched_resume();
}

static void __c2_level_schedule_handler(void *p_data, uint16_t length) {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	p_state->level++;
	mbp_state_c2_level_set(true);
	mbp_state_save();

	mbp_ui_popup("Botnet", "Level up from AND!XOR");
}

static void __c2_points_schedule_handler(void *p_data, uint16_t length) {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	p_state->points += 500;
	if (p_state->points > BOTNET_POINTS_MAX) {
		p_state->points = BOTNET_POINTS_MAX;
	}
	mbp_state_c2_points_set(true);
	mbp_state_save();

	mbp_ui_popup("Botnet", "500 points from AND!XOR");
}

static void __c2_unlock_schedule_handler(void *p_data, uint16_t length) {
	uint16_t unlock = mbp_state_unlock_get();
	unlock |= UNLOCK_MASK_C2;
	mbp_state_c2_unlock_set(true);
	mbp_state_unlock_set(unlock);
	mbp_state_save();

	mbp_ui_popup("Unlock", "Zapp avatar unlocked by AND!XOR");
}

static void __c2() {
	master_c2_t c2;

//Random sequence number
	uint32_t m = util_millis();
	c2.seq = (m >> 16) & 0xFFFF;

//Select a command
	int8_t cmd = __menu_pick_c2_cmd();
	if (cmd == -1) {
		return;
	}
	c2.cmd = cmd;

//Secondary menu for play mode
	if (c2.cmd == MASTER_C2_CMD_PLAY) {
		int8_t animation = __menu_pick_c2_play_animation();
		//Did they quit?
		if (animation == -1) {
			return;
		}
		c2.data = animation;
	}
	//Generate random data for all others
	else {
		c2.data = util_math_rand8();
	}

	util_ble_c2_set(&c2);
	m_c2 = c2;

	mbp_ui_popup("C2", "C2 Mode Launched, allow this to run for awhile to have greatest effect");
}

bool mbp_master_c2_infected() {
	return m_c2.seq > 0 && m_c2.cmd != MASTER_C2_CMD_STOP;
}

void mbp_master_c2_process(master_c2_t c2) {
	//Only process if sequence is later
	if (c2.seq <= m_c2.seq) {
		return;
	}

	bool valid = true;

	switch (c2.cmd) {
	case MASTER_C2_CMD_LEVEL:
		if (!mbp_state_c2_level_get()) {
			app_sched_event_put(NULL, 0, __c2_level_schedule_handler);
		}
		break;
	case MASTER_C2_CMD_PLAY:
		;
		uint8_t modes[] = C2_PLAY_MODES;
		for (uint8_t i = 0; i < C2_PLAY_COUNT; i++) {
			if (modes[i] == c2.data) {
				app_sched_event_put(&c2.data, sizeof(uint8_t), __c2_play_schedule_handler);
			}
		}
		break;
	case MASTER_C2_CMD_POINTS:
		if (!mbp_state_c2_points_get()) {
			app_sched_event_put(NULL, 0, __c2_points_schedule_handler);
		}
		break;

	case MASTER_C2_CMD_SERVICES:
		;
		botnet_state_t *p_state = mbp_state_botnet_state_get();
		for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
			p_state->services[i].enabled = true;
		}
		p_state->firewall_state = 0xFF;
		botnet_update_service_status();
		mbp_state_save();
		break;
	case MASTER_C2_CMD_STOP:
		util_gfx_draw_raw_file_stop();
		break;
	case MASTER_C2_CMD_UNLOCK:
		//Make sure this is the first time getting the C2 Unlock
		if (!mbp_state_c2_unlock_get()) {
			app_sched_event_put(NULL, 0, __c2_unlock_schedule_handler);
		}
		break;
	default:
		valid = false;
		break;
	}

	if (valid) {
		util_ble_c2_set(&c2);
		m_c2 = c2;
	}
}

void mbp_master_human() {
	ble_badge_db_t *p_db = util_ble_badge_db_get();
	menu_item_t items[p_db->badge_count];
	uint32_t actions[] = ACTION_DATA;

	for (uint16_t i = 0; i < p_db->badge_count; i++) {
		menu_item_t item;
		item.text = (char *) malloc(SETTING_NAME_LENGTH);
		item.callback = NULL;
		sprintf(item.text, "%s[%d]", p_db->badges[i].name, p_db->badges[i].level);
		items[i] = item;
	}

	menu_t menu;
	menu.items = items;
	menu.count = p_db->badge_count;
	menu.selected = 0;
	menu.title = "Humans";
	menu.top = 0;
	mbp_submenu(&menu);

	int8_t action = __menu_pick_human_action();
	if (action < 0) {
		return;
	}

	ble_badge_t badge = p_db->badges[menu.selected];

	mbp_ui_cls();
	util_gfx_cursor_area_reset();
	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_color(COLOR_WHITE);
	util_gfx_set_cursor(0, 0);

	char message[32];
	sprintf(message, "Connecting to %s...\n", badge.name);
	util_gfx_print(message);

	bool success = mbp_master_ble_send_data(&badge, actions[action]);

	if (success) {
		util_gfx_print("   Sending Data...\n");
		util_gfx_print("Done.\n");
	} else {
		util_gfx_print("Connection failed\n");
	}

	util_gfx_print("Any key to continue...");
	util_button_wait();
	util_button_clear();
}

void mbp_master_menu_cheats() {

	menu_t menu;
	menu_item_t items[10];
	menu.items = items;
	menu.count = 0;
	menu.selected = 0;
	menu.top = 0;
	menu.title = "Cheats";

	items[menu.count++] = (menu_item_t ) { "Level Up", NULL, NULL, &__cheat_level_up, NULL };
	items[menu.count++] = (menu_item_t ) { "Points", NULL, NULL, &__cheat_points, NULL };
	items[menu.count++] = (menu_item_t ) { "0 Days", NULL, NULL, &__cheat_0days, NULL };
	items[menu.count++] = (menu_item_t ) { "Unlock All", NULL, NULL, &__cheat_unlock_all, NULL };

	mbp_submenu(&menu);
}

void mbp_master_menu_main() {
	menu_t menu;
	menu_item_t items[10];
	menu.items = items;
	menu.count = 0;
	menu.title = "Master";

	items[menu.count++] = (menu_item_t ) { "C2", NULL, NULL, &__c2, NULL };
	items[menu.count++] = (menu_item_t ) { "Humans", NULL, NULL, &mbp_master_human, NULL };
	items[menu.count++] = (menu_item_t ) { "Cheats", NULL, NULL, &mbp_master_menu_cheats, NULL };

	mbp_submenu(&menu);
}
#endif
