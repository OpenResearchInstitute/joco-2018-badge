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
 *      @andnxor
 *      @zappbrandnxor
 *      @hyr0n1
 *      @andrewnriley
 *      @lacosteaef
 *      @bitstr3m
 * 
 * Further modifications made by
 *      @sconklin
 *      @mustbeart
 *
 *****************************************************************************/
#include "../system.h"

#define BLE_DATA_LEN                  10
#define BLE_DATA_INDEX_SCORE          2
#define BLE_DATA_INDEX_C2             4
#define BLE_DATA_INDEX_GLOBAL_TIME    8
#define BLE_TX_POWER                  0                 // 0dbm gain 
#define DEVICE_NAME                   "JOCO2018"
#define APP_ADV_INTERVAL              0x0320            // Advertising interval in units of 0.625ms
//#define APP_ADV_TIMEOUT_IN_SECONDS    180
#define APP_FEATURE_NOT_SUPPORTED     BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2 // Reply when unsupported features are requested.

#define GAP_TYPE_NAME                 0x09
#define GAP_TYPE_COMPANY_DATA         0xFF

//#define MIN_CONN_INTERVAL             MSEC_TO_UNITS(500, UNIT_1_25_MS)  // Minimum acceptable connection interval (0.5 seconds).
//#define MAX_CONN_INTERVAL             MSEC_TO_UNITS(1000, UNIT_1_25_MS) // Maximum acceptable connection interval (1 second).

#if (NRF_SD_BLE_API_VERSION == 3)
#define NRF_BLE_MAX_MTU_SIZE          GATT_MTU_SIZE_DEFAULT // MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event.
#endif

#define CENTRAL_LINK_COUNT            2 // Number of central links used by the application. When changing this number remember to adjust the RAM settings
#define PERIPHERAL_LINK_COUNT         1 // Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings

//Connection parameters
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, UTIL_TIMER_PRESCALER)  // Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds).
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, UTIL_TIMER_PRESCALER) // Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds).
#define MAX_CONN_PARAMS_UPDATE_COUNT  3 // Number of attempts before giving up the connection parameter negotiation.

//Scan settings
#define SCAN_INTERVAL                 0x0100   // Determines scan interval in units of 0.625 millisecond.
#define SCAN_WINDOW                   0x0060   // Determines scan window in units of 0.625 millisecond.
#define SLAVE_LATENCY                 0        // Slave latency.

//Security Settings
#define SEC_PARAM_BOND                1        // Perform bonding
#define SEC_PARAM_MITM                1        // Man in the middle protection, required for OOB
#define SEC_PARAM_LESC                0        // LE Secure Connections enabled.
#define SEC_PARAM_KEYPRESS            0        // Keypress notifications not enabled.
#define SEC_PARAM_IO_CAPABILITIES     BLE_GAP_IO_CAPS_NONE  // User isn't capable of entering a pin on the device, this will require OOB/MITM
#define SEC_PARAM_OOB                 1        // Out of band security data available (NFC pairing, PSK, etc)
#define SEC_PARAM_MIN_KEY_SIZE        7        // Minimum encryption key size.
#define SEC_PARAM_MAX_KEY_SIZE        16       // Maximum encryption key size.
#define SEC_OOB_AUTH_KEY     { \
        { \
            0xE4, 0xBB, 0xBA, 0x62, \
            0xF4, 0x5B, 0x25, 0x2C, \
            0x23, 0xF8, 0xAD, 0x03, \
            0xCA, 0xB4, 0xEE, 0x02 \
        } \
    }

//Connection settings
#define MIN_CONNECTION_INTERVAL       MSEC_TO_UNITS(7.5, UNIT_1_25_MS) // Determines minimum connection interval in millisecond.
#define MAX_CONNECTION_INTERVAL       MSEC_TO_UNITS(30, UNIT_1_25_MS)  // Determines maximum connection interval in millisecond.
#define SLAVE_LATENCY                 0                                // Determines slave latency in counts of connection events.
#define CONN_SUPERVISION_TIMEOUT      MSEC_TO_UNITS(1000, UNIT_10_MS)  // Determines supervision time-out in units of 10 millisecond.

