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

#define LED_PIN_DATA		17
#define LED_PIN_SCK			2

static uint8_t brightness = 20;

static volatile bool spi_transfer_done = false;
static nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(2);

void apa102_init() {
	nrf_drv_spi_config_t spi_config;
	spi_config.sck_pin = LED_PIN_SCK;
	spi_config.mosi_pin = LED_PIN_DATA;
	spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED;
	spi_config.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;
	spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
	spi_config.orc = 0x00;
	spi_config.frequency = NRF_DRV_SPI_FREQ_1M;
	spi_config.mode = NRF_DRV_SPI_MODE_0;
	spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

	//Read the LED file for brightness
	FIL led_file;
	FRESULT result = f_open(&led_file, "LED.MBP", FA_READ | FA_OPEN_EXISTING);
	if (result == FR_OK) {
		UINT count = 0;
		char b_str[4];
		f_read(&led_file, b_str, 3, &count);
		brightness = strtol(b_str, NULL, 10);
	}

	APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL));
}

void apa102_send(uint8_t *leds) {

	CRITICAL_REGION_ENTER();

	uint8_t buffer_size = 4 + (4 * LED_COUNT) + 4;
	uint8_t buffer[buffer_size];
	uint8_t rx;

	//Start frame 32 0s
	memset(buffer, 0x00, buffer_size);

	uint8_t buffer_index = 4;
	uint32_t color_index = 0;
	for (uint32_t i = 0; i < LED_COUNT; i++) {
		//Global 0xE0+brightness
		buffer[buffer_index++] = 0xFF;
		//Blue
		buffer[buffer_index++] = util_math_map(leds[color_index++], 0, 255, 0, brightness);
		//Green
		buffer[buffer_index++] = util_math_map(leds[color_index++], 0, 255, 0, brightness);
		//Red
		buffer[buffer_index++] = util_math_map(leds[color_index++], 0, 255, 0, brightness);
	}

	//End frame 32 1s
	for (uint8_t i = 0; i < 4; i++) {
		buffer[buffer_index++] = 0xFF;
	}

	nrf_drv_spi_transfer(&spi, buffer, buffer_size, &rx, 0);

	CRITICAL_REGION_EXIT();
}
