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

#define TYPE_BEACON				0x06
#define TYPE_OFFSET				2
#define DATA_LENGTH				31
#define UUID_OFFSET				9
#define UUID_LENGTH				16
#define BEACON_UUID_CPV			{0x04, 0xbb, 0xc1, 0x81, 0x13, 0xa6, 0x4f, 0x6a, 0x85, 0x1b, 0x94, 0xd3, 0x0c, 0x87, 0x49, 0xef}
#define BEACON_UUID_WH			{0x8f, 0xfd, 0xa0, 0x67, 0x0b, 0xb7, 0x41, 0x3f, 0xba, 0xcb, 0x91, 0xab, 0x7b, 0xa4, 0x07, 0x6f}
/*
static void __cpv_unlock(void * p_event_data, uint16_t event_size) {
	app_sched_pause();

	uint16_t unlock = mbp_state_unlock_get();
	unlock |= UNLOCK_MASK_CPV;
	mbp_state_unlock_set(unlock);
	mbp_state_save();
	mbp_ui_popup("Unlock", "Welcome to CPV. New bling mode unlocked");

	app_sched_resume();
}

static void __wh_unlock(void * p_event_data, uint16_t event_size) {
	app_sched_pause();

	uint16_t unlock = mbp_state_unlock_get();
	unlock |= UNLOCK_MASK_WH;
	mbp_state_unlock_set(unlock);

	mbp_ui_popup("Unlock", "You found the White Hat. 5 levels and 500 botnet points gained.");

	app_sched_resume();
}
*/

/*
static void __cpv_test(ble_gap_evt_adv_report_t *p_report) {
	//Do not unlock twice
	uint16_t unlock = mbp_state_unlock_get();
	if ((unlock & UNLOCK_MASK_CPV) > 0) {
		return;
	}

	uint8_t cpv_uuid[UUID_LENGTH] = BEACON_UUID_CPV;

	//Test for CPV beacon
	bool beacon_found = true;
	for (uint8_t i = 0; i < UUID_LENGTH; i++) {
		if (cpv_uuid[i] != p_report->data[i + UUID_OFFSET]) {
			beacon_found = false;
			break;
		}
	}
	if (beacon_found) {
		app_sched_event_put(NULL, 0, __cpv_unlock);
	}
}

static void __wh_test(ble_gap_evt_adv_report_t *p_report) {
	//Do not unlock twice
	uint16_t unlock = mbp_state_unlock_get();
	if ((unlock & UNLOCK_MASK_WH) > 0) {
		return;
	}

	uint8_t wh_uuid[UUID_LENGTH] = BEACON_UUID_WH;

	//Test for White Hat beacon
	bool beacon_found = true;
	for (uint8_t i = 0; i < UUID_LENGTH; i++) {
		if (wh_uuid[i] != p_report->data[i + UUID_OFFSET]) {
			beacon_found = false;
			break;
		}
	}
	if (beacon_found) {
		app_sched_event_put(NULL, 0, __wh_unlock);
	}
}
*/
void beacon_ble_on_ble_advertisement(ble_gap_evt_adv_report_t *p_report) {
	//Ensure it's a beacon
	if (p_report->dlen != DATA_LENGTH) {
		return;
	}

	uint8_t type = p_report->data[TYPE_OFFSET];
	if (type != TYPE_BEACON) {
		return;
	}

//	__cpv_test(p_report);
//	__wh_test(p_report);
}