static ble_advdata_t m_adv_data;                // Structure containing advertising parameters
static nrf_ble_gatt_t m_gatt;                   // Structure for gatt module
static ble_advdata_manuf_data_t m_manuf_data;   // Variable to hold manufacturer specific data
static ble_db_discovery_t m_ble_db_discovery;   // Structure used to identify the DB Discovery module.
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;
static ble_advdata_tk_value_t m_oob_auth_key = SEC_OOB_AUTH_KEY;
static ble_nus_t m_nus;                         // Structure to identify the Nordic UART Service.

//Connection parameters
ble_gap_conn_params_t m_conn_params = {
    (uint16_t) MIN_CONNECTION_INTERVAL,  // Minimum connection
    (uint16_t) MAX_CONNECTION_INTERVAL,  // Maximum connection
    (uint16_t) SLAVE_LATENCY,            // Slave latency.
    (uint16_t) CONN_SUPERVISION_TIMEOUT  // Supervision time-out
};

// Used for local badge information during processing
// A superset of active list fields
typedef struct {
    uint8_t address[BLE_GAP_ADDR_LEN]; // 6
    uint16_t company_id;               // 2
    char name[SETTING_NAME_LENGTH];    // 9
    bool said_hello;                   // 1
    uint16_t device_id;                // 2
    uint32_t first_seen;               // 4
    uint32_t last_seen;                // 4
    int8_t rssi;                       // 1
    uint8_t special;                   // 1
} ble_badge_t;

//Declare these early
static void __on_adv_evt(ble_adv_evt_t ble_adv_evt);
static void __on_ble_evt(ble_evt_t * p_ble_evt);
static void pm_evt_handler(pm_evt_t const * p_evt);

//Global time sync
uint16_t m_time = 0;
APP_TIMER_DEF(m_ble_global_time_timer);

