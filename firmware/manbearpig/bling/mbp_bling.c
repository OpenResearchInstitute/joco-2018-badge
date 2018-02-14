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
 *  @andnxor
 *  @zappbrandnxor
 *  @hyr0n1
 *  @andrewnriley
 *  @lacosteaef
 *  @bitstr3m
 *
 * Further modifications made by
 *      @sconklin
 *      @mustbeart
 *
 *****************************************************************************/

#include "../system.h"

#define TOOTH_EYES_TIME_MS          10

#define TOOTH_SAT                  1.0
#define TOOTH_VAL                  0.5 // (brightness)
#define TOOTH_FLASH_MINTIME            50 // min time between flashes in tens of mS
#define TOOTH_FLASH_MAXTIME            500 // max time between flashes in tens of mS
#define TOOTH_FLASH_LEN                3 // min flash length in tens of mS

#define EYE_HUE_STEP                .00015
#define SCROLL_CHAR_WIDTH           16
#define SCROLL_CHAR_HEIGHT          31
#define SCROLL_SPEED                -2
#define SCROLL_TIME_MS              20

APP_TIMER_DEF(m_tooth_timer);
APP_TIMER_DEF(m_scroll_led_timer);

// For a shiny gold tooth
static float m_tooth_hue = TOOTH_HUE_GOLD;
static float m_tooth_sat = 1.0;
static float m_tooth_val = 1.0;
static uint32_t tooth_flash_counter = 100;
static bool tooth_flashing = false;

static float m_eye_hue = 0;
static bool m_tooth_eye_running = false;

typedef struct {
    uint8_t index;
    float hue;
} bling_defag_state_t;

static void __rgb_file_callback(uint8_t frame, void *p_data) {
    util_led_play_rgb_frame((led_anim_t *) p_data);
}

/**
 * Generic call back that runs the leds in sparkle mode
 */
static void __led_sparkle(uint8_t frame, void *p_data) {
    uint8_t *data = (uint8_t *) p_data;
    uint8_t i = util_math_rand8_max(LED_COUNT);
    util_led_set_rgb(i, LED_COLOR_WHITE);
    util_led_show();

    nrf_delay_ms(20);

    //Unpack hue as a float
    float hue = ((float) *data / 10.0);

    util_led_set_rgb(i, util_led_hsv_to_rgb(hue, 1, 1));
    util_led_show();

    hue += 0.1;
    if (hue >= 1) {
        hue = 0;
    }

    //Pack the hue
    *data = (uint8_t) (hue * 10.0);
}

/*
static void __led_sparkle_single(uint8_t frame, void *p_data) {
    uint8_t *data = (uint8_t *) p_data;
    uint8_t i = util_math_rand8_max(LED_COUNT);
    util_led_clear();

    util_led_set(i, 255, 255, 255);
    util_led_show();

    nrf_delay_ms(20);

    //Unpack hue as a float
    float hue = ((float) *data / 10.0);

    util_led_set_rgb(i, util_led_hsv_to_rgb(hue, 1, 1));
    util_led_show();

    hue += 0.1;
    if (hue >= 1) {
        hue = 0;
    }

    //Pack the hue
    *data = (uint8_t) (hue * 10.0);
}
*/

static void __led_chase_cw_callback(uint8_t frame, void *p_data) {
    uint8_t *p_index = (uint8_t *) p_data;
    uint8_t order[] = { 0, 1, 2, 3, 7, 11, 10, 9, 8, 4 };

    util_led_set(order[*p_index], 255, 0, 0);
    util_led_show();
    util_led_set(order[*p_index], 0, 0, 0);

    (*p_index)++;
    if (*p_index > 9) {
        *p_index = 0;
    }
}

void __led_hue_cycle(uint8_t frame, void *p_data) {
    uint8_t *p_hue = (uint8_t *) p_data;
    float hue = (float) (*p_hue) / 100.0;
    uint32_t color = util_led_hsv_to_rgb(hue, .6, .8);
    util_led_set_all_rgb(color);
    util_led_show();

    hue += .015;
    if (hue >= 1) {
        hue = 0;
    }
    *p_hue = (hue * 100.0);
}

static void __spectrum_analyzer_callback(uint8_t frame, void *p_data) {

    if ((frame % 2) == 0) {
        uint32_t green = util_led_to_rgb(0, 255, 0);
        uint32_t yellow = util_led_to_rgb(255, 255, 0);
        uint32_t red = util_led_to_rgb(255, 0, 0);

        uint32_t colors[] = { green, yellow, red };

        uint8_t rows[LED_MATRIX_H][LED_MATRIX_W] = LED_MATRIX_ROWS;

        util_led_set_all(0, 0, 0);
        for (uint8_t x = 0; x < LED_MATRIX_W; x++) {
            uint8_t h = util_math_rand8_max(LED_MATRIX_H) + 1;
            for (uint8_t y = 0; y < h; y++) {
                util_led_set_rgb(rows[LED_MATRIX_H - y - 1][x], colors[y]);
            }
        }
        util_led_show();
    }
}

static void __mbp_bling_rainbow_eye_callback(uint8_t frame, void *data) {
    uint8_t *p_data = (uint8_t *) data;
    float hue = ((float) *p_data) / 100.0;

    uint32_t rgb = util_led_hsv_to_rgb(hue, 1.0, 1.0);
    util_led_set_rgb(LED_RIGHT_EYE_INDEX, rgb);
    util_led_set_rgb(6, rgb);
    util_led_show();

    hue -= .01;
    if (hue <= 0) {
        hue = .99;
    }

    *p_data = (uint8_t) (hue * 100.0);
}

#define GLITTER_HANG_TIME 10
static uint8_t hang_time = GLITTER_HANG_TIME;

static void __mbp_bling_glitter_callback(uint8_t frame, void *p_data) {
    uint8_t *p_index = (uint8_t *) p_data;

    // If the stored data is a valid index, set that LED to blue or black
    if (*p_index < LED_COUNT) {
    // half the time
    if (hang_time == 0) {
        hang_time = GLITTER_HANG_TIME;
        //Pack invalid data so next time around a new LED is picked
        *p_index = 0xFF;
    } else {
        util_led_set_rgb(*p_index, LED_COLOR_BLACK);
        hang_time--;
    }

    //Otherwise if it's an invalid index randomly pick one to flash
    } else {
    *p_index = util_math_rand8_max(LED_COUNT);
    util_led_set_rgb(*p_index, LED_COLOR_WHITE);
    }

    util_led_show();
}

