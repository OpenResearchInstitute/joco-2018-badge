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
#ifndef BOTNET_H_
#define BOTNET_H_

#define BOTNET_EXPLOIT_SOPHISTICATION_MAX	100
#define BOTNET_MAX_EXPLOITS					10
#define BOTNET_SERVICE_COUNT				8

#define BOTNET_ATTACK_IN_PROGRESS			0
#define BOTNET_ATTACK_SUCCESS				1
#define BOTNET_ATTACK_FAILED				2

#define BOTNET_SERVICE_NAMES				{"ssh",  "https", "smtp", "pop3", "ftp", "telnet", "vnc", "netcat"}

//Payloads
#define BOTNET_PAYLOAD_COUNT			10		/*Count of all payloads available for non-worms*/
//Actual payloads. Wormable payloads must be first
#define BOTNET_PAYLOAD_BSOD				0
#define BOTNET_PAYLOAD_CLIPPY			1
#define BOTNET_PAYLOAD_DAMON			2
#define BOTNET_PAYLOAD_WINDOWSXP		3
#define BOTNET_PAYLOAD_GOATSE			4
#define BOTNET_PAYLOAD_RICKROLL			5
#define BOTNET_PAYLOAD_WANNACRY			6
#define BOTNET_PAYLOAD_TROLL			7
#define BOTNET_PAYLOAD_ROOT				8
#define BOTNET_PAYLOAD_CLONE			9
//Name so of payloads, must match indices above
#define BOTNET_PAYLOAD_NAMES			{"BSOD", "Clippy", "Matt Damon", "Windows XP", "Goatse", "Rick Roll", "Wanna Cry", "Troll", "Root", "BLE Clone"}


#define BOTNET_POINTS_MAX				700
#define COST_FIREWALL					10
#define COST_SERVICE					10

//typedef enum {
//	BSOD,
//	CLIPPY,
//	RICKROLL,
//	STEAL,
//	WINDOWS_ME
//} botnet_payloads;

typedef struct {
	uint8_t target_index;
	uint8_t sophistication;
	uint8_t upgrade_count;
} botnet_exploit_t;

typedef struct {
	botnet_exploit_t exploit;
	uint8_t payload;
	uint8_t result;
	char name[SETTING_NAME_LENGTH];
	uint16_t result_data;
	uint8_t level;
} botnet_attack_t;

typedef struct {
	char *name;
	bool enabled;
	uint8_t difficulty;
} botnet_service_t;

typedef struct {
	uint8_t avatar;
	uint16_t experience;
	uint8_t level;
	int16_t points;
	botnet_exploit_t exploits[BOTNET_MAX_EXPLOITS];
	uint8_t exploit_count;
	uint8_t firewall_state;
	botnet_service_t services[BOTNET_SERVICE_COUNT];
	uint8_t service_state;
} botnet_state_t;

extern void botnet_eval_incoming_attacking(botnet_attack_t *p_attack);
//extern void botnet_execute_payload_sch_handler(void *p_data, uint16_t length);
extern bool botnet_immune();
extern uint8_t botnet_level_get();
extern void botnet_main_screen();
extern void botnet_new();
extern void botnet_screen_pick_avatar();
extern bool botnet_service_accessible(uint8_t index);
extern void botnet_start();
extern void botnet_update_service_status();
#endif /* BOTNET_H_ */
