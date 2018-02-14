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

#define BUTTON_WAIT_MS		200
#define NEXT 				(p_chip8->pc += 2)
#define WIDTH				128
#define HEIGHT				64

#define US_TO_TICKS(US, PRESCALER)\
            ((uint32_t)ROUNDED_DIV((US) * (uint64_t)APP_TIMER_CLOCK_FREQ / 1000, ((PRESCALER) + 1)))

static const uint8_t chip8_fontset[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0,
		0x20, 0x60, 0x20, 0x20, 0x70,
		0xF0, 0x10, 0xF0, 0x80, 0xF0,
		0xF0, 0x10, 0xF0, 0x10, 0xF0,
		0x90, 0x90, 0xF0, 0x10, 0x10,
		0xF0, 0x80, 0xF0, 0x10, 0xF0,
		0xF0, 0x80, 0xF0, 0x90, 0xF0,
		0xF0, 0x10, 0x20, 0x40, 0x40,
		0xF0, 0x90, 0xF0, 0x90, 0xF0,
		0xF0, 0x90, 0xF0, 0x10, 0xF0,
		0xF0, 0x90, 0xF0, 0x90, 0x90,
		0xE0, 0x90, 0xE0, 0x90, 0xE0,
		0xF0, 0x80, 0x80, 0x80, 0xF0,
		0xE0, 0x90, 0x90, 0x90, 0xE0,
		0xF0, 0x80, 0xF0, 0x80, 0xF0,
		0xF0, 0x80, 0xF0, 0x80, 0x80
};

//High resolution font set for Super Chip
static const uint8_t chip8_fontset_high[160] = {
		0xF0, 0xF0, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xF0, 0xF0,
		0x20, 0x20, 0x60, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70, 0x70, // 1
		0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, // 2
		0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, // 3
		0x90, 0x90, 0x90, 0x90, 0xF0, 0xF0, 0x10, 0x10, 0x10, 0x10, // 4
		0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, // 5
		0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, // 6
		0xF0, 0xF0, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40, // 7
		0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, // 8
		0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, // 9
		0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, 0x90, 0x90, 0x90, 0x90, // A
		0xE0, 0xE0, 0x90, 0x90, 0xE0, 0xE0, 0x90, 0x90, 0xE0, 0xE0, // B
		0xF0, 0xF0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF0, 0xF0, // C
		0xE0, 0xE0, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xE0, 0xE0, // D
		0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, // E
		0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, 0x80, 0x80, 0x80, 0x80  // F
		};

APP_TIMER_DEF(m_chip8_timer);

/**
 * @brief Initialize the chip-8 emulator state
 */
static void __init(chip8_t *p_chip8, chip8_game_t *p_game) {
	p_chip8->pc = PC_START;  	// Program counter starts at 0x200
	p_chip8->opcode = 0;   	// Reset current opcode
	p_chip8->I = 0;      	// Reset index register
	p_chip8->sp = 0;      	// Reset stack pointer
	p_chip8->draw_flag = 1;
	p_chip8->extended_graphics_flag = 0;
	p_chip8->exit_flag = 0;
	p_chip8->p_game = p_game;

	memset(p_chip8->memory, '\0', 4096);	//Clear memory
	memset(p_chip8->stack, '\0', 16);		//Clear stack
	memset(p_chip8->V, '\0', 16); 			//Clear registers
	memset(p_chip8->gfx, '\0', WIDTH * HEIGHT);	//Clear graphics

	//Load font set into memory
	memcpy(p_chip8->memory, chip8_fontset, 80);
	//Load extended hires fontset into memory
	memcpy(p_chip8->memory + 80, chip8_fontset_high, 160);

	for (uint32_t i = 0; i < p_game->len; ++i)
		p_chip8->memory[i + PC_START] = p_game->game_data[i];
}

