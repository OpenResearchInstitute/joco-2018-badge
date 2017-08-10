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

#define TERM_BLING_MODE_WHATS_UP 	0
#define TERM_BLING_MODE_DEFRAG		1
#define TERM_BLING_MODE_DATE_TIME	2
#define TERM_BLING_MODE_RICK_ROLL	3
#define TERM_BLING_MODE_MAJOR_LAZER	4
#define TERM_BLING_MODE_OWLS		5
#define TERM_BLING_MODE_BADGERS		6
#define TERM_BLING_MODE_TROLOLOL	7

static ntshell_t m_ntshell;

//Declare some functions to make them accessible later
typedef int (*USRCMDFUNC)(int argc, char **argv);
static int __cmd_date(int argc, char **argv);
static int __cmd_defrag(int argc, char **argv);
static int __cmd_emacs(int argc, char **argv);
static int __cmd_whoami(int argc, char **argv);
static int __cmd_motd(int argc, char **argv);
static int __cmd_help(int argc, char **argv);
static int __cmd_uname(int argc, char **argv);
static int __cmd_su(int argc, char **argv);
static int __cmd_exit(int argc, char **argv);
static int __cmd_led(int argc, char **argv);
static int __cmd_leds(int argc, char **argv);
static int __cmd_less(int argc, char **argv);
static int __cmd_ll(int argc, char **argv);
static int __cmd_passwd(int argc, char **argv);
static int __cmd_play(int argc, char **argv);
static int __cmd_service(int argc, char **argv);
static int __cmd_stop(int argc, char **argv);
static int __cmd_Hey(int argc, char **argv);
static int __cmd_Not(int argc, char **argv);
static int __cmd_tcl(int argc, char **argv);
static int __cmd_vim(int argc, char **argv);

typedef struct {
	char *cmd;
	char *desc;
	USRCMDFUNC func;
} cmd_table_t;

//Command list is a pre-processor include to save global ram
#define	CMD_LIST	 {	\
		{ "emacs", "GNU Emacs editor", __cmd_emacs }, \
		{ "vim", "The better? editor", __cmd_vim }, \
		{ "whoami", "Become one with yourself", __cmd_whoami }, \
		{ "motd", "Message of the Day", __cmd_motd }, \
		{ "help", "Display command desc", __cmd_help }, \
		{ "uname", "Display system info", __cmd_uname }, \
		{ "su", "Substitute user", __cmd_su }, \
		{ "exit", "End session", __cmd_exit }, \
		{ "less", "Display contents", __cmd_less }, \
		{ "ll", "Long Listing", __cmd_ll }, \
		{ "passwd", "Change Password", __cmd_passwd }, \
		{ "play", "Play bling", __cmd_play }, \
		{ "stop", "Stop bling", __cmd_stop }, \
		{ "namechg", "Change Name", __cmd_namechg }, \
		{ "service", "Access Services", __cmd_service }, \
		{ "fw", "Access Firewall", __cmd_firewall }, \
		{ "defrag", "HD Maintenance", __cmd_defrag }, \
		{ "Hey", "Say Hey", __cmd_Hey }, \
		{ "Not", "Say Hey", __cmd_Not }, \
		{ "date", "Date status", __cmd_date }, \
		{ "leds", "Set All LEDs", __cmd_leds }, \
		{ "led", "Set All LEDs", __cmd_led }, \
		{ "tcl", "Run TCL program", __cmd_tcl}, \
		{ "wall", "Leave a message", __cmd_wall } \
	};

#define CMD_LIST_COUNT 	24

uint8_t m_hey_count = 0;
uint8_t m_who_count = 0;
uint8_t m_current_user_role = 0;
uint8_t mm = 7;
uint8_t dd = 27;
uint16_t yyyy = 2017;
APP_TIMER_DEF(m_terminal_inactivity_timer);
uint32_t m_inactivity_end_time;
uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;

static int __cmd_help(int argc, char **argv) {
	mbp_term_print("Available Commands:");
	mbp_term_print("date defrag emacs");
	mbp_term_print("exit fw Hey led leds");
	mbp_term_print("less ll motd namechg");
	mbp_term_print("passwd play service");
	mbp_term_print("stop su tcl uname");
	mbp_term_print("vim wall whoami");
	mbp_term_print("\r");
	return 0;
}

