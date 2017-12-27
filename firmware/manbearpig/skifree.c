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

#include "system.h"

#define SKI_ACCELERATION		0.015
#define SKI_BG					COLOR_WHITE
#define SKI_FG					COLOR_BLUE
#define SKI_STEP_MS				100
#define SKI_X             		56
#define SKI_Y             		4
#define SKI_W					16
#define SKI_H					16
#define SKI_MARGIN				4
#define SKI_SPRITE_COUNT		32
#define SKI_WORLD_WIDTH			4
#define SKI_WORLD_HEIGHT		2
#define SKI_SPRITE_W			16
#define SKI_SPRITE_H			16
#define SKI_SPRITE_TYPE_COUNT	3
#define SKI_MBP_W				16
#define SKI_MBP_H				28
#define SKI_MBP_SPEED			3

typedef struct {
	float x, y;
	int16_t dx, dy;
	uint8_t sprite_index;
} sprite_t;

typedef struct {
	sprite_t sprites[SKI_SPRITE_COUNT];
	int8_t angle;
	float distance;
	float velocity;
	float velocity_angled;
	uint8_t left_raw[SKI_W * SKI_H * 2];
	uint8_t right_raw[SKI_W * SKI_H * 2];
	uint8_t down_raw[SKI_W * SKI_H * 2];
	uint8_t sprite_raw[SKI_SPRITE_TYPE_COUNT][SKI_SPRITE_W * SKI_SPRITE_H * 2];
	volatile bool exit_flag;
} ski_state_t;

APP_TIMER_DEF(m_ski_timer);

/**
 * Draw ski free game
 */
static void __draw(ski_state_t *p_state) {

	if (!util_gfx_is_valid_state()) {
		//Clear the screen
		util_gfx_fill_screen(SKI_BG);
		util_gfx_set_color(SKI_FG);
	}

	//Draw the sprites
	for (uint8_t i = 0; i < SKI_SPRITE_COUNT; i++) {
		sprite_t s = p_state->sprites[i];
		util_gfx_fill_rect(s.x - s.dx - 1, s.y - s.dy - 1, SKI_SPRITE_W + 2, SKI_SPRITE_H + 2, SKI_BG);
		util_gfx_draw_raw(s.x, s.y, SKI_SPRITE_W, SKI_SPRITE_H, p_state->sprite_raw[s.sprite_index]);
	}

	//Draw the skier
	switch (p_state->angle) {
	case -45:
		util_gfx_draw_raw(SKI_X, SKI_Y, SKI_W, SKI_H, p_state->left_raw);
		break;
	case 0:
		util_gfx_draw_raw(SKI_X, SKI_Y, SKI_W, SKI_H, p_state->down_raw);
		break;
	case 45:
		util_gfx_draw_raw(SKI_X, SKI_Y, SKI_W, SKI_H, p_state->right_raw);
		break;
	}

	char dist[32];
	sprintf(dist, "%dm", (int) p_state->distance);
	util_gfx_set_cursor(0, 0);

	util_gfx_fill_rect(0, 0, 30, util_gfx_font_height(), SKI_BG);
	util_gfx_print(dist);

	//Mark draw state as valid
	util_gfx_validate();
}

static void __mbp() {
	uint8_t mbp_raw[SKI_MBP_W * SKI_MBP_H * 2];
	util_gfx_load_raw(mbp_raw, "SKI/MBP.RAW", SKI_MBP_W * SKI_MBP_H * 2);
	for (int16_t x = 0 - SKI_MBP_W; x < SKI_X - SKI_MBP_W; x += SKI_MBP_SPEED) {
		util_gfx_fill_rect(x - SKI_MBP_SPEED, SKI_Y + 3, SKI_MBP_SPEED, SKI_MBP_H, COLOR_WHITE);
		util_gfx_draw_raw(x, SKI_Y + 3, SKI_MBP_W, SKI_MBP_H, mbp_raw);
		nrf_delay_ms(50);
	}
	nrf_delay_ms(1000);
}

