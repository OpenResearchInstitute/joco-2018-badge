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

#include "../system.h"

#define CIGAR_EYES_TIME_MS			10
#define CIGAR_HUE_LOW				0.05
#define CIGAR_HUE_HIGH				0.17
#define CIGAR_HUE_STEP				0.0005

#define EYE_HUE_STEP				.00015
#define SCROLL_CHAR_WIDTH			16
#define SCROLL_CHAR_HEIGHT			31
#define SCROLL_SPEED				-2
#define SCROLL_TIME_MS				20

APP_TIMER_DEF(m_cigar_timer);
APP_TIMER_DEF(m_scroll_led_timer);

static float m_cigar_hue = CIGAR_HUE_LOW;
static float m_cigar_hue_step = CIGAR_HUE_STEP;
static float m_eye_hue = 0;
static bool m_cigar_eyes_running = false;

typedef struct {
	uint8_t index;
	float hue;
} bling_defag_state_t;

static void __rgb_file_callback(uint8_t frame, void *p_data) {
	util_led_play_rgb_frame((led_anim_t *) p_data);
}

/**
 * Generic call back that runs the leds in sparkle mode
 */
static void __led_sparkle(uint8_t frame, void *p_data) {
	uint8_t *data = (uint8_t *) p_data;
	uint8_t i = util_math_rand8_max(LED_COUNT);
	util_led_set_rgb(i, LED_COLOR_WHITE);
	util_led_show();

	nrf_delay_ms(20);

	//Unpack hue as a float
	float hue = ((float) *data / 10.0);

	util_led_set_rgb(i, util_led_hsv_to_rgb(hue, 1, 1));
	util_led_show();

	hue += 0.1;
	if (hue >= 1) {
		hue = 0;
	}

	//Pack the hue
	*data = (uint8_t) (hue * 10.0);
}

//static void __led_sparkle_single(uint8_t frame, void *p_data) {
//	uint8_t *data = (uint8_t *) p_data;
//	uint8_t i = util_math_rand8_max(LED_COUNT);
//	util_led_clear();
//
//	util_led_set(i, 255, 255, 255);
//	util_led_show();
//
//	nrf_delay_ms(20);
//
//	//Unpack hue as a float
//	float hue = ((float) *data / 10.0);
//
//	util_led_set_rgb(i, util_led_hsv_to_rgb(hue, 1, 1));
//	util_led_show();
//
//	hue += 0.1;
//	if (hue >= 1) {
//		hue = 0;
//	}
//
//	//Pack the hue
//	*data = (uint8_t) (hue * 10.0);
//}

static void __spectrum_analyzer_callback(uint8_t frame, void *p_data) {

	if ((frame % 2) == 0) {
		uint32_t green = util_led_to_rgb(0, 255, 0);
		uint32_t yellow = util_led_to_rgb(255, 255, 0);
		uint32_t red = util_led_to_rgb(255, 0, 0);

		uint32_t colors[] = { green, yellow, red };

		uint8_t rows[LED_MATRIX_H][LED_MATRIX_W] = LED_MATRIX_ROWS;

		util_led_set_all(0, 0, 0);
		for (uint8_t x = 0; x < LED_MATRIX_W; x++) {
			uint8_t h = util_math_rand8_max(LED_MATRIX_H) + 1;
			for (uint8_t y = 0; y < h; y++) {
				util_led_set_rgb(rows[LED_MATRIX_H - y - 1][x], colors[y]);
			}
		}
		util_led_show();
	}
}

static void __mbp_bling_rainbow_eyes_callback(uint8_t frame, void *data) {
	uint8_t *p_data = (uint8_t *) data;
	float hue = ((float) *p_data) / 100.0;

	uint32_t rgb = util_led_hsv_to_rgb(hue, 1.0, 1.0);
	util_led_set_rgb(LED_LEFT_EYE_INDEX, rgb);
	util_led_set_rgb(LED_RIGHT_EYE_INDEX, rgb);
	util_led_show();

	hue -= .01;
	if (hue <= 0) {
		hue = .99;
	}

	*p_data = (uint8_t) (hue * 100.0);
}

static void __mbp_bling_backer_april_callback(uint8_t frame, void *p_data) {
	if ((frame % 2) == 0) {
		uint32_t neongreen = util_led_to_rgb(0, 199, 0);
		uint32_t blue = util_led_to_rgb(0, 72, 255);
		uint32_t magenta = util_led_to_rgb(166, 45, 170);

		uint32_t colors[] = { neongreen, blue, magenta };

		uint8_t rows[LED_MATRIX_H][LED_MATRIX_W] = LED_MATRIX_ROWS;

		util_led_set_all(0, 0, 0);
		for (uint8_t x = 0; x < LED_MATRIX_W; x++) {
			uint8_t h = util_math_rand8_max(LED_MATRIX_H) + 1;
			//Set the grill
			for (uint8_t y = 0; y < h; y++) {
				util_led_set_rgb(rows[LED_MATRIX_H - y - 1][x], colors[y]);
			}

			//Set the Eyes
			if ((h == LED_MATRIX_H) && (x == 0)) { //The first column & the column is high
				util_led_set_rgb(12, magenta);
			}
			if ((h == LED_MATRIX_H) && (x == 3)) { //The last column & the column is high
				util_led_set_rgb(13, magenta);
			}

			//Set the Cig
			if ((h >= (LED_MATRIX_H - 1)) && (x == 3)) {
				util_led_set_rgb(14, blue);
			}
		}
		util_led_show();
	}
}

void mbp_bling_backer_april() {
	util_led_clear();
	util_gfx_draw_raw_file("BLING/BACKERS/KSAPRIL.RAW", 0, 0, 128, 128, &__mbp_bling_backer_april_callback, true, NULL);
}