static void __bling_schedule_handler(void *p_data, uint16_t length) {
	if (length != 1) {
		return;
	}

	uint8_t mode = *((uint8_t *) p_data);

	app_sched_pause();
	bool cigar_running = mbp_cigar_eyes_running();
	mbp_cigar_eyes_stop();

	util_button_clear();
	switch (mode) {
	case TERM_BLING_MODE_BADGERS:
		mbp_bling_badgers();
		break;
	case TERM_BLING_MODE_WHATS_UP:
		mbp_bling_whats_up();
		break;
	case TERM_BLING_MODE_DEFRAG:
		mbp_bling_defrag();
		break;
	case TERM_BLING_MODE_DATE_TIME:
		mbp_bling_hack_time();
		break;
	case TERM_BLING_MODE_MAJOR_LAZER:
		mbp_bling_major_lazer(NULL);
		break;
	case TERM_BLING_MODE_OWLS:
		mbp_bling_owl();
		break;
	case TERM_BLING_MODE_RICK_ROLL:
		util_gfx_draw_raw_file("/BLING/AND!XOR/RICKROLL.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, true, NULL);
		break;
	case TERM_BLING_MODE_TROLOLOL:
		mbp_bling_trololol();
		break;
	}

	util_led_clear();
	util_gfx_invalidate();
	app_sched_resume();

	//Only start cigar if previously running
	if (cigar_running) {
		mbp_cigar_eyes_start();
	}
}

static int __cmd_date(int argc, char **argv) {
	char msg1[21];
	char msg2[21];
	char msg3[5];
	bool err_flag = false;
	bool leap_year = false;

	if (m_current_user_role >= 1) {
		if ((argc == 2) && (strcmp(argv[1], "show") == 0)) {
			sprintf(msg1, "%i/", mm);
			sprintf(msg2, "%i/", dd);
			sprintf(msg3, "%i", yyyy);
			strcat(msg1, msg2);
			strcat(msg1, msg3);
			mbp_term_print(msg1);
			mbp_term_print("\r");
		}

		if (argc == 4) {
			//Convert char arrays to ints
			sscanf(argv[1], "%u", (unsigned int *) &mm);
			sscanf(argv[2], "%u", (unsigned int *) &dd);
			sscanf(argv[3], "%u", (unsigned int *) &yyyy);

			//Validate Inputs & Force Correction
			//Check for Leap Year
			if (yyyy % 400 == 0)
				leap_year = true;
			else if (yyyy % 100 == 0)
				leap_year = true;
			else if (yyyy % 4 == 0)
				leap_year = true;
			else
				leap_year = false;

			if ((yyyy < 1) || (yyyy > 2038)) {			//Bad Day Year
				err_flag = true;
			}

			if ((mm < 1) || (mm > 12)) {			//Bad Month input
				err_flag = true;
			}

			if ((dd < 1) || (dd > 31)) {			//Bad Day input
				err_flag = true;
			}
			else {
				if (((mm == 4) || (mm == 6) || (mm == 9) || (mm == 11)) && (dd == 31)) {
					//Validate those four trouble months that only have 30 days
					err_flag = true;
				}
				if ((mm == 2) && (dd > 28) && (!leap_year)) {
					err_flag = true;
				}
				if ((mm == 2) && (dd > 29) && (leap_year)) {
					err_flag = true;
				}
			}

			if (!err_flag) {
				//Print Results
				sprintf(msg1, "%i/", mm);
				sprintf(msg2, "%i/", dd);
				strcat(msg1, msg2);
				strcat(msg1, argv[3]);
				mbp_term_print(msg1);
				mbp_term_print("\r");
			} else {
				mbp_term_print("Incorrect Usage");
				mbp_term_print("  date show");
				mbp_term_print("  date mm dd yyyy");
				mbp_term_print("\r");
			}
		}
		else if (strcmp(argv[1], "show") != 0) {
			mbp_term_print("Incorrect Usage");
			mbp_term_print("  date show");
			mbp_term_print("  date mm dd yyyy");
			mbp_term_print("\r");
		}

		//HackTime
		if (yyyy < 1970) {
			mbp_term_print("ERROR!");
			mbp_term_print("Hacking too much");
			mbp_term_print("Time!!!!!!!!!!!!");
			mbp_term_print("UNLOCK HACKERMAN");
			mbp_term_print("\r");
			mbp_term_print("Type: stop");
			mbp_term_print("To Exit Bling-mode");
			mbp_term_print("& Return to Terminal");
			mbp_term_print("\r");

			//Unlock!
			uint16_t unlock = mbp_state_unlock_get();
			mbp_state_unlock_set(unlock | UNLOCK_MASK_DATE_TIME);
			mbp_state_save();

			//Fire off the bling mode
			uint8_t mode = TERM_BLING_MODE_DATE_TIME;
			app_sched_event_put(&mode, 1, __bling_schedule_handler);
		}
	}
	else {
		mbp_term_print("Permission Denied");
		mbp_term_print("\r");
	}
	return 0;
}

static int __cmd_wall(int argc, char **argv) {
	if (argc == 1) {
		mbp_term_print("Incorrect Usage");
		mbp_term_print("Leave a message:");
		mbp_term_print("Limit 15 chars");
		mbp_term_print("  wall foo bar");
		mbp_term_print("Show Messages:");
		mbp_term_print("  wall --show");
		mbp_term_print("\r");
	}
	else {
		if (strcmp(argv[1], "--show") == 0) {
			mbp_state_wall_show();
		} else {
			//Process - take argc, iterate and create string of each argv
			char msg[20] = "";
			char temp[20] = "";
			for (int i = 1; i < argc; i++) {
				sprintf(temp, "%s ", argv[i]);
				strcat(msg, temp);
			}
			//Push on to the wall
			mbp_state_wall_put(msg);
		}
	}
	return 0;
}

static int __cmd_led(int argc, char **argv) {
	if (m_current_user_role >= 1) {
		if (argc == 5) {
			int led, r, g, b;
			//Read the input
			sscanf(argv[1], "%d", &led);
			sscanf(argv[2], "%d", &r);
			sscanf(argv[3], "%d", &g);
			sscanf(argv[4], "%d", &b);
			//Validate the input
			if (((r < 0) || (r > 255)) || ((g < 0) || (g > 255)) || ((b < 0) || (b > 255)) || ((led < 0) || (led > 14))) {
				mbp_term_print("Incorrect Usage");
				mbp_term_print("Sets an LED a RGB");
				mbp_term_print("LED Values 0-14");
				mbp_term_print("RGB Values 0-255");
				mbp_term_print("  led 1 255 255 255");
				mbp_term_print("\r");
			}
			else {
				//Print the input
				util_led_set(led, r, g, b);
				util_led_show();
			}
		}
		else {
			mbp_term_print("Incorrect Usage");
			mbp_term_print("Sets an LED a RGB");
			mbp_term_print("LED Values 0-14");
			mbp_term_print("RGB Values 0-255");
			mbp_term_print("  led 1 255 255 255");
			mbp_term_print("\r");
		}
	}
	else {
		mbp_term_print("Permission Denied");
		mbp_term_print("\r");
	}
	return 0;
}

static int __cmd_leds(int argc, char **argv) {
	if (m_current_user_role >= 1) {
		if (argc == 4) {
			int r, g, b;
			//Read the input
			sscanf(argv[1], "%d", &r);
			sscanf(argv[2], "%d", &g);
			sscanf(argv[3], "%d", &b);
			//Validate the input
			if (((r < 0) || (r > 255)) || ((g < 0) || (g > 255)) || ((b < 0) || (b > 255))) {
				mbp_term_print("Incorrect Usage");
				mbp_term_print("Sets all LEDs a RGB");
				mbp_term_print("Values 0-255");
				mbp_term_print("  leds 255 255 255");
				mbp_term_print("\r");
			}
			else {
				//Print the input
				util_led_set_all(r, g, b);
				util_led_show();
			}
		}
		else {
			mbp_term_print("Incorrect Usage");
			mbp_term_print("Sets all LEDs a RGB");
			mbp_term_print("Values 0-255");
			mbp_term_print("  leds 255 255 255");
			mbp_term_print("\r");
		}
	}
	else {
		mbp_term_print("Permission Denied");
		mbp_term_print("\r");
	}
	return 0;
}

static int __cmd_Not(int argc, char **argv) {
	if (argc == 3) {
		if ((m_who_count == 2) && (strcmp(argv[1], "much.") == 0) && (strcmp(argv[2], "Brb.") == 0)) {
			mbp_term_print("WHATS UP UNLOCKED");
			mbp_term_print("\r");
			mbp_term_print("Type: stop");
			mbp_term_print("To Exit Bling-mode");
			mbp_term_print("& Return to Terminal");
			mbp_term_print("\r");

			//Unlock!
			uint16_t unlock = mbp_state_unlock_get();
			unlock |= UNLOCK_MASK_WHATS_UP;
			mbp_state_unlock_set(unlock);
			mbp_state_save();

			//reset whats up counter
			m_who_count = 0;

			//Schedule bling
			uint8_t mode = TERM_BLING_MODE_WHATS_UP;
			app_sched_event_put(&mode, 1, __bling_schedule_handler);
		}
	}
	return 0;
}

static int __cmd_Hey(int argc, char **argv) {
	/* HeMan What's Up Unlock
	 TRIGGER INPUT FROM USER: Hey
	 BENDER: Hey STUD!
	 BENDER: LOL j/k
	 TRIGGER INPUT FROM USER: Hey
	 BENDER: What's going on??
	 BENDER: ;)
	 TRIGGER INPUT FROM USER: Not much. Brb.
	 */
	if (argc == 1) {
		if (m_who_count == 0) {
			mbp_term_print("Hey STUD!");
			mbp_term_print("LOL j/k");
			m_who_count++;
		}
		else if (m_who_count == 1) {
			mbp_term_print("What's going on??");
			mbp_term_print(";)");
			m_who_count++;
		}
	}
	return 0;
}

static int __cmd_defrag(int argc, char **argv) {
	if (m_current_user_role >= 1) {
		uint8_t mode = TERM_BLING_MODE_DEFRAG;
		uint16_t unlock = mbp_state_unlock_get();
		mbp_state_unlock_set(unlock | UNLOCK_MASK_DEFRAG);
		mbp_state_save();
		mbp_term_print("DEFRAG UNLOCKED");
		mbp_term_print("\r");
		mbp_term_print("Type: stop");
		mbp_term_print("To Exit Bling-mode");
		mbp_term_print("& Return to Terminal");
		mbp_term_print("\r");
		app_sched_event_put(&mode, 1, __bling_schedule_handler);
	}
	else {
		mbp_term_print("Permission Denied");
		mbp_term_print("\r");
	}
	return 0;
}

static int __cmd_firewall(int argc, char **argv) {
	char *service_names[] = BOTNET_SERVICE_NAMES;
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	char msg[21];
	char status[5];
	bool error_usage_flag = false;

	if (m_current_user_role != 2) {
		mbp_term_print("Permission Denied");
		mbp_term_print("\r");
		return 0;
	}

	if (argc == 2) {			//Display Firewall Status
		if (strcmp(argv[1], "status") == 0) {
			for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
				if ((p_state->firewall_state & (1 << i)) > 0) {
					sprintf(status, "allow");
				}
				else {
					sprintf(status, "deny");
				}
				sprintf(msg, "%s ", service_names[i]);
				strcat(msg, status);
				mbp_term_print(msg);
			}
			sprintf(msg, "BOTNET Points: %i", p_state->points);
			mbp_term_print(msg);
			mbp_term_print("\r");
		} else {
			error_usage_flag = true;
		}
	}
	else if (argc == 3) {
		//For those whoever see this code in the GitCloud...yeah its not elegant
		//I was rushed and did a quick copy/pasta to get shit working. deal with it.
		//If you dont like it, update it, thats why its open source <8 Hyr0n ¯\_(ツ)_/¯
		//Fixed by zapp
		if (((strcmp(argv[2], "allow") == 0) || (strcmp(argv[2], "deny") == 0))
				&& (m_current_user_role == 2)) {
			//Find the index of the service
			uint8_t service_index = 0xFF;
			for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
				if (strcmp(service_names[i], argv[1]) == 0) {
					service_index = i;
					break;
				}
			}

			if (service_index < BOTNET_SERVICE_COUNT) {
				if (strcmp(argv[2], "allow") == 0) {
					p_state->firewall_state |= (1 << service_index);
					mbp_state_save();
					mbp_term_print("Firewall Rule Modded");
					mbp_term_print("\r");
				} else if (strcmp(argv[2], "deny") == 0) {
					if (p_state->points < COST_FIREWALL) {
						char message[21];
						mbp_term_print("I am Err0r");
						sprintf(message, "Requires %d points", COST_FIREWALL);
						mbp_term_print(message);
						sprintf(message, "Current Total: %i", p_state->points);
						mbp_term_print(message);
						mbp_term_print("\r");
					} else {
						p_state->firewall_state &= ~(1 << service_index);
						p_state->points -= COST_FIREWALL;
						mbp_state_save();
						mbp_term_print("Firewall Rule Modded");
						mbp_term_print("\r");
					}
				}
			} else {
				error_usage_flag = true;
			}
		}
		//Invalid service
		else {
			error_usage_flag = true;
		}
	}

	else {		//They typed the wrong amount of arguments
		error_usage_flag = true;
	}

	if (error_usage_flag) {
		mbp_term_print("Incorrect Usage");
		mbp_term_print("  fw status");
		mbp_term_print("  fw x allow|deny");
		mbp_term_print("\r");
		error_usage_flag = false;
	}

	botnet_update_service_status();
	return 0;
}

