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

typedef struct {
	struct tcl *p_tcl;
	void *p_data;
} tcl_context_t;

static bool m_quit = false;
static uint8_t m_io_pins[] = { 31, 10, 9, 1, 0 };
static uint16_t m_color = COLOR_WHITE;
#define IO_PIN_MAX	5

/**
 * Convert 8-bit color value to 16-bit
 *
 * 8-bit colors are stored as RRRGGGBB
 */
static uint16_t __tcl_color(tcl_value_t *tcl_color) {
	int color8 = tcl_int(tcl_color);

	uint16_t red = (color8 & 0xE0) >> 5;
	uint16_t green = (color8 & 0x1C) >> 2;
	uint16_t blue = (color8 & 0x03);

	uint16_t color16 = (red << 13) | (green << 8) | (blue << 3);
	return color16;
}

static int __tcl_chr(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *var_value = tcl_list_at(args, 1);
	int value = tcl_int(var_value);

	char r_str[2];
	if (value < 0x20 || value > 0x7E) {
		r_str[0] = ' ';
	} else {
		r_str[0] = value;
	}
	r_str[1] = 0;

	tcl_free(var_value);
	return tcl_result(tcl, FNORMAL, tcl_alloc(r_str, strlen(r_str)));
}

static int __tcl_circle(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_x = tcl_list_at(args, 1);
	tcl_value_t *tcl_y = tcl_list_at(args, 2);
	tcl_value_t *tcl_r = tcl_list_at(args, 3);
	tcl_value_t *tcl_color = tcl_list_at(args, 4);

	int16_t x = tcl_int(tcl_x);
	int16_t y = tcl_int(tcl_y);
	int16_t r = tcl_int(tcl_r);
	uint16_t color = __tcl_color(tcl_color);

	util_gfx_draw_circle(x, y, r, color);

	tcl_free(tcl_x);
	tcl_free(tcl_y);
	tcl_free(tcl_r);
	tcl_free(tcl_color);
	return FNORMAL;
}

static int __tcl_cls(struct tcl *tcl, tcl_value_t *args, void *arg) {
	mbp_ui_cls();
	return FNORMAL;
}

static int __tcl_delay(struct tcl *tcl, tcl_value_t *args, void *arg) {
	uint32_t ms = tcl_int(tcl_list_at(args, 1));
	nrf_delay_ms(ms);
	return FNORMAL;
}

static int __tcl_fill_rect(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_x = tcl_list_at(args, 1);
	tcl_value_t *tcl_y = tcl_list_at(args, 2);
	tcl_value_t *tcl_w = tcl_list_at(args, 3);
	tcl_value_t *tcl_h = tcl_list_at(args, 4);
	tcl_value_t *tcl_color = tcl_list_at(args, 5);

	int16_t x = tcl_int(tcl_x);
	int16_t y = tcl_int(tcl_y);
	int16_t w = tcl_int(tcl_w);
	int16_t h = tcl_int(tcl_h);
	uint16_t color = __tcl_color(tcl_color);

	util_gfx_fill_rect(x, y, w, h, color);

	tcl_free(tcl_x);
	tcl_free(tcl_y);
	tcl_free(tcl_w);
	tcl_free(tcl_h);
	tcl_free(tcl_color);
	return FNORMAL;
}

/**
 * for { set a 10}  {$a < 20} {incr a} {
 * 		//Do stuff
 * }
 *
 * Format: for {init} {condition} {increment} {body}
 */
