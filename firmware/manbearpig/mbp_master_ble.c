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

#define MASTER_S_UUID 			0x3375
#define MASTER_CHAR_UUID		0x029d

static uint32_t m_master_data = 0;
static uint16_t m_master_service_handle = BLE_GATT_HANDLE_INVALID;
static uint16_t m_master_char_value_handle = BLE_GATT_HANDLE_INVALID;
static uint16_t m_master_conn_handle = BLE_CONN_HANDLE_INVALID;
static bool m_connecting = false;

/**
 * Initialize the attack characteristic.
 * This is where attacking badges write their attack
 */
static uint32_t __init_char() {
	ble_gatts_char_md_t char_md;
	ble_gatts_attr_t attr_char_value;
	ble_uuid_t ble_uuid;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_char_handles_t char_handles;

	memset(&char_md, 0, sizeof(char_md));

	//Badge attack characteristic
	char_md.char_props.write = 1;			//Writing allowed
	char_md.char_props.write_wo_resp = 0;	//Writing w/o response not allowed
	char_md.char_props.read = 0;			//Reading not allowed
	char_md.p_char_user_desc = NULL;
	char_md.p_char_pf = NULL;
	char_md.p_user_desc_md = NULL;
	char_md.p_cccd_md = NULL;
	char_md.p_sccd_md = NULL;

	BLE_UUID_BLE_ASSIGN(ble_uuid, MASTER_CHAR_UUID);

	memset(&attr_md, 0, sizeof(attr_md));

	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
	attr_md.vloc = BLE_GATTS_VLOC_USER;
	attr_md.rd_auth = 0;
	attr_md.wr_auth = 0;
	attr_md.vlen = 0;

	memset(&attr_char_value, 0, sizeof(attr_char_value));

	attr_char_value.p_uuid = &ble_uuid;
	attr_char_value.p_attr_md = &attr_md;
	attr_char_value.init_len = 0;
	attr_char_value.init_offs = 0;
	attr_char_value.max_len = sizeof(m_master_data);
	attr_char_value.p_value = (uint8_t *) &m_master_data;

	uint32_t result = sd_ble_gatts_characteristic_add(m_master_service_handle, &char_md, &attr_char_value, &char_handles);
	m_master_char_value_handle = char_handles.value_handle;
	return result;
}

static void __on_master_data_schedule_handler(void *p_data, uint16_t length) {
	app_sched_pause();
	char *message = (char *) p_data;
	mbp_ui_popup("Master", message);
	app_sched_resume();
}

static void __on_master_data(uint32_t data) {
	botnet_state_t *p_state = mbp_state_botnet_state_get();

	switch (data) {
	case MASTER_ACTION_FREE_0DAY:
		;
		botnet_exploit_t exploit;
		exploit.sophistication = BOTNET_EXPLOIT_SOPHISTICATION_MAX;
		exploit.target_index = util_math_rand8_max(BOTNET_SERVICE_COUNT);

		//Add new exploit to the end or overwrite the last one
		uint8_t i = MIN(p_state->exploit_count, BOTNET_MAX_EXPLOITS-1);
		p_state->exploits[i] = exploit;
		p_state->exploit_count = (p_state->exploit_count + 1) % BOTNET_MAX_EXPLOITS;

		//Save
		mbp_state_save();

		//Tell the user
		char *service_names[] = BOTNET_SERVICE_NAMES;
		char message[32];
		sprintf(message, "Received 0day for %s!", service_names[exploit.target_index]);
		app_sched_event_put(message, 32, __on_master_data_schedule_handler);
		break;

	case MASTER_ACTION_FREE_POINTS:
		;
		p_state->points += 200;
		if (p_state->points > BOTNET_POINTS_MAX) {
			p_state->points = BOTNET_POINTS_MAX;
		}
		mbp_state_save();

		app_sched_event_put("Received 200 points!", 32, __on_master_data_schedule_handler);
		break;

	case MASTER_ACTION_LEVEL_UP:
		if (p_state->level < UINT8_MAX) {
			p_state->level++;
			p_state->experience = 0;
		}
		mbp_state_save();
		app_sched_event_put("Leveled up by a master badge!", 32, __on_master_data_schedule_handler);
		break;

	case MASTER_ACTION_UNLOCK:
		mbp_state_unlock_set(mbp_state_unlock_get() | UNLOCK_MASK_MASTER);
		mbp_state_save();

		app_sched_event_put("Rager mode unlocked by a master badge!", 32, __on_master_data_schedule_handler);
		break;
	}
}