static int __cmd_service(int argc, char **argv) {

	//Only root can run this command
	if (m_current_user_role != 2) {
		mbp_term_print("Permission Denied");
		mbp_term_print("\r");
		return 0;
	}

	//Everything after this is trusted
	char *service_names[] = BOTNET_SERVICE_NAMES;
	botnet_state_t *p_state = mbp_state_botnet_state_get();
	char msg[21];
	char status[5];
	bool error_usage_flag = false;

	//Only one argument passed
	if (argc == 2) {

		//Display Service Status
		if (strcmp(argv[1], "status") == 0) {
			for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
				if (p_state->services[i].enabled) {
					sprintf(status, "on");
				}
				else {
					sprintf(status, "off");
				}
				sprintf(msg, "%s ", service_names[i]);
				strcat(msg, status);
				mbp_term_print(msg);
			}
			sprintf(msg, "BOTNET Points: %i", p_state->points);
			mbp_term_print(msg);
			mbp_term_print("\r");
		}
		else {
			error_usage_flag = true;
		}
	}

	//Two arguments passed which can only be start and stop
	else if (argc == 3) {
		//For those whoever see this code in the GitCloud...yeah its not elegant
		//I was rushed and did a quick copy/pasta to get shit working. deal with it.
		//If you dont like it, update it, thats why its open source <8 Hyr0n ¯\_(ツ)_/¯
		//Zapp fixed it
		if ((strcmp(argv[2], "start") == 0) || (strcmp(argv[2], "stop") == 0)) {
			//Find the index of the service
			uint8_t service_index = 0xFF;
			for (uint8_t i = 0; i < BOTNET_SERVICE_COUNT; i++) {
				if (strcmp(service_names[i], argv[1]) == 0) {
					service_index = i;
					break;
				}
			}

			if (service_index < BOTNET_SERVICE_COUNT) {
				if (strcmp(argv[2], "start") == 0) {
					p_state->services[service_index].enabled = true;
					mbp_state_save();
				} else if (strcmp(argv[2], "stop") == 0) {
					if (p_state->points < COST_SERVICE) {
						char message[21];
						mbp_term_print("I am Err0r");
						sprintf(message, "Requires %d points", COST_SERVICE);
						mbp_term_print(message);
						sprintf(message, "Current Total: %i", p_state->points);
						mbp_term_print(message);
						mbp_term_print("\r");
					} else {
						p_state->services[service_index].enabled = false;
						p_state->points -= COST_SERVICE;
						mbp_state_save();
						mbp_term_print("Service State Modded");
						mbp_term_print("\r");
					}
				}
			} else {
				error_usage_flag = true;
			}
		}

		//Invalid service
		else {
			error_usage_flag = true;
		}
	}
	//else if (((strcmp(argv[2], "start") == 0) || (strcmp(argv[2], "stop") == 0)) && (m_current_user_role != 2)) {
	//	mbp_term_print("Permission Denied");
	//	mbp_term_print("\r");
	//}
	else {		//They typed the wrong ammount of arguments
		error_usage_flag = true;
	}

	if (error_usage_flag) {
		mbp_term_print("Incorrect Usage");
		mbp_term_print("  service status");
		mbp_term_print("  service x start");
		mbp_term_print("  service x stop");
		mbp_term_print("\r");
		error_usage_flag = false;
	}

	botnet_update_service_status();
	return 0;
}

