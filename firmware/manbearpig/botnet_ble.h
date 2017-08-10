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

#ifndef BOTNET_BLE_H_
#define BOTNET_BLE_H_

extern void botnet_ble_attack(ble_badge_t *p_badge, botnet_attack_t *p_attack);
extern bool botnet_ble_connect_blocking(ble_badge_t *p_badge) ;
extern void botnet_ble_on_ble_evt(const ble_evt_t * p_ble_evt);
extern void botnet_ble_on_db_disc_evt(const ble_db_discovery_evt_t * p_evt);
extern uint32_t botnet_ble_init();
extern uint8_t botnet_ble_scan_get(ble_badge_t *p_badge);
extern void botnet_ble_update_service_status();

#endif /* BOTNET_BLE_H_ */
