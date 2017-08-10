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
#define VERSION							"v1.6"
#define VERSION_SD						20
//#define BLE_ONLY_BUILD					1

#define SETTING_INPUT_MAX				8
#define SETTING_NAME_DEFAULT			"BENDER"
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

#define UNLOCK_MASK_MASTER				0x0001
#define UNLOCK_MASK_WHISKEY_PIRATES		0x0002
#define UNLOCK_MASK_CPV					0x0004
#define UNLOCK_MASK_WHATS_UP			0x0008
#define UNLOCK_MASK_DATE_TIME			0x0010
#define UNLOCK_MASK_DEFRAG				0x0020
#define UNLOCK_MASK_C2					0x0040
#define UNLOCK_MASK_TCL					0x0080
#define UNLOCK_MASK_TWITTER				0x0100
#define UNLOCK_MASK_SEEKRIT				0x0200
#define UNLOCK_MASK_DAMON				0x0400
#define UNLOCK_MASK_CHIP8				0x0800
#define UNLOCK_MASK_SCROLL				0x1000
#define UNLOCK_MASK_WH					0x2000
#define UNLOCK_MASK_CARD				0x4000

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

//Nordic external FATFS
#include "diskio_blkdev.h"
#include "diskio.h"
#include "ff.h"

//Libraries
#include "gfxfont.h"

//AND!XOR
#include "beacon_ble.h"
#include "botnet.h"
#include "chip8.h"
#include "drv_apa102.h"
#include "drv_st7735.h"
#include "drv_ws2812b.h"
#include "bling/mbp_bling.h"
#include "bling/mbp_custom_bling.h"
#include "flappydefcon.h"
#include "mbp_master.h"
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
#include "util_button.h"
#include "util_gfx.h"
#include "util_led.h"
#include "util_math.h"
#include "util_sd.h"
#include "util_tilt.h"
#include "botnet_ble.h"
#include "mbp_master_ble.h"

#endif /* SYSTEM_H_ */