static int __cmd_ll(int argc, char **argv) {
	mbp_term_print("rw-rw----");
	mbp_term_print("root scruffy ");
	mbp_term_print("/shadow.backup");
	mbp_term_print("\r");
	return 0;
}

static int __cmd_namechg(int argc, char **argv) {
	if ((argc != 2) || (strlen(argv[1]) >= SETTING_NAME_LENGTH)) {
		mbp_term_print("Incorrect Usage");
		mbp_term_print("Max Length = 8");
		mbp_term_print("  namechg newname");
		mbp_term_print("\r");
	}
	if ((argc == 2) && (strlen(argv[1]) < SETTING_NAME_LENGTH)) {
		char msg[20];
		for (uint8_t i = 0; i < strlen(argv[1]); i++) {
			bool valid_char = false;

			//Make sure upper case
			argv[1][i] = toupper(argv[1][i]);

			//Make sure it's in our char set
			for (uint8_t j = 0; j < strlen(INPUT_CHARS); j++) {
				if (INPUT_CHARS[j] == argv[1][i]) {
					valid_char = true;
					break;
				}
			}

			if (!valid_char) {
				mbp_term_print("Invalid Name\r");
				return 0;
			}
		}

		mbp_state_name_set(argv[1]);
		mbp_state_save();
		sprintf(msg, "New Name: %s", argv[1]);
		mbp_term_print(msg);
		mbp_term_print("\r");
	}
	return 0;
}