void __cycle(chip8_t *p_chip8) {
	uint8_t src, n, s, k, w, x, y;
	int32_t result;
	chip8_game_t *p_game = p_chip8->p_game;

	// Fetch Opcode
	p_chip8->draw_flag = 0;
	p_chip8->opcode = p_chip8->memory[p_chip8->pc] << 8
			| p_chip8->memory[p_chip8->pc + 1];

	// Decode Opcode & execute
	switch (p_chip8->opcode & 0xF000) {
	case 0x0000:		//Default is jump but some special codes exist with 0x0 as upper 4-bits
		if (p_chip8->opcode == 0x00E0) {	//CLS
			util_gfx_fill_screen(mbp_state_chip8_bg_color_get());
			memset(p_chip8->gfx, 0x80, WIDTH * HEIGHT);
			p_chip8->draw_flag = 0;
			NEXT;
		}
		//Return
		else if (p_chip8->opcode == 0x00EE) {
			p_chip8->pc = p_chip8->stack[--p_chip8->sp];	//Pop the stack
		}
		//SCHIP 0x00CN Scroll display N lines down
		else if ((p_chip8->opcode & 0x00F0) == 0x00C0) {
			n = p_chip8->opcode & 0x000F;
			for (int8_t yy = 63 - n; yy >= 0 - n; yy--) {
				for (uint8_t xx = 0; xx < WIDTH; xx++) {
					src = 0;
					if (yy >= 0) {
						src = p_chip8->gfx[(yy * 128) + xx];
					}

					uint8_t dst = p_chip8->gfx[((yy + n) * 128) + xx];

					//If new pixel needs to change to a 1, mark it that way
					if ((src & 0x1) > 0 && (dst & 0x1) == 0) {
						p_chip8->gfx[((yy + n) * 128) + xx] = 0x1;
					}
					//If new pixel needs to change to a 0, mark it that way
					else if ((src & 0x1) == 0 && (dst & 0x1) > 0) {
						p_chip8->gfx[((yy + n) * 128) + xx] = 0x0;
					}
				}
			}

			p_chip8->draw_flag = 1;
			NEXT;
		}
		//SCHIP 0x00FB Scroll display 4 pixels right
		else if (p_chip8->opcode == 0x00FB) {
			for (int16_t yy = 0; yy < 64; yy++) {
				for (int16_t xx = 128; xx >= 4; xx--) {
					src = p_chip8->gfx[(yy * 128) + xx - 4];

					uint8_t new_b = p_chip8->gfx[(yy * 128) + xx];

					//If new pixel needs to change to a 1, mark it that way
					if ((src & 0x1) > 0 && (new_b & 0x1) == 0) {
						p_chip8->gfx[(yy * 128) + xx] = 0x1;
					}
					//If new pixel needs to change to a 0, mark it that way
					else if ((src & 0x1) == 0 && (new_b & 0x1) > 0) {
						p_chip8->gfx[(yy * 128) + xx] = 0x0;
					}
				}

				//Blank the 4 pixels on the left
				for (int8_t xx = 3; xx >= 0; xx--) {
					p_chip8->gfx[(yy * 128) + xx] = 0x0;
				}
			}
			p_chip8->draw_flag = true;
			NEXT;
		}
		//SCHIP 0x00FC Scroll display 4 pixels left
		else if (p_chip8->opcode == 0x00FC) {
			for (int16_t yy = 0; yy < 64; yy++) {
				for (int16_t xx = 0; xx < 128 - 4; xx++) {
					src = p_chip8->gfx[(yy * 128) + xx + 4];
					uint8_t new_b = p_chip8->gfx[(yy * 128) + xx];

					//If new pixel needs to change to a 1, mark it that way
					if ((src & 0x1) > 0 && (new_b & 0x1) == 0) {
						p_chip8->gfx[(yy * 128) + xx] = 0x1;
					}
					//If new pixel needs to change to a 0, mark it that way
					else if ((src & 0x1) == 0 && (new_b & 0x1) > 0) {
						p_chip8->gfx[(yy * 128) + xx] = 0x0;
					}
				}

				//Blank the 4 pixels on the right
				for (int16_t xx = 124; xx < 128; xx++) {
					p_chip8->gfx[(yy * 128) + xx] = 0x0;
				}
			}
			p_chip8->draw_flag = true;
			NEXT;
		}
		//SCHIP 0x00FD Exit
		else if (p_chip8->opcode == 0x00FD) {
			p_chip8->exit_flag = 1;
		}
		//SCHIP 0x00FE Disable extended graphics
		else if (p_chip8->opcode == 0x00FE) {
			p_chip8->extended_graphics_flag = 0;
			NEXT;
		}
		//SCHIP 0x00FF Enable extended graphics
		else if (p_chip8->opcode == 0x00FF) {
			p_chip8->extended_graphics_flag = 1;
			NEXT;
		}
		break;

		// 1NNN: Jump to NNN
	case 0x1000:
		p_chip8->pc = p_chip8->opcode & 0x0FFF;
		break;
	case 0x2000: //Call subroutine at NNN
		p_chip8->stack[p_chip8->sp] = p_chip8->pc + 2;	//Push the stack
		p_chip8->sp++;
		p_chip8->pc = p_chip8->opcode & 0xFFF;
		break;
	case 0x3000: //Skip next instruction if Vx = kk 0x3XKK
		x = (p_chip8->opcode & 0x0F00) >> 8;
		k = p_chip8->opcode & 0x00FF;
		if (p_chip8->V[x] == k) {
			NEXT;	//skip
		}
		NEXT;
		break;
	case 0x4000: //4xkk - SNE Vx, byte		Skip next instruction if Vx != kk.
		x = (p_chip8->opcode & 0x0F00) >> 8;
		k = p_chip8->opcode & 0x00FF;
		if (p_chip8->V[x] != k) {
			NEXT;	//skip
		}
		NEXT;
		break;
	case 0x5000: //5xy0 - SE Vx, Vy		Skip next instruction if Vx = Vy.
		x = (p_chip8->opcode & 0x0F00) >> 8;
		y = (p_chip8->opcode & 0x00F0) >> 4;
		if (p_chip8->V[x] == p_chip8->V[y]) {
			NEXT;	//skip
		}
		NEXT;
		break;
	case 0x6000: //6xkk - LD Vx, byte	Set Vx = kk.
		x = (p_chip8->opcode & 0x0F00) >> 8;
		k = p_chip8->opcode & 0x00FF;
		p_chip8->V[x] = k;
		NEXT;
		break;
	case 0x7000: //7xkk - ADD Vx, byte	Set Vx = Vx + kk.
		x = (p_chip8->opcode & 0x0F00) >> 8;
		k = p_chip8->opcode & 0x00FF;
		p_chip8->V[x] += k;
		NEXT;
		break;
	case 0x8000: //register operations 8xys (s changes operation)
		x = (p_chip8->opcode & 0x0F00) >> 8;
		y = (p_chip8->opcode & 0x00F0) >> 4;
		s = (p_chip8->opcode & 0x000F);
		switch (s) {
		case 0: //LD Vx, Vy	Set Vx = Vy
			p_chip8->V[x] = p_chip8->V[y];
			break;
		case 1: //OR Vx, Vy Set Vx = Vx OR Vy.
			p_chip8->V[x] = p_chip8->V[x] | p_chip8->V[y];
			break;
		case 2: //AND Vx, Vy Set Vx = Vx AND Vy.
			p_chip8->V[x] = p_chip8->V[x] & p_chip8->V[y];
			break;
		case 3: //XOR Vx, Vy Set Vx = Vx XOR Vy.
			p_chip8->V[x] = p_chip8->V[x] ^ p_chip8->V[y];
			break;
		case 4: //ADD Vx, Vy Set Vx = Vx + Vy, set VF = carry.
			result = p_chip8->V[x] + p_chip8->V[y];
			//Set carry bit
			if (result > 0xFF) {
				p_chip8->V[0xF] = 1;
//				result -= 0xFF;
			} else {
				p_chip8->V[0xF] = 0;
			}
			//store result lose precision
			p_chip8->V[x] = (uint8_t) (result & 0xFF);
			break;
		case 5: //SUB Vx, Vy Set Vx = Vx - Vy, set VF = NOT borrow.
			if (p_chip8->V[x] >= p_chip8->V[y])
				p_chip8->V[0xF] = 1;
			else
				p_chip8->V[0xF] = 0;
			p_chip8->V[x] = p_chip8->V[x] - p_chip8->V[y];
			break;
		case 6: //SHR Vx {, Vy} Set Vx = Vy >> 1.
			p_chip8->V[0xF] = p_chip8->V[x] & 0x1;
			p_chip8->V[x] = p_chip8->V[x] >> 1;
			break;
		case 7: //SUBN Vx, Vy	Set Vx = Vy - Vx, set VF = NOT borrow.
			if (p_chip8->V[y] >= p_chip8->V[x])
				p_chip8->V[0xF] = 1;
			else
				p_chip8->V[0xF] = 0;
			p_chip8->V[x] = p_chip8->V[y] - p_chip8->V[x];
			break;
		case 0xE: //SHL Vx {, Vy}	Set Vx = Vy << 1.
			p_chip8->V[0xF] = (p_chip8->V[x] & 0x80) >> 7;
			p_chip8->V[x] = p_chip8->V[x] << 1;
			break;
		}
		NEXT;
		break;

	case 0x9000: //SNE Vx, Vy	Skip next instruction if Vx != Vy.
		x = (p_chip8->opcode & 0x0F00) >> 8;
		y = (p_chip8->opcode & 0x00F0) >> 4;
		if (p_chip8->V[x] != p_chip8->V[y]) {
			NEXT;
		}
		NEXT;
		break;

	case 0xA000: // ANNN: Sets I to NNN
		p_chip8->I = p_chip8->opcode & 0x0FFF;
		NEXT;
		break;

	case 0xB000: //JP V0, addr	Jump to location nnn + V0.
		p_chip8->pc = p_chip8->V[0] + (p_chip8->opcode & 0x0FFF);
		break;

	case 0xC000: //Cxkk RND Vx, byte	Set Vx = random byte AND kk.
		x = (p_chip8->opcode & 0x0F00) >> 8;
		p_chip8->V[x] = util_math_rand8() & (p_chip8->opcode & 0x00FF);
		NEXT;
		break;

	case 0xD000: //Dxyn - DRW Vx, Vy, nibble	Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
		;
		int8_t coord_x = p_chip8->V[(p_chip8->opcode & 0x0F00) >> 8];
		int8_t coord_y = p_chip8->V[(p_chip8->opcode & 0x00F0) >> 4];
		n = (p_chip8->opcode & 0x000F);	//height
		uint8_t pixel;
		uint32_t gi;	//Buffer for current pixel value from gfx memory

		//Test for 16x16 extended sprite (SCHIP)
		if (n == 0) {
			n = 16;
			w = 16;
		} else {
			w = 8;
		}

		p_chip8->V[0xF] = 0;
		uint8_t x_offset = 0;

		for (uint16_t i = 0; i < (w * n / 8); i++) {
			//Read a byte from memory for the sprite
			pixel = p_chip8->memory[p_chip8->I + i];
			//Read each bit from the byte
			for (uint8_t bit_offset = 0; bit_offset < 8; bit_offset++) {

				//Clip y coordinate
				if (coord_y >= 0 && coord_y < 64) {
					//bit is a 1
					if ((pixel & (0x80 >> bit_offset)) != 0) {

						//Get the value of the pixel from gfx buffer stripping off extra
						gi = (coord_y * 128) + x_offset + coord_x;
						uint8_t b = p_chip8->gfx[gi] & 0x1;
						//data that was stuffed in there
						if (b == 1) {
							p_chip8->V[0xF] = 1;
							b = 0;
						} else {
							b = 1;
						}

						//Write to the graphics buffer safely
						if (gi < (64 * 128)) {
							p_chip8->gfx[gi] = b;
						}
					}
				}

				//Move to the next pixel horizontally
				x_offset++;

				//Wrap to the next row
				if (x_offset >= w) {
					coord_y++;
					x_offset = 0;
				}
			}
		}

		p_chip8->draw_flag = 1;

		NEXT;
		break;

	case 0xE000:
		x = (p_chip8->opcode & 0x0F00) >> 8;
		uint8_t button = util_button_state();
		uint8_t key_state = button & p_game->key_mappings[p_chip8->V[x]];

		//Ex9E - SKP Vx Skip next instruction if key with the value of Vx is pressed.
		if ((p_chip8->opcode & 0x00FF) == 0x009E) {
			if (key_state > 0) {
				NEXT;
			}
		}
		//ExA1 - SKNP Vx Skip next instruction if key with the value of Vx is not pressed.
		else if ((p_chip8->opcode & 0x00FF) == 0x00A1) {
			if (key_state == 0) {
				NEXT;
			}
		}

		NEXT;
		break;

	case 0xF000:
		x = (p_chip8->opcode & 0x0F00) >> 8;

		//Fx07 - LD Vx, DT Set Vx = delay timer value.
		//VVV
		if ((p_chip8->opcode & 0x00FF) == 0x0007) {
			p_chip8->V[x] = p_chip8->delay_timer;
		}
		//Fx0A - LD Vx, K Wait for a key press, store the value of the key in Vx.
		//VVV
		else if ((p_chip8->opcode & 0x00FF) == 0x000A) {
//			util_button_clear();
			uint8_t button = util_button_wait();
			nrf_delay_ms(BUTTON_WAIT_MS);

			//Determine if button is pressed by comparing masks in the mapping
			//to the current button state.
			for (uint8_t i = 0; i < 16; i++) {
				if ((button & p_game->key_mappings[i]) > 0) {
					p_chip8->V[x] = i;
				}
			}
		}
		//Fx15 - LD DT, Vx Set delay timer = Vx.
		//VVV
		else if ((p_chip8->opcode & 0x00FF) == 0x0015) {
			p_chip8->delay_timer = p_chip8->V[x];
		}
		//Fx18 - LD ST, Vx Set sound timer = Vx.
		//VVV
		else if ((p_chip8->opcode & 0x00FF) == 0x0018) {
			p_chip8->sound_timer = p_chip8->V[x];
		}
		//Fx1E - ADD I, Vx Set I = I + Vx.
		//VVV
		else if ((p_chip8->opcode & 0x00FF) == 0x001E) {
			p_chip8->I += p_chip8->V[x];

			//Handle overflow
			if (p_chip8->I > 0xFFF) {
				p_chip8->I -= 0xFFF;
				p_chip8->V[0xF] = 1;
			} else {
				p_chip8->V[0xF] = 0;
			}
		}
		//Fx29 LD F, Vx   Set I = location of sprite for digit Vx.
		else if ((p_chip8->opcode & 0x00FF) == 0x0029) {
			p_chip8->I = p_chip8->V[x] * 5;
		}
		//SCHIP Fx30 LD F, Vx   Set I = location of extended 10-byte font for digit Vx.
		//10-byte fonts start after the low res fonts in memory
		else if ((p_chip8->opcode & 0x00FF) == 0x0030) {
			p_chip8->I = 80 + (p_chip8->V[x] * 10);
		}
		//Fx33 - LD B, Vx Store BCD representation of Vx in memory locations I, I+1, and I+2
		else if ((p_chip8->opcode & 0x00FF) == 0x0033) {
			p_chip8->memory[p_chip8->I] = (p_chip8->V[x] / 100);
			p_chip8->memory[p_chip8->I + 1] = ((p_chip8->V[x] / 10) % 10);
			p_chip8->memory[p_chip8->I + 2] = ((p_chip8->V[x] % 100) % 10);
		}
		//Fx55 - LD [I], Vx Store registers V0 through Vx in memory starting at location I.
		else if ((p_chip8->opcode & 0x00FF) == 0x0055) {
			for (uint8_t i = 0; i <= x; i++) {
				p_chip8->memory[p_chip8->I + i] = p_chip8->V[i];
			}
		}
		//Fx65 - LD Vx, [I] Read registers V0 through Vx from memory starting at location I.
		else if ((p_chip8->opcode & 0x00FF) == 0x0065) {
			for (uint8_t i = 0; i <= x; i++) {
				p_chip8->V[i] = p_chip8->memory[p_chip8->I + i];
			}
		}
		//SCHIP Fx75 Store V0..VX in RPL user flags (X <= 7)
		else if ((p_chip8->opcode & 0x00FF) == 0x0075) {
			x = (x % 8);
			for (uint8_t i = 0; i <= x; i++) {
				p_chip8->rpl[i] = p_chip8->V[i];
			}
		}
		//SCHIP Fx85 Read V0..VX from RPL user flags (X <= 7)
		else if ((p_chip8->opcode & 0x00FF) == 0x0085) {
			x = (x % 8);
			for (uint8_t i = 0; i <= x; i++) {
				p_chip8->V[i] = p_chip8->rpl[i];
			}
		}
		NEXT;
		break;
	default:
		break;
	}

	// Update timers
	if (p_chip8->delay_timer > 0)
		p_chip8->delay_timer--;

	if (p_chip8->sound_timer > 0) {
		if (p_chip8->sound_timer == 1) {
			if(mbp_state_game_led_sound_get()){
				//Map Sound to LED Blinks
				uint16_t fg_color_565 = mbp_state_chip8_fg_color_get();
				util_led_set_all_rgb(util_led_565_to_rgb(fg_color_565));
				util_led_show();
				nrf_delay_ms(5);
				util_led_clear();
			}
		}
		p_chip8->sound_timer--;
	}
}