static void __ski_timer_handler(void *data) {
	ski_state_t *p_state = (ski_state_t *) data;

	//Move the sprites (not the skier)
	float dx = 0, dy = 0;

	//Determine the velocities
	switch (p_state->angle) {
	case -45:
		dx = p_state->velocity_angled;
		dy = 0 - p_state->velocity_angled;
		break;
	case 0:
		dx = 0;
		dy = 0 - p_state->velocity;
		break;
	case 45:
		dx = 0 - p_state->velocity_angled;
		dy = 0 - p_state->velocity_angled;
		break;
	}

	//Move the sprites
	for (uint8_t i = 0; i < SKI_SPRITE_COUNT; i++) {
		p_state->sprites[i].x += dx;
		p_state->sprites[i].y += dy;
		p_state->sprites[i].dx = dx;
		p_state->sprites[i].dy = dy;

		//sprite is off the screen re-generate
		if (p_state->sprites[i].y < 0 - SKI_H) {

			bool overlap = true;

			//Keep trying utnil we know something was picked that does not overlap
			while (overlap) {

				//Move the sprite around
				p_state->sprites[i].x = (int16_t)util_math_rand32_max(GFX_WIDTH * SKI_WORLD_WIDTH) - ((SKI_WORLD_WIDTH / 2) * GFX_WIDTH);  //Generate sprites three screens wide
				p_state->sprites[i].y = util_math_rand32_max(GFX_HEIGHT * SKI_WORLD_HEIGHT) + GFX_HEIGHT;
				p_state->sprites[i].sprite_index = util_math_rand8_max(SKI_SPRITE_TYPE_COUNT);
				float x_min = p_state->sprites[i].x;
				float x_max = x_min + SKI_SPRITE_W;
				float y_min = p_state->sprites[i].y;
				float y_max = y_min + SKI_SPRITE_H;

				//Check all sprites before it for overlap, assume none until we figure out otherwise
				overlap = false;
				for (uint8_t j = 0; j < SKI_SPRITE_COUNT; j++) {
					//Make sure we're not looking at ourselv
					if (i != j) {
						if (!(x_min > p_state->sprites[j].x + SKI_SPRITE_W) &&
								!(x_max < p_state->sprites[j].x) &&
								!(y_min > p_state->sprites[j].y + SKI_SPRITE_H) &&
								!(y_max < p_state->sprites[j].y)) {
							overlap = true;
							break;
						}
					}
				}
			}
		}

		//Detect collisions
		if (p_state->sprites[i].x + SKI_W > SKI_X + SKI_MARGIN &&
				p_state->sprites[i].x < SKI_X + SKI_W - SKI_MARGIN &&
				p_state->sprites[i].y + SKI_H > SKI_Y + SKI_MARGIN &&
				p_state->sprites[i].y < SKI_Y + SKI_H - SKI_MARGIN) {

			p_state->exit_flag = true;
		}
	}

	p_state->distance += p_state->velocity;
	p_state->velocity += SKI_ACCELERATION;
	p_state->velocity_angled = sqrt(pow(p_state->velocity, 2));

	if (util_button_left() > 0) {
		p_state->angle = -45;
	}
	if (util_button_right() > 0) {
		p_state->angle = 45;
	}
	if (util_button_down() > 0) {
		p_state->angle = 0;
	}
	util_button_clear();

	__draw(p_state);
}

void ski() {
	char *sprite_files[] = { "SKI/LIFT.RAW", "SKI/ROCK.RAW", "SKI/TREE.RAW" };

	mbp_tooth_eye_stop();
	app_sched_pause();

	//Initialize the skier
	ski_state_t state;
	state.exit_flag = false;
	state.angle = 0;
	state.distance = 0;
	state.velocity = 2.0;
	state.velocity_angled = sqrt(pow(state.velocity, 2));
	util_gfx_load_raw(state.left_raw, "SKI/SKILEFT.RAW", SKI_W * SKI_H * 2);
	util_gfx_load_raw(state.right_raw, "SKI/SKIRIGHT.RAW", SKI_W * SKI_H * 2);
	util_gfx_load_raw(state.down_raw, "SKI/SKIDOWN.RAW", SKI_W * SKI_H * 2);

	//Load the sprites
	for (uint8_t i = 0; i < SKI_SPRITE_TYPE_COUNT; i++) {
		util_gfx_load_raw(state.sprite_raw[i], sprite_files[i], SKI_SPRITE_W * SKI_SPRITE_H * 2);
	}

	//Initialize the sprites
	for (uint8_t i = 0; i < SKI_SPRITE_COUNT; i++) {
		bool overlap = true;
		while (overlap) {
			state.sprites[i].x = (int16_t)util_math_rand32_max(GFX_WIDTH * SKI_WORLD_WIDTH) - ((SKI_WORLD_WIDTH / 2) * GFX_WIDTH);  //Generate sprites three screens wide
			state.sprites[i].y = util_math_rand32_max(GFX_HEIGHT * SKI_WORLD_HEIGHT) + GFX_HEIGHT;
			state.sprites[i].sprite_index = util_math_rand8_max(SKI_SPRITE_TYPE_COUNT);
			float x_min = state.sprites[i].x;
			float x_max = x_min + SKI_SPRITE_W;
			float y_min = state.sprites[i].y;
			float y_max = y_min + SKI_SPRITE_H;

			//Check all sprites before it for overlap
			overlap = false;
			for (uint8_t j = 0; j < i; j++) {
				if (!(x_min > state.sprites[j].x + SKI_SPRITE_W) &&
						!(x_max < state.sprites[j].x) &&
						!(y_min > state.sprites[j].y + SKI_SPRITE_H) &&
						!(y_max < state.sprites[j].y)) {
					overlap = true;
					break;
				}
			}
		}
	}

	uint32_t err_code;
	err_code = app_timer_create(&m_ski_timer, APP_TIMER_MODE_REPEATED, __ski_timer_handler);
	APP_ERROR_CHECK(err_code);

	//Ensure full game state is drawn
	util_gfx_invalidate();

	//Start main game timer
	uint32_t ticks = APP_TIMER_TICKS(SKI_STEP_MS, UTIL_TIMER_PRESCALER);
	err_code = app_timer_start(m_ski_timer, ticks, (void *) &state);
	APP_ERROR_CHECK(err_code);

	while (!state.exit_flag) {
		err_code = sd_app_evt_wait();
		APP_ERROR_CHECK(err_code);

		app_sched_execute();
	}

	app_timer_stop(m_ski_timer);

	//Bring out Man Bear Pig
	__mbp();

	app_sched_resume();
	mbp_ui_popup("Ski", "Game Over!");
	mbp_tooth_eye_start();
}