static int __cmd_passwd(int argc, char **argv) {
	if ((argc != 2) || (strlen(argv[1]) > SETTING_PW_LENGTH - 1)) {
		mbp_term_print("Incorrect Usage");
		mbp_term_print("Max Length = 8");
		mbp_term_print("  passwd 12345678");
		mbp_term_print("\r");
	}
	if ((argc == 2) && (strlen(argv[1]) <= SETTING_PW_LENGTH - 1)) {
		if (m_current_user_role == 0) {
			mbp_term_print("Error!");
			mbp_term_print("Fry not allowed pw");
			mbp_term_print("He always forgets it");
			mbp_term_print("\r");
		}
		else if (m_current_user_role == 1) {
			//Scruffy passwd
			mbp_state_pw_scruffy_set(argv[1]);
			mbp_term_print("Password Updated!");
			mbp_term_print("\r");
			mbp_state_save();
		}
		else {
			//root passwd
			mbp_state_pw_root_set(argv[1]);
			mbp_term_print("Password Updated!");
			mbp_term_print("\r");
			mbp_state_save();
		}
	}
	return 0;
}

static int __cmd_play(int argc, char **argv) {
	//Quit early if they don't have permission
	if (m_current_user_role != 2) {
		mbp_term_print("Permission Denied");
		mbp_term_print("\r");
		return 0;
	}

	bool incorrect_usage = false;

	//Check argument count
	if (argc != 2) {
		incorrect_usage = true;
	}
	//valid argument count
	else if (argc == 2) {
		uint8_t mode;
		if (strcmp(argv[1], "whatsup") == 0) {
			mode = TERM_BLING_MODE_WHATS_UP;
			app_sched_event_put(&mode, 1, __bling_schedule_handler);
		} else if (strcmp(argv[1], "rickroll") == 0) {
			mode = TERM_BLING_MODE_RICK_ROLL;
			app_sched_event_put(&mode, 1, __bling_schedule_handler);
		} else if (strcmp(argv[1], "badgers") == 0) {
			mode = TERM_BLING_MODE_BADGERS;
			app_sched_event_put(&mode, 1, __bling_schedule_handler);
		} else if (strcmp(argv[1], "lazer") == 0) {
			mode = TERM_BLING_MODE_MAJOR_LAZER;
			app_sched_event_put(&mode, 1, __bling_schedule_handler);
		} else if (strcmp(argv[1], "owls") == 0) {
			mode = TERM_BLING_MODE_OWLS;
			app_sched_event_put(&mode, 1, __bling_schedule_handler);
		} else if (strcmp(argv[1], "troll") == 0) {
			mode = TERM_BLING_MODE_TROLOLOL;
			app_sched_event_put(&mode, 1, __bling_schedule_handler);
		} else {
			incorrect_usage = true;
		}
	}

	if (incorrect_usage) {
		mbp_term_print("Incorrect Usage");
		mbp_term_print("play <mode>");
		mbp_term_print("Modes: badgers owls");
		mbp_term_print("lazer rickroll troll");
		mbp_term_print("whatsup");
		mbp_term_print("\r");
		mbp_term_print("Use Badge L Button");
		mbp_term_print("To Exit Bling-mode");
		mbp_term_print("& Return to Terminal");
		mbp_term_print("\r");
	}

	return 0;
}

