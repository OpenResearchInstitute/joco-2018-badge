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
 *  @sconklin
 *  @mustbeart
 *  @abraxas3d
 *****************************************************************************/

#include "system.h"

#define JOCO_UI_MARGIN         3
#define JOCO_UI_MARGIN_RIGHT  12


int8_t gamelevel() {
    return (mbp_state_score_get()/POINTS_PER_LEVEL);
}

void add_to_score(int16_t points, char *name) {
    int16_t scorenow;
    scorenow = mbp_state_score_get() + points;
    if (scorenow > MAX_POINTS) {
        scorenow = MAX_POINTS;
    }
    mbp_state_score_set(scorenow);
    mbp_state_save();
    // schedule a score bling display
    // -spc- TODO change the bling for a level up
    APP_ERROR_CHECK(app_sched_event_put(name, strlen(name), mbp_bling_score_schedule_handler));
}

void game_status_screen() {

    //buffer for formatting text
    char temp[32];

    bool redraw = true;

    while (1) {
	if (redraw || !util_gfx_is_valid_state()) {
	    uint8_t level = gamelevel();
	    uint8_t lastlevel = mbp_state_lastlevel_get();

	    //Make sure there's no clipping
	    util_gfx_cursor_area_reset();

	    //Draw background
	    mbp_ui_cls();
	    //util_gfx_draw_raw_file("MENU/SCOREBG.RAW", 0, 0, 128, 128, NULL, false, NULL);

	    //Print their name
	    util_gfx_set_font(FONT_LARGE);
	    util_gfx_set_color(COLOR_WHITE);
	    util_gfx_set_cursor(0, JOCO_UI_MARGIN);
	    mbp_state_name_get(temp);
	    util_gfx_print(temp);

	    util_gfx_set_color(COLOR_LIGHTBLUE);
	    strcpy(temp, "GAME STATS");
	    util_gfx_set_cursor(JOCO_UI_MARGIN, 26);
	    util_gfx_print(temp);

	    //Print their level
	    util_gfx_set_color(COLOR_YELLOW);
	    sprintf(temp, "LEVEL %d", level);
	    util_gfx_set_cursor(JOCO_UI_MARGIN, 60);
	    util_gfx_print(temp);

	    //Print points
	    util_gfx_set_color(COLOR_WHITE);
	    sprintf(temp, "POINTS: %d\n", mbp_state_score_get());
	    util_gfx_set_font(FONT_SMALL);
	    util_gfx_set_cursor(JOCO_UI_MARGIN, 80);
	    util_gfx_print(temp);

	    //Print trinkets available
	    sprintf(temp, "Trinkets avail: %d\n", level - lastlevel);
	    util_gfx_set_cursor(JOCO_UI_MARGIN, 100);
	    util_gfx_print(temp);

	    redraw = false;
	}

	//validate screen state
	util_gfx_validate();

	util_button_wait();
	util_button_clear();
	return;
    }
}
