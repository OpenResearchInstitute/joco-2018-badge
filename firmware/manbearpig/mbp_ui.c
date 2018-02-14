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
#include "system.h"

#define BUTTON_COLOR		COLOR_LIGHTGREY
#define BUTTON_BG			COLOR_DARKBLUE
#define BUTTON_FG			COLOR_WHITE
#define BUTTON_SELECTED		COLOR_YELLOW
#define ERROR_TITLE_BG		COLOR_RED
#define ERROR_TITLE_FG		COLOR_WHITE
#define TOGGLE_BUTTON_H		18
#define TOGGLE_COLOR			COLOR_WHITE
#define TOGGLE_TITLE_BG		COLOR_DARKCYAN
#define TOGGLE_TITLE_FG		COLOR_WHITE
#define TOGGLE_TITLE_H		18
#define TOGGLE_PADDING		2
#define POPUP_SHADOW		COLOR_BLACK
#define POPUP_SHADOW_SIZE	2
#define POPUP_TITLE_BG		COLOR_DARKBLUE
#define POPUP_BODY_BG		COLOR_BLACK		//Background color to use in the popup
#define POPUP_FG			COLOR_WHITE			//Foreground color (title bar)
#define POPUP_TITLE_H		18
#define POPUP_MARGIN		10				//Margin around the popup
#define POPUP_PADDING		4				//Padding inside the body of the popup
#define POPUP_SPACING		3				//Spacing between popup objects

//parameters for input window
#define INPUT_CURSOR_HEIGHT	3				//Height of cursor
#define INPUT_FG			COLOR_WHITE
#define INPUT_PADDING		2				//Padding within the input object
#define INPUT_SELECTED_BG	COLOR_CYAN
#define INPUT_TITLE_BG		COLOR_DARKGREEN
#define INPUT_TITLE_H		16

#define SCROLL_COLOR		COLOR_CYAN
#define SCROLL_SPEED		6
#define TEXTBOX_BG			COLOR_BLACK
#define TEXTBOX_FG			COLOR_WHITE
#define TEXTBOX_PADDING		3

#define BUTTON_DELAY		200 // mS

