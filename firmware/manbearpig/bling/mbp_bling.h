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
#ifndef MBP_ANIM_H_
#define MBP_ANIM_H_

#define LED_LEFT_EYE_INDEX		13
#define LED_RIGHT_EYE_INDEX		12
#define LED_CIGAR_INDEX			14
#define LED_MATRIX_LAST_INDEX	11

extern void mbp_bling_backer_credits();
extern void mbp_bling_backer_april();
extern void mbp_bling_backer_btcctf();
extern void mbp_bling_backer_abraxas3d(void *data);
extern void mbp_bling_backer_cybersulu();
extern void mbp_bling_backer_sol(void *data);
extern void mbp_bling_badgers();
extern void mbp_bling_bender();
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
extern void mbp_bling_party();
extern void mbp_bling_rager();
extern void mbp_bling_pirate();
extern void mbp_bling_rickroll();
extern uint8_t mbp_bling_scroll(char *text, bool loop);
extern void mbp_bling_scroll_cycle();
extern void mbp_bling_hello_schedule_handler(void * p_event_data, uint16_t event_size);
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
extern bool mbp_cigar_eyes_running();
extern void mbp_cigar_eyes_start();
extern void mbp_cigar_eyes_stop();

#endif /* MBP_ANIM_H_ */
