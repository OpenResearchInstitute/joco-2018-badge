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
#ifndef SYSTEM_H_
#define SYSTEM_H_
#define VERSION							"v1.9"
#define VERSION_SD						27

// Various testing options

//#define NO_DB_SAVE 1
//#define UNLOCK_ALL_BLING

#define SETTING_INPUT_MAX				8
#define SETTING_NAME_DEFAULT			"REDSHIRT"
#define SETTING_NAME_LENGTH				9
#define SETTING_PW_LENGTH				9
#define SETTING_AIRPLANE_MODE_DEFAULT	false
#define SETTING_TILT_ENABLED_DEFAULT	true
#define SETTING_GAME_EXIT_POPUP_DEFAULT	true
#define SETTING_GAME_LED_SOUND_DEFAULT	true
#define SETTING_CHIP8_FG_COLOR_DEFAULT	COLOR_GREENYELLOW
#define SETTING_CHIP8_BG_COLOR_DEFAULT	COLOR_BLACK
#define SETTING_MASTER_DEFAULT			false
#define SETTING_UNLOCK_DEFAULT			0

#define UNLOCK_MASK_MASTER_1			0x0001 // master unlock
#define UNLOCK_MASK_MASTER_2			0x0002 // master unlock
#define UNLOCK_MASK_MASTER_3			0x0004 // master unlock
#define UNLOCK_MASK_MASTER_4			0x0008 // master unlock
#define UNLOCK_MASK_DATE_TIME			0x0010 // hack time bling (in terminal)
#define UNLOCK_MASK_DEFRAG			0x0020 // defrag bling (in terminal)
#define UNLOCK_MASK_WHATS_UP			0x0040 // What's up (in terminal)
#define UNLOCK_MASK_WHEATON    			0x0080 // Wheaton
#define UNLOCK_MASK_TWITTER			0x0100 // Twitter bling
#define UNLOCK_MASK_SEEKRIT			0x0200 // trololol bling
#define UNLOCK_MASK_DAMON			0x0400 // Matt Damon bling
#define UNLOCK_MASK_0800			0x0800 // joco_unused
#define UNLOCK_MASK_1000			0x1000 // joco_unused
#define UNLOCK_MASK_WH				0x2000 // White Hat detected  (joco_unused)
#define UNLOCK_MASK_4000			0x4000 // joco_unused

#define UNLOCK_MASK_ALLMASTERS (UNLOCK_MASK_MASTER_1 | UNLOCK_MASK_MASTER_2 | UNLOCK_MASK_MASTER_3 | UNLOCK_MASK_MASTER_4)

//Nordic config
#include "sdk_config.h"

//Standard
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

//Nordic Includes
#include <app_button.h>
#include <app_error.h>
#include <app_fifo.h>
#include <app_gpiote.h>
#include <app_scheduler.h>
#include <app_sdcard.h>
#include <app_util_platform.h>
#include <ble_advdata.h>
#include <ble_advertising.h>
#include <ble_conn_params.h>
#include <ble_conn_state.h>
#include <ble_db_discovery.h>
#include <ble_nus.h>
#include <ble_types.h>
#include <fds.h>
#include <fstorage.h>
#include <nrf_assert.h>
#include <nrf52_bitfields.h>
#include <nrf_ble_gatt.h>
#include <nrf_block_dev_sdc.h>
#include <nrf_csense.h>
#include <nrf_delay.h>
#include <nrf_ecb.h>
#include <nrf_gpio.h>
#include <nrf_drv_clock.h>
#include <nrf_drv_gpiote.h>
#include <nrf_drv_i2s.h>
#include <nrf_drv_rng.h>
#include <nrf_drv_rtc.h>
#include <nrf_drv_saadc.h>
#include <nrf_drv_spi.h>
#include <nrf_drv_timer.h>
#include <nrf_gpiote.h>
#include <nrf_nvic.h>
#include <nrf_queue.h>
#include <nrf_saadc.h>
#include <peer_manager.h>
#include <softdevice_handler.h>
#include <hal_nfc_t2t.h>
#include <nfc_t2t_lib.h>
#include <nfc_ndef_msg.h>
#include <nfc_text_rec.h>

//Nordic external FATFS
#include "diskio_blkdev.h"
#include "diskio.h"
#include "ff.h"

//Libraries
#include "gfxfont.h"

//JOCO
#include "beacon_ble.h"
#include "chip8.h"
#include "drv_apa102.h"
#include "drv_st7735.h"
#include "drv_ws2812b.h"
#include "bling/mbp_bling.h"
#include "bling/mbp_custom_bling.h"
#include "joco_gamedata.h"
#include "joco_gameplay.h"
#include "joco_db.h"
#include "score_ble.h"
#include "mbp_medea.h"
#include "mbp_menu.h"
#include "mbp_state.h"
#include "mbp_system.h"
#include "mbp_tcl.h"
#include "mbp_term.h"
#include "mbp_ui.h"
#include "ntshell.h"
#include "ntopt.h"
#include "partcl/tcl.h"
#include "skifree.h"
#include "util.h"
#include "util_ble.h"
#include "util_hello.h"
#include "util_ble_lists.h"
#include "util_nfc.h"
#include "util_button.h"
#include "util_gfx.h"
#include "util_led.h"
#include "util_math.h"
#include "util_sd.h"
#include "util_tilt.h"
#include "util_crypto.h"

#endif /* SYSTEM_H_ */
