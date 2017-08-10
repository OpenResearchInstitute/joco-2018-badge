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
//#define BOTNET_UUID {0x37,0x13,0x19,0xDC,0xAD,0xDE,0xEF,0xBE,0x9E,0x04,0x37,0x13,0xc5,0x3e,0xbb,0x3d}
#define BOTNET_UUID 			0x1337
#define BOTNET_CHAR_SCAN_UUID	0x2C04
#define BOTNET_CHAR_ATTACK_UUID	0xBB38

typedef struct {
	botnet_attack_t attack;
	bool response;
} botnet_ble_attack_packet_t;

//TODO: Get these onto the stack!
static uint16_t m_botnet_service_handle;
static volatile uint16_t m_c_scan_handle;
static volatile uint8_t m_c_scan_result;
static volatile uint16_t m_s_conn_handle = BLE_CONN_HANDLE_INVALID;
static volatile uint16_t m_c_conn_handle = BLE_CONN_HANDLE_INVALID;
static volatile uint16_t m_c_attack_handle;
static volatile uint16_t m_s_attack_handle;
static volatile uint16_t m_s_service_status_handle;
static volatile bool m_connecting = false;
static volatile bool m_waiting = false;
static botnet_ble_attack_packet_t m_attack_packet;

/**
 * Initialize the attack characteristic.
 * This is where attacking badges write their attack
 */
static uint32_t __init_char_attack() {
	ble_gatts_char_md_t char_md;
	ble_gatts_attr_t attr_char_value;
	ble_uuid_t ble_uuid;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_char_handles_t char_handles;

	memset(&char_md, 0, sizeof(char_md));

	//Badge attack characteristic
	char_md.char_props.write = 1;			//Writing allowed
	char_md.char_props.write_wo_resp = 0;	//Writing w/o response not allowed
	char_md.char_props.read = 1;			//Reading not allowed
	char_md.p_char_user_desc = NULL;
	char_md.p_char_pf = NULL;
	char_md.p_user_desc_md = NULL;
	char_md.p_cccd_md = NULL;
	char_md.p_sccd_md = NULL;

	BLE_UUID_BLE_ASSIGN(ble_uuid, BOTNET_CHAR_ATTACK_UUID);

	memset(&attr_md, 0, sizeof(attr_md));

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
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
	attr_char_value.max_len = sizeof(botnet_ble_attack_packet_t);
	attr_char_value.p_value = (uint8_t *) &m_attack_packet;

	uint32_t result = sd_ble_gatts_characteristic_add(m_botnet_service_handle, &char_md, &attr_char_value, &char_handles);
	m_s_attack_handle = char_handles.value_handle;
	return result;
}

/**
 * Initialize the port scan characteristic
 * This is simply a read of what ports are available
 */
static uint32_t __init_char_scan() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();

	ble_gatts_char_md_t char_md;
	ble_gatts_attr_t attr_char_value;
	ble_uuid_t ble_uuid;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_char_handles_t char_handles;

	memset(&char_md, 0, sizeof(char_md));

	//Badge port scanning characteristic
	char_md.char_props.write_wo_resp = 0;	//Writing not allowed
	char_md.char_props.read = 1;	//Read the data
	char_md.p_char_user_desc = NULL;
	char_md.p_char_pf = NULL;
	char_md.p_user_desc_md = NULL;
	char_md.p_cccd_md = NULL;
	char_md.p_sccd_md = NULL;

	BLE_UUID_BLE_ASSIGN(ble_uuid, BOTNET_CHAR_SCAN_UUID);

	memset(&attr_md, 0, sizeof(attr_md));

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
	attr_md.vloc = BLE_GATTS_VLOC_STACK;
	attr_md.rd_auth = 0;
	attr_md.wr_auth = 0;
	attr_md.vlen = 0;

	memset(&attr_char_value, 0, sizeof(attr_char_value));

	attr_char_value.p_uuid = &ble_uuid;
	attr_char_value.p_attr_md = &attr_md;
	attr_char_value.init_len = 0;
	attr_char_value.init_offs = 0;
	attr_char_value.max_len = sizeof(uint8_t);
	attr_char_value.p_value = &(p_state->service_state);

	uint32_t result = sd_ble_gatts_characteristic_add(m_botnet_service_handle, &char_md, &attr_char_value, &char_handles);
	m_s_service_status_handle = char_handles.value_handle;
	return result;
}

static void __on_read_response(const ble_evt_t *p_ble_evt) {
	const ble_gattc_evt_read_rsp_t * p_response;

	// Check if the event if on the link for this instance
	if (m_c_conn_handle != p_ble_evt->evt.gattc_evt.conn_handle) {
		return;
	}

	p_response = &p_ble_evt->evt.gattc_evt.params.read_rsp;

	//Check the handle of the response and pull the data out
	if (p_response->handle == m_c_scan_handle) {
		m_c_scan_result = p_response->data[0];
		m_waiting = false;
	} else if (p_response->handle == m_c_attack_handle) {
		memcpy(&m_attack_packet, p_response->data, sizeof(botnet_ble_attack_packet_t));
		m_waiting = false;
	}
}