static int __tcl_for(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *init = tcl_list_at(args, 1);
	tcl_value_t *condition = tcl_list_at(args, 2);
	tcl_value_t *increment = tcl_list_at(args, 3);
	tcl_value_t *loop = tcl_list_at(args, 4);

	uint32_t result;

	//Evaluate the init
	result = tcl_eval(tcl, tcl_string(init), tcl_length(init) + 1);
	if (result != FNORMAL) {
		tcl_free(init);
		tcl_free(condition);
		tcl_free(increment);
		tcl_free(loop);
		return FERROR;
	}

	//Run the for loop as a while loop
	for (;;) {
		result = tcl_eval(tcl, tcl_string(condition),
				tcl_length(condition) + 1);
		if (result != FNORMAL) {
			tcl_free(init);
			tcl_free(condition);
			tcl_free(increment);
			tcl_free(loop);
			return result;
		}
		if (!tcl_int(tcl->result)) {
			tcl_free(init);
			tcl_free(condition);
			tcl_free(increment);
			tcl_free(loop);
			return FNORMAL;
		}

		int r = tcl_eval(tcl, tcl_string(loop), tcl_length(loop) + 1);

		switch (r) {
		case FBREAK:
			tcl_free(init);
			tcl_free(condition);
			tcl_free(increment);
			tcl_free(loop);
			return FNORMAL;
		case FRETURN:
			tcl_free(init);
			tcl_free(condition);
			tcl_free(increment);
			tcl_free(loop);
			return FRETURN;
		case FAGAIN:
			continue;
		case FERROR:
			tcl_free(init);
			tcl_free(condition);
			tcl_free(increment);
			tcl_free(loop);
			return FERROR;
		}

		//Evaluate the increment section
		result = tcl_eval(tcl, tcl_string(increment), tcl_length(increment) + 1);

		if (result != FNORMAL) {
			tcl_free(init);
			tcl_free(condition);
			tcl_free(increment);
			tcl_free(loop);
			return FERROR;
		}
	}
}

/**
 * image x y w h path
 *
 * Example: image 10 10 64 96 "/BLING/AND!XOR.RAW"
 */
static int __tcl_image(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_x = tcl_list_at(args, 1);
	tcl_value_t *tcl_y = tcl_list_at(args, 2);
	tcl_value_t *tcl_w = tcl_list_at(args, 3);
	tcl_value_t *tcl_h = tcl_list_at(args, 4);
	tcl_value_t *tcl_path = tcl_list_at(args, 5);

	int16_t x = tcl_int(tcl_x);
	int16_t y = tcl_int(tcl_y);
	int16_t w = tcl_int(tcl_w);
	int16_t h = tcl_int(tcl_h);
	char *path = (char *) tcl_string(tcl_path);

	util_gfx_draw_raw_file(path, x, y, w, h, NULL, false, NULL);

	tcl_free(tcl_x);
	tcl_free(tcl_y);
	tcl_free(tcl_w);
	tcl_free(tcl_h);
	tcl_free(tcl_path);

	return FNORMAL;
}

/**
 * incr i
 *
 * Equivalent to i++ or set i [+ $i 1]
 */
static int __tcl_incr(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *var_name = tcl_list_at(args, 1);

	int len = 14 + (2 * strlen(var_name));
	char cmd[len + 1];
	memset(cmd, '\0', len + 1);
	sprintf(cmd, "set %s [+ $%s 1]", var_name, var_name);
	int result = tcl_eval(tcl, cmd, len);
	tcl_free(var_name);

	return result;
}

static int __tcl_io_read(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_io = tcl_list_at(args, 1);
	uint8_t io = tcl_int(tcl_io);

	if (io > IO_PIN_MAX) {
		char message[32];
		sprintf(message, "IO %d out of range. > %d", io, IO_PIN_MAX);
		mbp_ui_error(message);

		tcl_free(tcl_io);
		return FERROR;
	}

	char r_str[2];
	nrf_gpio_cfg_input(m_io_pins[io], NRF_GPIO_PIN_NOPULL);
	if (nrf_gpio_pin_read(m_io_pins[io]) > 0) {
		r_str[0] = '1';
	} else {
		r_str[0] = '0';
	}
	r_str[1] = 0;

	tcl_free(tcl_io);
	return tcl_result(tcl, FNORMAL, tcl_alloc(r_str, strlen(r_str)));
}

/**
 * Write to digital IO
 *
 * Example: 	io_write 1 low;	Sets IO 1 high
 * 				io_write 0 low;	Sets IO 0 low
 */
static int __tcl_io_write(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_io = tcl_list_at(args, 1);
	tcl_value_t *tcl_value = tcl_list_at(args, 2);

	uint8_t io = tcl_int(tcl_io);
	char *value = (char *) tcl_string(tcl_value);

	if (io > IO_PIN_MAX) {
		char message[32];
		sprintf(message, "IO %d out of range. > %d", io, IO_PIN_MAX);
		mbp_ui_error(message);

		tcl_free(tcl_io);
		tcl_free(tcl_value);
		return FERROR;
	}

	if (strcasecmp("HIGH", value) == 0) {
		nrf_gpio_pin_set(m_io_pins[io]);
	} else if (strcasecmp("LOW", value) == 0) {
		nrf_gpio_pin_clear(m_io_pins[io]);
	} else {
		char message[32];
		sprintf(message, "Invalid pin value. Use HIGH or LOW.");
		mbp_ui_error(message);

		tcl_free(tcl_io);
		tcl_free(tcl_value);
		return FERROR;
	}

	tcl_free(tcl_io);
	tcl_free(tcl_value);
	return FNORMAL;
}