static void __mbp_bling_backer_abraxas3d_callback(uint8_t frame, void *p_data) {
    uint32_t indigo = LED_COLOR_INDIGO;
    uint32_t coral = LED_COLOR_CORAL;
    uint32_t gold = LED_COLOR_GOLD;
    uint32_t yellow = LED_COLOR_YELLOW;
    uint32_t white = LED_COLOR_WHITE;

    //Unpack cycle
    uint8_t *data = (uint8_t *) p_data;
    uint8_t cycle = (uint8_t) *data;

    if (cycle == 0) {
        //Move Column 1 & 4 Up Vertically
        util_led_set_rgb(13, indigo);
        util_led_set_rgb(0, gold);
        util_led_set_rgb(4, coral);
        util_led_set_rgb(8, yellow);
        util_led_set_rgb(12, indigo);
        util_led_set_rgb(3, gold);
        util_led_set_rgb(7, coral);
        util_led_set_rgb(11, yellow);

        //1-5-9 / 2-6-10
        util_led_set(1, 0, 0, 0); //CLEAR
        util_led_set(2, 0, 0, 0); //CLEAR
        util_led_set(6, 0, 0, 0); //CLEAR
        util_led_set(10, 0, 0, 0); //CLEAR
        util_led_set(9, 0, 0, 0); //CLEAR
        util_led_set(5, 0, 0, 0); //CLEAR
        util_led_set_rgb(1, indigo);
        util_led_set_rgb(5, indigo);
        util_led_set_rgb(9, indigo);
        util_led_set_rgb(14, indigo); //Set the Cig
    }

    else if (cycle == 1) {
        //Move Column 1 & 4 Up Vertically
        util_led_set_rgb(13, gold);
        util_led_set_rgb(0, coral);
        util_led_set_rgb(4, yellow);
        util_led_set_rgb(8, indigo);
        util_led_set_rgb(12, gold);
        util_led_set_rgb(3, coral);
        util_led_set_rgb(7, yellow);
        util_led_set_rgb(11, indigo);

        //1-5-9 / 2-6-10
        util_led_set(1, 0, 0, 0); //CLEAR
        util_led_set(2, 0, 0, 0); //CLEAR
        util_led_set(6, 0, 0, 0); //CLEAR
        util_led_set(10, 0, 0, 0); //CLEAR
        util_led_set(9, 0, 0, 0); //CLEAR
        util_led_set(5, 0, 0, 0); //CLEAR
        util_led_set_rgb(1, white);
        util_led_set_rgb(5, white);
        util_led_set_rgb(9, white);

        util_led_set_rgb(14, white); //Set the Cig
    }

    else if (cycle == 2) {
        //Move Column 1 & 4 Up Vertically
        util_led_set_rgb(13, coral);
        util_led_set_rgb(0, yellow);
        util_led_set_rgb(4, indigo);
        util_led_set_rgb(8, gold);
        util_led_set_rgb(12, coral);
        util_led_set_rgb(3, yellow);
        util_led_set_rgb(7, indigo);
        util_led_set_rgb(11, gold);

        //1-5-9 / 2-6-10
        util_led_set(1, 0, 0, 0); //CLEAR
        util_led_set(2, 0, 0, 0); //CLEAR
        util_led_set(6, 0, 0, 0); //CLEAR
        util_led_set(10, 0, 0, 0); //CLEAR
        util_led_set(9, 0, 0, 0); //CLEAR
        util_led_set(5, 0, 0, 0); //CLEAR
        util_led_set_rgb(2, indigo);
        util_led_set_rgb(6, indigo);
        util_led_set_rgb(10, indigo);
        util_led_set_rgb(14, indigo); //Set the Cig
    }

    else {
        //Move Column 1 & 4 Up Vertically
        util_led_set_rgb(13, yellow);
        util_led_set_rgb(0, indigo);
        util_led_set_rgb(4, gold);
        util_led_set_rgb(8, coral);
        util_led_set_rgb(12, yellow);
        util_led_set_rgb(3, indigo);
        util_led_set_rgb(7, gold);
        util_led_set_rgb(11, coral);

        //1-5-9 / 2-6-10
        util_led_set(1, 0, 0, 0); //CLEAR
        util_led_set(2, 0, 0, 0); //CLEAR
        util_led_set(6, 0, 0, 0); //CLEAR
        util_led_set(10, 0, 0, 0); //CLEAR
        util_led_set(9, 0, 0, 0); //CLEAR
        util_led_set(5, 0, 0, 0); //CLEAR
        util_led_set_rgb(2, white);
        util_led_set_rgb(6, white);
        util_led_set_rgb(10, white);
        util_led_set_rgb(14, white); //Set the Cig
    }

    util_led_show();
    nrf_delay_ms(30);

    //Increment the Cycle
    if (cycle < 3) {
        cycle++;
    }
    else {
        cycle = 0;
    }

    //Pack the cycle
    *data = (uint8_t) cycle;
}

static void __mbp_bling_backer_andnxor_callback(uint8_t frame, void *p_data) {
    if (frame < 128) {
        util_led_set_all(0, frame * 2, 0);
    } else if (frame < 200) {
        util_led_set(util_math_rand8_max(LED_COUNT), 255, 255, 255);
    } else {
        uint8_t b = (222 - frame) * 11;
        util_led_set_all(b, b, b);
    }

    util_led_show();
}

