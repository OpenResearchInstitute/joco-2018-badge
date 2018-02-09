/*****************************************************************************
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
 * 	@sconklin
 * 	@mustbeart
 * 	@abraxas3d
 *****************************************************************************/
#ifndef JOCO_GAMEPLAY_H_
#define JOCO_GAMEPLAY_H_

// Start a new badge with these
#define GAME_SCORE_DEFAULT 0
#define GAME_LASTLEVEL_DEFAULT 0

// Gameplay definitions
#define POINTS_PER_LEVEL 250
// Max points must be less than 32768 because we use the high bit to indicate trinkets should be dispensed
#define MAX_POINTS 24750 // 32768 MAX
#define MAX_LEVEL 99
#define SPARKLE_ODDS 1
#define SPARKLE_LENGTH_SECONDS 30
#define POINTS_4_VISIT 50
#define POINTS_4_SPARKLE_VISIT 5
#define POINTS_4_LOCAL_GAME 20
#define POINTS_4_LOCALGAME_REPEAT 1
#define POINTS_4_DISPENSING 100
#define POINTS_4_ALL_MASTERS 200

// 'visiting' parameters
#define VISIT_RSSI_MIN -55
#define VISIT_TIME_LENGTH 60000 // Milliseconds
#define VISIT_LOST_TIME_LENGTH 15000
#define SPARKLE_VISIT_RSSI_MIN -55

extern int8_t gamelevel();
extern void add_to_score(int16_t points, char *name);

#endif /* JOCO_GAMEPLAY_H_ */