#define BLE_GLOBAL_TIME_SECRET        0x8dc0
#define BLE_GLOBAL_TIME_UPDATE_MS     (1000 * 1) // Update the global time in the advertisement every second)

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void __advertising_init(void) {
    uint32_t err_code;
    uint16_t device_id = util_get_device_id();

    m_manuf_data.company_identifier = COMPANY_ID_JOCO;
    m_manuf_data.data.p_data = (uint8_t *) malloc(BLE_DATA_LEN);
    memset(m_manuf_data.data.p_data, 0, BLE_DATA_LEN);
    m_manuf_data.data.size = BLE_DATA_LEN;

    m_manuf_data.data.p_data[0] = device_id & 0xFF;
    m_manuf_data.data.p_data[1] = device_id >> 8;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&m_adv_data, 0, sizeof(m_adv_data));

    m_adv_data.name_type = BLE_ADVDATA_FULL_NAME;
    m_adv_data.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    m_adv_data.p_manuf_specific_data = &m_manuf_data;
    m_adv_data.include_appearance = true;

    ble_adv_modes_config_t options = { 0 };
    options.ble_adv_fast_enabled = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout = 0;

    err_code = ble_advertising_init(&m_adv_data, NULL, &options, __on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void __ble_evt_dispatch(ble_evt_t * p_ble_evt) {
    ble_conn_state_on_ble_evt(p_ble_evt);
    pm_on_ble_evt(p_ble_evt);
    ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    //mbp_master_ble_on_ble_evt(p_ble_evt);
    score_ble_on_ble_evt(p_ble_evt);
    mbp_medea_on_ble_evt(p_ble_evt);
    nrf_ble_gatt_on_ble_evt(&m_gatt, p_ble_evt);
    ble_nus_on_ble_evt(&m_nus, p_ble_evt);
    __on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
}

void __check_for_master_unlock(uint8_t special) {
    // Four 'master' badges trigger unlocking of bling when visited
    uint16_t unlock = mbp_state_unlock_get();

    if (!(unlock & UNLOCK_MASK_MASTER_1)) {
	if (special == MASTER_1_SPECIAL_ID) {
	    mbp_state_unlock_set(unlock | UNLOCK_MASK_MASTER_1);
	    mbp_state_save();
	}
    }

    if (!(unlock & UNLOCK_MASK_MASTER_2)) {
	if (special == MASTER_2_SPECIAL_ID) {
	    mbp_state_unlock_set(unlock | UNLOCK_MASK_MASTER_2);
	    mbp_state_save();
	}
    }

    if (!(unlock & UNLOCK_MASK_MASTER_3)) {
	if (special == MASTER_3_SPECIAL_ID) {
	    mbp_state_unlock_set(unlock | UNLOCK_MASK_MASTER_3);
	    mbp_state_save();
	}
    }

    if (!(unlock & UNLOCK_MASK_MASTER_4)) {
	if (special == MASTER_4_SPECIAL_ID) {
	    mbp_state_unlock_set(unlock | UNLOCK_MASK_MASTER_4);
	    mbp_state_save();
	}
    }
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt) {
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void __conn_params_init(void) {
    uint32_t err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail = false;
    cp_init.evt_handler = on_conn_params_evt;
    cp_init.error_handler = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

static void __db_discovery_handler(ble_db_discovery_evt_t * p_evt) {
    //mbp_master_ble_on_db_disc_evt(p_evt);
    mbp_medea_on_db_disc_evt(p_evt);
}

/**
 * @brief Database discovery collector initialization.
 */
static void __db_discovery_init(void) {
    uint32_t err_code = ble_db_discovery_init(__db_discovery_handler);
    APP_ERROR_CHECK(err_code);
}

/**
 * GATT Event handler
 */
void __gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t * p_evt) {
//do something here?
}

/**
 * GATT module init
 */
static void __gatt_init(void) {
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, __gatt_evt_handler);
    APP_ERROR_CHECK(err_code);
}

static void __global_time_advertisement_process(uint16_t t, uint16_t id) {
    t = t ^ id ^ BLE_GLOBAL_TIME_SECRET;

    uint32_t millis = (uint32_t) t << 14;

    //Jump forward
    if (millis > util_millis()) {
            util_millis_offset_set(millis - util_millis());
    }
}

static void __global_time_schedule_handler(void *p_data, uint16_t length) {
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    uint16_t id = util_get_device_id();
    uint16_t t = ((util_millis() >> 14) & 0xFFFF);
    t = t ^ BLE_GLOBAL_TIME_SECRET ^ id;
    m_adv_data.p_manuf_specific_data->data.p_data[BLE_DATA_INDEX_GLOBAL_TIME] = (t >> 8);
    m_adv_data.p_manuf_specific_data->data.p_data[BLE_DATA_INDEX_GLOBAL_TIME + 1] = t;

    ble_advdata_set(&m_adv_data, NULL);
}

/**
 * Update the global time being advertised
 */

static void __global_time_timer_handler(void *p_data) {
    //update the ble GAP data on main
    app_sched_event_put(NULL, 0, __global_time_schedule_handler);
}


/**
 * @brief Parses advertisement data, providing length and location of the field in case
 *        matching data is found.
 *
 * @param[in]  Advertisement report length and pointer to report.
 *
 * @retval NRF_SUCCESS if the data type is found in the report.
 * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
 */

static void __handle_advertisement(ble_gap_evt_adv_report_t *p_report) {
    uint32_t adv_index = 0;
    uint8_t *p_data;

    p_data = p_report->data;
    ble_badge_t badge;
    ble_badge_active_entry_t *p_active_entry;
    bool friendly = false;
    bool valid_name = false;
    uint8_t seen_flags;

    badge.rssi = p_report->rssi;
    badge.said_hello = false;

    // peer adress includes the address type, we only care about the 6 byte GAP address
    memcpy(&badge.address, &p_report->peer_addr.addr, BLE_GAP_ADDR_LEN);

    //Advertised time
    uint16_t time;

    //C2 Data
    master_c2_t c2;
    c2.cmd = 0;
    c2.data = 0;
    c2.seq = 0;

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

        //Complete name field must be made available
        if (field_type == GAP_TYPE_NAME && field_length > 1) {
//        snprintf(badge.name, MIN(SETTING_NAME_LENGTH, field_length), "%s", (char *) field_data);
            memset(badge.name, 0x00, SETTING_NAME_LENGTH);
            uint8_t index = 0;
	    badge.special = 0;
	    char inchar;

            for (uint8_t i = 0; i < strlen((char*) field_data); i++) {
		inchar = field_data[i];
		if (islower((int)inchar))
		    badge.special |= (1 << i);
                const char *ptr = strchr(INPUT_CHARS, toupper((int)inchar));
                badge.name[index++] = *ptr;
                if (index >= SETTING_NAME_LENGTH - 1) {
                    break;
                }
            }
            valid_name = true;
        }

        //Company data should at least contain an identifier (two bytes) + device id (two bytes)
        else if (field_type == GAP_TYPE_COMPANY_DATA) {
            badge.company_id = field_data[0] | (field_data[1] << 8);

	    switch (badge.company_id) {
	    case COMPANY_ID_JOCO:
                badge.device_id = field_data[2] | (field_data[3] << 8);
                //Parse C2 data
                memcpy(&c2, field_data + BLE_DATA_INDEX_C2 + 2, sizeof(master_c2_t));
                //Parse global time sync
                time = (field_data[BLE_DATA_INDEX_GLOBAL_TIME + 2] << 8) | field_data[BLE_DATA_INDEX_GLOBAL_TIME + 3];
		break;
	    case COMPANY_ID:
                sprintf(badge.name, "AND!XOR");
                badge.device_id = p_report->peer_addr.addr[1] << 8 | p_report->peer_addr.addr[0];
		break;
	    case COMPANY_ID_CPV:
                sprintf(badge.name, "CPV");
                badge.device_id = p_report->peer_addr.addr[1] << 8 | p_report->peer_addr.addr[0];
		friendly = true;
		break;
	    case COMPANY_ID_DC503:
                sprintf(badge.name, "DC503");
                badge.device_id = p_report->peer_addr.addr[1] << 8 | p_report->peer_addr.addr[0];
		friendly = true;
		break;
	    case COMPANY_ID_DC801:
                sprintf(badge.name, "DC801");
                badge.device_id = p_report->peer_addr.addr[1] << 8 | p_report->peer_addr.addr[0];
		friendly = true;
		break;
	    case COMPANY_ID_QUEERCON:
                sprintf(badge.name, "Queercon");
                badge.device_id = p_report->peer_addr.addr[1] << 8 | p_report->peer_addr.addr[0];
		friendly = true;
		break;
            }
        }

        //Advance the index
        adv_index += field_length + 1;
    }

    // Now we're done parsing all the BLE data


    // joco badges we have counted as 'visits' are stored in the SD card database
    // This is the slowest access, and should only be accessed after checking the seen list

    // Badges we are considering as ongoing potential as a 'visit' are in the active list

    // joco badges we have counted as 'visits' are cached in the seen list if we've recently seen them
    // 'friend' badges we consider saying 'hi' to are cached in the seen list

//#define FILL_THE_LISTS
#ifdef FILL_THE_LISTS
    // For testing, we can randomly munge the address so we never find a match and the lists fill
    if ( friendly || (((badge.company_id == COMPANY_ID_JOCO) || (badge.company_id == COMPANY_ID)) && valid_name)) {
	badge.address[0] = util_math_rand8();
	badge.address[1] = util_math_rand8();
    }
#endif

    if ((badge.company_id == COMPANY_ID_JOCO) && valid_name) {
	p_active_entry = in_active_list(badge.address, badge.device_id, badge.name);
	if (p_active_entry) {
	    p_active_entry->last_seen = util_local_millis();
	    if ((p_active_entry->last_seen - p_active_entry->first_seen) > VISIT_TIME_LENGTH) {
		// SCORE!!
		uint8_t flags;
		// add it to the db on disk
		save_contact(badge.address, badge.device_id);
		// See if it's one of the master badges
		__check_for_master_unlock(badge.special);
		// add it to the seen list with visited set
		if (p_active_entry->said_hello)
		    flags = (SEEN_FLAG_VISITED | SEEN_FLAG_SAID_HELLO);
		else
		    flags = SEEN_FLAG_VISITED;
		add_to_seen(badge.address, badge.device_id, badge.name, SEEN_TYPE_JOCO, flags);
		// flag it for removal from the active list
		p_active_entry->first_seen = 0;
		// sort the active list and reset the visit timers
		// resets all 'first seen' to now.
		sort_active(true); // INVALIDATES last_entry pointer
		// add to the badge score
		add_to_score(POINTS_4_VISIT, badge.name); // schedules a bling automatically
	    } else if ((!p_active_entry->said_hello) && ( badge.rssi > HELLO_MIN_RSSI)) {
		if (try_to_hello(badge.company_id, badge.name))
		    p_active_entry->said_hello = true;
	    }
	    // DONE
	} else {
	    // Not active, but have we seen it?
	    seen_flags = check_and_add_to_seen(badge.address, badge.device_id, badge.name, SEEN_TYPE_JOCO);

	    if(seen_flags) {
		if((!(seen_flags & SEEN_FLAG_SAID_HELLO)) && (badge.rssi > HELLO_MIN_RSSI)) {
		    if (try_to_hello(badge.company_id, badge.name))
			set_seen_flags(badge.address, badge.device_id, SEEN_FLAG_SAID_HELLO);
		}
	    // DONE
	    } else {
		//we've not seen this one before
		// add to active if there's room and rssi is high enough
		// if we can't add we just ignore this one
		// If added it, No need to resort list since this one has zero visit time and will be at end
		try_to_add_to_active(badge.address, badge.device_id, badge.rssi, badge.name);
	    }
	}
    } else if (((badge.company_id == COMPANY_ID) && valid_name) || friendly) {
	// NOT joco
	// put it on the seen list if it's not already, automatic add for PEER type
	seen_flags = check_and_add_to_seen(badge.address, badge.device_id, badge.name, SEEN_TYPE_PEER);
	if (!(seen_flags & SEEN_FLAG_SAID_HELLO)) {
	    if (try_to_hello(badge.company_id, badge.name))
		set_seen_flags(badge.address, badge.device_id, SEEN_FLAG_SAID_HELLO);
	}
    }

    //handle global time sync with other joco badges
    if (badge.company_id == COMPANY_ID_JOCO && time > 0) {
	__global_time_advertisement_process(time, badge.device_id);
    }

    //Look for medea vodka
    mbp_medea_on_advertisement(p_report);

    //Handle beacon scanning
    beacon_ble_on_ble_advertisement(p_report);
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void __on_adv_evt(ble_adv_evt_t ble_adv_evt) {
    uint32_t err_code = 0;
    UNUSED_VARIABLE(err_code);

    switch (ble_adv_evt) {
    case BLE_ADV_EVT_FAST:
        APP_ERROR_CHECK(err_code);
        break; // BLE_ADV_EVT_FAST

    case BLE_ADV_EVT_IDLE:
        ble_advertising_start(BLE_ADV_MODE_FAST);
        break; // BLE_ADV_EVT_IDLE

    default:
        break;
    }
}

/**@brief Function for the Peer Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Peer Manager.
 */
static void __on_ble_evt(ble_evt_t * p_ble_evt) {
    uint32_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id) {

    case BLE_GAP_EVT_ADV_REPORT:
        __handle_advertisement(&(p_ble_evt->evt.gap_evt.params.adv_report));
        break;

    case BLE_GAP_EVT_AUTH_KEY_REQUEST:
        ;
        err_code = sd_ble_gap_auth_key_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_AUTH_KEY_TYPE_OOB, m_oob_auth_key.tk);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
        // Accepting parameters requested by peer.
        err_code = sd_ble_gap_conn_param_update(p_ble_evt->evt.gap_evt.conn_handle,
                                                &p_ble_evt->evt.gap_evt.params.conn_param_update_request.conn_params);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        m_conn_handle = BLE_CONN_HANDLE_INVALID;
        util_ble_scan_start();
        break; // BLE_GAP_EVT_DISCONNECTED

    case BLE_GAP_EVT_CONNECTED:
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        memset(&m_ble_db_discovery, 0, sizeof(m_ble_db_discovery));
        APP_ERROR_CHECK(ble_db_discovery_start(&m_ble_db_discovery, m_conn_handle));
        break;

    case BLE_GAP_EVT_TIMEOUT:
//      if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN) {
        util_ble_scan_start();
//      }
        break;

    case BLE_GATTC_EVT_TIMEOUT:
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break; // BLE_GATTC_EVT_TIMEOUT

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break; // BLE_GATTS_EVT_TIMEOUT

    case BLE_EVT_USER_MEM_REQUEST:
        err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
        APP_ERROR_CHECK(err_code);
        break; // BLE_EVT_USER_MEM_REQUEST
    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST: {
        ble_gatts_evt_rw_authorize_request_t req;
        ble_gatts_rw_authorize_reply_params_t auth_reply;

        req = p_ble_evt->evt.gatts_evt.params.authorize_request;

        if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID) {
            if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)
                || (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
                || (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL)) {
                if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
                    auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                } else {
                    auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                }
                auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                err_code = sd_ble_gatts_rw_authorize_reply(
                    p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
                APP_ERROR_CHECK(err_code);
            }
        }
    }
        break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