static void __draw(volatile chip8_t *p_chip8) {
	if (!p_chip8->extended_graphics_flag) {
		for (uint8_t x = 0; x < 64; x++) {
			for (uint8_t y = 0; y < 32; y++) {
				uint8_t b = p_chip8->gfx[(y * 128) + x];
				if (b == 1 || (!util_gfx_is_valid_state() && b == 0x81)) {
					util_gfx_fill_rect(x * 2, y * 4, 2, 4, mbp_state_chip8_fg_color_get());
					p_chip8->gfx[(y * 128) + x] = 0x81;
				} else if (b == 0 || (!util_gfx_is_valid_state() && b == 0x80)) {
					util_gfx_fill_rect(x * 2, y * 4, 2, 4, mbp_state_chip8_bg_color_get());
					p_chip8->gfx[(y * 128) + x] = 0x80;
				}
			}
		}
	}

	//Extended graphics
	else {
		//	util_gfx_fill_screen(p_lcd, ILI9163_BLACK);
		for (uint8_t x = 0; x < 128; x++) {
			for (uint8_t y = 0; y < 64; y++) {
				uint8_t b = p_chip8->gfx[(y * 128) + x];
				if (b == 1 || (!util_gfx_is_valid_state() && b == 0x81)) {
					util_gfx_fill_rect(x, y * 2, 1, 2, mbp_state_chip8_fg_color_get()); //Resolves #295
					p_chip8->gfx[(y * 128) + x] = 0x81;
				} else if (b == 0 || (!util_gfx_is_valid_state() && b == 0x80)) {
					util_gfx_fill_rect(x, y * 2, 1, 2, mbp_state_chip8_bg_color_get());
					p_chip8->gfx[(y * 128) + x] = 0x80;
				}
			}
		}
	}
}