static void __mbp_bling_backer_btcctf_callback(uint8_t frame, void *p_data) {
	uint8_t i = util_math_rand8_max(LED_COUNT);
	util_led_set_rgb(i, LED_COLOR_WHITE);
	util_led_show();

	nrf_delay_ms(20);

	if (frame % 2 == 0) {
		util_led_set_rgb(i, LED_COLOR_YELLOW);
	}
	else {
		util_led_set_rgb(i, LED_COLOR_PURPLE);
	}

	util_led_show();
}

void mbp_bling_backer_btcctf() {
	util_led_clear();
	util_gfx_draw_raw_file("BLING/BACKERS/KSBTCCTF.RAW", 0, 0, 128, 128, &__mbp_bling_backer_btcctf_callback, true, NULL);
}

static void __mbp_bling_backer_abraxas3d_callback(uint8_t frame, void *p_data) {
	uint32_t indigo = LED_COLOR_INDIGO;
	uint32_t coral = LED_COLOR_CORAL;
	uint32_t gold = LED_COLOR_GOLD;
	uint32_t yellow = LED_COLOR_YELLOW;
	uint32_t white = LED_COLOR_WHITE;

	//Unpack cycle
	uint8_t *data = (uint8_t *) p_data;
	uint8_t cycle = (uint8_t) *data;

	if (cycle == 0) {
		//Move Column 1 & 4 Up Vertically
		util_led_set_rgb(13, indigo);
		util_led_set_rgb(0, gold);
		util_led_set_rgb(4, coral);
		util_led_set_rgb(8, yellow);
		util_led_set_rgb(12, indigo);
		util_led_set_rgb(3, gold);
		util_led_set_rgb(7, coral);
		util_led_set_rgb(11, yellow);

		//1-5-9 / 2-6-10
		util_led_set(1, 0, 0, 0); //CLEAR
		util_led_set(2, 0, 0, 0); //CLEAR
		util_led_set(6, 0, 0, 0); //CLEAR
		util_led_set(10, 0, 0, 0); //CLEAR
		util_led_set(9, 0, 0, 0); //CLEAR
		util_led_set(5, 0, 0, 0); //CLEAR
		util_led_set_rgb(1, indigo);
		util_led_set_rgb(5, indigo);
		util_led_set_rgb(9, indigo);
		util_led_set_rgb(14, indigo); //Set the Cig
	}

	else if (cycle == 1) {
		//Move Column 1 & 4 Up Vertically
		util_led_set_rgb(13, gold);
		util_led_set_rgb(0, coral);
		util_led_set_rgb(4, yellow);
		util_led_set_rgb(8, indigo);
		util_led_set_rgb(12, gold);
		util_led_set_rgb(3, coral);
		util_led_set_rgb(7, yellow);
		util_led_set_rgb(11, indigo);

		//1-5-9 / 2-6-10
		util_led_set(1, 0, 0, 0); //CLEAR
		util_led_set(2, 0, 0, 0); //CLEAR
		util_led_set(6, 0, 0, 0); //CLEAR
		util_led_set(10, 0, 0, 0); //CLEAR
		util_led_set(9, 0, 0, 0); //CLEAR
		util_led_set(5, 0, 0, 0); //CLEAR
		util_led_set_rgb(1, white);
		util_led_set_rgb(5, white);
		util_led_set_rgb(9, white);

		util_led_set_rgb(14, white); //Set the Cig
	}

	else if (cycle == 2) {
		//Move Column 1 & 4 Up Vertically
		util_led_set_rgb(13, coral);
		util_led_set_rgb(0, yellow);
		util_led_set_rgb(4, indigo);
		util_led_set_rgb(8, gold);
		util_led_set_rgb(12, coral);
		util_led_set_rgb(3, yellow);
		util_led_set_rgb(7, indigo);
		util_led_set_rgb(11, gold);

		//1-5-9 / 2-6-10
		util_led_set(1, 0, 0, 0); //CLEAR
		util_led_set(2, 0, 0, 0); //CLEAR
		util_led_set(6, 0, 0, 0); //CLEAR
		util_led_set(10, 0, 0, 0); //CLEAR
		util_led_set(9, 0, 0, 0); //CLEAR
		util_led_set(5, 0, 0, 0); //CLEAR
		util_led_set_rgb(2, indigo);
		util_led_set_rgb(6, indigo);
		util_led_set_rgb(10, indigo);
		util_led_set_rgb(14, indigo); //Set the Cig
	}

	else {
		//Move Column 1 & 4 Up Vertically
		util_led_set_rgb(13, yellow);
		util_led_set_rgb(0, indigo);
		util_led_set_rgb(4, gold);
		util_led_set_rgb(8, coral);
		util_led_set_rgb(12, yellow);
		util_led_set_rgb(3, indigo);
		util_led_set_rgb(7, gold);
		util_led_set_rgb(11, coral);

		//1-5-9 / 2-6-10
		util_led_set(1, 0, 0, 0); //CLEAR
		util_led_set(2, 0, 0, 0); //CLEAR
		util_led_set(6, 0, 0, 0); //CLEAR
		util_led_set(10, 0, 0, 0); //CLEAR
		util_led_set(9, 0, 0, 0); //CLEAR
		util_led_set(5, 0, 0, 0); //CLEAR
		util_led_set_rgb(2, white);
		util_led_set_rgb(6, white);
		util_led_set_rgb(10, white);
		util_led_set_rgb(14, white); //Set the Cig
	}

	util_led_show();
	nrf_delay_ms(30);

	//Increment the Cycle
	if (cycle < 3) {
		cycle++;
	}
	else {
		cycle = 0;
	}

	//Pack the cycle
	*data = (uint8_t) cycle;
}

