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
#ifndef MBP_MASTER_H_
#define MBP_MASTER_H_

//Master C2 Commands
#define MASTER_C2_CMD_LEVEL			0xFC
#define MASTER_C2_CMD_PLAY			0x9F
#define MASTER_C2_CMD_POINTS		0x6A
#define MASTER_C2_CMD_UNLOCK		0x29
#define MASTER_C2_CMD_SERVICES		0x1F
#define MASTER_C2_CMD_STOP			0x22

//Master play modes
#define MASTER_C2_PLAY_RICKROLL		0xE9
#define MASTER_C2_PLAY_WHATS_UP		0x3f
#define MASTER_C2_PLAY_NYAN			0x5F
#define MASTER_C2_PLAY_MAJOR_LAZER	0x27
#define MASTER_C2_PLAY_TOAD			0x64
#define MASTER_C2_PLAY_DAMON		0x12
#define MASTER_C2_PLAY_BENDER		0xF3


//Master to human commands
#define MASTER_ACTION_FREE_0DAY		0xC8CBC39A
#define MASTER_ACTION_FREE_POINTS	0xE1099021
#define MASTER_ACTION_LEVEL_UP		0x36E58BBE
#define MASTER_ACTION_UNLOCK		0xDEA48230

typedef struct {
	uint16_t seq;
	uint8_t cmd;
	uint8_t data;
} master_c2_t;

extern bool mbp_master_c2_infected();
extern void mbp_master_c2_process(master_c2_t c2);
extern void mbp_master_human();
extern void mbp_master_menu_cheats();
extern void mbp_master_menu_main();

#endif /* MBP_MASTER_H_ */