static void __chip8_timer_handler(void * p_data) {
	chip8_t *p_c8 = (chip8_t *) p_data;

	__cycle(p_c8);
	app_sched_execute();

	//Draw the current game state
	if (p_c8->draw_flag > 0 || !util_gfx_is_valid_state()) {
		__draw(p_c8);
		util_gfx_validate();
	}

	if (util_button_left() && util_button_down() && util_button_action()) { //Resolves #245
		nrf_delay_ms(1000);
		if (util_button_left() && util_button_down() && util_button_action()) {
			p_c8->exit_flag = 1;
		}
	}
}

/**
 * Called by sub menu when a game is picked.
 * Read associated .cfg file for settings and start the game.
 */
static void __chip8_callback(void *data) {
	char *filename = (char*) data;
	chip8_run_file(filename);
}

void chip8_menu() {
	menu_t chip8_menu;
	menu_item_t chip8_menu_items[100];
	chip8_menu.items = chip8_menu_items;
	chip8_menu.title = "Games";
	chip8_menu.count = 0;
	chip8_menu.selected = 0;
	chip8_menu.top = 0;

	FRESULT result;
	DIR dir;
	static FILINFO fno;

	result = f_opendir(&dir, "CHIP8"); /* Open the directory */
	if (result == FR_OK) {
		for (;;) {
			result = f_readdir(&dir, &fno); /* Read a directory item */
			if (result != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR) { /* It is a directory */
				//ignore
			} else { /* It is a file. */
				char *ext = strrchr(fno.fname, '.') + 1;

				//Look for chip-8 or superchip files
				if (strcmp(ext, "CH8") == 0 || strcmp(ext, "SC") == 0) {

					menu_item_t item;
					item.callback = &__chip8_callback;
					item.icon = NULL;
					item.preview = NULL;
					item.text = (char *) malloc(16);
					item.data = (char *) malloc(16);

					snprintf(item.text, ext - fno.fname, "%s", fno.fname);
					sprintf(item.data, "%s", fno.fname);
					chip8_menu_items[chip8_menu.count++] = item;
				}
			}
		}
		f_closedir(&dir);
	}

	mbp_sort_menu(&chip8_menu);

	mbp_submenu(&chip8_menu);

	//Cleanup
	for (uint16_t i = 0; i < chip8_menu.count; i++) {
		free(chip8_menu_items[i].data);
		free(chip8_menu_items[i].text);
	}
}