void mbp_bling_backer_abraxas3d(void *data) {
	uint8_t hue = 0;
	util_led_clear();
	util_gfx_draw_raw_file("BLING/BACKERS/KSABRAXA.RAW", 0, 0, 128, 128, &__mbp_bling_backer_abraxas3d_callback, true, &hue);
}

void mbp_bling_led_rainbow_callback(uint8_t frame, void *p_data) {
	//Unpack the data
	uint8_t *data = (uint8_t *) p_data;
	float hue = (float) *data / 100.0;

	//Define led matrix
	uint8_t led_map[LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT] = LED_MATRIX_FULL_MAPPING
	;

	for (uint8_t row = 0; row < LED_MATRIX_FULL_ROW_COUNT; row++) {
		float rowhue = hue + (row * .08);
		if (rowhue >= 1)
			rowhue--;

		uint32_t color = util_led_hsv_to_rgb(rowhue, 1, .7);
		for (uint8_t i = 0; i < LED_MATRIX_FULL_COL_COUNT; i++) {
			uint8_t index = (row * LED_MATRIX_FULL_COL_COUNT) + i;
			if (led_map[index] < LED_COUNT) {
				util_led_set_rgb(led_map[index], color);
			}
		}

		//Increment row and color and loop around
		hue += .01;
		if (hue >= 1)
			hue = 0;
	}

	util_led_show();

	//Pack the data and store for next time
	*data = (int) (hue * 100);
}

void mbp_bling_backer_cybersulu() {
	uint8_t cycle = 0;
	util_led_clear();
	util_gfx_draw_raw_file("BLING/BACKERS/KSCYSULU.RAW", 0, 0, 128, 128, &mbp_bling_led_rainbow_callback, true, &cycle);
}

static void __mbp_bling_backer_sol_callback(uint8_t frame, void *p_data) {
	uint8_t i = util_math_rand8_max(LED_COUNT);
	uint8_t j = util_math_rand8_max(LED_COUNT);
	util_led_set(i, 255, 255, 255);
	util_led_set(j, 255, 255, 255);
	util_led_show();

	nrf_delay_ms(20);

	//Super Fourth of July 'Merica Sparkle
	if (frame % 2 == 0) {
		util_led_set(i, 255, 0, 0); //RED
		util_led_set(j, 0, 0, 255); //BLUE
	}
	else {
		util_led_set(j, 255, 0, 0); //RED
		util_led_set(i, 0, 0, 255); //BLUE
	}

	util_led_show();
}

void mbp_bling_backer_sol(void *data) {
	util_led_clear();
	util_gfx_draw_raw_file("BLING/BACKERS/KSSOL.RAW", 0, 0, 128, 128, &__mbp_bling_backer_sol_callback, true, NULL);
}

void mbp_bling_backer_credits() {
	util_led_clear();
	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/GOLD.RGB", &anim);
	util_gfx_draw_raw_file("BLING/BACKERS/BACKERS.RAW", 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
}

void mbp_bling_badgers() {
	uint8_t hue = 0;
	util_led_clear();
	util_gfx_draw_raw_file("BLING/AND!XOR/BADGERS.RAW", 0, 0, 128, 128, &__mbp_bling_rainbow_eyes_callback, true, &hue);
}

static void __led_bender(uint8_t f_unused, void *p_data) {
	uint8_t mouth_closed[] = { 4, 5, 6, 7 };
	uint8_t mouth_open[] = { 1, 2, 4, 7, 9, 10 };
	uint8_t frame = *((uint8_t *) p_data);

	//Clear all colors
	for (uint8_t i = 0; i < LED_COUNT; i++) {
		util_led_set_rgb(i, LED_COLOR_BLACK);
	}

	//Compute and set the eye colors
	float eye_hue = 0.025 * (float) frame;
	uint32_t eye_color = util_led_hsv_to_rgb(eye_hue, 1, 1);
	util_led_set_rgb(LED_LEFT_EYE_INDEX, eye_color);
	util_led_set_rgb(LED_RIGHT_EYE_INDEX, eye_color);

	//Compute and set the cig color
	float cig_hue;
	if (frame < 20) {
		cig_hue = .00625 * (float) frame;
	} else {
		cig_hue = .00625 * (float) (40 - frame);
	}
	uint32_t cig_color = util_led_hsv_to_rgb(cig_hue, 1, 1);
	util_led_set_rgb(LED_CIGAR_INDEX, cig_color);

	//Mouth
	if (frame < 20) {
		for (uint8_t i = 0; i < 6; i++) {
			util_led_set_rgb(mouth_open[i], LED_COLOR_WHITE);
		}
	} else {
		for (uint8_t i = 0; i < 4; i++) {
			util_led_set_rgb(mouth_closed[i], LED_COLOR_WHITE);
		}
	}

	//latch
	util_led_show();

	frame = (frame + 1) % 40;
	*((uint8_t *) p_data) = frame;
}

static void __led_chase_cw_callback(uint8_t frame, void *p_data) {
	uint8_t *p_index = (uint8_t *) p_data;
	uint8_t order[] = { 0, 1, 2, 3, 7, 11, 10, 9, 8, 4 };

	util_led_set(order[*p_index], 255, 0, 0);
	util_led_show();
	util_led_set(order[*p_index], 0, 0, 0);

	(*p_index)++;
	if (*p_index > 9) {
		*p_index = 0;
	}
}

extern void __led_hue_cycle(uint8_t frame, void *p_data) {
	uint8_t *p_hue = (uint8_t *) p_data;
	float hue = (float) (*p_hue) / 100.0;
	uint32_t color = util_led_hsv_to_rgb(hue, .6, .8);
	util_led_set_all_rgb(color);
	util_led_show();

	hue += .015;
	if (hue >= 1) {
		hue = 0;
	}
	*p_hue = (hue * 100.0);
}

void mbp_bling_bender() {
	util_led_clear();
	uint8_t index = 0;
	uint8_t count = 5;

	//Unlock more bender bling
	if ((mbp_state_unlock_get() & UNLOCK_MASK_CARD) > 0) {
		count = 8;
	}

	char *modes[] = { "BLING/AND!XOR/BENDER8.RAW", "BLING/AND!XOR/BENDER9.RAW", "BLING/AND!XOR/BENDERA.RAW", "BLING/AND!XOR/BENDERB.RAW",
			"BLING/AND!XOR/BENDERC.RAW", "BLING/AND!XOR/BENDERD.RAW", "BLING/AND!XOR/BENDERE.RAW", "BLING/AND!XOR/BENDERH.RAW" };
	uint8_t button = 0;

	util_led_clear();

	//If anything other than left button is pressed cycle modes
	while ((button & BUTTON_MASK_LEFT) == 0) {
		uint8_t frame = 0;
		button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__led_bender, true, &frame);
		index = (index + 1) % count;
	}
}

