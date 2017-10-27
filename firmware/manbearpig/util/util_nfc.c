/*****************************************************************************
 * (C) Copyright 2017 
 *
 * PROPRIETARY AND CONFIDENTIAL UNTIL FEBRUARY 26TH, 2018 then,
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
 *****************************************************************************/
#include "../system.h"

//#defines

//Local nfc data



/**@brief Function for registering the sequence of internal bytes.
 *
 * @details This refers to the first 10 bytes of the tag memory. The library will
 * set a sensible default for these bytes. The application can use this function
 * to override the default.
 *
 * See: https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v12.2.0%2Fgroup__nfc__t2t__lib.html
 *
 * @param[in] p_data  pointer to the data.
 * @param[in] data_length  data length.
 */
void nfc_t2t_internal_set(uint8_t *p_data, size_t data_length) {
}

/**@brief Function for handling the Application's NFC events.
 *
 * @details This function will be called for NFC events which are passed to the application.
 *
 * @param[in] TODO_FILL_THIS_IN  Advertising event.
 */
static void __on_nfc_evt(void *p_context, hal_nfc_event_t nfc_evt, const uint8_t *p_data, size_t data_length) {
	uint32_t err_code = 0;
	UNUSED_VARIABLE(err_code);

	switch (nfc_evt) {
	case HAL_NFC_EVENT_FIELD_ON:
		break; // HAL_NFC_EVENT_FIELD_ON
	case HAL_NFC_EVENT_FIELD_OFF:
		break; // HAL_NFC_EVENT_FIELD_OFF
	case HAL_NFC_EVENT_DATA_RECEIVED:
		break; // HAL_NFC_EVENT_DATA_RECEIVED
	case HAL_NFC_EVENT_DATA_TRANSMITTED:
		break; // HAL_NFC_EVENT_DATA_TRANSMITTED
	default:
		break;
	}
}

/**@brief Function for Setting up NFC
 *
 * @param[in] TBD_CHANGE_THIS
 */
/**@brief Function for starting advertising.
 */
void util_nfc_start() {
	uint32_t err_code;

	err_code = hal_nfc_start();
	APP_ERROR_CHECK(err_code);
}

void util_nfc_init() {
	uint32_t err_code;
        // Intialize context if needed, for now we don't use it
        void *p_context = 0;

        err_code = hal_nfc_setup(__on_nfc_evt, p_context);
	APP_ERROR_CHECK(err_code);

        // Set parameters as needed
        //err_code = hal_nfc_parameter_set (hal_nfc_param_id_t id, void *p_data, size_t data_length)
	//APP_ERROR_CHECK(err_code);
}