void chip8_run(chip8_game_t *p_game) {

	chip8_t c8;
	__init(&c8, p_game);

	if (mbp_ui_toggle_popup(p_game->name, 0, "Start", "Cancel", p_game->note) == 1) {
		return;
	}

	//How to Exit Game  - Pop Up Hint - Resolves #283
	if (mbp_state_game_exit_pop_up_get()) {
		mbp_ui_popup("Hint", "To exit game hold LEFT + DOWN + ACTION for 1 second. This pop up can be disabled in badge settings.");
	}

	uint32_t err_code;
	err_code = app_timer_create(&m_chip8_timer, APP_TIMER_MODE_REPEATED, __chip8_timer_handler);
	APP_ERROR_CHECK(err_code);

	//Stop tooth/eyes
	util_led_clear();
	mbp_tooth_eye_stop();
	app_sched_pause();

	uint32_t ticks = APP_TIMER_TICKS(1000 / p_game->hz, UTIL_TIMER_PRESCALER);
	err_code = app_timer_start(m_chip8_timer, ticks, (void *) &c8);
	APP_ERROR_CHECK(err_code);

	while (!c8.exit_flag) {
		err_code = sd_app_evt_wait();
		APP_ERROR_CHECK(err_code);
	}

	app_timer_stop(m_chip8_timer);
	util_button_clear();

	//restart tooth/eyes
	app_sched_resume();
	mbp_tooth_eye_start();
	util_led_clear();
	util_gfx_invalidate();
}

