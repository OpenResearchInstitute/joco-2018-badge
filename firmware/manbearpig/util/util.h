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
#ifndef UTIL_UTIL_H_
#define UTIL_UTIL_H_

#define UTIL_TIMER_PRESCALER			0
#define UTIL_TIMER_OP_QUEUE_SIZE 		30

#define TILT_STATE_NORMAL				0
#define TILT_STATE_INVERTED				1

extern uint16_t util_get_device_id();
extern int16_t util_index_of(char *p_text, char c);
extern uint32_t util_millis();
extern uint32_t util_local_millis();
extern void util_millis_offset_set(uint32_t offset);
extern void util_timers_start();
extern void util_hex_encode(uint8_t *, uint8_t *, uint8_t);

#endif /* UTIL_UTIL_H_ */