static int __cmd_less(int argc, char **argv) {
	if (argc == 1) {
		mbp_term_print("Incorrect Usage");
		mbp_term_print("  less f.u");
		mbp_term_print("\r");
	}

	if (argc == 2) {
		if ((strcmp(argv[1], "shadow.backup") == 0) && (m_current_user_role == 0)) {
			mbp_term_print("Permission Denied");
			mbp_term_print("\r");
		}
		if ((strcmp(argv[1], "shadow.backup") == 0) && (m_current_user_role > 0)) {
			mbp_term_print("#Backup for scruffy");
			mbp_term_print("root:ff24fefce7040369fd4fda33fd2d036d");
			mbp_term_print("\r");
			//Create Root Password Hash https://md5hashing.net/hash
			//Default Root Password = goodnews
			//User must less the file and crack MD5 encrypted password
			//Hope they change it...
		}
	}
	return 0;
}

static int __cmd_exit(int argc, char **argv) {
	switch (m_current_user_role) {
	case 0:
		mbp_term_print("Huh. Did everything");
		mbp_term_print("just taste purple");
		mbp_term_print("for a second?");
		mbp_term_print("\r");
		break;
	case 1:
		mbp_term_print("Session End: scruffy");
		mbp_term_print("Current User: fry");
		mbp_term_print("\r");
		m_current_user_role = 0; //always jump to lowest privilege on exit
		break;
	case 2:
		mbp_term_print("Session End: root");
		mbp_term_print("Current User: fry");
		mbp_term_print("\r");
		m_current_user_role = 0; //always jump to lowest privilege on exit
		break;
	}
	return 0;
}

static int __cmd_stop(int argc, char **argv) {
	//Quit early if they don't have permission
	if (m_current_user_role != 2) {
		mbp_term_print("Permission Denied");
		mbp_term_print("\r");
		return 0;
	}

	util_gfx_draw_raw_file_stop();
	mbp_term_print("Bling stopped.");

	return 0;
}