static int __tcl_led_set(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_index = tcl_list_at(args, 1);
	tcl_value_t *tcl_r = tcl_list_at(args, 2);
	tcl_value_t *tcl_g = tcl_list_at(args, 3);
	tcl_value_t *tcl_b = tcl_list_at(args, 4);

	uint8_t index = tcl_int(tcl_index);
	uint8_t r = tcl_int(tcl_r);
	uint8_t g = tcl_int(tcl_g);
	uint8_t b = tcl_int(tcl_b);

	util_led_set(index, r, g, b);
	util_led_show();

	tcl_free(tcl_r);
	tcl_free(tcl_g);
	tcl_free(tcl_b);
	return FNORMAL;
}

static int __tcl_led_set_hsv(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_index = tcl_list_at(args, 1);
	tcl_value_t *tcl_h = tcl_list_at(args, 2);
	tcl_value_t *tcl_s = tcl_list_at(args, 3);
	tcl_value_t *tcl_v = tcl_list_at(args, 4);

	uint8_t index = tcl_int(tcl_index);
	float h = (float) tcl_int(tcl_h);
	float s = (float) tcl_int(tcl_s);
	float v = (float) tcl_int(tcl_v);

	if (h > 100)
		h = 100;
	if (s > 100)
		s = 100;
	if (v > 100)
		v = 100;

	h = h / 100.0;
	s = s / 100.0;
	v = v / 100.0;

	uint32_t
	rgb = util_led_hsv_to_rgb(h, s, v);
	util_led_set_rgb(index, rgb);
	util_led_show();

	tcl_free(tcl_h);
	tcl_free(tcl_s);
	tcl_free(tcl_v);
	return FNORMAL;
}

static int __tcl_line(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_x1 = tcl_list_at(args, 1);
	tcl_value_t *tcl_y1 = tcl_list_at(args, 2);
	tcl_value_t *tcl_x2 = tcl_list_at(args, 3);
	tcl_value_t *tcl_y2 = tcl_list_at(args, 4);
	tcl_value_t *tcl_color = tcl_list_at(args, 5);

	int16_t x1 = tcl_int(tcl_x1);
	int16_t y1 = tcl_int(tcl_y1);
	int16_t x2 = tcl_int(tcl_x2);
	int16_t y2 = tcl_int(tcl_y2);
	uint16_t color = __tcl_color(tcl_color);

	util_gfx_draw_line(x1, y1, x2, y2, color);

	tcl_free(tcl_x1);
	tcl_free(tcl_y1);
	tcl_free(tcl_x2);
	tcl_free(tcl_y2);
	tcl_free(tcl_color);
	return FNORMAL;
}

static int __tcl_pixel(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_x = tcl_list_at(args, 1);
	tcl_value_t *tcl_y = tcl_list_at(args, 2);
	tcl_value_t *tcl_color = tcl_list_at(args, 3);

	int16_t x = tcl_int(tcl_x);
	int16_t y = tcl_int(tcl_y);
	uint16_t color = __tcl_color(tcl_color);

	util_gfx_set_pixel(x, y, color);

	tcl_free(tcl_x);
	tcl_free(tcl_y);
	tcl_free(tcl_color);
	return FNORMAL;
}

//static void __tcl_play_frame_callback(uint8_t frame, void *data) {
//	tcl_context_t *p_context = (tcl_context_t *) data;
//
//	char *body = (char *) p_context->p_data;
//	int result = tcl_eval(p_context->p_tcl, body, strlen(body) + 1);
//	switch (result) {
//	case FNORMAL:
//		//do nothing
//		break;
//	case FBREAK:
//		m_quit = true;
//		break;
//	default:
//		m_quit = true;
//		break;
//	}
//}

