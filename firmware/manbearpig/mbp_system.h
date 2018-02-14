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
#ifndef CONFIG_MBP_SYSTEM_H_
#define CONFIG_MBP_SYSTEM_H_

//uncomment this to hide secret features
//#define OPSEC				1
//uncomment this for red badges
//TODO: This should be a file check like APA102
//#define RED_BADGE			1

extern void mbp_system_about();
extern void mbp_system_airplane_mode_select();
extern void mbp_system_code();
//extern bool mbp_system_interuptable_get();
//extern void mbp_system_interuptable_set(bool interuptable);
extern void mbp_system_shouts();
extern void mbp_system_special_edit();
extern void mbp_system_name_edit();
extern void mbp_system_name_select();
extern void mbp_system_game_menu();
extern uint16_t mbp_system_color_selection_menu(uint16_t curr_color);

extern void mbp_system_reset();
extern bool mbp_system_seekrit_get();
extern void mbp_system_test();

extern void mbp_system_tilt_mode_select();
extern void mbp_system_unlock_state();

#endif /* CONFIG_MBP_SYSTEM_H_ */