static int __cmd_su(int argc, char **argv) {
	char pw_scruffy[9];
	mbp_state_pw_scruffy_get(pw_scruffy);

	char pw_root[9];
	mbp_state_pw_root_get(pw_root);

	if (argc != 3) {
		mbp_term_print("Incorrect Usage");
		mbp_term_print("  su user pass");
		mbp_term_print("\r");
	}

	if (argc == 3) {
		if ((strcmp(argv[1], "scruffy") == 0) && (strcmp(argv[2], pw_scruffy) == 0)) {
			mbp_term_print("Welcome: Scruffy");
			mbp_term_print("MOTD: My job.");
			mbp_term_print("Toilets 'n boilers, ");
			mbp_term_print("boilers 'n toilets.");
			mbp_term_print("Dont forget to exit!");
			mbp_term_print("\r");
			m_current_user_role = 1;
		}
		else if ((strcmp(argv[1], "root") == 0) && ((strcmp(argv[2], pw_root) == 0) || (strcmp(argv[2], "xxxxxxxx") == 0))) {
			//Did i just sneak in a backdoor?
			mbp_term_print("ROOT ACCESS");
			mbp_term_print("Dont forget to exit!");
			mbp_term_print("\r");
			m_current_user_role = 2;
		}
		else {
			mbp_term_print("Incorrect User or PW");
			mbp_term_print("\r");
		}
	}
	return 0;
}

//static void __cmd_tcl_schedule_handler(void *p_data, uint16_t length) {
//	char *filename = (char *) p_data;
//	//Run the TCL file
//	mbp_term_print("Running...");
//	mbp_tcl_exec_file(filename);
//	mbp_term_print("Done.");
//	mbp_term_print("\r");
//}

static int __cmd_tcl(int argc, char **argv) {
	bool incorrect_usage = false;
	if (argc != 2) {
		incorrect_usage = true;
	} else if (strlen(argv[2]) > 12) {
		incorrect_usage = true;
	}

	if (!incorrect_usage) {
		//Build path to TCL file
		char path[17];
		snprintf(path, 17, "TCL/%s", argv[1]);
		FILINFO info;
		FRESULT result = f_stat(path, &info);
		if (result != FR_OK) {
			mbp_term_print("File not found.");
			mbp_term_print("\r");
			return 0;
		}

		mbp_tcl_exec_file(argv[1]);
//		app_sched_event_put(argv[1], strlen(argv[1]) + 1, __cmd_tcl_schedule_handler);
	} else {
		mbp_term_print("Incorrect usage. tcl <file.tcl>");
		mbp_term_print("\r");
	}

	return 0;
}

static int __cmd_uname(int argc, char **argv) {
	//Display Name
	char name[SETTING_NAME_LENGTH];
	mbp_state_name_get(name);
	char message[20];
	sprintf(message, "Badge Name: %s", name);
	mbp_term_print(message);

	//Display Firmware Version
	char version[32] = VERSION;
	sprintf(message, "Firmware: %s", version);
	mbp_term_print(message);

	//Display Soft Device
	ble_version_t ble;
	sd_ble_version_get(&ble);
	sprintf(message, "SoftDevice: %#x", ble.subversion_number);
	mbp_term_print(message);

	//Display Device ID
	uint16_t serial = util_get_device_id();
	sprintf(message, "Device ID: %#x", serial);
	mbp_term_print(message);
	mbp_term_print("\r");

	return 0;
}

static int __cmd_motd(int argc, char **argv) {
	mbp_term_print("AND!XOR DC25 BADGE");
	mbp_term_print("@andnxor");
	mbp_term_print("@zappbrandnxor");
	mbp_term_print("@lacosteaef");
	mbp_term_print("@andrewnriley");
	mbp_term_print("@bitst3m");
	mbp_term_print("@hyr0n1");
	mbp_term_print("╚▒▒-▤8−◦");
	mbp_term_print("\r");
	mbp_term_print("Type 'help' to");
	mbp_term_print("Kill All Humans!");
	mbp_term_print("\r");
	return 0;
}

static int __cmd_emacs(int argc, char **argv) {
	mbp_term_print("get a real editor");
	mbp_term_print("#vi4life");
	mbp_term_print("\r");
	return 0;
}

static int __cmd_vim(int argc, char **argv) {
	mbp_term_print("You have chosen....");
	mbp_term_print("wisely");
	mbp_term_print("\r");
	return 0;
}

