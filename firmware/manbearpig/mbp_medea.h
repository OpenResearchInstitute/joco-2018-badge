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

#ifndef MBP_MEDEA_H_
#define MBP_MEDEA_H_

#define MEDEA_DB_SIZE 				5

typedef struct {
	uint32_t last_seen;
	ble_gap_addr_t address;
} medea_bottle_t;

extern void mbp_medea_ble_init();
extern uint8_t mbp_medea_bottle_count();
extern medea_bottle_t *mbp_medea_bottle_db_get();
extern void mbp_medea_hack(void *p_data);
extern void mbp_medea_on_advertisement(ble_gap_evt_adv_report_t *p_report);
extern void mbp_medea_on_ble_evt(const ble_evt_t * p_ble_evt);
extern void mbp_medea_on_db_disc_evt(const ble_db_discovery_evt_t * p_evt);
#endif /* MBP_MEDEA_H_ */