#if (NRF_SD_BLE_API_VERSION == 3)
    case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
        err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                   NRF_BLE_MAX_MTU_SIZE);
        APP_ERROR_CHECK(err_code);
        break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
#endif

    default:
        // No implementation needed.
        break;
    }
}

static void __pm_init() {
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);
    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

// Security parameters to be used for all security procedures.
    sec_param.bond = SEC_PARAM_BOND;
    sec_param.mitm = SEC_PARAM_MITM;
    sec_param.lesc = SEC_PARAM_LESC;
    sec_param.keypress = SEC_PARAM_KEYPRESS;
    sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob = SEC_PARAM_OOB;
    sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc = 1;
    sec_param.kdist_own.id = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

static void __services_init() {
    mbp_medea_ble_init();
    //mbp_master_ble_init();

    //Init NUS
    ble_nus_init_t nus_init;
    memset(&nus_init, 0, sizeof(nus_init));
    nus_init.data_handler = mbp_term_nus_data_handler;
    APP_ERROR_CHECK(ble_nus_init(&m_nus, &nus_init));
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
/**@brief Function for starting advertising.
 */
void util_ble_advertising_start() {
    uint32_t err_code;

    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void __gap_init(void) {
    uint32_t err_code;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(APPEARANCE_ID_ANDNXOR_DC25);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONNECTION_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONNECTION_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUPERVISION_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_tx_power_set(BLE_TX_POWER);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the Application's system events.
 *
 * @param[in]   sys_evt   system event.
 */
static void __sys_evt_handler(uint32_t sys_evt) {
    switch (sys_evt)
    {
    case NRF_EVT_FLASH_OPERATION_SUCCESS:
        break;
    case NRF_EVT_FLASH_OPERATION_ERROR:
        break;

    default:
        // No implementation needed.
        break;
    }
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt) {
    ret_code_t err_code;

    switch (p_evt->evt_id) {
    case PM_EVT_BONDED_PEER_CONNECTED: {
        // NRF_LOG_INFO("Connected to a previously bonded device.\r\n");
    }
        break;

    case PM_EVT_CONN_SEC_SUCCEEDED: {
        //NRF_LOG_INFO(
        //    "Connection secured. Role: %d. conn_handle: %d, Procedure: %d\r\n",
        //    ble_conn_state_role(p_evt->conn_handle), p_evt->conn_handle,
        //    p_evt->params.conn_sec_succeeded.procedure);
    }
        break;

    case PM_EVT_CONN_SEC_FAILED: {
        /* Often, when securing fails, it shouldn't be restarted, for security reasons.
         * Other times, it can be restarted directly.
         * Sometimes it can be restarted, but only after changing some Security Parameters.
         * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
         * Sometimes it is impossible, to secure the link, or the peer device does not support it.
         * How to handle this error is highly application dependent. */
    }
        break;

    case PM_EVT_CONN_SEC_CONFIG_REQ: {
        // Reject pairing request from an already bonded peer.
        pm_conn_sec_config_t conn_sec_config = { .allow_repairing = false };
        pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
    }
        break;

    case PM_EVT_STORAGE_FULL: {
        // Run garbage collection on the flash.
        err_code = fds_gc();
        if (err_code == FDS_ERR_BUSY
            || err_code == FDS_ERR_NO_SPACE_IN_QUEUES) {
            // Retry.
        } else {
            APP_ERROR_CHECK(err_code);
        }
    }
        break;

    case PM_EVT_PEERS_DELETE_SUCCEEDED: {
//              advertising_start();
    }
        break;

    case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED: {
        // The local database has likely changed, send service changed indications.
        pm_local_database_has_changed();
    }
        break;

    case PM_EVT_PEER_DATA_UPDATE_FAILED: {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
    }
        break;

    case PM_EVT_PEER_DELETE_FAILED: {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
    }
        break;

    case PM_EVT_PEERS_DELETE_FAILED: {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
    }
        break;

    case PM_EVT_ERROR_UNEXPECTED: {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
    }
        break;

    case PM_EVT_CONN_SEC_START:
    case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
    case PM_EVT_PEER_DELETE_SUCCEEDED:
    case PM_EVT_LOCAL_DB_CACHE_APPLIED:
    case PM_EVT_SERVICE_CHANGED_IND_SENT:
    case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
    default:
        break;
    }
}

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt) {
// Dispatch the system event to the fstorage module, where it will be
// dispatched to the Flash Data Storage (FDS) module.
    fs_sys_event_handler(sys_evt);

// Dispatch to the Advertising module last, since it will check if there are any
// pending flash operations in fstorage. Let fstorage process system events first,
// so that it can report correctly to the Advertising module.
    ble_advertising_on_sys_evt(sys_evt);

    __sys_evt_handler(sys_evt);
}

void util_ble_score_update() {
    uint16_t score;
    ble_gap_conn_sec_mode_t sec_mode;

    score = mbp_state_score_get();
    if (mbp_state_lastlevel_get() < gamelevel()) {
        score |= 0x8000; // high bit indicates that there are trinkets to be retrieved
    }

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    m_manuf_data.data.p_data[BLE_DATA_INDEX_SCORE] = (score >> 8);
    m_manuf_data.data.p_data[BLE_DATA_INDEX_SCORE + 1] = score;
    ble_advdata_set(&m_adv_data, NULL);
}

void util_ble_badge_read() {
//sd_ble_gattc_read( , handle, offset)
}

uint32_t util_ble_connect(ble_gap_addr_t *p_address) {
    sd_ble_gap_scan_stop();

    //Setup the scan parameters for the badge to badge connection
    ble_gap_scan_params_t scan;
    scan.active = 0;
    scan.interval = SCAN_INTERVAL;
    scan.window = SCAN_WINDOW;

// Don't use whitelist.
#if (NRF_SD_BLE_API_VERSION == 3)
    scan.use_whitelist = 0;
    scan.adv_dir_report = 0;
#else
    scan.selective = 0;
    scan.p_whitelist = NULL;
#endif
    scan.timeout = 5; // seconds

    uint32_t result = sd_ble_gap_connect(p_address, &scan, &m_conn_params);
    return result;
}

uint32_t util_ble_disconnect() {
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        return sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    }

    return NRF_SUCCESS;
}

void util_ble_init() {
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = MBP_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT, &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
#if (NRF_SD_BLE_API_VERSION == 3)
    ble_enable_params.gatt_enable_params.att_mtu = NRF_BLE_MAX_MTU_SIZE;
#endif
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(__ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    __pm_init();
    __gap_init();
    __db_discovery_init();      //Must happen before init services
    __services_init();
    __gatt_init();
    __advertising_init();
    __conn_params_init();

    //Set transmit power
    sd_ble_gap_tx_power_set(BLE_TX_POWER);

    //Startup global time sync timer
    APP_ERROR_CHECK(app_timer_create(&m_ble_global_time_timer, APP_TIMER_MODE_REPEATED, __global_time_timer_handler));
    APP_ERROR_CHECK(app_timer_start(m_ble_global_time_timer, APP_TIMER_TICKS(BLE_GLOBAL_TIME_UPDATE_MS, UTIL_TIMER_PRESCALER), NULL));
}

static void __nus_send_schedule_handler(void *p_data, uint16_t length) {
    while (length > 0) {
        nrf_delay_ms(20);
        //The delay is to mitigate a race condition in the app.
        //Trying to avoid changing app code so that iOS and Android both work the same.
        uint8_t count = MIN(length, BLE_NUS_MAX_DATA_LEN);
        uint32_t result = ble_nus_string_send(&m_nus, (uint8_t *) p_data, count);
        if (result == NRF_SUCCESS) {
            p_data += count;
            length -= count;
        }
    }
}

uint32_t util_ble_nus_send(char *p_string, uint16_t length) {
    app_sched_event_put(p_string, length, __nus_send_schedule_handler);
    return NRF_SUCCESS;
}

void util_ble_off() {
    sd_ble_gap_adv_stop();
    sd_ble_gap_scan_stop();
}

void util_ble_on() {
#ifndef OPSEC
    util_ble_scan_start();
    util_ble_advertising_start();
#endif
}

void util_ble_name_get(char *name) {
    uint16_t len = 0;
    sd_ble_gap_device_name_get((uint8_t *) name, &len);
}

void util_ble_name_set(char *name) {
    uint8_t special = mbp_state_special_get();
    int maxlen = 8;
    char name_temp[9];

    strcpy(name_temp, name);

    int sl = strlen(name_temp);
    if (sl < maxlen)
	maxlen = sl;

    for (int i=0; i<maxlen ; i++) {
	if ((1<<i) & special) {
	    name_temp[i] = tolower((int) name_temp[i]);
	}
    }    

    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&sec_mode);
    APP_ERROR_CHECK(sd_ble_gap_device_name_set(&sec_mode, (const uint8_t * ) name_temp, strlen(name_temp)));
    ble_advdata_set(&m_adv_data, NULL);
}

void util_ble_scan_start() {
    uint32_t err_code;

    sd_ble_gap_scan_stop();

    ble_gap_scan_params_t scan;
    scan.active = 0;
    scan.interval = SCAN_INTERVAL;
    scan.window = SCAN_WINDOW;

// Don't use whitelist.
#if (NRF_SD_BLE_API_VERSION == 3)
    scan.use_whitelist = 0;
    scan.adv_dir_report = 0;
#else
    scan.selective = 0;
    scan.p_whitelist = NULL;
#endif
    scan.timeout = 0x0000; // No timeout.

    err_code = sd_ble_gap_scan_start(&scan);
    APP_ERROR_CHECK(err_code);
}

void util_ble_c2_set(master_c2_t *p_c2) {
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    //Copy c2 data packet into advertisement
    memcpy((m_adv_data.p_manuf_specific_data->data.p_data) + BLE_DATA_INDEX_C2, p_c2, sizeof(master_c2_t));

    ble_advdata_set(&m_adv_data, NULL);
}