void mbp_bling_damon() {
	util_led_clear();
	uint8_t index = 0;
	uint8_t count = 4;

	char *modes[] = { "BLING/AND!XOR/DAMON1.RAW", "BLING/AND!XOR/DAMON2.RAW", "BLING/AND!XOR/DAMON3.RAW", "BLING/AND!XOR/DAMON4.RAW" };
	uint8_t button = 0;

	util_led_clear();

	//If anything other than left button is pressed cycle modes
	while ((button & BUTTON_MASK_LEFT) == 0) {
		uint8_t hue = 0;
		button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__led_chase_cw_callback, true, &hue);
		index = (index + 1) % count;
	}
}

void mbp_bling_flames(void *data) {
	util_led_clear();
	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/FLAMES.RGB", &anim);
	util_gfx_draw_raw_file("BLING/AND!XOR/FLAMES.RAW", 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
}

void mbp_bling_meatspin() {
	util_led_clear();
	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/AND!XOR/MEATSPIN.RGB", &anim);
	util_gfx_draw_raw_file("BLING/AND!XOR/MEATSPIN.RAW", 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
}

void mbp_bling_hack_time() {
	util_led_clear();
	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/PINKBLUE.RGB", &anim);
	util_gfx_draw_raw_file("BLING/AND!XOR/HACKTIME.RAW", 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
}

void mbp_bling_illusion() {
	uint8_t index = 0;
	uint8_t count = 6;

	char *modes[] = { "BLING/AND!XOR/ILLUS3.RAW", "BLING/AND!XOR/ILLUS2.RAW", "BLING/AND!XOR/ILLUS1.RAW", "BLING/AND!XOR/ILLUS4.RAW",
			"BLING/AND!XOR/ILLUS5.RAW", "BLING/AND!XOR/ILLUS6.RAW" };
	uint8_t button = 0;

	util_led_clear();

	//If anything other than left button is pressed cycle modes
	while ((button & BUTTON_MASK_LEFT) == 0) {
		uint8_t hue = 0;
		button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__led_hue_cycle, true, &hue);
		index = (index + 1) % count;
	}
}

void mbp_bling_whats_up() {
	util_led_clear();
	uint8_t hue = 0; //hue is normally a float 0 to 1, pack it in an 8 bit int
	util_gfx_draw_raw_file("BLING/AND!XOR/HEMANHEY.RAW", 0, 0, 128, 128, &__led_sparkle, true, &hue);
}

void mbp_bling_led_botnet(uint8_t frame, void *data) {
//
//	for (uint8_t i = 0; i < LED_COUNT; i++) {
//		led_rgb_t rgb = util_led_get(i);
//		if (rgb.red > 0) {
//			util_led_set(i, (rgb.red - 1), 0, 0);
//		}
//	}
	//Turn off an led
	util_led_set(util_math_rand8_max(LED_COUNT), 0, 0, 0);

	//Turn on an led
	uint8_t red = util_math_rand8_max(100) + 155;
	uint8_t i = util_math_rand8_max(LED_COUNT);
	util_led_set(i, red, 0, 0);
	util_led_show();
}

void mbp_bling_major_lazer(void *data) {
	util_led_clear();
	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/MAJORL1.RGB", &anim);
	util_gfx_draw_raw_file("BLING/AND!XOR/MAJORL1.RAW", 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
}

static void __matrix_callback(uint8_t frame, void *p_data) {
	uint8_t brightness[5][4];
	memcpy(brightness, p_data, 5 * 4);
	if ((frame % 5) == 0) {

		//Move drops down
		for (uint8_t x = 0; x < 5; x++) {
			for (uint8_t y = 0; y < 4; y++) {
				if (brightness[x][y] == 240) {
					brightness[x][y] = 190;
					brightness[x][(y + 1) % 4] = 240;
					brightness[x][(y + 2) % 4] = 0;
					brightness[x][(y + 3) % 4] = 100;
					break;
				}
			}
		}

		//Map XY to LED indices
		uint8_t mapping[LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT] = LED_MATRIX_FULL_MAPPING;
		for (uint8_t x = 0; x < LED_MATRIX_FULL_COL_COUNT; x++) {
			for (uint8_t y = 0; y < LED_MATRIX_FULL_ROW_COUNT; y++) {
				uint8_t index = mapping[(y * LED_MATRIX_FULL_COL_COUNT) + x];
				if (index < LED_COUNT) {
					util_led_set(index, 0, brightness[x][y], 0);
				}
			}
		}

		util_led_show();
	}

	memcpy(p_data, brightness, 5 * 4);
}

void mbp_bling_matrix() {
	util_led_clear();
	uint8_t brightness[5][4];
	for (uint8_t x = 0; x < 5; x++) {
		uint8_t r = util_math_rand8_max(4);
		brightness[x][r] = 240;
	}
	util_gfx_draw_raw_file("BLING/AND!XOR/MATRIX.RAW", 0, 0, 128, 128, &__matrix_callback, true, (void *) &brightness);
}

void mbp_bling_nyan() {
	util_led_clear();
	uint8_t hue = 0; //hue is normally a float 0 to 1, pack it in an 8 bit int
	util_gfx_draw_raw_file("BLING/AND!XOR/NAYAN.RAW", 0, 0, 128, 128, &__led_sparkle, true, &hue);
}

void mbp_bling_owl() {
	uint8_t hue = 0;
	uint8_t index = 0;
	uint8_t count = 3;

	char *modes[] = { "BLING/AND!XOR/OWL1.RAW", "BLING/AND!XOR/OWL2.RAW", "BLING/AND!XOR/OWL3.RAW" };
	uint8_t button = 0;

	util_led_clear();

	//If anything other than left button is pressed cycle modes
	while ((button & BUTTON_MASK_LEFT) == 0) {
		button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__mbp_bling_rainbow_eyes_callback, true, &hue);
		index = (index + 1) % count;
	}
}

void mbp_bling_party() {
	uint8_t index = 0;
	uint8_t count = 3;

	if ((mbp_state_unlock_get() & UNLOCK_MASK_SCROLL) > 0) {
		count = 6;
	}
	char *modes[] = { "BLING/AND!XOR/FRY1.RAW", "BLING/AND!XOR/BENDER5.RAW", "BLING/AND!XOR/ZOIDBRG2.RAW", "BLING/AND!XOR/BENDER6.RAW",
			"BLING/AND!XOR/BENDER7.RAW", "BLING/AND!XOR/ZOIDBRG4.RAW" };
	uint8_t button = 0;

	util_led_clear();

	//If anything other than left button is pressed cycle modes
	while ((button & BUTTON_MASK_LEFT) == 0) {
		button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__spectrum_analyzer_callback, true, NULL);
		index = (index + 1) % count;
	}
}

void mbp_bling_rager() {
	uint8_t index = 0;
	uint8_t count = 3;

	if ((mbp_state_unlock_get() & UNLOCK_MASK_SCROLL) > 0) {
		count = 6;
	}
	char *modes[] = { "BLING/AND!XOR/RGFRY.RAW", "BLING/AND!XOR/RGBW.RAW", "BLING/AND!XOR/RGZB1.RAW", "BLING/AND!XOR/RGBN.RAW",
			"BLING/AND!XOR/RGHM.RAW", "BLING/AND!XOR/RGZB2.RAW" };
	uint8_t button = 0;

	util_led_clear();

	//If anything other than left button is pressed cycle modes
	while ((button & BUTTON_MASK_LEFT) == 0) {
		button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__spectrum_analyzer_callback, true, NULL);
		index = (index + 1) % count;
	}
}

