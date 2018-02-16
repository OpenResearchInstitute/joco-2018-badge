/*****************************************************************************
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
 * 	@sconklin
 * 	@mustbeart
 * 	@abraxas3d
 *****************************************************************************/
#include "system.h"

#define SCORE_SVC_UUID			0xbd7e
#define	SCORE_CHAR_SCORE_UUID	0x2e15	// read the badge's score and LLD
#define	SCORE_CHAR_CMD_UUID		0x553f	// write command codes to the badge
#define	SCORE_CHAR_LLDINCR_UUID	0xa4b4	// read command to increment LLD

// Secret read validation code
#define	SCORE_READ_COOKIE	0x8cd1e5a6

// Secret command codes
#define	SCORE_CMD_SET_LLD	0x6e46d5fad7441e00	// LSbyte filled with value
#define SCORE_CMD_SET_SCORE	0xfb9a251e1e5e0000	// LSword filled with value


typedef struct {
		util_crypto_cryptable_t	cryptable;
		uint64_t				code;
} __attribute__ ((packed)) score_ble_command_code_t;
static score_ble_command_code_t m_command_code;

typedef struct {
	util_crypto_cryptable_t	cryptable;
	uint32_t	cookie;
	uint16_t	device_id;
	uint16_t	score;
	uint8_t		lld;
} __attribute__ ((packed)) score_ble_readscore_t;

static score_ble_readscore_t m_score_ble_readscore;

static uint16_t m_score_service_handle;
static volatile uint16_t m_s_conn_handle = BLE_CONN_HANDLE_INVALID;
static volatile uint16_t m_s_cmd_handle = BLE_CONN_HANDLE_INVALID;
static volatile uint16_t m_s_lldi_handle = BLE_CONN_HANDLE_INVALID;

void score_ble_score_update(void) {
	uint32_t err_code;

	m_score_ble_readscore.cryptable.data_len = UTIL_CRYPTO_STORAGE_LEN(m_score_ble_readscore);
	m_score_ble_readscore.cookie = SCORE_READ_COOKIE;
	m_score_ble_readscore.device_id = util_get_device_id();
	m_score_ble_readscore.score = mbp_state_score_get();
	m_score_ble_readscore.lld = mbp_state_lastlevel_get();

	err_code = util_crypto_encrypt(&(m_score_ble_readscore.cryptable));
	if (NRF_SUCCESS != err_code) {
		mbp_ui_error("Could not encrypt score");
	}
}

void score_increment_lld(void) {
	uint8_t lld = mbp_state_lastlevel_get();

	mbp_state_lastlevel_set(lld+1);
}

void score_eval_command(void) {
	uint32_t err_code;

	err_code = util_crypto_decrypt(&(m_command_code.cryptable));
	if (NRF_SUCCESS != err_code) {
		return;
	}

	if ((m_command_code.code & 0xFFFFFFFFFFFFFF00) == SCORE_CMD_SET_LLD) {
		mbp_state_lastlevel_set((uint8_t)(m_command_code.code & 0xFF));
		mbp_state_save();
	} else if ((m_command_code.code & 0xFFFFFFFFFFFF0000) == SCORE_CMD_SET_SCORE) {
		mbp_state_score_set((uint16_t)(m_command_code.code & 0x7FFF));
		mbp_state_save();
	}
}


/**
 * Initialize the cmd characteristic.
 * This is where the station writes commands to the badge.
 */
static uint32_t __init_char_cmd() {
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

	BLE_UUID_BLE_ASSIGN(ble_uuid, SCORE_CHAR_CMD_UUID);

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
	attr_char_value.max_len = UTIL_CRYPTO_STORAGE_LEN(m_command_code);
	attr_char_value.p_value = (uint8_t *) &UTIL_CRYPTO_STORAGE(m_command_code);

	uint32_t result = sd_ble_gatts_characteristic_add(m_score_service_handle, &char_md, &attr_char_value, &char_handles);
	m_s_cmd_handle = char_handles.value_handle;
	return result;
}


/**
 * Initialize the score characteristic
 * This reads an encrypted packet containing the badge's score and lld.
 */