static int __tcl_play(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_path = tcl_list_at(args, 1);
	tcl_value_t *tcl_body = tcl_list_at(args, 2);

	char * path = (char *) tcl_string(tcl_path);
	m_quit = false;
	util_gfx_draw_raw_file(path, 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);

	tcl_free(tcl_body);
	tcl_free(tcl_path);
	return FNORMAL;
}

/**
 * Print text to the screen
 *
 * Example: print 10 64 "Hello World!"
 */
static int __tcl_print(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_x = tcl_list_at(args, 1);
	tcl_value_t *tcl_y = tcl_list_at(args, 2);
	tcl_value_t *tcl_text = tcl_list_at(args, 3);

	int16_t x = tcl_int(tcl_x);
	int16_t y = tcl_int(tcl_y);
	char *text = (char *) tcl_string(tcl_text);

	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_color(m_color);
	util_gfx_set_cursor(x, y);
	util_gfx_print(text);

	tcl_free(tcl_x);
	tcl_free(tcl_y);
	tcl_free(tcl_text);
	return FNORMAL;
}

static int __tcl_rand(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_max = tcl_list_at(args, 1);
	uint32_t max = tcl_int(tcl_max);

	uint32_t r = util_math_rand32_max(max);
	char r_str[16];
	sprintf(r_str, "%ld", r);
	return tcl_result(tcl, FNORMAL, tcl_alloc(r_str, strlen(r_str)));
}

static int __tcl_rect(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_x = tcl_list_at(args, 1);
	tcl_value_t *tcl_y = tcl_list_at(args, 2);
	tcl_value_t *tcl_w = tcl_list_at(args, 3);
	tcl_value_t *tcl_h = tcl_list_at(args, 4);
	tcl_value_t *tcl_color = tcl_list_at(args, 5);

	int16_t x = tcl_int(tcl_x);
	int16_t y = tcl_int(tcl_y);
	int16_t w = tcl_int(tcl_w);
	int16_t h = tcl_int(tcl_h);
	uint16_t color = __tcl_color(tcl_color);

	util_gfx_draw_rect(x, y, w, h, color);

	tcl_free(tcl_x);
	tcl_free(tcl_y);
	tcl_free(tcl_w);
	tcl_free(tcl_h);
	tcl_free(tcl_color);
	return FNORMAL;
}

/**
 * Scrolls text across the screen once
 */
static int __tcl_scroll(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_text = tcl_list_at(args, 1);
	char *text = (char *) tcl_string(tcl_text);

	mbp_bling_scroll(text, false);

	tcl_free(tcl_text);
	return FNORMAL;
}

/**
 * Returns the state of the right button
 */
static int __tcl_set_color(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_color = tcl_list_at(args, 1);
	m_color = __tcl_color(tcl_color);
	tcl_free(tcl_color);
	return FNORMAL;
}

static int __tcl_triangle(struct tcl *tcl, tcl_value_t *args, void *arg) {
	tcl_value_t *tcl_x0 = tcl_list_at(args, 1);
	tcl_value_t *tcl_y0 = tcl_list_at(args, 2);
	tcl_value_t *tcl_x1 = tcl_list_at(args, 3);
	tcl_value_t *tcl_y1 = tcl_list_at(args, 4);
	tcl_value_t *tcl_x2 = tcl_list_at(args, 5);
	tcl_value_t *tcl_y2 = tcl_list_at(args, 6);
	tcl_value_t *tcl_color = tcl_list_at(args, 7);

	int16_t x0 = tcl_int(tcl_x0);
	int16_t y0 = tcl_int(tcl_y0);
	int16_t x1 = tcl_int(tcl_x1);
	int16_t y1 = tcl_int(tcl_y1);
	int16_t x2 = tcl_int(tcl_x2);
	int16_t y2 = tcl_int(tcl_y2);
	uint16_t color = __tcl_color(tcl_color);

	util_gfx_draw_triangle(x0, y0, x1, y1, x2, y2, color);

	tcl_free(tcl_x0);
	tcl_free(tcl_y0);
	tcl_free(tcl_x1);
	tcl_free(tcl_y1);
	tcl_free(tcl_x2);
	tcl_free(tcl_y2);
	tcl_free(tcl_color);
	return FNORMAL;
}

/**
 * Clears buttons state.
 * Example: button_clear;
 */