void simple_filebased_bling(char *rawfile, char *rgbfile) {
    util_led_clear();
    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file(rgbfile, &anim);
    util_gfx_draw_raw_file(rawfile, 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
}

void mbp_bling_backer_abraxas3d(void *data) {
    uint8_t hue = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/BACKERS/KSABRAXA.RAW", 0, 0, 128, 128, &__mbp_bling_backer_abraxas3d_callback, true, &hue);
}

void mbp_bling_backer_andnxor(void *data) {
    uint8_t hue = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/BACKERS/ANDNXOR.RAW", 0, 0, 128, 128, &__mbp_bling_backer_andnxor_callback, true, &hue);
}

void mbp_bling_led_rainbow_callback(uint8_t frame, void *p_data) {
    //Unpack the data
    uint8_t *data = (uint8_t *) p_data;
    float hue = (float) *data / 100.0;

    //Define led matrix
    uint8_t led_map[LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT] = LED_MATRIX_FULL_MAPPING
    ;

    for (uint8_t row = 0; row < LED_MATRIX_FULL_ROW_COUNT; row++) {
        float rowhue = hue + (row * .08);
        if (rowhue >= 1)
            rowhue--;

        uint32_t color = util_led_hsv_to_rgb(rowhue, 1, .7);
        for (uint8_t i = 0; i < LED_MATRIX_FULL_COL_COUNT; i++) {
            uint8_t index = (row * LED_MATRIX_FULL_COL_COUNT) + i;
            if (led_map[index] < LED_COUNT) {
                util_led_set_rgb(led_map[index], color);
            }
        }

        //Increment row and color and loop around
        hue += .01;
        if (hue >= 1)
            hue = 0;
    }

    util_led_show();

    //Pack the data and store for next time
    *data = (int) (hue * 100);
}

void mbp_bling_badgers() {
    uint8_t hue = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/AND!XOR/BADGERS.RAW", 0, 0, 128, 128, &__mbp_bling_rainbow_eye_callback, true, &hue);
}

void mbp_bling_wheaton() {
    uint8_t hue = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/JOCO/WWSPIN.RAW", 0, 0, 128, 128, &__led_chase_cw_callback, true, &hue);
}

static void __led_jollyroger(uint8_t f_unused, void *p_data) {
    uint8_t eye_closed[] = { 4, 5, 6, 7 };
    uint8_t eye_open[] = { 1, 2, 4, 7, 9, 10 };
    uint8_t frame = *((uint8_t *) p_data);

    //Clear all colors
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        util_led_set_rgb(i, LED_COLOR_BLACK);
    }

    //Compute and set the eye colors
    util_led_set_rgb(LED_RIGHT_EYE_INDEX, LED_COLOR_EYES);

    uint32_t tooth_color = util_led_hsv_to_rgb(TOOTH_HUE_GOLD, 1, 0.5);
    util_led_set_rgb(LED_TOOTH_INDEX, tooth_color);

    //Large Eye
    if (frame < 20) {
        for (uint8_t i = 0; i < 6; i++) {
            util_led_set_rgb(eye_open[i], LED_COLOR_EYES);
        }
    } else {
        for (uint8_t i = 0; i < 4; i++) {
            util_led_set_rgb(eye_closed[i], LED_COLOR_EYES);
        }
    }

    //latch
    util_led_show();

    frame = (frame + 1) % 40;
    *((uint8_t *) p_data) = frame;
}

void mbp_bling_skull_crossbones() {
    uint8_t hue = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/JOCO/SKLCROSS.RAW", 0, 0, 128, 128, &__led_jollyroger, true, &hue);
}

void mbp_bling_5th_element_dance(void *data) { simple_filebased_bling("BLING/JOCO/5THEL.RAW", "BLING/FADEBLUE.RGB"); }

void mbp_bling_candy_mountain(void *data) { simple_filebased_bling("BLING/JOCO/CANDYMTN.RAW", "BLING/BLUPRPSW.RGB"); }

void mbp_bling_concert_flame(void *data) { simple_filebased_bling("BLING/JOCO/CFLAME.RAW", "BLING/FLAMES.RGB"); }

void mbp_bling_dancing_cyberman(void *data) { simple_filebased_bling("BLING/JOCO/CYBERMAN.RAW", "BLING/FADEGRN.RGB"); }

void mbp_bling_drwho_time(void *data) { simple_filebased_bling("BLING/JOCO/DRWHOTIM.RAW", "BLING/TUNNEL.RGB");}

void mbp_bling_duckhunt(void *data) { simple_filebased_bling("BLING/JOCO/DUCKHUNT.RAW", "BLING/COLORS3.RGB"); }

void mbp_bling_fallout_boygirl_drinking(void *data) { simple_filebased_bling("BLING/JOCO/FODRINK.RAW", "BLING/BOUNCE.RGB"); }

void mbp_bling_fallout_boy_science(void *data) { simple_filebased_bling("BLING/JOCO/FOSCI.RAW", "BLING/TUNNEL.RGB"); }

void mbp_bling_get_on_my_horse(void *data) { simple_filebased_bling("BLING/JOCO/MYHORSE.RAW", "BLING/IRADESC.RGB"); }

void mbp_bling_multipass_leelo(void *data) { simple_filebased_bling("BLING/JOCO/MLTIPASS.RAW", "BLING/REDORNG.RGB"); }

void mbp_bling_outer_limits() {
    uint8_t hue = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/JOCO/OUTERLIM.RAW", 0, 0, 128, 128, &__mbp_bling_glitter_callback, true, &hue);
}

void mbp_bling_portal_frying_pan(void *data) { simple_filebased_bling("BLING/JOCO/PORTALFP.RAW", "BLING/FADEYLLW.RGB"); }

void mbp_bling_portal_wink(void *data) { simple_filebased_bling("BLING/JOCO/PORTALWN.RAW", "BLING/CIRCLES.RGB"); }

void mbp_bling_portals(void *data) { simple_filebased_bling("BLING/JOCO/PORTALS.RAW", "BLING/TETRIS.RGB"); }

void mbp_bling_sleestaks(void *data) { simple_filebased_bling("BLING/JOCO/SLEESTAK.RAW", "BLING/GRNYELL.RGB"); }

void mbp_bling_tardis_nyan(void *data) { simple_filebased_bling("BLING/JOCO/TARDNYAN.RAW", "BLING/BOUNCE.RGB"); }

void mbp_bling_twilight_zone() {
    uint8_t hue = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/JOCO/TWILITE.RAW", 0, 0, 128, 128, &__mbp_bling_glitter_callback, true, &hue);
}

