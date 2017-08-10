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

/**
 * APA102 Driver
 *
 * This driver utilizes SPI2 on NRF52. IT IS LIMITED TO 20 LEDS! Limitation is due
 * to the fact that NRF52 can only transfer 255 bytes over SPI at a time. Any more
 * data will require additional SPI transfers. Delays between transfers causes the
 * WS2812B LEDs to latch too early. This may fixable by using SPI list transfers
 * or by using I2S to send more data over DMA.
 */

#include "system.h"

uint8_t brightness = 20;

#ifdef RED_BADGE
#define LED_PIN_DATA			10
#else
#define LED_PIN_DATA			2
#endif

#define LED_PIN_SCK_DUMMY		22

#define WS2812B_PATTERN_0 				(0b1000)
#define WS2812B_PATTERN_1 				(0b1110)

static nrf_drv_spi_t spi2 = NRF_DRV_SPI_INSTANCE(2);

bool ws2812b_init() {
	nrf_drv_spi_config_t spi_config;
	spi_config.sck_pin = LED_PIN_SCK_DUMMY;
	spi_config.mosi_pin = LED_PIN_DATA;
	spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED;
	spi_config.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;
	spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
	spi_config.orc = 0xff;
	spi_config.frequency = NRF_DRV_SPI_FREQ_4M;
	spi_config.mode = NRF_DRV_SPI_MODE_1;
	spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

	uint32_t err_code = nrf_drv_spi_init(&spi2, &spi_config, NULL);
	APP_ERROR_CHECK(err_code);


	//Read the LED file for brightness
	FIL led_file;
	FRESULT result = f_open(&led_file, "LED.MBP", FA_READ | FA_OPEN_EXISTING);
	if (result == FR_OK) {
		UINT count = 0;
		char b_str[4];
		f_read(&led_file, b_str, 3, &count);
		brightness = strtol(b_str, NULL, 10);
	}

	return err_code == NRF_SUCCESS;
}

void ws2812b_send(uint8_t *leds) {

	int16_t buffer_size = (LED_COUNT * 4 * 3);	//2 bytes per bit, reset latch is 6 bytes
	uint8_t buffer[buffer_size];
	uint8_t rx;

	//Zero the buffer
	memset(buffer, 0x00, buffer_size);

	uint16_t buffer_index = 0;
	uint32_t color_index = 0;
	uint8_t red, green, blue;
	for (uint32_t i = 0; i < LED_COUNT; i++) {
		blue = util_math_map(leds[color_index++], 0, 255, 0, brightness);
		green = util_math_map(leds[color_index++], 0, 255, 0, brightness);
		red = util_math_map(leds[color_index++], 0, 255, 0, brightness);

		//GREEN
		for (uint8_t j = 0; j < 8; j++) {
			if (((green << j) & 0x80) > 0)
				buffer[buffer_index] = WS2812B_PATTERN_1 << 4;
			else
				buffer[buffer_index] = WS2812B_PATTERN_0 << 4;

			j++;

			if (((green << j) & 0x80) > 0)
				buffer[buffer_index] |= WS2812B_PATTERN_1;
			else
				buffer[buffer_index] |= WS2812B_PATTERN_0;

			buffer_index++;
		}

		//RED
		for (uint8_t j = 0; j < 8; j++) {
			if (((red << j) & 0x80) > 0)
				buffer[buffer_index] = WS2812B_PATTERN_1 << 4;
			else
				buffer[buffer_index] = WS2812B_PATTERN_0 << 4;

			j++;

			if (((red << j) & 0x80) > 0)
				buffer[buffer_index] |= WS2812B_PATTERN_1;
			else
				buffer[buffer_index] |= WS2812B_PATTERN_0;

			buffer_index++;
		}

		//BLUE
		for (uint8_t j = 0; j < 8; j++) {
			if (((blue << j) & 0x80) > 0)
				buffer[buffer_index] = WS2812B_PATTERN_1 << 4;
			else
				buffer[buffer_index] = WS2812B_PATTERN_0 << 4;

			j++;

			if (((blue << j) & 0x80) > 0)
				buffer[buffer_index] |= WS2812B_PATTERN_1;
			else
				buffer[buffer_index] |= WS2812B_PATTERN_0;

			buffer_index++;
		}
	}

	//Send the data
	nrf_drv_spi_transfer(&spi2, buffer, buffer_size, &rx, 0);

	//TODO This should be done by inserting 0s into the end of the stream, we're using 180 of 255 bytes.
	nrf_delay_us(80);	//add some delay for newer WS2812b
}