void chip8_run_file(char *filename) {
	chip8_game_t game;
	char *ext = strrchr(filename, '.') + 1;
	char base_filename[8 + 1 + 3 + 1];				//<filename>.<ext>\0
	char cfg_filename[5 + 1 + 8 + 1 + 3 + 1];	//CHIP8/<filename>.<ext>\0
	bool schip = strcmp(ext, "SC") == 0;

	//Prepend full path
	char path[strlen(filename) + 7];
	sprintf(path, "CHIP8/%s", filename);

	//Parse the base file name
	snprintf(base_filename, ext - filename, "%s", filename);
	//Create the related .cfg file name - every [s]chip-8 game must have a .cfg file
	sprintf(cfg_filename, "CHIP8/%s.CFG", base_filename);

	UINT count;
	FIL cfg, game_file;
	char config[1024];

	//Open the config file for reading
	FRESULT result = f_open(&cfg, cfg_filename, FA_READ | FA_OPEN_EXISTING);
	if (result != FR_OK) {
		mbp_ui_error("Could not open file");
		return;
	}
	f_read(&cfg, config, 1024, &count);

	//Set the game name to the name of the file
	sprintf(game.name, "%s", base_filename);

	char *hz = strstr(config, "CLOCK");
	hz = strchr(hz, '=') + 1;
	game.hz = strtol(hz, NULL, 10);

	//Parse the note
	char *note = strstr(config, "NOTE");
	note = strchr(note, '=') + 1;
	char *newline = strchr(note, '\n');
	game.note = malloc(newline - note + 1);
	snprintf(game.note, newline - note + 1, "%s", note + 1);
	f_close(&cfg);

	//Load game data
	uint32_t fsize = util_sd_file_size(path);
	if (fsize == 0) {
		mbp_ui_error("Could not stat game file");
		free(game.note);
		return;
	}
	result = f_open(&game_file, path, FA_READ | FA_OPEN_EXISTING);
	if (result != FR_OK) {
		mbp_ui_error("Could not open game file");
		free(game.note);
		return;
	}
	game.game_data = (uint8_t *) malloc(fsize);
	f_read(&game_file, game.game_data, fsize, &count);
	f_close(&game_file);
	game.len = count;

	//Reset key mappings
	memset(game.key_mappings, 0, 16);

	char *up = strstr(config, "U = ") + 4;
	char *down = strstr(config, "D = ") + 4;
	char *left = strstr(config, "L = ") + 4;
	char *right = strstr(config, "R = ") + 4;
	char *action = strstr(config, "A = ") + 4;

	if (schip) {
		char mapping[] = { '.', '7', '8', '9', //0-3
				'4', '5', '6', '1', //4-7
				'2', '3', '0', '_', //8-B
				'/', '*', '-', '+', //C-F
				};
		for (uint8_t i = 0; i < 16; i++) {
			if (up[0] == mapping[i]) {
				game.key_mappings[i] = BUTTON_MASK_UP;
			}
			if (down[0] == mapping[i]) {
				game.key_mappings[i] = BUTTON_MASK_DOWN;
			}
			if (left[0] == mapping[i]) {
				game.key_mappings[i] = BUTTON_MASK_LEFT;
			}
			if (right[0] == mapping[i]) {
				game.key_mappings[i] = BUTTON_MASK_RIGHT;
			}
			if (action[0] == mapping[i]) {
				game.key_mappings[i] = BUTTON_MASK_ACTION;
			}
		}
	} else {
		char mapping[] = {
				'0', '1', '2', '3', //0-3
				'4', '5', '6', '7', //4-7
				'8', '9', 'A', 'B', //8-B
				'C', 'D', 'E', 'F', //C-F
				};
		for (uint8_t i = 0; i < 16; i++) {
			if (up[0] == mapping[i]) {
				game.key_mappings[i] = BUTTON_MASK_UP;
			}
			if (down[0] == mapping[i]) {
				game.key_mappings[i] = BUTTON_MASK_DOWN;
			}
			if (left[0] == mapping[i]) {
				game.key_mappings[i] = BUTTON_MASK_LEFT;
			}
			if (right[0] == mapping[i]) {
				game.key_mappings[i] = BUTTON_MASK_RIGHT;
			}
			if (action[0] == mapping[i]) {
				game.key_mappings[i] = BUTTON_MASK_ACTION;
			}
		}
	}

        uint32_t start_timestamp = util_local_millis();

	//Go time
	chip8_run(&game);

        uint32_t stop_timestamp = util_local_millis();
        if ((stop_timestamp - start_timestamp) > GAME_TIME_4_SCORE) {
            add_to_score(POINTS_4_LOCAL_GAME, "gameplay"); // schedules a bling automatically
        }
        
	//cleanup
	free(game.game_data);
	free(game.note);
}