/**
 * Handler for master ble events
 */
void mbp_master_ble_on_ble_evt(const ble_evt_t * p_ble_evt) {
	if (p_ble_evt == NULL) {
		return;
	}

	switch (p_ble_evt->header.evt_id) {
	//Connected to as a service
	case BLE_GAP_EVT_CONNECTED:
		m_master_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
		break;
	case BLE_GAP_EVT_DISCONNECTED:
		m_master_conn_handle = BLE_CONN_HANDLE_INVALID;
		m_connecting = false;
		break;
	case BLE_GAP_EVT_TIMEOUT:
		m_master_conn_handle = BLE_CONN_HANDLE_INVALID;
		m_connecting = false;
		break;
	case BLE_GATTC_EVT_HVX:
		break;

	case BLE_GATTC_EVT_WRITE_RSP:
		break;

	case BLE_GATTC_EVT_READ_RSP:
		break;

	case BLE_GATTS_EVT_WRITE:
		//Incoming master data
		if (p_ble_evt->evt.gatts_evt.params.write.handle == m_master_char_value_handle) {
			__on_master_data(m_master_data);
		}
		break;

	case BLE_EVT_USER_MEM_REQUEST:
		break;

	case BLE_EVT_USER_MEM_RELEASE:
		break;

	default:
		break;
	}
}

uint32_t mbp_master_ble_init() {
	uint32_t err_code;
	ble_uuid_t ble_uuid;

	// Add service
	BLE_UUID_BLE_ASSIGN(ble_uuid, MASTER_S_UUID);
	err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &m_master_service_handle);
	if (err_code != NRF_SUCCESS) {
		return err_code;
	}

	//Register with discovery db so we can talk to GATT on other badges
	APP_ERROR_CHECK(ble_db_discovery_evt_register(&ble_uuid));

	__init_char();

	return err_code;
}

/**
 * Master BLE discovery event handler
 */
void mbp_master_ble_on_db_disc_evt(const ble_db_discovery_evt_t * p_evt) {
	// Check if the botnet game Service was discovered.
	if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE
			&& p_evt->params.discovered_db.srv_uuid.uuid == MASTER_S_UUID
			&& p_evt->params.discovered_db.srv_uuid.type == BLE_UUID_TYPE_BLE) {

		m_master_conn_handle = p_evt->conn_handle;

		for (uint8_t i = 0; i < p_evt->params.discovered_db.char_count; i++) {

			if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid == MASTER_CHAR_UUID) {
				// Found scan characteristic
				m_master_char_value_handle = p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
			}
		}

		//all done
		m_connecting = false;
	}
}

bool mbp_master_ble_send_data(ble_badge_t *p_badge, uint32_t data) {

	m_connecting = true;
	m_master_conn_handle = BLE_CONN_HANDLE_INVALID;
	util_ble_connect(&(p_badge->address));
	while (m_connecting) {
		APP_ERROR_CHECK(sd_app_evt_wait());
	}

	if (m_master_conn_handle == BLE_CONN_HANDLE_INVALID) {
		return false;
	}

	ble_gattc_write_params_t write_params;
	write_params.offset = 0;
	write_params.len = sizeof(m_master_data);
	write_params.p_value = (uint8_t *) &data;
	write_params.handle = m_master_char_value_handle;
	write_params.write_op = BLE_GATT_OP_WRITE_REQ;

	//Send the master data
	APP_ERROR_CHECK(sd_ble_gattc_write(m_master_conn_handle, &write_params));

	util_ble_disconnect();
	return true;
}