static int __tcl_button_clear(struct tcl *tcl, tcl_value_t *args, void *arg) {
	util_button_clear();
	return FNORMAL;
}

/**
 * Returns buttons state, just apply masks!
 * Example: button_state;
 */
static int __tcl_button_state(struct tcl *tcl, tcl_value_t *args, void *arg) {
	char r_str[4];
	uint8_t button = util_button_state();
	sprintf(r_str, "%d", button);
	return tcl_result(tcl, FNORMAL, tcl_alloc(r_str, strlen(r_str)));
}

/**
 * Waits for button press
 * Example: button_wait;
 */
static int __tcl_button_wait(struct tcl *tcl, tcl_value_t *args, void *arg) {
	util_button_wait();
	return FNORMAL;
}

/**
 * Returns the state of the up button
 */
static int __tcl_up(struct tcl *tcl, tcl_value_t *args, void *arg) {
	char r_str[2];
	sprintf(r_str, "%d", util_button_up());
	return tcl_result(tcl, FNORMAL, tcl_alloc(r_str, strlen(r_str)));
}

/**
 * Returns the state of the down button
 */
static int __tcl_down(struct tcl *tcl, tcl_value_t *args, void *arg) {
	char r_str[2];
	sprintf(r_str, "%d", util_button_down());
	return tcl_result(tcl, FNORMAL, tcl_alloc(r_str, strlen(r_str)));
}
/**
 * Returns the state of the left button
 */
static int __tcl_left(struct tcl *tcl, tcl_value_t *args, void *arg) {
	char r_str[2];
	sprintf(r_str, "%d", util_button_left());
	return tcl_result(tcl, FNORMAL, tcl_alloc(r_str, strlen(r_str)));
}
/**
 * Returns the state of the right button
 */
static int __tcl_right(struct tcl *tcl, tcl_value_t *args, void *arg) {
	char r_str[2];
	sprintf(r_str, "%d", util_button_right());
	return tcl_result(tcl, FNORMAL, tcl_alloc(r_str, strlen(r_str)));
}
/**
 * Returns the state of the action button
 */
static int __tcl_action(struct tcl *tcl, tcl_value_t *args, void *arg) {
	char r_str[2];
	sprintf(r_str, "%d", util_button_action());
	return tcl_result(tcl, FNORMAL, tcl_alloc(r_str, strlen(r_str)));
}

static void __tcl_menu_callback(void *data) {
	char *filename = (char *) data;
	mbp_tcl_exec_file(filename);
}

void mbp_tcl_exec(char *p_code) {
	struct tcl tcl;

//	bool f = false;
//	bool t = true;

	tcl_init(&tcl);
	tcl_register(&tcl, "chr", &__tcl_chr, 2, NULL);
	tcl_register(&tcl, "cls", &__tcl_cls, 1, NULL);
	tcl_register(&tcl, "delay", &__tcl_delay, 2, NULL);
	tcl_register(&tcl, "for", &__tcl_for, 5, NULL);
	tcl_register(&tcl, "incr", &__tcl_incr, 2, NULL);
	tcl_register(&tcl, "io_read", &__tcl_io_read, 2, NULL);
	tcl_register(&tcl, "io_write", &__tcl_io_write, 3, NULL);
	tcl_register(&tcl, "led_set", &__tcl_led_set, 5, NULL);
	tcl_register(&tcl, "led_set_hsv", &__tcl_led_set_hsv, 5, NULL);
//	tcl_register(&tcl, "loop", &__tcl_play, 3, &t);
	tcl_register(&tcl, "play", &__tcl_play, 2, NULL);
	tcl_register(&tcl, "print", &__tcl_print, 4, NULL);
	tcl_register(&tcl, "rand", &__tcl_rand, 2, NULL);
	tcl_register(&tcl, "scroll", &__tcl_scroll, 2, NULL);
	tcl_register(&tcl, "set_color", &__tcl_set_color, 2, NULL);

	//Drawing
	tcl_register(&tcl, "circle", &__tcl_circle, 5, NULL);
	tcl_register(&tcl, "image", &__tcl_image, 6, NULL);
	tcl_register(&tcl, "line", &__tcl_line, 6, NULL);
	tcl_register(&tcl, "rect", &__tcl_rect, 6, NULL);
	tcl_register(&tcl, "fill_rect", &__tcl_fill_rect, 6, NULL);
	tcl_register(&tcl, "pixel", &__tcl_pixel, 4, NULL);
	tcl_register(&tcl, "triangle", &__tcl_triangle, 8, NULL);

	//Buttons
	tcl_register(&tcl, "up", &__tcl_up, 1, NULL);
	tcl_register(&tcl, "down", &__tcl_down, 1, NULL);
	tcl_register(&tcl, "left", &__tcl_left, 1, NULL);
	tcl_register(&tcl, "right", &__tcl_right, 1, NULL);
	tcl_register(&tcl, "action", &__tcl_action, 1, NULL);
	tcl_register(&tcl, "button_clear", &__tcl_button_clear, 1, NULL);
	tcl_register(&tcl, "button_state", &__tcl_button_state, 1, NULL);
	tcl_register(&tcl, "button_wait", &__tcl_button_wait, 1, NULL);

	//Setup IO pins for badges with WS2812B
	if (!util_led_has_apa102()) {
		for (uint8_t i = 0; i < IO_PIN_MAX; i++) {
			nrf_gpio_cfg_output(m_io_pins[i]);
		}
	}

	//Setup the screen
	util_gfx_set_font(FONT_SMALL);
	util_gfx_set_color(COLOR_WHITE);
	mbp_ui_cls();

	//Execute the TCL script
	if (tcl_eval(&tcl, p_code, strlen(p_code)) != FERROR) {
		const char *result = tcl_string(tcl.result);
		UNUSED_VARIABLE(result);
	}
	tcl_destroy(&tcl);

	util_button_clear();
}