void botnet_ble_attack(ble_badge_t *p_badge, botnet_attack_t *p_attack) {
	m_waiting = true;

	//Build the attack packet
	botnet_ble_attack_packet_t packet;
	packet.response = false;
	memcpy(&packet.attack, p_attack, sizeof(botnet_attack_t));

	ble_gattc_write_params_t write_params;
	write_params.offset = 0;
	write_params.len = sizeof(botnet_ble_attack_packet_t);
	write_params.p_value = (uint8_t *) &packet;
	write_params.handle = m_c_attack_handle;
	write_params.write_op = BLE_GATT_OP_WRITE_REQ;

	//Send the attack
	//TODO: Returning error code 0x7
	APP_ERROR_CHECK(sd_ble_gattc_write(m_c_conn_handle, &write_params));

	nrf_delay_ms(500);

	//Fire off a read request
	APP_ERROR_CHECK(sd_ble_gattc_read(m_c_conn_handle, m_c_attack_handle, 0));

	uint32_t end_time = util_millis() + 5000;
	while (m_waiting) {
		APP_ERROR_CHECK(sd_app_evt_wait());
		if (util_millis() > end_time) {
			m_waiting = false;
		}
	}

	memcpy(p_attack, &m_attack_packet.attack, sizeof(botnet_attack_t));
}

bool botnet_ble_connect_blocking(ble_badge_t *p_badge) {
	m_connecting = true;
	m_c_conn_handle = BLE_CONN_HANDLE_INVALID;
	util_ble_connect(&(p_badge->address));
	while (m_connecting) {
		APP_ERROR_CHECK(sd_app_evt_wait());
	}

	return m_c_conn_handle != BLE_CONN_HANDLE_INVALID;
}

/**
 * Handler for botnet ble events
 */
uint32_t botnet_ble_init() {
	uint32_t err_code;
	ble_uuid_t ble_uuid;

	// Add service
	BLE_UUID_BLE_ASSIGN(ble_uuid, BOTNET_UUID);
	err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &m_botnet_service_handle);
	if (err_code != NRF_SUCCESS) {
		return err_code;
	}

	//Register with discovery db so we can talk to GATT on other badges
	APP_ERROR_CHECK(ble_db_discovery_evt_register(&ble_uuid));

	__init_char_scan();
	__init_char_attack();

	return err_code;
}

void botnet_ble_on_ble_evt(const ble_evt_t * p_ble_evt) {
	if (p_ble_evt == NULL) {
		return;
	}

	switch (p_ble_evt->header.evt_id) {
	//Connected to as a service
	case BLE_GAP_EVT_CONNECTED:
		m_s_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
		break;
	case BLE_GAP_EVT_DISCONNECTED:
		m_s_conn_handle = BLE_CONN_HANDLE_INVALID;
		m_c_conn_handle = BLE_CONN_HANDLE_INVALID;
		break;
	case BLE_GAP_EVT_TIMEOUT:
		m_connecting = false;
		m_s_conn_handle = BLE_CONN_HANDLE_INVALID;
		m_c_conn_handle = BLE_CONN_HANDLE_INVALID;
		break;

	case BLE_GATTC_EVT_HVX:
		break;

	case BLE_GATTC_EVT_WRITE_RSP:
		break;

	case BLE_GATTC_EVT_READ_RSP:
		__on_read_response(p_ble_evt);
		break;

#ifndef BLE_ONLY_BUILD
		case BLE_GATTS_EVT_WRITE:
		if (p_ble_evt->evt.gatts_evt.params.write.handle == m_s_attack_handle) {
			botnet_eval_incoming_attacking(&(m_attack_packet.attack));
			m_attack_packet.response = true;
		}
		break;
#endif

	case BLE_EVT_USER_MEM_REQUEST:
		break;

	case BLE_EVT_USER_MEM_RELEASE:
		break;

	default:
		break;
	}
}

/**
 * Botnet game BLE discovery event handler
 */
void botnet_ble_on_db_disc_evt(const ble_db_discovery_evt_t * p_evt) {
	// Check if the botnet game Service was discovered.
	if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE
			&& p_evt->params.discovered_db.srv_uuid.uuid == BOTNET_UUID
			&& p_evt->params.discovered_db.srv_uuid.type == BLE_UUID_TYPE_BLE) {

		m_c_conn_handle = p_evt->conn_handle;

		for (uint8_t i = 0; i < p_evt->params.discovered_db.char_count; i++) {

			if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid == BOTNET_CHAR_SCAN_UUID) {
				// Found scan characteristic
				m_c_scan_handle = p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
			} else if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid == BOTNET_CHAR_ATTACK_UUID) {
				// Found attack characteristic
				m_c_attack_handle = p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
			}
		}

		APP_ERROR_CHECK(pm_conn_secure(m_c_conn_handle, false));

		//all done
		m_connecting = false;
	}
}

uint8_t botnet_ble_scan_get(ble_badge_t *p_badge) {
	m_waiting = true;

	//Fire off a read request
	sd_ble_gattc_read(m_c_conn_handle, m_c_scan_handle, 0);

	uint32_t end_time = util_millis() + 5000;
	while (m_waiting) {
		APP_ERROR_CHECK(sd_app_evt_wait());
		if (util_millis() > end_time) {
			m_waiting = false;
		}
	}

	return m_c_scan_result;
}

void botnet_ble_update_service_status() {
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	ble_gatts_value_t value;
	value.len = sizeof(uint8_t);
	value.offset = 0;
	value.p_value = &(p_state->service_state);
	sd_ble_gatts_value_set(m_s_conn_handle, m_s_service_status_handle, &value);
}
#endif