char INPUT_CHARS[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.!?,()[]{}<>/\\|;:&^%$#@*-_+";
char INPUT_DIGITS[] = "0123456789";

static void __mbp_ui_popup(char *title, char *text, uint16_t title_bg, uint16_t title_fg) {
	int16_t x = POPUP_MARGIN;
	int16_t ix = x + POPUP_PADDING;
	int16_t y = POPUP_MARGIN;
	int16_t body_y = y + POPUP_TITLE_H;
	int16_t w = GFX_WIDTH - (POPUP_MARGIN * 2);
	int16_t h = GFX_HEIGHT - (POPUP_MARGIN * 2);
	int16_t body_h = h - POPUP_TITLE_H;
	int16_t iw = w - (POPUP_PADDING * 2);
	int16_t ih = h - (POPUP_PADDING * 2);
	area_t cursor_area_title = { ix, y + POPUP_PADDING, ix + iw, body_y };
	UNUSED_VARIABLE(ih);
	int16_t scroll_y = 0;

	//Draw popup boxes and borders
	util_gfx_fill_rect(x + POPUP_SHADOW_SIZE, y + POPUP_SHADOW_SIZE, w, h, POPUP_SHADOW);
	util_gfx_fill_rect(x, y, w, POPUP_TITLE_H, title_bg);
	util_gfx_draw_rect(x, y, w, h, POPUP_FG);
	util_gfx_draw_line(x, body_y, x + w, body_y, title_fg);

	//Print the title
	util_gfx_set_font(FONT_LARGE);
	//Setup space for title text
	util_gfx_cursor_area_set(cursor_area_title);
	util_gfx_set_cursor(ix, y + POPUP_PADDING + 3);
	util_gfx_set_color(title_fg);
	util_gfx_print(title);

	bool draw = true;
	bool truncated = false;

	app_sched_pause();

	util_button_clear();

	//Handle user input until they quit. Only redraw body for scrolls. Assume nothing overlaps us
	while (1) {
		if (draw) {
			truncated = mbp_ui_textbox(x + 1, body_y + 1, w - 1, body_h - 1, scroll_y, text);
			draw = false;
		}

		//Wait for user
		while (util_button_wait() == 0)
			;

		if (util_button_down()) {
			if (truncated) {
				scroll_y -= SCROLL_SPEED;
				draw = true;
			}
		}

		if (util_button_up()) {
			if (scroll_y < 0) {
				scroll_y += SCROLL_SPEED;
				draw = true;
			}
			if (scroll_y > 0) {
				scroll_y = 0;
				draw = true;
			}
		}

		if (util_button_action() || util_button_left()) {
			break;
		}
	}

	//Cleanup
	app_sched_resume();
	util_button_clear();
	util_gfx_cursor_area_reset();
}

static inline uint16_t __word_length(const char * str) {
	int tempindex = 0;
	while (str[tempindex] != ' ' && str[tempindex] != 0 && str[tempindex] != '\n') {
		++tempindex;
	}
	return (tempindex);
}

void __wrap_text(char * s, uint16_t max_width) {

	int index = 0;
	int curlinelen = 0;
	while (s[index] != '\0') {

		if (s[index] == '\n') {
			curlinelen = 0;
		}
		else if (s[index] == ' ') {

			uint16_t word_len = __word_length(&s[index + 1]);
			if (curlinelen + word_len >= max_width) {
				s[index] = '\n';
				curlinelen = 0;
			}

		}

		curlinelen++;
		index++;
	}

}

void mbp_ui_button(int16_t x, int16_t y, uint8_t w, uint8_t h, char *text, bool selected) {
	uint16_t tw, th;
	int16_t tx, ty;
	uint16_t border_color = BUTTON_COLOR;

	if (selected) {
		border_color = BUTTON_SELECTED;
	}

	util_gfx_set_font(FONT_SMALL);
	util_gfx_get_text_bounds(text, x, y, &tw, &th);

	tx = (w - tw) / 2;
	ty = (h - th) / 2;

	util_gfx_fill_rect(x, y, w, h, BUTTON_BG);
	util_gfx_draw_rect(x, y, w, h, border_color);

	area_t area = { x, y, x + w, y + h };
	util_gfx_cursor_area_set(area);
	util_gfx_set_color(BUTTON_FG);
	util_gfx_set_cursor(x + tx, y + ty);
	util_gfx_print(text);

	util_gfx_cursor_area_reset();
}

void mbp_ui_cls() {
	util_gfx_fill_screen(COLOR_BLACK);
}

void mbp_ui_error(char *text) {
	__mbp_ui_popup("ERROR", text, ERROR_TITLE_BG, ERROR_TITLE_FG);
}

void mbp_ui_input(char *p_title, char *p_label, char *p_input, uint8_t max_chars, bool numeric) {
	int8_t cursor = 0;
	int16_t input_x = INPUT_PADDING;
	int16_t input_y = 70;
	char *input_charset;
	int16_t input_charset_count;

	if (numeric) {
	    input_charset = INPUT_DIGITS;
	    input_charset_count = INPUT_DIGITS_COUNT;
	} else {
	    input_charset = INPUT_CHARS;
	    input_charset_count = INPUT_CHARS_COUNT;
	}

	if (max_chars > SETTING_INPUT_MAX)
	    max_chars = SETTING_INPUT_MAX;

	//Make sure input doesn't extend past max input
	if (strlen(p_input) > max_chars) {
		p_input[max_chars] = 0;
	}

	//Setup temporary storage for input
	char temp_input[SETTING_INPUT_MAX + 1];
	//Make sure input is all spaces, we'll truncate at the end
	memset(temp_input, ' ', SETTING_INPUT_MAX);
	//Ensure null termination
	temp_input[max_chars] = 0;
	memcpy(temp_input, p_input, MIN(strlen(p_input), max_chars));

	uint16_t temp_w, temp_h;

	util_gfx_invalidate();

	//Iteractively get input from user
	while (1) {

		//Redraw entire input window if we have an invalid state
		if (!util_gfx_is_valid_state()) {
			mbp_ui_cls();
			//Draw the title
			util_gfx_fill_rect(0, 0, GFX_WIDTH, INPUT_TITLE_H, INPUT_TITLE_BG);
			util_gfx_draw_line(0, INPUT_TITLE_H, GFX_WIDTH, INPUT_TITLE_H, INPUT_FG);
			util_gfx_set_font(FONT_LARGE);
			//Setup space for title text to prevent wrapping
			area_t cursor_area_title = { 0, 0, GFX_WIDTH, INPUT_TITLE_H };
			util_gfx_cursor_area_set(cursor_area_title);

			util_gfx_get_text_bounds(p_title, 0, 0, &temp_w, &temp_h);
			util_gfx_set_cursor(INPUT_PADDING + ((GFX_WIDTH - temp_w) / 2), 2 + INPUT_PADDING + ((INPUT_TITLE_H - temp_h) / 2));
			util_gfx_set_color(INPUT_FG);
			util_gfx_print(p_title);
		}

		//highlight selected character
		//util_gfx_fill_rect(x + (cursor * font_w), y + font_h - INPUT_CURSOR_HEIGHT, font_w, INPUT_CURSOR_HEIGHT, INPUT_SELECTED_BG);

		util_gfx_cursor_area_reset();
		util_gfx_set_font(FONT_MEDIUM);

		//Print the current text
		util_gfx_set_cursor(input_x, input_y);

		char temp[2];

		for (uint8_t i = 0; i < max_chars; i++) {
			int8_t index = util_index_of(input_charset, temp_input[i]);

			for (int8_t j = -3; j <= 3; j++) {
				int8_t temp_index = index + j;
				if (temp_index < 0) {
					temp_index += input_charset_count;
				} else if (temp_index >= input_charset_count) {
					temp_index -= input_charset_count;
				}

				//Drawing row for actual user text
				if (j == 0) {
					if (i == cursor) {
						util_gfx_set_color(COLOR_YELLOW);
					} else {
						util_gfx_set_color(COLOR_WHITE);
					}
					util_gfx_set_cursor(8 + (i * 14), 66 - (j * 15));
					sprintf(temp, "%c", input_charset[temp_index]);
					util_gfx_print(temp);
				}
				//All other rows
				else if (i == cursor) {
					util_gfx_set_color(COLOR_DARKGREY);
					util_gfx_set_cursor(8 + (i * 14), 66 - (j * 15));
					sprintf(temp, "%c", input_charset[temp_index]);
					util_gfx_print(temp);
				}
			}
		}

		util_gfx_validate();

		//Wait for the user
		util_button_wait();

		//If right is pressed move cursor to the right
		if (util_button_right() > 0) {
			nrf_delay_ms(BUTTON_DELAY);
			util_gfx_fill_rect(0, INPUT_TITLE_H + 1, GFX_WIDTH, GFX_HEIGHT - INPUT_TITLE_H, COLOR_BLACK);
			cursor = (cursor + 1) % SETTING_INPUT_MAX;
		}

		//If left is pressed move cursor to the right
		if (util_button_left() > 0) {
			nrf_delay_ms(BUTTON_DELAY);
			util_gfx_fill_rect(0, INPUT_TITLE_H + 1, GFX_WIDTH, GFX_HEIGHT - INPUT_TITLE_H, COLOR_BLACK);
			cursor--;
			if (cursor < 0) {
				cursor += SETTING_INPUT_MAX;
			}
		}

		//If up is pressed, rotate rolodex
		if (util_button_up() > 0) {
			nrf_delay_ms(BUTTON_DELAY);
			util_gfx_fill_rect(0, INPUT_TITLE_H + 1, GFX_WIDTH, GFX_HEIGHT - INPUT_TITLE_H, COLOR_BLACK);
			int8_t index = util_index_of(input_charset, temp_input[cursor]);
			index = (index + 1) % input_charset_count;
			temp_input[cursor] = input_charset[index];
		}

		//If down is pressed, rotate rolodex
		if (util_button_down() > 0) {
			nrf_delay_ms(BUTTON_DELAY);
			util_gfx_fill_rect(0, INPUT_TITLE_H + 1, GFX_WIDTH, GFX_HEIGHT - INPUT_TITLE_H, COLOR_BLACK);
			int8_t index = util_index_of(input_charset, temp_input[cursor]);
			index--;
			if (index < 0) {
				index += input_charset_count;
			}
			temp_input[cursor] = input_charset[index];
		}

		//If action button pressed, quit
		if (util_button_action() > 0) {
			break;
		}
	}

	//Ensure null terminated
	temp_input[max_chars] = 0;

	//Truncate spaces
	for (uint8_t i = max_chars - 1; i >= 0; i--) {
		if (temp_input[i] == ' ') {
			temp_input[i] = 0;
		} else {
			break;
		}
	}

	memcpy(p_input, temp_input, max_chars + 1);

	//Cleanup
	util_button_clear();
	util_gfx_cursor_area_reset();
}

/**
 * Display the game popup with start and cancel buttons with a space to scroll the description.
 * @param p_game pointer to chip_8 game data
 *
 * @return true if the user wants to start the game
 */
void mbp_ui_popup(char *title, char *text) {
	app_sched_pause();
	__mbp_ui_popup(title, text, POPUP_TITLE_BG, POPUP_FG);
	app_sched_resume();
}

/**
 *
 * @return if truncated
 */
bool mbp_ui_textbox(int16_t x, int16_t y, uint8_t w, uint8_t h, int16_t scroll_y, char *text) {
	area_t area = { x + TEXTBOX_PADDING, y + TEXTBOX_PADDING, x + w - TEXTBOX_PADDING, y + h - TEXTBOX_PADDING };
	util_gfx_fill_rect(x, y, w, h, TEXTBOX_BG);
	util_gfx_draw_rect(x, y, w, h, TEXTBOX_FG);
	util_gfx_set_font(FONT_SMALL);
	util_gfx_cursor_area_set(area);
	util_gfx_set_color(TEXTBOX_FG);
	util_gfx_set_cursor(x + TEXTBOX_PADDING, y + TEXTBOX_PADDING + scroll_y + 3);
	uint8_t font_width = util_gfx_font_width();

	//Wrap the text assuming fixed width
	//Copy text onto heap in case it's a const
	char *text_heap = (char*) malloc(strlen(text) + 1);
	memcpy(text_heap, text, strlen(text) + 1);
	__wrap_text(text_heap, (area.xe - area.xs) / font_width);
	util_gfx_print(text_heap);
	free(text_heap);

	//If any rows truncated, indicate to the user
	if (util_gfx_cursor_y_get() > area.ye) {
		util_gfx_draw_triangle(
				x + w - 8,
				y + h - 8,
				x + w - 4,
				y + h - 8,
				x + w - 6,
				y + h - 4,
				SCROLL_COLOR);
	}

	//If we've scrolled down at all, indicate to user
	if (scroll_y < 0) {
		util_gfx_draw_triangle(
				x + w - 8,
				y + 8,
				x + w - 4,
				y + 8,
				x + w - 6,
				y + 4,
				SCROLL_COLOR);
	}

	area.xs = x;
	area.xe = x + w;
	area.ys = y;
	area.ye = y + h;
	util_gfx_cursor_area_set(area);
	int16_t c = util_gfx_cursor_y_get();
	uint8_t font_h = util_gfx_font_height();
	return c > (area.ye - font_h);
}

/**
 * Draw a popup box allowing the user to select between two options
 *
 * Index of the selected option is returned
 */
uint8_t mbp_ui_toggle_popup(char *p_title, uint8_t selected_option, char *p_option1, char *p_option2, char *p_note) {
	int16_t y = 0;
	int16_t button_y = TOGGLE_TITLE_H + (2 * TOGGLE_PADDING);
	int16_t textbox_y = button_y + TOGGLE_BUTTON_H + TOGGLE_PADDING;
	uint8_t selected = selected_option; //0 = Left Option, 1 = Right Option
	uint16_t tw, th;

	//Draw the text box
	uint8_t textbox_h = GFX_HEIGHT - textbox_y - TOGGLE_PADDING;
	uint8_t textbox_w = GFX_WIDTH - (TOGGLE_PADDING * 2);
	int16_t scroll_y = 0;

	bool truncated = false;	//Indicate if text box was truncated at all

	//force full draw
	bool draw = true;
	util_gfx_invalidate();

	while (1) {

		if (!util_gfx_is_valid_state()) {
			mbp_ui_cls();
			y = 0;

			//Draw title
			util_gfx_set_font(FONT_MEDIUM);
			util_gfx_fill_rect(0, 0, GFX_WIDTH, TOGGLE_TITLE_H, TOGGLE_TITLE_BG);
			util_gfx_draw_rect(0, 0, GFX_WIDTH, TOGGLE_TITLE_H, TOGGLE_TITLE_FG);

			y += TOGGLE_PADDING;
			util_gfx_cursor_area_reset();
			util_gfx_get_text_bounds(p_title, 0, 0, &tw, &th);
			util_gfx_set_cursor((GFX_WIDTH - tw) / 2, y + (TOGGLE_TITLE_H - th) / 2);
			util_gfx_print(p_title);
		}

		if (draw || !util_gfx_is_valid_state()) {
			mbp_ui_button(TOGGLE_PADDING, button_y, 60, TOGGLE_BUTTON_H, p_option1, (selected == 0));
			mbp_ui_button(64 + TOGGLE_PADDING, button_y, 60, TOGGLE_BUTTON_H, p_option2, (selected == 1));
			truncated = mbp_ui_textbox(TOGGLE_PADDING, textbox_y, textbox_w, textbox_h, scroll_y, p_note);
			draw = false;
		}

		//Mark graphics state as complete
		util_gfx_validate();

		//wait for the user
		util_button_clear();
		util_button_wait();

		if (util_button_down()) {
			if (truncated) {
				scroll_y -= SCROLL_SPEED;
				draw = true;
			}
		}

		if (util_button_up()) {
			if (scroll_y < 0) {
				scroll_y += SCROLL_SPEED;
				draw = true;
			}
			if (scroll_y > 0) {
				scroll_y = 0;
				draw = true;
			}
		}

		if (util_button_left()) {
			if (selected == 1) {
				selected = 0;
				draw = true;
			}
		}

		if (util_button_right()) {
			if (selected == 0) {
				selected = 1;
				draw = true;
			}
		}

		if (util_button_action()) {
			util_button_clear();
			return selected;
		}
	}

	return 0xFF;
}