void mbp_bling_pirate() {
	util_led_clear();
	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/GOLD.RGB", &anim);

	uint8_t button = 0;
	//Prevent escape from bling (so we can catch action button)
	while ((button & BUTTON_MASK_LEFT) == 0) {
		button = util_gfx_draw_raw_file("BLING/AND!XOR/PIRATES.RAW", 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
		if ((button & BUTTON_MASK_ACTION) > 0) {
			util_gfx_draw_raw_file("BLING/AND!XOR/WPCODE.RAW", 0, 0, 128, 128, NULL, false, NULL);
			nrf_delay_ms(2000);
		}
	}
}

void mbp_bling_rickroll() {
	util_led_clear();
	util_gfx_draw_raw_file("/BLING/AND!XOR/RICKROLL.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, &__spectrum_analyzer_callback, true, NULL);
}

uint8_t mbp_bling_scroll(char *text, bool loop) {
	util_gfx_invalidate();

	//Make sure all scroll text is upper case
	for (uint8_t i = 0; i < strlen(text); i++) {
		text[i] = toupper(text[i]);
	}
	int16_t y = (GFX_HEIGHT - SCROLL_CHAR_HEIGHT) / 2;
	int16_t x = GFX_WIDTH;

	//" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.!?,()[]{}<>/\\|;:&^%$#@*-_+"
	char *files[] = {
			"SCROLL/SPACE.FNT",
			"SCROLL/A.FNT",
			"SCROLL/B.FNT",
			"SCROLL/C.FNT",
			"SCROLL/D.FNT",
			"SCROLL/E.FNT",
			"SCROLL/F.FNT",
			"SCROLL/G.FNT",
			"SCROLL/H.FNT",
			"SCROLL/I.FNT",
			"SCROLL/J.FNT",
			"SCROLL/K.FNT",
			"SCROLL/L.FNT",
			"SCROLL/M.FNT",
			"SCROLL/N.FNT",
			"SCROLL/O.FNT",
			"SCROLL/P.FNT",
			"SCROLL/Q.FNT",
			"SCROLL/R.FNT",
			"SCROLL/S.FNT",
			"SCROLL/T.FNT",
			"SCROLL/U.FNT",
			"SCROLL/V.FNT",
			"SCROLL/W.FNT",
			"SCROLL/X.FNT",
			"SCROLL/Y.FNT",
			"SCROLL/Z.FNT",
			"SCROLL/0.FNT",
			"SCROLL/1.FNT",
			"SCROLL/2.FNT",
			"SCROLL/3.FNT",
			"SCROLL/4.FNT",
			"SCROLL/5.FNT",
			"SCROLL/6.FNT",
			"SCROLL/7.FNT",
			"SCROLL/8.FNT",
			"SCROLL/9.FNT",
			"SCROLL/PERIOD.FNT",
			"SCROLL/EXCL.FNT",
			"SCROLL/QUESTION.FNT",
			"SCROLL/COMMA.FNT",
			"SCROLL/LPAREN.FNT",
			"SCROLL/RPAREN.FNT",
			"SCROLL/LBRACKET.FNT",
			"SCROLL/RBRACKET.FNT",
			"SCROLL/LBRACE.FNT",
			"SCROLL/RBRACE.FNT",
			"SCROLL/LT.FNT",
			"SCROLL/GT.FNT",
			"SCROLL/FSLASH.FNT",
			"SCROLL/BSLASH.FNT",
			"SCROLL/PIPE.FNT",
			"SCROLL/SEMI.FNT",
			"SCROLL/COLON.FNT",
			"SCROLL/AMP.FNT",
			"SCROLL/CARROT.FNT",
			"SCROLL/PCT.FNT",
			"SCROLL/DOLLAR.FNT",
			"SCROLL/HASH.FNT",
			"SCROLL/AT.FNT",
			"SCROLL/STAR.FNT",
			"SCROLL/DASH.FNT",
			"SCROLL/USCORE.FNT",
			"SCROLL/PLUS.FNT"
	};

	while (1) {
		if (!util_gfx_is_valid_state()) {
			mbp_ui_cls();
		}

		util_gfx_fill_rect(0, y, SCROLL_CHAR_WIDTH, SCROLL_CHAR_HEIGHT, COLOR_BLACK);

		for (uint8_t i = 0; i < strlen(text); i++) {
			const char *ptr = strchr(INPUT_CHARS, text[i]);

			if (ptr) {
				uint16_t xx = x + (i * SCROLL_CHAR_WIDTH);
				int index = ptr - INPUT_CHARS;

				if (xx > 0 && xx < (GFX_WIDTH - SCROLL_CHAR_WIDTH))
					util_gfx_draw_raw_file(files[index], xx, y, SCROLL_CHAR_WIDTH, SCROLL_CHAR_HEIGHT, NULL, false, NULL);
			}
		}
		util_gfx_validate();

		x += SCROLL_SPEED;

		int16_t x_min = 0 - (SCROLL_CHAR_WIDTH * strlen(text));
		//If we run off the left edge, loop around maybe
		if (x < x_min) {
			if (!loop)
				break;
			x = GFX_WIDTH;
		}

		if ((util_button_action() && loop) || util_button_left()) {
			uint8_t button = util_button_state();
			util_button_clear();
			return button;
		}

		app_sched_execute();
		nrf_delay_ms(SCROLL_TIME_MS);
	}

	util_gfx_invalidate();
	uint8_t button = util_button_state();
	util_button_clear();
	return button;
}

static void __scroll_callback(void *p_data) {
	led_anim_t *p_anim = (led_anim_t *) p_data;
	util_led_play_rgb_frame(p_anim);
}

void mbp_bling_scroll_cycle() {
	util_led_clear();
	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/KIT.RGB", &anim);

	//Get the usesrname
	char name[SETTING_NAME_LENGTH];
	mbp_state_name_get(name);

	uint8_t index = 0;
	uint8_t count = 3;
	char *messages[] = { "DEFCON 25", "AND!XOR", name };

	//Start up led timer for scroll
	APP_ERROR_CHECK(app_timer_create(&m_scroll_led_timer, APP_TIMER_MODE_REPEATED, __scroll_callback));
	APP_ERROR_CHECK(app_timer_start(m_scroll_led_timer, APP_TIMER_TICKS(1000/20, UTIL_TIMER_PRESCALER), &anim));

	while (1) {
		uint8_t button = mbp_bling_scroll(messages[index], false);
		index = (index + 1) % count;

		if ((button & BUTTON_MASK_LEFT) > 0) {
			break;
		}
	}

	app_timer_stop(m_scroll_led_timer);

	util_gfx_invalidate();
	util_button_clear();
}

void mbp_bling_hello_schedule_handler(void * p_event_data, uint16_t event_size) {
	char *name = (char *) p_event_data;
	uint16_t w, h;
	app_sched_pause();
	bool cigar = mbp_cigar_eyes_running();
	mbp_cigar_eyes_stop();

	//Pick colors
	float h1 = ((float) util_math_rand8_max(100) / 100.0);
	float h2 = h1 + 0.5;
	if (h2 >= 1.0) {
		h2 -= 1.0;
	}
	uint32_t color_1 = util_led_hsv_to_rgb(h1, 1, 1);
	uint32_t color_2 = util_led_hsv_to_rgb(h2, 1, 1);

	util_gfx_draw_raw_file("BG.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);
	uint16_t fg = util_gfx_rgb_to_565(color_1);
	uint16_t bg = util_gfx_rgb_to_565(color_2);

	//Compute name coords
	util_gfx_set_font(FONT_LARGE);
	util_gfx_get_text_bounds(name, 0, 0, &w, &h);
	uint16_t y = (GFX_HEIGHT / 2) + 4;
	uint16_t x = (GFX_WIDTH - w) / 2;

	//Print shadow
	util_gfx_set_color(bg);
	util_gfx_set_cursor(x + 1, y + 1);
	util_gfx_print(name);

	//Print name
	util_gfx_set_color(fg);
	util_gfx_set_cursor(x, y);
	util_gfx_print(name);

	//Compute hello coords
	char hello[] = "Hello";
	util_gfx_get_text_bounds(hello, 0, 0, &w, &h);
	x = (GFX_WIDTH - w) / 2;
	y = (GFX_HEIGHT / 2) - 4 - h;

	//Print shadow
	util_gfx_set_color(bg);
	util_gfx_set_cursor(x + 1, y + 1);
	util_gfx_print(hello);

	//Print hello
	util_gfx_set_color(fg);
	util_gfx_set_cursor(x, y);
	util_gfx_print(hello);

	//Set all LEDs
	util_led_set_all_rgb(color_1);
	util_led_show();
	nrf_delay_ms(2000);

	uint8_t cols[5][4] = {
			{ 8, 4, 0, 12 },
			{ 9, 5, 1, 255 },
			{ 10, 6, 2, 255 },
			{ 11, 7, 3, 13 },
			{ 255, 255, 255, 14 }
	};
	uint8_t height[] = { 4, 4, 4, 4, 4 };

	while (1) {
		//pick a random column to lower
		uint8_t col = util_math_rand8_max(5);
		if (height[col] > 0) {
			height[col]--;
			uint8_t index = cols[col][height[col]];

			if (index < LED_COUNT) {
				util_led_set_rgb(index, color_2);
				util_led_show();
				nrf_delay_ms(40);
				util_led_set(index, 0, 0, 0);
				util_led_show();
			}
		}

		nrf_delay_ms(30);

		bool done = true;
		for (uint8_t i = 0; i < 5; i++) {
			if (height[i] > 0) {
				done = false;
				break;
			}
		}

		if (done) {
			break;
		}
	}

	//Cleanup and give control back to user
	util_gfx_invalidate();
	if (cigar) {
		mbp_cigar_eyes_start();
	}
	app_sched_resume();
	util_button_clear();
}

void mbp_bling_hello_cpv_schedule_handler(void * p_event_data, uint16_t event_size) {
	app_sched_pause();
	bool cigar = mbp_cigar_eyes_running();
	mbp_cigar_eyes_stop();

	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/PINKBLUE.RGB", &anim);
	util_gfx_draw_raw_file("B2B/CPV.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);

	//4 second hello time (20 FPS)
	for (uint16_t i = 0; i < (20 * 4); i++) {
		util_led_play_rgb_frame(&anim);
		nrf_delay_ms(50);
	}

	//Cleanup and give control back to user
	util_led_clear();
	util_gfx_invalidate();
	if (cigar) {
		mbp_cigar_eyes_start();
	}
	app_sched_resume();
	util_button_clear();
}

void mbp_bling_hello_dc503_schedule_handler(void * p_event_data, uint16_t event_size) {
	app_sched_pause();
	bool cigar = mbp_cigar_eyes_running();
	mbp_cigar_eyes_stop();

	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/TUNNEL2.RGB", &anim);
	util_gfx_draw_raw_file("B2B/DC503.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);

	//4 second hello time (20 FPS)
	for (uint16_t i = 0; i < (20 * 4); i++) {
		util_led_play_rgb_frame(&anim);
		nrf_delay_ms(50);
	}

	//Cleanup and give control back to user
	util_led_clear();
	util_gfx_invalidate();
	if (cigar) {
		mbp_cigar_eyes_start();
	}
	app_sched_resume();
	util_button_clear();
}

void mbp_bling_hello_dc801_schedule_handler(void * p_event_data, uint16_t event_size) {
	app_sched_pause();
	bool cigar = mbp_cigar_eyes_running();
	mbp_cigar_eyes_stop();

	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/GRNBLUE.RGB", &anim);
	util_gfx_draw_raw_file("B2B/DC801.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);

	//4 second hello time (20 FPS)
	for (uint16_t i = 0; i < (20 * 4); i++) {
		util_led_play_rgb_frame(&anim);
		nrf_delay_ms(50);
	}

	//Cleanup and give control back to user
	util_led_clear();
	util_gfx_invalidate();
	if (cigar) {
		mbp_cigar_eyes_start();
	}
	app_sched_resume();
	util_button_clear();
}

void mbp_bling_hello_queercon_schedule_handler(void * p_event_data, uint16_t event_size) {
	app_sched_pause();
	bool cigar = mbp_cigar_eyes_running();
	mbp_cigar_eyes_stop();

	UTIL_LED_ANIM_INIT(anim);
	util_led_load_rgb_file("BLING/COLORS.RGB", &anim);
	util_gfx_draw_raw_file("B2B/QUEERCON.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);

	//4 second hello time (20 FPS)
	for (uint16_t i = 0; i < (20 * 4); i++) {
		util_led_play_rgb_frame(&anim);
		nrf_delay_ms(50);
	}

	//Cleanup and give control back to user
	util_led_clear();
	util_gfx_invalidate();
	if (cigar) {
		mbp_cigar_eyes_start();
	}
	app_sched_resume();
	util_button_clear();
}

static void __mbp_bling_toad_callback(uint8_t frame, void *p_data) {
//Unpack hue as a float
	uint8_t *data = (uint8_t *) p_data;
	float hue = ((float) *data / 100.0);

	util_led_set_all_rgb(util_led_hsv_to_rgb(hue, 1, .9));
	util_led_show();

	hue += 0.07;
	if (hue >= 1) {
		hue = 0;
	}

	//Pack the hue
	*data = (uint8_t) (hue * 100.0);
}

void mbp_bling_toad() {
	util_led_clear();
	uint8_t hue = 0; //hue is normally a float 0 to 1, pack it in an 8 bit int
	util_gfx_draw_raw_file("BLING/AND!XOR/TOAD2.RAW", 0, 0, 128, 128, &__mbp_bling_toad_callback, true, &hue);
}

static void __mbp_bling_twitter_callback(uint8_t frame, void *p_data) {
	uint8_t *p_index = (uint8_t *) p_data;

	//If the stored data is a valid index, set that LED to blue or black
	if (*p_index < LED_COUNT) {
		if (util_math_rand8_max(2) == 0) {
			util_led_set(*p_index, 29, 161, 243);
		} else {
			util_led_set_rgb(*p_index, LED_COLOR_BLACK);
		}

		//Pack invalid data so next time around a new LED is picked
		*p_index = 0xFF;
	}
	//Otherwise if it's an invalid index randomly pick one to flash
	else {
		*p_index = util_math_rand8_max(LED_COUNT);
		util_led_set_rgb(*p_index, LED_COLOR_WHITE);
	}

	util_led_show();
}

void mbp_bling_trololol() {
	util_led_clear();
	util_gfx_draw_raw_file("BLING/AND!XOR/TROLOLOL.RAW", 0, 0, 128, 128, &mbp_bling_led_botnet, true, NULL);
}

void mbp_bling_twitter() {
	util_led_clear();
	uint8_t index = 0xFF; //set index out of bounds of led count
	util_gfx_draw_raw_file("BLING/TWITTER/TWITTER.RAW", 0, 0, 128, 128, &__mbp_bling_twitter_callback, true, &index);
}

static void __mbp_defrag_callback(uint8_t frame, void *p_data) {
	bling_defag_state_t *p_defrag = (bling_defag_state_t *) p_data;
	uint8_t count = (LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT);
	uint8_t led_mapping[LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT] = LED_MATRIX_FULL_MAPPING
	;
	uint32_t rgb = util_led_hsv_to_rgb(p_defrag->hue, 1, .8);
	uint8_t index = led_mapping[p_defrag->index];

	if (index < LED_COUNT) {
		util_led_set_rgb(index, rgb);
		util_led_show();
	}

	p_defrag->index++;
	if (p_defrag->index >= count) {
		p_defrag->index = 0;
		p_defrag->hue += .05;

		if (p_defrag->hue >= 1) {
			p_defrag->hue = 0;
		}
	}
}

void mbp_bling_defrag() {
	bling_defag_state_t defrag;
	defrag.hue = 0;
	defrag.index = 0;
	util_led_clear();
	util_gfx_draw_raw_file("BLING/AND!XOR/DEFRAG.RAW", 0, 0, 128, 128, &__mbp_defrag_callback, true, &defrag);
}

static void __cigar_sch_handler(void * p_event_data, uint16_t event_size) {
	uint32_t eye_rgb = util_led_hsv_to_rgb(m_eye_hue, 1.0, 1.0);
	uint32_t rgb = util_led_hsv_to_rgb(m_cigar_hue, 1.0, 1.0);

	//Update the LEDs
	util_led_set_rgb(LED_LEFT_EYE_INDEX, eye_rgb);

	//Offset right eye hue if infected
	if (mbp_master_c2_infected()) {
		float hue = m_eye_hue + 0.5;
		if (hue > 1) {
			hue -= 1;
		}
		uint32_t rgb2 = util_led_hsv_to_rgb(hue, 1, 1);
		util_led_set_rgb(LED_RIGHT_EYE_INDEX, rgb2);
	} else {
		util_led_set_rgb(LED_RIGHT_EYE_INDEX, eye_rgb);
	}
	util_led_set_rgb(LED_CIGAR_INDEX, rgb);
	util_led_show();

	//Change the hue
	m_eye_hue += EYE_HUE_STEP;
	if (m_eye_hue >= 1.0) {
		m_eye_hue -= 1.0;
	}

	//Change cigar hue
	m_cigar_hue += m_cigar_hue_step;
	if (m_cigar_hue > CIGAR_HUE_HIGH) {
		m_cigar_hue = CIGAR_HUE_HIGH;
		m_cigar_hue_step = 0 - m_cigar_hue_step;
	} else if (m_cigar_hue < CIGAR_HUE_LOW) {
		m_cigar_hue = CIGAR_HUE_LOW;
		m_cigar_hue_step = 0 - m_cigar_hue_step;
	}
}

static void __cigar_timer_handler(void *p_data) {
	app_sched_event_put(NULL, 0, __cigar_sch_handler);
}

bool mbp_cigar_eyes_running() {
	return m_cigar_eyes_running;
}

void mbp_cigar_eyes_start() {
	if (!m_cigar_eyes_running) {
		//Start up cigar flicker timer
		APP_ERROR_CHECK(app_timer_create(&m_cigar_timer, APP_TIMER_MODE_REPEATED, __cigar_timer_handler));
		APP_ERROR_CHECK(app_timer_start(m_cigar_timer, APP_TIMER_TICKS(CIGAR_EYES_TIME_MS, UTIL_TIMER_PRESCALER), NULL));

		m_cigar_eyes_running = true;
	}
}

void mbp_cigar_eyes_stop() {
	if (m_cigar_eyes_running) {
		util_led_set(LED_LEFT_EYE_INDEX, 0, 0, 0);
		util_led_set(LED_RIGHT_EYE_INDEX, 0, 0, 0);
		util_led_set(LED_CIGAR_INDEX, 0, 0, 0);
		util_led_show();
		APP_ERROR_CHECK(app_timer_stop(m_cigar_timer));

		m_cigar_eyes_running = false;
	}
}