void mbp_bling_zombie_nyan(void *data) { simple_filebased_bling("BLING/JOCO/ZOMBNYAN.RAW", "BLING/BOUNCE.RGB"); }

void mbp_bling_damon() {
    util_led_clear();
    uint8_t index = 0;
    uint8_t count = 4;

    char *modes[] = { "BLING/AND!XOR/DAMON1.RAW", "BLING/AND!XOR/DAMON2.RAW", "BLING/AND!XOR/DAMON3.RAW", "BLING/AND!XOR/DAMON4.RAW" };
    uint8_t button = 0;

    util_led_clear();

    //If anything other than left button is pressed cycle modes
    while ((button & BUTTON_MASK_LEFT) == 0) {
        uint8_t hue = 0;
        button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__led_chase_cw_callback, true, &hue);
        index = (index + 1) % count;
    }
}

void mbp_bling_flames(void *data) { simple_filebased_bling("BLING/AND!XOR/FLAMES.RAW", "BLING/FLAMES.RGB"); }

void mbp_bling_meatspin(void *data) { simple_filebased_bling("BLING/AND!XOR/MEATSPIN.RAW", "BLING/AND!XOR/MEATSPIN.RGB"); }

void mbp_bling_hack_time(void *data) { simple_filebased_bling("BLING/AND!XOR/HACKTIME.RAW", "BLING/PINKBLUE.RGB"); }

void mbp_bling_illusion() {
    uint8_t index = 0;
    uint8_t count = 6;

    char *modes[] = { "BLING/AND!XOR/ILLUS3.RAW", "BLING/AND!XOR/ILLUS2.RAW", "BLING/AND!XOR/ILLUS1.RAW", "BLING/AND!XOR/ILLUS4.RAW",
            "BLING/AND!XOR/ILLUS5.RAW", "BLING/AND!XOR/ILLUS6.RAW" };
    uint8_t button = 0;

    util_led_clear();

    //If anything other than left button is pressed cycle modes
    while ((button & BUTTON_MASK_LEFT) == 0) {
        uint8_t hue = 0;
        button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__led_hue_cycle, true, &hue);
        index = (index + 1) % count;
    }
}

void mbp_bling_whats_up() {
    util_led_clear();
    uint8_t hue = 0; //hue is normally a float 0 to 1, pack it in an 8 bit int
    util_gfx_draw_raw_file("BLING/AND!XOR/HEMANHEY.RAW", 0, 0, 128, 128, &__led_sparkle, true, &hue);
}

void mbp_bling_led_botnet(uint8_t frame, void *data) {
//
//  for (uint8_t i = 0; i < LED_COUNT; i++) {
//      led_rgb_t rgb = util_led_get(i);
//      if (rgb.red > 0) {
//          util_led_set(i, (rgb.red - 1), 0, 0);
//      }
//  }
    //Turn off an led
    util_led_set(util_math_rand8_max(LED_COUNT), 0, 0, 0);

    //Turn on an led
    uint8_t red = util_math_rand8_max(100) + 155;
    uint8_t i = util_math_rand8_max(LED_COUNT);
    util_led_set(i, red, 0, 0);
    util_led_show();
}

