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

#define CANARY				(0xba)
#define STATE_FILE_PATH		"SHADOW.DAT"

//Badge State, global storage.
static badge_state_t m_badge_state;


void mbp_state_new() {
	memcpy(&(m_badge_state.name), SETTING_NAME_DEFAULT, SETTING_NAME_LENGTH);
	util_ble_name_set(m_badge_state.name);

	m_badge_state.airplane_mode_enabled = SETTING_AIRPLANE_MODE_DEFAULT;
	m_badge_state.tilt_enabled = SETTING_TILT_ENABLED_DEFAULT;
	m_badge_state.canary = CANARY;
	m_badge_state.special = 0;
	m_badge_state.game_exit_pop_up = SETTING_GAME_EXIT_POPUP_DEFAULT;
	m_badge_state.game_led_sound = SETTING_GAME_LED_SOUND_DEFAULT;
	m_badge_state.chip8_fg_color = SETTING_CHIP8_FG_COLOR_DEFAULT;
	m_badge_state.chip8_bg_color = SETTING_CHIP8_BG_COLOR_DEFAULT;
	m_badge_state.unlock_state = SETTING_UNLOCK_DEFAULT;
	m_badge_state.master_badge = SETTING_MASTER_DEFAULT;
	m_badge_state.joco_score = GAME_SCORE_DEFAULT;
	m_badge_state.joco_last_level_dispensed = GAME_LASTLEVEL_DEFAULT;

	strcpy(m_badge_state.pw_glados, "aperture");
	strcpy(m_badge_state.pw_root,   "diligent");

	strcpy(m_badge_state.wall_messages[0], "Msg 1 None");
	strcpy(m_badge_state.wall_messages[1], "Msg 2 None");
	strcpy(m_badge_state.wall_messages[2], "Msg 3 None");
	strcpy(m_badge_state.wall_messages[3], "Msg 4 None");
	strcpy(m_badge_state.wall_messages[4], "Msg 5 None");
	m_badge_state.wall_current_spot = 0;
}

bool mbp_state_load() {
	uint32_t err_code;
	FIL file;
	FILINFO info;
	FRESULT result;
	UINT expected_count = UTIL_CRYPTO_STORAGE_LEN(m_badge_state);
	UINT count;

	//check if file exists, if not create it
	if (f_stat(STATE_FILE_PATH, &info) != FR_OK) {
		mbp_state_new();
		mbp_state_save();
	}

	//Open settings file
	result = f_open(&file, STATE_FILE_PATH, FA_READ | FA_OPEN_EXISTING);
	if (result != FR_OK) {
		return false;
	}

	result = f_read(&file, (void *) &UTIL_CRYPTO_STORAGE(m_badge_state),
					expected_count, &count);
	if (result != FR_OK || count != expected_count) {
		mbp_ui_error("Could not read settings file.");
		return false;
	}

	result = f_close(&file);
	if (result != FR_OK) {
		mbp_ui_error("Could not close settings file.");
		return false;
	}

	m_badge_state.cryptable.data_len = expected_count;
	err_code = util_crypto_decrypt(&(m_badge_state.cryptable));
	APP_ERROR_CHECK(err_code);

	if (err_code != NRF_SUCCESS) {
		mbp_ui_error("Could not decrypt settings file.");
		return false;
	}

	if ((m_badge_state.canary == CANARY)) {
		util_ble_name_set(m_badge_state.name);
		util_ble_score_update();
		score_ble_score_update();
		return true;
	}

	return false;
}

