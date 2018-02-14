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
#ifndef UTIL_BUTTON_H_
#define UTIL_BUTTON_H_

#include <stdint.h>

#define BUTTON_MASK_UP		0b00000001
#define BUTTON_MASK_DOWN	0b00000010
#define BUTTON_MASK_LEFT	0b00000100
#define BUTTON_MASK_RIGHT	0b00001000
#define BUTTON_MASK_ACTION	0b00010000

#define BUTTON_REPEAT_DELAY   140

extern void util_button_clear();
extern void util_button_init();
extern uint8_t util_button_state();
extern uint8_t util_button_wait();
extern uint8_t util_button_down();
extern uint8_t util_button_up();
extern uint8_t util_button_left();
extern uint8_t util_button_right();
extern uint8_t util_button_action();

#endif