void mbp_bling_major_lazer(void *data) {
    util_led_clear();
    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file("BLING/MAJORL1.RGB", &anim);
    util_gfx_draw_raw_file("BLING/AND!XOR/MAJORL1.RAW", 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
}

static void __matrix_callback(uint8_t frame, void *p_data) {
    uint8_t brightness[5][4];
    memcpy(brightness, p_data, 5 * 4);
    if ((frame % 5) == 0) {

        //Move drops down
        for (uint8_t x = 0; x < 5; x++) {
            for (uint8_t y = 0; y < 4; y++) {
                if (brightness[x][y] == 240) {
                    brightness[x][y] = 190;
                    brightness[x][(y + 1) % 4] = 240;
                    brightness[x][(y + 2) % 4] = 0;
                    brightness[x][(y + 3) % 4] = 100;
                    break;
                }
            }
        }

        //Map XY to LED indices
        uint8_t mapping[LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT] = LED_MATRIX_FULL_MAPPING;
        for (uint8_t x = 0; x < LED_MATRIX_FULL_COL_COUNT; x++) {
            for (uint8_t y = 0; y < LED_MATRIX_FULL_ROW_COUNT; y++) {
                uint8_t index = mapping[(y * LED_MATRIX_FULL_COL_COUNT) + x];
                if (index < LED_COUNT) {
                    util_led_set(index, 0, brightness[x][y], 0);
                }
            }
        }

        util_led_show();
    }

    memcpy(p_data, brightness, 5 * 4);
}

void mbp_bling_matrix() {
    util_led_clear();
    uint8_t brightness[5][4];
    for (uint8_t x = 0; x < 5; x++) {
        uint8_t r = util_math_rand8_max(4);
        brightness[x][r] = 240;
    }
    util_gfx_draw_raw_file("BLING/AND!XOR/MATRIX.RAW", 0, 0, 128, 128, &__matrix_callback, true, (void *) &brightness);
}

void mbp_bling_nyan() {
    util_led_clear();
    uint8_t hue = 0; //hue is normally a float 0 to 1, pack it in an 8 bit int
    util_gfx_draw_raw_file("BLING/AND!XOR/NAYAN.RAW", 0, 0, 128, 128, &__led_sparkle, true, &hue);
}

void mbp_bling_owl() {
    uint8_t hue = 0;
    uint8_t index = 0;
    uint8_t count = 3;

    char *modes[] = { "BLING/AND!XOR/OWL1.RAW", "BLING/AND!XOR/OWL2.RAW", "BLING/AND!XOR/OWL3.RAW" };
    uint8_t button = 0;

    util_led_clear();

    //If anything other than left button is pressed cycle modes
    while ((button & BUTTON_MASK_LEFT) == 0) {
        button = util_gfx_draw_raw_file(modes[index], 0, 0, 128, 128, &__mbp_bling_rainbow_eye_callback, true, &hue);
        index = (index + 1) % count;
    }
}

void mbp_bling_pirate() {
    util_led_clear();
    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file("BLING/GOLD.RGB", &anim);

    uint8_t button = 0;
    //Prevent escape from bling (so we can catch action button)
    while ((button & BUTTON_MASK_LEFT) == 0) {
        button = util_gfx_draw_raw_file("BLING/AND!XOR/PIRATES.RAW", 0, 0, 128, 128, &__rgb_file_callback, true, (void *) &anim);
    }
}

void mbp_bling_rickroll() {
    util_led_clear();
    util_gfx_draw_raw_file("/BLING/AND!XOR/RICKROLL.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, &__spectrum_analyzer_callback, true, NULL);
}

uint8_t mbp_bling_scroll(char *text, bool loop) {
    util_gfx_invalidate();

    //Make sure all scroll text is upper case
    for (uint8_t i = 0; i < strlen(text); i++) {
        text[i] = toupper((int)text[i]);
    }
    int16_t y = (GFX_HEIGHT - SCROLL_CHAR_HEIGHT) / 2;
    int16_t x = GFX_WIDTH;

    //" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.!?,()[]{}<>/\\|;:&^%$#@*-_+"
    char *files[] = {
            "SCROLL/SPACE.FNT",
            "SCROLL/A.FNT",
            "SCROLL/B.FNT",
            "SCROLL/C.FNT",
            "SCROLL/D.FNT",
            "SCROLL/E.FNT",
            "SCROLL/F.FNT",
            "SCROLL/G.FNT",
            "SCROLL/H.FNT",
            "SCROLL/I.FNT",
            "SCROLL/J.FNT",
            "SCROLL/K.FNT",
            "SCROLL/L.FNT",
            "SCROLL/M.FNT",
            "SCROLL/N.FNT",
            "SCROLL/O.FNT",
            "SCROLL/P.FNT",
            "SCROLL/Q.FNT",
            "SCROLL/R.FNT",
            "SCROLL/S.FNT",
            "SCROLL/T.FNT",
            "SCROLL/U.FNT",
            "SCROLL/V.FNT",
            "SCROLL/W.FNT",
            "SCROLL/X.FNT",
            "SCROLL/Y.FNT",
            "SCROLL/Z.FNT",
            "SCROLL/0.FNT",
            "SCROLL/1.FNT",
            "SCROLL/2.FNT",
            "SCROLL/3.FNT",
            "SCROLL/4.FNT",
            "SCROLL/5.FNT",
            "SCROLL/6.FNT",
            "SCROLL/7.FNT",
            "SCROLL/8.FNT",
            "SCROLL/9.FNT",
            "SCROLL/PERIOD.FNT",
            "SCROLL/EXCL.FNT",
            "SCROLL/QUESTION.FNT",
            "SCROLL/COMMA.FNT",
            "SCROLL/LPAREN.FNT",
            "SCROLL/RPAREN.FNT",
            "SCROLL/LBRACKET.FNT",
            "SCROLL/RBRACKET.FNT",
            "SCROLL/LBRACE.FNT",
            "SCROLL/RBRACE.FNT",
            "SCROLL/LT.FNT",
            "SCROLL/GT.FNT",
            "SCROLL/FSLASH.FNT",
            "SCROLL/BSLASH.FNT",
            "SCROLL/PIPE.FNT",
            "SCROLL/SEMI.FNT",
            "SCROLL/COLON.FNT",
            "SCROLL/AMP.FNT",
            "SCROLL/CARROT.FNT",
            "SCROLL/PCT.FNT",
            "SCROLL/DOLLAR.FNT",
            "SCROLL/HASH.FNT",
            "SCROLL/AT.FNT",
            "SCROLL/STAR.FNT",
            "SCROLL/DASH.FNT",
            "SCROLL/USCORE.FNT",
            "SCROLL/PLUS.FNT"
    };

    while (1) {
        if (!util_gfx_is_valid_state()) {
            mbp_ui_cls();
        }

        util_gfx_fill_rect(0, y, SCROLL_CHAR_WIDTH, SCROLL_CHAR_HEIGHT, COLOR_BLACK);

        for (uint8_t i = 0; i < strlen(text); i++) {
            const char *ptr = strchr(INPUT_CHARS, text[i]);

            if (ptr) {
                uint16_t xx = x + (i * SCROLL_CHAR_WIDTH);
                int index = ptr - INPUT_CHARS;

                if (xx > 0 && xx < (GFX_WIDTH - SCROLL_CHAR_WIDTH))
                    util_gfx_draw_raw_file(files[index], xx, y, SCROLL_CHAR_WIDTH, SCROLL_CHAR_HEIGHT, NULL, false, NULL);
            }
        }
        util_gfx_validate();

        x += SCROLL_SPEED;

        int16_t x_min = 0 - (SCROLL_CHAR_WIDTH * strlen(text));
        //If we run off the left edge, loop around maybe
        if (x < x_min) {
            if (!loop)
                break;
            x = GFX_WIDTH;
        }

        if ((util_button_action() && loop) || util_button_left()) {
            uint8_t button = util_button_state();
            util_button_clear();
            return button;
        }

        app_sched_execute();
        nrf_delay_ms(SCROLL_TIME_MS);
    }

    util_gfx_invalidate();
    uint8_t button = util_button_state();
    util_button_clear();
    return button;
}

static void __scroll_callback(void *p_data) {
    led_anim_t *p_anim = (led_anim_t *) p_data;
    util_led_play_rgb_frame(p_anim);
}

void mbp_bling_scroll_cycle() {
    util_led_clear();
    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file("BLING/KIT.RGB", &anim);

    //Get the usesrname
    char name[SETTING_NAME_LENGTH];
    mbp_state_name_get(name);

    uint8_t index = 0;
    uint8_t count = 3;
    char *messages[] = { "JOCO 2018 ", "STINKING BADGES ", name };

    //Start up led timer for scroll
    APP_ERROR_CHECK(app_timer_create(&m_scroll_led_timer, APP_TIMER_MODE_REPEATED, __scroll_callback));
    APP_ERROR_CHECK(app_timer_start(m_scroll_led_timer, APP_TIMER_TICKS(1000/20, UTIL_TIMER_PRESCALER), &anim));

    while (1) {
        uint8_t button = mbp_bling_scroll(messages[index], false);
        index = (index + 1) % count;

        if ((button & BUTTON_MASK_LEFT) > 0) {
            break;
        }
    }

    app_timer_stop(m_scroll_led_timer);

    util_gfx_invalidate();
    util_button_clear();
}

void mbp_bling_score_schedule_handler(void * p_event_data, uint16_t event_size) {
    char *name = (char *) p_event_data;
    uint16_t w, h;
    app_sched_pause();
    bool tooth = mbp_tooth_eye_running();
    mbp_tooth_eye_stop();

    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file("BLING/GRNBLUE.RGB", &anim);
    util_gfx_draw_raw_file("MBSCORE.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);

    uint16_t fg = util_gfx_rgb_to_565(COLOR_BLACK);

    //Compute name coords
    util_gfx_set_font(FONT_LARGE);
    util_gfx_get_text_bounds(name, 0, 0, &w, &h);
    uint16_t y = 80;
    uint16_t x = (GFX_WIDTH - w) / 2;

    //Print name
    util_gfx_set_color(fg);
    util_gfx_set_cursor(x, y);
    util_gfx_print(name);

    // 6 second score time (20 FPS)
    for (uint16_t i = 0; i < (20 * SCORE_DISPLAY_TIME); i++) {
        util_led_play_rgb_frame(&anim);
        nrf_delay_ms(50);
    }

    //Cleanup and give control back to user
    util_gfx_invalidate();
    if (tooth) {
        mbp_tooth_eye_start();
    }
    app_sched_resume();
    util_button_clear();
}

void mbp_bling_hello_joco_schedule_handler(void * p_event_data, uint16_t event_size) {
    char *name = (char *) p_event_data;
    uint16_t w, h;
    app_sched_pause();
    bool tooth = mbp_tooth_eye_running();
    mbp_tooth_eye_stop();

    //Pick colors
    float h1 = ((float) util_math_rand8_max(100) / 100.0);
    float h2 = h1 + 0.5;
    if (h2 >= 1.0) {
        h2 -= 1.0;
    }
    uint32_t color_1 = util_led_hsv_to_rgb(h1, 1, 1);
    uint32_t color_2 = util_led_hsv_to_rgb(h2, 1, 1);

    util_gfx_draw_raw_file("B2B/PM_HELLO.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);
    uint16_t fg = util_gfx_rgb_to_565(color_1);
    uint16_t bg = util_gfx_rgb_to_565(color_2);

    //Compute name coords
    util_gfx_set_font(FONT_LARGE);
    util_gfx_get_text_bounds(name, 0, 0, &w, &h);
    uint16_t y = (GFX_HEIGHT / 2) + 4;
    uint16_t x = (GFX_WIDTH - w) / 2;

    //Print shadow
    util_gfx_set_color(bg);
    util_gfx_set_cursor(x + 1, y + 1);
    util_gfx_print(name);

    //Print name
    util_gfx_set_color(fg);
    util_gfx_set_cursor(x, y);
    util_gfx_print(name);

    //Compute hello coords
    char hello[] = "Hello";
    util_gfx_get_text_bounds(hello, 0, 0, &w, &h);
    x = (GFX_WIDTH - w) / 2;
    y = (GFX_HEIGHT / 2) - 4 - h;

    //Print shadow
    util_gfx_set_color(bg);
    util_gfx_set_cursor(x + 1, y + 1);
    util_gfx_print(hello);

    //Print hello
    util_gfx_set_color(fg);
    util_gfx_set_cursor(x, y);
    util_gfx_print(hello);

    //Set all LEDs
    util_led_set_all_rgb(color_1);
    util_led_show();
    nrf_delay_ms(2000);

    uint8_t cols[5][4] = {
            { 8, 4, 0, 12 },
            { 9, 5, 1, 255 },
            { 10, 6, 2, 255 },
            { 11, 7, 3, 13 },
            { 255, 255, 255, 14 }
    };
    uint8_t height[] = { 4, 4, 4, 4, 4 };

    while (1) {
        //pick a random column to lower
        uint8_t col = util_math_rand8_max(5);
        if (height[col] > 0) {
            height[col]--;
            uint8_t index = cols[col][height[col]];

            if (index < LED_COUNT) {
                util_led_set_rgb(index, color_2);
                util_led_show();
                nrf_delay_ms(40);
                util_led_set(index, 0, 0, 0);
                util_led_show();
            }
        }

        nrf_delay_ms(30);

        bool done = true;
        for (uint8_t i = 0; i < 5; i++) {
            if (height[i] > 0) {
                done = false;
                break;
            }
        }

        if (done) {
            break;
        }
    }

    //Cleanup and give control back to user
    util_gfx_invalidate();
    if (tooth) {
        mbp_tooth_eye_start();
    }
    app_sched_resume();
    util_button_clear();
}

void mbp_bling_hello_bender_schedule_handler(void * p_event_data, uint16_t event_size) {
    char *name = (char *) p_event_data;
    uint16_t w, h;
    app_sched_pause();
    bool tooth = mbp_tooth_eye_running();
    mbp_tooth_eye_stop();

    //Pick colors
    float h1 = ((float) util_math_rand8_max(100) / 100.0);
    float h2 = h1 + 0.5;
    if (h2 >= 1.0) {
        h2 -= 1.0;
    }
    uint32_t color_1 = util_led_hsv_to_rgb(h1, 1, 1);
    uint32_t color_2 = util_led_hsv_to_rgb(h2, 1, 1);

    util_gfx_draw_raw_file("BG.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);
    uint16_t fg = util_gfx_rgb_to_565(color_1);
    uint16_t bg = util_gfx_rgb_to_565(color_2);

    //Compute name coords
    util_gfx_set_font(FONT_LARGE);
    util_gfx_get_text_bounds(name, 0, 0, &w, &h);
    uint16_t y = (GFX_HEIGHT / 2) + 4;
    uint16_t x = (GFX_WIDTH - w) / 2;

    //Print shadow
    util_gfx_set_color(bg);
    util_gfx_set_cursor(x + 1, y + 1);
    util_gfx_print(name);

    //Print name
    util_gfx_set_color(fg);
    util_gfx_set_cursor(x, y);
    util_gfx_print(name);

    //Compute hello coords
    char hello[] = "Hello";
    util_gfx_get_text_bounds(hello, 0, 0, &w, &h);
    x = (GFX_WIDTH - w) / 2;
    y = (GFX_HEIGHT / 2) - 4 - h;

    //Print shadow
    util_gfx_set_color(bg);
    util_gfx_set_cursor(x + 1, y + 1);
    util_gfx_print(hello);

    //Print hello
    util_gfx_set_color(fg);
    util_gfx_set_cursor(x, y);
    util_gfx_print(hello);

    //Set all LEDs
    util_led_set_all_rgb(color_1);
    util_led_show();
    nrf_delay_ms(2000);

    uint8_t cols[5][4] = {
            { 8, 4, 0, 12 },
            { 9, 5, 1, 255 },
            { 10, 6, 2, 255 },
            { 11, 7, 3, 13 },
            { 255, 255, 255, 14 }
    };
    uint8_t height[] = { 4, 4, 4, 4, 4 };

    while (1) {
        //pick a random column to lower
        uint8_t col = util_math_rand8_max(5);
        if (height[col] > 0) {
            height[col]--;
            uint8_t index = cols[col][height[col]];

            if (index < LED_COUNT) {
                util_led_set_rgb(index, color_2);
                util_led_show();
                nrf_delay_ms(40);
                util_led_set(index, 0, 0, 0);
                util_led_show();
            }
        }

        nrf_delay_ms(30);

        bool done = true;
        for (uint8_t i = 0; i < 5; i++) {
            if (height[i] > 0) {
                done = false;
                break;
            }
        }

        if (done) {
            break;
        }
    }

    //Cleanup and give control back to user
    util_gfx_invalidate();
    if (tooth) {
        mbp_tooth_eye_start();
    }
    app_sched_resume();
    util_button_clear();
}

void mbp_bling_hello_cpv_schedule_handler(void * p_event_data, uint16_t event_size) {
    app_sched_pause();
    bool tooth = mbp_tooth_eye_running();
    mbp_tooth_eye_stop();

    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file("BLING/PINKBLUE.RGB", &anim);
    util_gfx_draw_raw_file("B2B/CPV.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);

    //4 second hello time (20 FPS)
    for (uint16_t i = 0; i < (20 * 4); i++) {
        util_led_play_rgb_frame(&anim);
        nrf_delay_ms(50);
    }

    //Cleanup and give control back to user
    util_led_clear();
    util_gfx_invalidate();
    if (tooth) {
        mbp_tooth_eye_start();
    }
    app_sched_resume();
    util_button_clear();
}


void mbp_bling_hello_dc503_schedule_handler(void * p_event_data, uint16_t event_size) {
    app_sched_pause();
    bool tooth = mbp_tooth_eye_running();
    mbp_tooth_eye_stop();

    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file("BLING/TUNNEL2.RGB", &anim);
    util_gfx_draw_raw_file("B2B/DC503.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);

    //4 second hello time (20 FPS)
    for (uint16_t i = 0; i < (20 * 4); i++) {
        util_led_play_rgb_frame(&anim);
        nrf_delay_ms(50);
    }

    //Cleanup and give control back to user
    util_led_clear();
    util_gfx_invalidate();
    if (tooth) {
        mbp_tooth_eye_start();
    }
    app_sched_resume();
    util_button_clear();
}

void mbp_bling_hello_dc801_schedule_handler(void * p_event_data, uint16_t event_size) {
    app_sched_pause();
    bool tooth = mbp_tooth_eye_running();
    mbp_tooth_eye_stop();

    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file("BLING/GRNBLUE.RGB", &anim);
    util_gfx_draw_raw_file("B2B/DC801.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);

    //4 second hello time (20 FPS)
    for (uint16_t i = 0; i < (20 * 4); i++) {
        util_led_play_rgb_frame(&anim);
        nrf_delay_ms(50);
    }

    //Cleanup and give control back to user
    util_led_clear();
    util_gfx_invalidate();
    if (tooth) {
        mbp_tooth_eye_start();
    }
    app_sched_resume();
    util_button_clear();
}

void mbp_bling_hello_queercon_schedule_handler(void * p_event_data, uint16_t event_size) {
    app_sched_pause();
    bool tooth = mbp_tooth_eye_running();
    mbp_tooth_eye_stop();

    UTIL_LED_ANIM_INIT(anim);
    util_led_load_rgb_file("BLING/COLORS.RGB", &anim);
    util_gfx_draw_raw_file("B2B/QUEERCON.RAW", 0, 0, GFX_WIDTH, GFX_HEIGHT, NULL, false, NULL);

    //4 second hello time (20 FPS)
    for (uint16_t i = 0; i < (20 * 4); i++) {
        util_led_play_rgb_frame(&anim);
        nrf_delay_ms(50);
    }

    //Cleanup and give control back to user
    util_led_clear();
    util_gfx_invalidate();
    if (tooth) {
        mbp_tooth_eye_start();
    }
    app_sched_resume();
    util_button_clear();
}

static void __mbp_bling_toad_callback(uint8_t frame, void *p_data) {
//Unpack hue as a float
    uint8_t *data = (uint8_t *) p_data;
    float hue = ((float) *data / 100.0);

    util_led_set_all_rgb(util_led_hsv_to_rgb(hue, 1, .9));
    util_led_show();

    hue += 0.07;
    if (hue >= 1) {
        hue = 0;
    }

    //Pack the hue
    *data = (uint8_t) (hue * 100.0);
}

void mbp_bling_toad() {
    util_led_clear();
    uint8_t hue = 0; //hue is normally a float 0 to 1, pack it in an 8 bit int
    util_gfx_draw_raw_file("BLING/AND!XOR/TOAD2.RAW", 0, 0, 128, 128, &__mbp_bling_toad_callback, true, &hue);
}

static void __mbp_bling_twitter_callback(uint8_t frame, void *p_data) {
    uint8_t *p_index = (uint8_t *) p_data;

    //If the stored data is a valid index, set that LED to blue or black
    if (*p_index < LED_COUNT) {
        if (util_math_rand8_max(2) == 0) {
            util_led_set(*p_index, 29, 161, 243);
        } else {
            util_led_set_rgb(*p_index, LED_COLOR_BLACK);
        }

        //Pack invalid data so next time around a new LED is picked
        *p_index = 0xFF;
    }
    //Otherwise if it's an invalid index randomly pick one to flash
    else {
        *p_index = util_math_rand8_max(LED_COUNT);
        util_led_set_rgb(*p_index, LED_COLOR_WHITE);
    }

    util_led_show();
}

void mbp_bling_trololol() {
    util_led_clear();
    util_gfx_draw_raw_file("BLING/AND!XOR/TROLOLOL.RAW", 0, 0, 128, 128, &mbp_bling_led_botnet, true, NULL);
}

void mbp_bling_twitter() {
    util_led_clear();
    uint8_t index = 0xFF; //set index out of bounds of led count
    util_gfx_draw_raw_file("BLING/TWITTER/TWITTER.RAW", 0, 0, 128, 128, &__mbp_bling_twitter_callback, true, &index);
}

static void __mbp_defrag_callback(uint8_t frame, void *p_data) {
    bling_defag_state_t *p_defrag = (bling_defag_state_t *) p_data;
    uint8_t count = (LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT);
    uint8_t led_mapping[LED_MATRIX_FULL_COL_COUNT * LED_MATRIX_FULL_ROW_COUNT] = LED_MATRIX_FULL_MAPPING
    ;
    uint32_t rgb = util_led_hsv_to_rgb(p_defrag->hue, 1, .8);
    uint8_t index = led_mapping[p_defrag->index];

    if (index < LED_COUNT) {
        util_led_set_rgb(index, rgb);
        util_led_show();
    }

    p_defrag->index++;
    if (p_defrag->index >= count) {
        p_defrag->index = 0;
        p_defrag->hue += .05;

        if (p_defrag->hue >= 1) {
            p_defrag->hue = 0;
        }
    }
}

void mbp_bling_defrag() {
    bling_defag_state_t defrag;
    defrag.hue = 0;
    defrag.index = 0;
    util_led_clear();
    util_gfx_draw_raw_file("BLING/AND!XOR/DEFRAG.RAW", 0, 0, 128, 128, &__mbp_defrag_callback, true, &defrag);
}

// called every TOOTH_EYES_TIME_MS
// 10
static int eye_cycle_count;
#define EYE_CYCLE_LEN 500
#define EYE_CYCLE_HALF 20

static void __tooth_sch_handler(void * p_event_data, uint16_t event_size) {
    uint8_t eye_closed[] = { 4, 5, 6, 7 };
    uint8_t eye_open[] = { 1, 2, 4, 7, 9, 10 };

    //Clear all colors
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        util_led_set_rgb(i, LED_COLOR_BLACK);
    }

    //Compute and set the eye colors
    uint32_t eye_color = util_led_hsv_to_rgb(m_eye_hue, 1, 0.8);
    util_led_set_rgb(LED_RIGHT_EYE_INDEX, eye_color);

    uint32_t tooth_color = util_led_hsv_to_rgb(m_tooth_hue, 1, 0.5);
    util_led_set_rgb(LED_TOOTH_INDEX, tooth_color);

    if (eye_cycle_count > EYE_CYCLE_HALF) {
        //Large Eye
        for (uint8_t i = 0; i < 6; i++) {
            util_led_set_rgb(eye_open[i], eye_color);
        }
    } else {
        //Small Eye
        for (uint8_t i = 0; i < 4; i++) {
            util_led_set_rgb(eye_closed[i], eye_color);
        }
    }
    if (++eye_cycle_count > EYE_CYCLE_LEN)
        eye_cycle_count = 0;

    util_led_show();

        // Update the tooth for next pass
        if (tooth_flashing) {
            // we're twinkling, see if we're done
            if (--tooth_flash_counter == 0) {
                m_tooth_hue = TOOTH_HUE_GOLD;
                m_tooth_sat = TOOTH_SAT;
                m_tooth_val = TOOTH_VAL;
                tooth_flash_counter = util_math_rand32_max(TOOTH_FLASH_MAXTIME-TOOTH_FLASH_MINTIME) + TOOTH_FLASH_MINTIME;
                tooth_flashing = false;
            }
        } else {
            // we're waiting to twinkle again, see if we're done
            if (--tooth_flash_counter == 0) {
                // start a flash
                m_tooth_hue = TOOTH_HUE_GOLD;
                m_tooth_sat = 0;
                m_tooth_val = TOOTH_VAL;
                tooth_flash_counter = TOOTH_FLASH_LEN;
                tooth_flashing = true;
            }
        }

    //Change the eye hue for next pass
    m_eye_hue += EYE_HUE_STEP;
    if (m_eye_hue >= 1.0) {
        m_eye_hue -= 1.0;
    }
}

static void __tooth_timer_handler(void *p_data) {
    app_sched_event_put(NULL, 0, __tooth_sch_handler);
}

bool mbp_tooth_eye_running() {
    return m_tooth_eye_running;
}

void mbp_tooth_eye_start() {
    if (!m_tooth_eye_running) {
        //Start up tooth flicker timer
        APP_ERROR_CHECK(app_timer_create(&m_tooth_timer, APP_TIMER_MODE_REPEATED, __tooth_timer_handler));
        APP_ERROR_CHECK(app_timer_start(m_tooth_timer, APP_TIMER_TICKS(TOOTH_EYES_TIME_MS, UTIL_TIMER_PRESCALER), NULL));

        m_tooth_eye_running = true;
    }
}

void mbp_tooth_eye_stop() {
    if (m_tooth_eye_running) {
        util_led_set(LED_RIGHT_EYE_INDEX, 0, 0, 0);
        util_led_set(LED_TOOTH_INDEX, 0, 0, 0);
        util_led_show();
        APP_ERROR_CHECK(app_timer_stop(m_tooth_timer));

        m_tooth_eye_running = false;
    }
}
