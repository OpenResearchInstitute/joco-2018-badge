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
#ifndef MBP_MASTER_BLE_H_
#define MBP_MASTER_BLE_H_

#include "system.h"

extern uint32_t mbp_master_ble_init();
extern void mbp_master_ble_on_db_disc_evt(const ble_db_discovery_evt_t * p_evt);
extern void mbp_master_ble_on_ble_evt(const ble_evt_t * p_ble_evt);
extern bool mbp_master_ble_send_data(ble_badge_t *p_badge, uint32_t data);

#endif /* MBP_MASTER_BLE_H_ */
