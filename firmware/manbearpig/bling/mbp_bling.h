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
#ifndef MBP_ANIM_H_
#define MBP_ANIM_H_

//#define LED_LEFT_EYE_INDEX		13
#define LED_RIGHT_EYE_INDEX		12
#define LED_TOOTH_INDEX			13
#define LED_MATRIX_LAST_INDEX	11

extern void simple_filebased_bling(char *rawfile, char *rgbfile);
extern void mbp_bling_backer_abraxas3d(void *data);
extern void mbp_bling_backer_andnxor(void *data);
extern void mbp_bling_badgers();
extern void mbp_bling_wheaton();
extern void mbp_bling_damon();
extern void mbp_bling_defrag();
extern void mbp_bling_flames(void *data);
extern void mbp_bling_illusion();
extern void mbp_bling_led_botnet(uint8_t frame, void *data);
extern void mbp_bling_led_rainbow_callback(uint8_t frame, void *p_data);
extern void mbp_bling_major_lazer(void *data);
extern void mbp_bling_matrix();
extern void mbp_bling_nyan();
extern void mbp_bling_hack_time();
extern void mbp_bling_owl();
extern void mbp_bling_pirate();
extern void mbp_bling_rickroll();
extern uint8_t mbp_bling_scroll(char *text, bool loop);
extern void mbp_bling_scroll_cycle();
extern void mbp_bling_score_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_bender_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_joco_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_cpv_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_dc503_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_dc801_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_hello_queercon_schedule_handler(void * p_event_data, uint16_t event_size);
extern void mbp_bling_snake();
extern void mbp_bling_toad();
extern void mbp_bling_trololol();
extern void mbp_bling_twitter();
extern void mbp_bling_meatspin();
extern void mbp_bling_whats_up();
//
extern void mbp_bling_skull_crossbones();
extern void mbp_bling_5th_element_dance();
extern void mbp_bling_candy_mountain();
extern void mbp_bling_concert_flame();
extern void mbp_bling_dancing_cyberman();
extern void mbp_bling_drwho_time();
extern void mbp_bling_duckhunt();
extern void mbp_bling_fallout_boygirl_drinking();
extern void mbp_bling_fallout_boy_science();
extern void mbp_bling_get_on_my_horse();
extern void mbp_bling_multipass_leelo();
extern void mbp_bling_outer_limits();
extern void mbp_bling_portal_frying_pan();
extern void mbp_bling_portal_wink();
extern void mbp_bling_portals();
extern void mbp_bling_sleestaks();
extern void mbp_bling_tardis_nyan();
extern void mbp_bling_twilight_zone();
extern void mbp_bling_zombie_nyan();
//
extern bool mbp_tooth_eye_running();
extern void mbp_tooth_eye_start();
extern void mbp_tooth_eye_stop();

#endif /* MBP_ANIM_H_ */