static int __cmd_whoami(int argc, char **argv) {
	switch (m_who_count) {
	case 0:
		mbp_term_print("First, you must ask");
		mbp_term_print("who do you think you");
		mbp_term_print("are...");
		mbp_term_print("\r");
		break;
	case 1:
		mbp_term_print("...Only then can you");
		mbp_term_print(" find peace...");
		mbp_term_print("\r");
		break;
	case 2:
		mbp_term_print("Look deep into your");
		mbp_term_print("mind and search for");
		mbp_term_print("your true identity..");
		mbp_term_print("\r");
		break;
	case 3:
		mbp_term_print("As the Dalai Lama");
		mbp_term_print("once....wait....");
		mbp_term_print("why do you");
		mbp_term_print("keep asking?");
		mbp_term_print("\r");
		break;
	case 4:
		mbp_term_print("Okay, I admit.");
		mbp_term_print("I don't know...");
		mbp_term_print("just stop,");
		mbp_term_print("that's all I ask");
		mbp_term_print("\r");
		break;
	case 5:
		mbp_term_print("STOP,");
		mbp_term_print("for the love of god.");
		mbp_term_print("\r");
		break;
	case 6:
		mbp_term_print("First, you must ask");
		mbp_term_print("who do you think you");
		mbp_term_print("are...");
		mbp_term_print("\r");
		break;
	case 7:
		mbp_term_print("Ah, you thought I ");
		mbp_term_print("started over didn't");
		mbp_term_print("you, but you have to");
		mbp_term_print("be so damn");
		mbp_term_print("PERSISTANT!");
		mbp_term_print("\r");
		break;
	case 8:
		mbp_term_print("Really have nothing");
		mbp_term_print("better to do?");
		mbp_term_print("\r");
		break;
	case 9:
		mbp_term_print("Did you really think");
		mbp_term_print("This would work?");
		mbp_term_print("We didnt reuse *.*");
		mbp_term_print("From DC24...");
		mbp_term_print("¯\\_(ツ)_/¯");
		mbp_term_print("\r");
		break;
	}
	m_who_count = (m_who_count + 1) % 10;
	return 0;
}

static int __nt_option_callback(int argc, char **argv, void *extobj) {
	if (argc == 0) {
		return 0;
	}
	const cmd_table_t commands[CMD_LIST_COUNT] = CMD_LIST
	;

	for (int i = 0; i < CMD_LIST_COUNT; i++) {
		if (strcmp((const char *) argv[0], commands[i].cmd) == 0) {
			return commands[i].func(argc, argv);
		}
	}
	mbp_term_print("command not found");
	mbp_term_print("\r");
	return 0;
}

static int __nt_prompt_callback(const char *text, void *extobj) {
	return ntopt_parse(text, __nt_option_callback, 0);
}

/**
 * We have to provide a read function to NTShell but since we're not truly interactive this does nothing
 */
static int __nt_read(char *buf, int cnt, void *extobj) {
	UNUSED_PARAMETER(buf);
	UNUSED_PARAMETER(cnt);
	UNUSED_PARAMETER(extobj);
	return cnt;
}

/**
 * @brief We have to provide a write function to NTShell but since we're not truly interactive this does nothing
 */
static int __nt_write(const char *buf, int cnt, void *extobj) {
	UNUSED_PARAMETER(extobj);
	UNUSED_PARAMETER(buf);
	UNUSED_PARAMETER(cnt);
	return cnt;
}

/**
 * @brief Handler that disconnects the terminal if no user activity for a period of time
 *
 * @param[in] p_data	Pointer to data from when the timer was created. Nothing is expected here
 */
static void __inactivity_timer_handler(void *p_data) {
	//Disconnect during 5 second window when time is out
	if (util_millis() > m_inactivity_end_time && m_conn_handle != BLE_CONN_HANDLE_INVALID) {
		sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		m_conn_handle = BLE_CONN_HANDLE_INVALID;
	}
}

/**
 * @brief Initailize the terminal
 */
void mbp_term_init() {
	ntshell_init(&m_ntshell, __nt_read, __nt_write, __nt_prompt_callback, (void *) &m_ntshell);
}

/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send it to NTShell
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
void mbp_term_nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length) {
	//Only respond to NUS if badge is activated
	if (!mbp_state_activated_get()) {
		return;
	}

	if(length > 20){ //todo verify if this is valid
		return;
	}

	char eol[] = "\r\n";
	vtrecv_execute(&(m_ntshell.vtrecv), p_data, length);
	//Send new EOL to shell to force execution
	vtrecv_execute(&(m_ntshell.vtrecv), (uint8_t *) eol, 2);
	m_inactivity_end_time = util_millis() + (2 * 60 * 1000); 	//2 minutes
	m_conn_handle = p_nus->conn_handle;
}

/**
 * @brief Function for printing up to 20 characters to NUS
 *
 * @param[in] text	Char array of text to send to UART over BLE
 */
void mbp_term_print(char *text) {
	util_ble_nus_send(text, strlen(text));
}

void mbp_term_start() {
	uint32_t err_code;
	err_code = app_timer_create(&m_terminal_inactivity_timer, APP_TIMER_MODE_REPEATED, __inactivity_timer_handler);
	APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(m_terminal_inactivity_timer, APP_TIMER_TICKS(1000, UTIL_TIMER_PRESCALER), NULL);
	APP_ERROR_CHECK(err_code);
}

void mbp_term_user_role_set(uint8_t role) {
	if (role <= 2) {
		m_current_user_role = role;
	}
}
