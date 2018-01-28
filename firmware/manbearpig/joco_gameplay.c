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

#include "system.h"

int8_t gamelevel() {
    return (mbp_state_score_get()/POINTS_PER_LEVEL);
}

void add_to_score(int16_t points) {
    int16_t scorenow;
    scorenow = mbp_state_score_get() + points;
    if (scorenow > MAX_POINTS) {
        scorenow = MAX_POINTS;
    }
    mbp_state_score_set(scorenow);
    mbp_state_save();
}