static void __save_schedule_handler(void *p_data, uint16_t length) {
	uint32_t err_code;
	FIL file;
	FRESULT result;
	UINT count;
	badge_state_t encrypted_badge_state = m_badge_state;

	//Let util_crypto know the length of our data
	encrypted_badge_state.cryptable.data_len = sizeof(badge_state_t)
						- sizeof(encrypted_badge_state.cryptable.data_len);

	//Encrypt the data
	err_code = util_crypto_encrypt(&(encrypted_badge_state.cryptable));
	APP_ERROR_CHECK(err_code);

	//Write the data to SD
	result = f_open(&file, STATE_FILE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
	if (result != FR_OK) {
		mbp_ui_error("Could not open settings for writing.");
		return;
	}

	result = f_write(&file, (void *) &(encrypted_badge_state.cryptable.cryptinfo),
					encrypted_badge_state.cryptable.data_len, &count);
	if (result != FR_OK || count != encrypted_badge_state.cryptable.data_len) {
		mbp_ui_error("Could not write to settings file.");
	}

	result = f_close(&file);
	if (result != FR_OK) {
		mbp_ui_error("Could not close settings file.");
	}
}

void mbp_state_save() {
	app_sched_event_put(NULL, 0, __save_schedule_handler);
}

bool mbp_state_airplane_mode_get() {
	return m_badge_state.airplane_mode_enabled;
}

void mbp_state_airplane_mode_set(bool enabled) {
	m_badge_state.airplane_mode_enabled = enabled;
}

void mbp_state_name_get(char *name) {
	snprintf(name, SETTING_NAME_LENGTH, "%s", m_badge_state.name);
}

void mbp_state_name_set(char *name) {
	snprintf(m_badge_state.name, SETTING_NAME_LENGTH, "%s", name);
	util_ble_name_set(name);
	util_nfc_reload_payload();
}

bool mbp_state_game_led_sound_get() {
	return m_badge_state.game_led_sound;
}

void mbp_state_game_led_sound_set(bool b) {
	m_badge_state.game_led_sound = b;
}

bool mbp_state_game_exit_pop_up_get() {
	//gets the current state of the CHIP8 "How to Exit" pop up
	return m_badge_state.game_exit_pop_up;
}

void mbp_state_game_exit_pop_up_set(bool b) {
	//sets the current state of the CHIP8 "How to Exit" pop up
	m_badge_state.game_exit_pop_up = b;
}

uint16_t mbp_state_chip8_fg_color_get() {
	//gets the current state of the CHIP8 foreground color
	return m_badge_state.chip8_fg_color;
}

void mbp_state_chip8_fg_color_set(uint16_t c) {
	//sets the current state of the CHIP8 foreground color
	m_badge_state.chip8_fg_color = c;
}

uint16_t mbp_state_chip8_bg_color_get() {
	//gets the current state of the CHIP8 background color
	return m_badge_state.chip8_bg_color;
}

void mbp_state_chip8_bg_color_set(uint16_t c) {
	//sets the current state of the CHIP8 background color
	m_badge_state.chip8_bg_color = c;
}

bool mbp_state_master_get() {
	return m_badge_state.master_badge;
}

void mbp_state_master_set(bool master) {
	m_badge_state.master_badge = master;
}

bool mbp_state_tilt_get() {
	return m_badge_state.tilt_enabled;
}

void mbp_state_tilt_set(bool tilt_state) {
	m_badge_state.tilt_enabled = tilt_state;
}

uint16_t mbp_state_unlock_get() {
	return m_badge_state.unlock_state;
}

void mbp_state_unlock_set(uint16_t unlock_state) {
	m_badge_state.unlock_state = unlock_state;
}

uint16_t mbp_state_score_get() {
	return m_badge_state.joco_score;
}

void mbp_state_score_set(uint16_t score_state) {
	m_badge_state.joco_score = score_state;
	util_ble_score_update();
	score_ble_score_update();
}

uint8_t mbp_state_lastlevel_get() {
	return m_badge_state.joco_last_level_dispensed;
}

void mbp_state_lastlevel_set(uint8_t lastlevel_state) {
	m_badge_state.joco_last_level_dispensed = lastlevel_state;
	util_ble_score_update();
	score_ble_score_update();
}

uint8_t mbp_state_special_get() {
	return m_badge_state.special;
}

void mbp_state_special_set(uint8_t special) {
	m_badge_state.special = special;
	util_ble_name_set(m_badge_state.name);
}

void mbp_state_pw_glados_set(char *pw) {
	snprintf(m_badge_state.pw_glados, SETTING_PW_LENGTH, "%s", pw);
}

void mbp_state_pw_glados_get(char *pw) {
	snprintf(pw, SETTING_PW_LENGTH, "%s", m_badge_state.pw_glados);
}

void mbp_state_pw_root_set(char *pw) {
	snprintf(m_badge_state.pw_root, SETTING_PW_LENGTH, "%s", pw);
}

void mbp_state_pw_root_get(char *pw) {
	snprintf(pw, SETTING_PW_LENGTH, "%s", m_badge_state.pw_root);
}

void mbp_state_wall_show(){
	/*todo v0.8 Polish
	char temp[16];
	for (int i = 0; i< 5; i++){
		sprintf(temp, "MSG%i ", i);
		strcat(temp, m_badge_state.wall_messages[i]);
		mbp_term_print(temp);
	}
	*/
	mbp_term_print(m_badge_state.wall_messages[0]);
	mbp_term_print(m_badge_state.wall_messages[1]);
	mbp_term_print(m_badge_state.wall_messages[2]);
	mbp_term_print(m_badge_state.wall_messages[3]);
	mbp_term_print(m_badge_state.wall_messages[4]);
	mbp_term_print("\r");
}

extern void mbp_state_wall_put(char *msg){
	snprintf(m_badge_state.wall_messages[m_badge_state.wall_current_spot], 16, "%s", msg);

	if(m_badge_state.wall_current_spot < 4){
		m_badge_state.wall_current_spot++;
	}
	else{
		m_badge_state.wall_current_spot = 0;
	}

	mbp_state_save();
}

// Functions for saving and reading a simple 'database' of other badges we've visited.