void mbp_tcl_exec_file(char *filename) {
	FIL file;
	FRESULT result;
	UINT count;

	//Prepend full path
	char full_path[strlen(filename) + 6];
	sprintf(full_path, "TCL/%s", filename);

	uint32_t fsize = util_sd_file_size(full_path);

	//Limit size of file to 20kb
	if (fsize > 20000) {
		mbp_ui_error("TCL file to large.");
		return;
	}

	char tcl[fsize];

	result = f_open(&file, full_path, FA_READ | FA_OPEN_EXISTING);

	//Try once more after recovering
	if (result != FR_OK) {
		util_sd_recover();
		result = f_open(&file, full_path, FA_READ | FA_OPEN_EXISTING);
	}

	//Something bad happened
	if (result != FR_OK) {
		mbp_ui_error("Could not open file");
		return;
	}
	f_read(&file, tcl, fsize, &count);
	f_close(&file);

	mbp_tooth_eye_stop();
	//Clear out app_scheduler
	app_sched_execute();
	util_led_clear();

	mbp_tcl_exec(tcl);
	mbp_tooth_eye_start();
	util_led_clear();
	util_gfx_invalidate();
}

void mbp_tcl_menu(void *data) {
	menu_t menu;
	menu_item_t tcl_menu_items[100];
	menu.items = tcl_menu_items;
	FRESULT result;
	DIR dir;
	static FILINFO fno;

	menu.count = 0;
	menu.top = 0;
	menu.selected = 0;
	menu.title = "TCL";

	result = f_opendir(&dir, "TCL"); /* Open the directory */
	if (result == FR_OK) {
		for (;;) {
			result = f_readdir(&dir, &fno); /* Read a directory item */
			if (result != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR) { /* It is a directory */
				//ignore
			} else { /* It is a file. */
				char *ext = strrchr(fno.fname, '.') + 1;

				if (strcmp(ext, "TCL") == 0) {

					menu_item_t item;
					item.callback = &__tcl_menu_callback;
					item.icon = NULL;
					item.preview = NULL;
					item.text = (char *) malloc(16);
					item.data = (char *) malloc(16);

					snprintf(item.text, ext - fno.fname, "%s", fno.fname);
					sprintf(item.data, "%s", fno.fname);
					tcl_menu_items[menu.count++] = item;
				}
			}
		}
		f_closedir(&dir);
	}

	mbp_sort_menu(&menu);
	mbp_submenu(&menu);

	//Cleanup
	for (uint16_t i = 0; i < menu.count; i++) {
		free(tcl_menu_items[i].data);
		free(tcl_menu_items[i].text);
	}
}
