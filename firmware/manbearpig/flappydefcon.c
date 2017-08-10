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

//Flappy settings
#define FLAPPY_BG				COLOR_LIGHTBLUE
#define FLAPPY_FG				COLOR_BLUE
#define FLAPPY_COLLISION_MARGIN	2
#define FLAPPY_PIPE_COUNT		4
#define FLAPPY_PIPE_GAP         44
#define FLAPPY_PIPE_MIN_HEIGHT  8
#define FLAPPY_PIPE_SPACING     100
#define FLAPPY_PLAYER_X         20
#define FLAPPY_PIPE_MAX_HEIGHT  64 - PIPE_GAP - (2 * PIPE_MIN_HEIGHT)
#define FLAPPY_PIPE_VELOCITY    3
#define FLAPPY_UP_TIME_MS       400
#define FLAPPY_STEP_MS          80
#define FLAPPY_MAX_VELOCITY     8
#define FLAPPY_VELOCITY_UP      -1.7
#define FLAPPY_GRAVITY          0.8

//SPRITD DIMENSIONS
#define FLAPPY_SKULL_W			24
#define FLAPPY_SKULL_H			20
#define FLAPPY_SKULL_BC			(FLAPPY_SKULL_H * FLAPPY_SKULL_W * 2)
#define FLAPPY_PIPE_W			24
#define FLAPPY_PIPE_H			88
#define FLAPPY_PIPE_BC			(FLAPPY_PIPE_W * FLAPPY_PIPE_H * 2)

typedef struct {
	int16_t x;
	int16_t y;
	uint8_t h;
	bool passed;
} pipe_t;

typedef struct {
	volatile bool exit_flag;
	bool flap_up;
	uint32_t flap_end_time;
	float velocity;
	float player_y;
	pipe_t pipes[FLAPPY_PIPE_COUNT];
	uint16_t pipes_passed;

	uint8_t skull_down_raw[FLAPPY_SKULL_BC];
	uint8_t skull_up_raw[FLAPPY_SKULL_BC];
	uint8_t pipe_raw[FLAPPY_PIPE_BC];
} flappy_state_t;

APP_TIMER_DEF( m_flappy_defcon_timer);

static void __draw(flappy_state_t *p_state) {
	if (!util_gfx_is_valid_state()) {
		//Setup display
		util_gfx_fill_screen(FLAPPY_BG);
		util_gfx_set_color(FLAPPY_FG);
		util_gfx_set_font(FONT_LARGE);
	}

	//Erase player
	util_gfx_fill_rect(FLAPPY_PLAYER_X, 0, FLAPPY_SKULL_W, GFX_HEIGHT, FLAPPY_BG);

	//Erase the text this has to happen before pipes so the pipes can go on top of the BG
	util_gfx_fill_rect(0, 0, 32, util_gfx_font_height(), FLAPPY_BG);

	//Draw the pipes
	for (uint8_t i = 0; i < FLAPPY_PIPE_COUNT; i++) {
		if (p_state->pipes[i].x <= GFX_WIDTH) {
			int8_t top_y = (0 - FLAPPY_PIPE_H) + p_state->pipes[i].h + FLAPPY_PIPE_MIN_HEIGHT;
			int8_t bottom_y = p_state->pipes[i].h + FLAPPY_PIPE_MIN_HEIGHT + FLAPPY_PIPE_GAP;
			uint32_t top_offset = (0 - top_y) * FLAPPY_PIPE_W * 2;

			//Erase old pipes
			util_gfx_fill_rect(p_state->pipes[i].x + FLAPPY_PIPE_VELOCITY, 0, FLAPPY_PIPE_W, GFX_HEIGHT, FLAPPY_BG);

			//Draw new pipes
			util_gfx_draw_raw(p_state->pipes[i].x, 0, FLAPPY_PIPE_W, FLAPPY_PIPE_H + top_y, p_state->pipe_raw + top_offset);
			util_gfx_draw_raw(p_state->pipes[i].x, bottom_y, FLAPPY_PIPE_W, GFX_HEIGHT - bottom_y, p_state->pipe_raw);
		}
	}

	//Draw the correct player images
	if (p_state->flap_up)
		util_gfx_draw_raw(FLAPPY_PLAYER_X, p_state->player_y, FLAPPY_SKULL_W, FLAPPY_SKULL_H, p_state->skull_up_raw);
	else
		util_gfx_draw_raw(FLAPPY_PLAYER_X, p_state->player_y, FLAPPY_SKULL_W, FLAPPY_SKULL_H, p_state->skull_down_raw);

	//Draw Points
	util_gfx_set_cursor(0, 3);
	char text[16];
	sprintf(text, "%d", p_state->pipes_passed);
	util_gfx_print(text);

	//Mark graphics state as valid
	util_gfx_validate();
}

