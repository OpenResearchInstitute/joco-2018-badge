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
#include "../system.h"

static bool m_apa102 = false;
static uint8_t leds[LED_COUNT * 3 * sizeof(uint8_t)];

static const uint8_t gamma_values[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
		2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
		5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
		10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
		17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
		25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
		37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
		51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
		69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
		90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
		115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
		144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
		177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
		215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

/**
 * Converts a 16-bit 565 color to RGB color suitable for LEDs
 */
uint32_t util_led_565_to_rgb(uint16_t color) {
	//Incoming color is assumed to be RGB565
	//LED color is 0x00RRGGBB
	return ((0xF800 & color) << 8) | ((0x07E0 & color) << 5) | (0x1F & color) << 3;
}

inline void util_led_clear() {
	util_led_set_all(0, 0, 0);
	util_led_show();
}

led_rgb_t util_led_get(uint8_t index) {
	led_rgb_t rgb;
	rgb.red = leds[index * 3];
	rgb.green = leds[(index * 3) + 1];
	rgb.blue = leds[(index * 3) + 1];
	return rgb;
}

uint32_t util_led_hsv_to_rgb(float H, float S, float V) {
	float h = H * 6;
	uint8_t i = floor(h);
	float a = V * (1 - S);
	float b = V * (1 - S * (h - i));
	float c = V * (1 - (S * (1 - (h - i))));
	float rf, gf, bf;

	switch (i) {
	case 0:
		rf = V * 255;
		gf = c * 255;
		bf = a * 255;
		break;
	case 1:
		rf = b * 255;
		gf = V * 255;
		bf = a * 255;
		break;
	case 2:
		rf = a * 255;
		gf = V * 255;
		bf = c * 255;
		break;
	case 3:
		rf = a * 255;
		gf = b * 255;
		bf = V * 255;
		break;
	case 4:
		rf = c * 255;
		gf = a * 255;
		bf = V * 255;
		break;
	case 5:
		default:
		rf = V * 255;
		gf = a * 255;
		bf = b * 255;
		break;
	}

	uint8_t R = rf;
	uint8_t G = gf;
	uint8_t B = bf;

	uint32_t RGB = (R << 16) + (G << 8) + B;
	return RGB;
}

bool util_led_has_apa102() {
	return m_apa102;
}

void util_led_init() {
	memset(leds, 0x00, sizeof(uint8_t) * LED_COUNT * 3);

	//If APA102.MBP exists, use the APA102 driver
	FILINFO info;
	FRESULT result = f_stat("APA102.MBP", &info);
	m_apa102 = (result == FR_OK);

	if (m_apa102) {
		apa102_init();

	} else {
		ws2812b_init();
	}
}

void util_led_load_rgb_file(char *filename, led_anim_t *p_anim) {
	FIL rgb_file;

	//Stat the file to determine frame count
	uint32_t size = util_sd_file_size(filename);
	if (size == 0) {
		char message[128];
		sprintf(message, "Could not stat %s.", filename);
		mbp_ui_error(message);
		return;
	}

	//Make sure RGB file isn't too big
	if (size > 6000) {
		mbp_ui_error("RGB file too large. ");
		return;
	}

	p_anim->frame = 0;
	p_anim->frames = size / LED_RGB_COUNT / 3;

	// Open requested file on SD card
	FRESULT result = f_open(&rgb_file, filename, FA_READ | FA_OPEN_EXISTING);
	if (result != FR_OK) {
		char message[128];
		sprintf(message, "Could not open %s.", filename);
		mbp_ui_error(message);
		return;
	}

	//Read the RGB led data into the struct
	result = f_read(&rgb_file, p_anim->rgb_data, size, (UINT*) &p_anim->size);

	//Check for read error
	if (result != FR_OK) {
		char message[128];
		sprintf(message, "Could not read %s.", filename);
		mbp_ui_error(message);

		free(p_anim->rgb_data);
		f_close(&rgb_file);
		return;
	}

	f_close(&rgb_file);
}

void util_led_play_rgb_frame(led_anim_t *p_anim) {
	uint8_t led_index;
	util_led_set_all(0, 0, 0);
	uint8_t rgb_led_mapping[] = LED_MATRIX_FULL_MAPPING;

	//MBP2 has eyes swapped
	if (util_led_has_apa102()) {
		rgb_led_mapping[0] = 13;
		rgb_led_mapping[3] = 12;
	}

	uint16_t offset = p_anim->frame * LED_RGB_COUNT * 3;
	for (uint8_t i = 0; i < LED_RGB_COUNT; i++) {
		led_index = rgb_led_mapping[i];
		if (led_index < LED_COUNT) {
			uint8_t r = p_anim->rgb_data[offset + (i * 3)];
			uint8_t g = p_anim->rgb_data[offset + (i * 3) + 1];
			uint8_t b = p_anim->rgb_data[offset + (i * 3) + 2];
			util_led_set(led_index, r, g, b);
		}
	}
	util_led_show();

	//advance the frame, wrapping around
	p_anim->frame = (p_anim->frame + 1) % p_anim->frames;
}

void util_led_set(uint32_t index, uint8_t r, uint8_t g, uint8_t b) {
	uint32_t offset = index * 3;
	leds[offset] = gamma_values[b];
	leds[offset + 1] = gamma_values[g];
	leds[offset + 2] = gamma_values[r];
}

void util_led_set_all(uint8_t red, uint8_t green, uint8_t blue) {
	for (uint32_t i = 0; i < LED_COUNT; i++) {
		util_led_set(i, red, green, blue);
	}
}

void util_led_set_all_rgb(uint32_t rgb) {
	for (uint32_t i = 0; i < LED_COUNT; i++) {
		util_led_set_rgb(i, rgb);
	}
}

void util_led_set_rgb(uint32_t index, uint32_t rgb) {
	util_led_set(index, (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

void util_led_show() {
	if (m_apa102) {
		apa102_send(leds);
	} else {
		ws2812b_send(leds);
	}
}

uint32_t util_led_to_rgb(uint8_t red, uint8_t green, uint8_t blue) {
	return ((uint32_t)(0xFF & red) << 16) | ((uint32_t)(0xFF & green) << 8) | (uint32_t)(0xFF & blue);
}
