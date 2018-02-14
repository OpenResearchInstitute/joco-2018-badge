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
#ifndef MBP_UI_H_
#define MBP_UI_H_

#include "system.h"

extern char INPUT_CHARS[];
extern char INPUT_DIGITS[];
#define INPUT_DIGITS_COUNT	10
#define INPUT_CHARS_COUNT	64

extern void mbp_ui_button(int16_t x, int16_t y, uint8_t w, uint8_t h, char *text, bool selected);
extern void mbp_ui_cls();
extern void mbp_ui_error(char *text);
extern void mbp_ui_input(char *p_title, char *p_label, char *p_input, uint8_t max_chars, bool numeric);
extern void mbp_ui_popup(char * title, char *text);
extern bool mbp_ui_textbox(int16_t x, int16_t y, uint8_t w, uint8_t h, int16_t scroll_y, char *text);
extern uint8_t mbp_ui_toggle_popup(char *p_title, uint8_t selected_option, char *p_option1, char *p_option2, char *p_note);

#endif /* MBP_UI_H_ */