static uint32_t __init_char_score() {
	ble_gatts_char_md_t char_md;
	ble_gatts_attr_t attr_char_value;
	ble_uuid_t ble_uuid;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_char_handles_t char_handles;

	memset(&char_md, 0, sizeof(char_md));

	//Badge score characteristic
	char_md.char_props.write = 0;			//Writing not allowed
	char_md.char_props.write_wo_resp = 0;	//Writing not allowed
	char_md.char_props.read = 1;	//Read the data
	char_md.p_char_user_desc = NULL;
	char_md.p_char_pf = NULL;
	char_md.p_user_desc_md = NULL;
	char_md.p_cccd_md = NULL;
	char_md.p_sccd_md = NULL;

	BLE_UUID_BLE_ASSIGN(ble_uuid, SCORE_CHAR_SCORE_UUID);

	memset(&attr_md, 0, sizeof(attr_md));

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
	attr_md.vloc = BLE_GATTS_VLOC_USER;
	attr_md.rd_auth = 0;
	attr_md.wr_auth = 0;
	attr_md.vlen = 0;

	memset(&attr_char_value, 0, sizeof(attr_char_value));

	attr_char_value.p_uuid = &ble_uuid;
	attr_char_value.p_attr_md = &attr_md;
	attr_char_value.init_len = UTIL_CRYPTO_STORAGE_LEN(m_score_ble_readscore);
	attr_char_value.init_offs = 0;
	attr_char_value.max_len = UTIL_CRYPTO_STORAGE_LEN(m_score_ble_readscore);
	attr_char_value.p_value = (uint8_t *) &UTIL_CRYPTO_STORAGE(m_score_ble_readscore);

	uint32_t result = sd_ble_gatts_characteristic_add(m_score_service_handle, &char_md, &attr_char_value, &char_handles);
	// m_s_score_handle = char_handles.value_handle;
	return result;
}


/**
 * Initialize the magic increment-lld read characteristic
 * This reads a dummy value to trigger an increment of LLD.
 */
static uint32_t __init_char_lldincrement(void) {
	ble_gatts_char_md_t char_md;
	ble_gatts_attr_t attr_char_value;
	ble_uuid_t ble_uuid;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_char_handles_t char_handles;
	static uint16_t device_id;

	memset(&char_md, 0, sizeof(char_md));

	//Badge score characteristic
	char_md.char_props.write = 0;			//Writing not allowed
	char_md.char_props.write_wo_resp = 0;	//Writing not allowed
	char_md.char_props.read = 1;	//Read the data
	char_md.p_char_user_desc = NULL;
	char_md.p_char_pf = NULL;
	char_md.p_user_desc_md = NULL;
	char_md.p_cccd_md = NULL;
	char_md.p_sccd_md = NULL;

	BLE_UUID_BLE_ASSIGN(ble_uuid, SCORE_CHAR_LLDINCR_UUID);

	memset(&attr_md, 0, sizeof(attr_md));

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
	attr_md.vloc = BLE_GATTS_VLOC_STACK;
	attr_md.rd_auth = 1;	// Require authorization so we find out it was read
	attr_md.wr_auth = 0;
	attr_md.vlen = 0;

	memset(&attr_char_value, 0, sizeof(attr_char_value));

	attr_char_value.p_uuid = &ble_uuid;
	attr_char_value.p_attr_md = &attr_md;
	attr_char_value.init_len = 2;
	attr_char_value.init_offs = 0;
	attr_char_value.max_len = 2;
	attr_char_value.p_value = (uint8_t *) &device_id;

	uint32_t result = sd_ble_gatts_characteristic_add(m_score_service_handle, &char_md, &attr_char_value, &char_handles);
	m_s_lldi_handle = char_handles.value_handle;
	return result;
}


/**
 * Initialization for Score services under BLE
 */
uint32_t score_ble_init(void) {
	uint32_t err_code;
	ble_uuid_t ble_uuid;

	// Set up initial values for the score read characteristic
	score_ble_score_update();

	// Add service
	BLE_UUID_BLE_ASSIGN(ble_uuid, SCORE_SVC_UUID);
	err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &m_score_service_handle);
	if (err_code != NRF_SUCCESS) {
		return err_code;
	}

	//Register with discovery db so we can talk to GATT
	APP_ERROR_CHECK(ble_db_discovery_evt_register(&ble_uuid));

	__init_char_score();
	__init_char_cmd();
	__init_char_lldincrement();

	return err_code;
}

void score_ble_on_ble_evt(const ble_evt_t * p_ble_evt) {
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
		break;
	case BLE_GAP_EVT_TIMEOUT:
		m_s_conn_handle = BLE_CONN_HANDLE_INVALID;
		break;

	case BLE_GATTC_EVT_HVX:
		break;

	case BLE_GATTC_EVT_WRITE_RSP:
		break;

	case BLE_GATTC_EVT_READ_RSP:
		break;

	case BLE_GATTS_EVT_WRITE:
		if (p_ble_evt->evt.gatts_evt.params.write.handle == m_s_cmd_handle) {
			m_command_code.cryptable.data_len = p_ble_evt->evt.gatts_evt.params.write.len;
			score_eval_command();
		}
		break;

	case BLE_EVT_USER_MEM_REQUEST:
		break;

	case BLE_EVT_USER_MEM_RELEASE:
		break;

	case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
		{
		ble_gatts_rw_authorize_reply_params_t auth_reply;
		uint16_t err_code;

		auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
		auth_reply.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
		err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
												   &auth_reply);
		APP_ERROR_CHECK(err_code);

		score_increment_lld();		// the read triggers the increment
		mbp_state_save();
		}
		break;

	default:
		break;
	}
}