void __flappy_timer_handler(void *data) {
	flappy_state_t *p_state = (flappy_state_t *) data;

	p_state->flap_up = util_millis() < p_state->flap_end_time;
	__draw(p_state);

	if (p_state->flap_up) {
		p_state->velocity = FLAPPY_VELOCITY_UP;
	}

	//Player falls
	p_state->player_y += p_state->velocity;
	//Accelerates towards ground due to gravity
	p_state->velocity += FLAPPY_GRAVITY;
	if (p_state->velocity >= FLAPPY_MAX_VELOCITY)
		p_state->velocity = FLAPPY_MAX_VELOCITY;

	//Move the pipes and detect collisions
	for (int8_t i = 0; i < FLAPPY_PIPE_COUNT; i++) {
		p_state->pipes[i].x -= FLAPPY_PIPE_VELOCITY;

		//Detect if the pipe goes off the screen
		if (p_state->pipes[i].x <= 0 - FLAPPY_PIPE_W) {
			//Wrap the pipe after the last pipe
			p_state->pipes[i].x = p_state->pipes[(i + FLAPPY_PIPE_COUNT - 1) % FLAPPY_PIPE_COUNT].x + FLAPPY_PIPE_SPACING;
			//Randomly pick a new height
			p_state->pipes[i].h = util_math_rand32_max(GFX_HEIGHT - (2 * FLAPPY_PIPE_MIN_HEIGHT) - FLAPPY_PIPE_GAP);
			//reset passed value
			p_state->pipes[i].passed = false;
		}

		//Detect if this pipe is passing by the player
		if ((p_state->pipes[i].x + FLAPPY_PIPE_W >= FLAPPY_PLAYER_X + FLAPPY_COLLISION_MARGIN) &&
				(p_state->pipes[i].x <= FLAPPY_PLAYER_X + FLAPPY_SKULL_W - FLAPPY_COLLISION_MARGIN)) {

			//Detect if player is hitting a pipe
			if ((p_state->player_y + FLAPPY_COLLISION_MARGIN < p_state->pipes[i].h + FLAPPY_PIPE_MIN_HEIGHT) ||
					(p_state->player_y + FLAPPY_SKULL_H - FLAPPY_COLLISION_MARGIN > p_state->pipes[i].h + FLAPPY_PIPE_MIN_HEIGHT + FLAPPY_PIPE_GAP)) {
				p_state->exit_flag = true;
			}
		}

		//Count some XP for the user if it's behind them
		if ((p_state->pipes[i].x + FLAPPY_PIPE_W) < FLAPPY_PLAYER_X) {
			if (!p_state->pipes[i].passed) {
				p_state->pipes[i].passed = true;
				p_state->pipes_passed++;
			}
		}
	}

	//Detect ground collision
	if (p_state->player_y + FLAPPY_SKULL_H >= GFX_HEIGHT) {
		p_state->exit_flag = true;
		return;
	}

	//Detect up/action button, flap up
	if (util_button_up() || util_button_action()) {
		p_state->flap_end_time = util_millis() + FLAPPY_UP_TIME_MS;
	}
}

void flappy() {
	uint32_t err_code;
	flappy_state_t state;
	state.exit_flag = false;
	state.pipes_passed = 0;
	state.player_y = 32;
	state.flap_end_time = 0;
	state.velocity = 0;

	mbp_cigar_eyes_stop();
	app_sched_pause();

	util_gfx_load_raw(state.skull_down_raw, "FLAPPY/SKULLD.RAW", FLAPPY_SKULL_BC);
	util_gfx_load_raw(state.skull_up_raw, "FLAPPY/SKULLU.RAW", FLAPPY_SKULL_BC);
	util_gfx_load_raw(state.pipe_raw, "FLAPPY/PIPE.RAW", FLAPPY_PIPE_BC);

	//initialize the pipes
	for (uint8_t i = 0; i < FLAPPY_PIPE_COUNT; i++) {
		state.pipes[i].x = GFX_WIDTH + (i * FLAPPY_PIPE_SPACING);
		state.pipes[i].h = util_math_rand32_max(GFX_HEIGHT - (2 * FLAPPY_PIPE_MIN_HEIGHT) - FLAPPY_PIPE_GAP);
		state.pipes[i].passed = false;
	}

	__draw(&state);

	err_code = app_timer_create(&m_flappy_defcon_timer, APP_TIMER_MODE_REPEATED, __flappy_timer_handler);
	APP_ERROR_CHECK(err_code);

	//Force graphics to draw
	util_gfx_invalidate();

	uint32_t ticks = APP_TIMER_TICKS(FLAPPY_STEP_MS, UTIL_TIMER_PRESCALER);
	err_code = app_timer_start(m_flappy_defcon_timer, ticks, (void *) &state);
	APP_ERROR_CHECK(err_code);

	while (!state.exit_flag) {
		err_code = sd_app_evt_wait();
		APP_ERROR_CHECK(err_code);
		app_sched_execute();
	}

	app_timer_stop(m_flappy_defcon_timer);

	nrf_delay_ms(2000);
	mbp_ui_popup("Flappy", "Game Over!");
	app_sched_resume();
	mbp_cigar_eyes_start();
}
