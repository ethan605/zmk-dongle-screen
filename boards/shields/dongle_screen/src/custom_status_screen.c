/*
 * Copyright (c) 2024 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#include "custom_status_screen.h"

#include <palette.h>

#if CONFIG_DONGLE_SCREEN_OUTPUT_ACTIVE
#include "widgets/output_status.h"
static struct zmk_widget_output_status output_status_widget;
#endif

#if CONFIG_DONGLE_SCREEN_LAYER_ACTIVE
#include "widgets/layer_roller.h"          /* prospector roller */
static struct zmk_widget_layer_roller layer_roller_widget;
#endif

#if CONFIG_DONGLE_SCREEN_MODIFIER_ACTIVE
#include "widgets/mod_status.h"
static struct zmk_widget_mod_status mod_widget;
#endif

#if CONFIG_DONGLE_SCREEN_BATTERY_ACTIVE
#include "widgets/battery_bar.h"           /* prospector battery bar */
static struct zmk_widget_battery_bar battery_bar_widget;
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

lv_style_t global_style;

lv_obj_t *zmk_display_status_screen()
{
    lv_obj_t *screen;

    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(SNAZZY_BLACK), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, 255, LV_PART_MAIN);

    lv_style_init(&global_style);
    lv_style_set_text_color(&global_style, lv_color_hex(SNAZZY_WHITE));
    lv_style_set_text_letter_space(&global_style, 1);
    lv_style_set_text_line_space(&global_style, 1);
    lv_obj_add_style(screen, &global_style, LV_PART_MAIN);

    /* --- Layout for 280x240 (landscape: 280px wide, 240px tall) ---
     *
     *  y=0  ┌─────────────────────────────────┐
     *       │                  ┌────────────┐ │ connections (170×36)
     *  y=42 │                  └────────────┘ │ TOP_RIGHT -8, 6
     *  y=45 │                                 │
     *       │ ┌──────────────┐  ┌───────────┐ │
     *       │ │              │  │           │ │
     *       │ │ layer_roller │  │ mod 2×2   │ │
     *       │ │   150×150    │  │  110×100  │ │
     *       │ │  LEFT_MID    │  │ RIGHT_MID │ │
     *       │ │    6, 0      │  │  -8, 0    │ │
     *       │ └──────────────┘  └───────────┘ │
     * y=195 ├─────────────────────────────────┤
     *       │       battery_bar (280×45)       │ BOTTOM_MID 0, 0
     * y=240 └─────────────────────────────────┘
     *
     * Fine-tune offsets after first on-device flash.
     */

#if CONFIG_DONGLE_SCREEN_OUTPUT_ACTIVE
    zmk_widget_output_status_init(&output_status_widget, screen);
    lv_obj_set_size(zmk_widget_output_status_obj(&output_status_widget), 170, 36);
    lv_obj_align(zmk_widget_output_status_obj(&output_status_widget),
                 LV_ALIGN_TOP_RIGHT, -8, 6);
#endif

#if CONFIG_DONGLE_SCREEN_LAYER_ACTIVE
    zmk_widget_layer_roller_init(&layer_roller_widget, screen);
    lv_obj_set_size(zmk_widget_layer_roller_obj(&layer_roller_widget), 136, 150);
    /* Center vertically in the band from the top edge (y=0) to the battery
     * bars (y=195): midpoint 97.5 vs screen-mid 120 => y offset -22.
     * x=14 gives a left margin. */
    lv_obj_align(zmk_widget_layer_roller_obj(&layer_roller_widget),
                 LV_ALIGN_LEFT_MID, 14, -22);
#endif

#if CONFIG_DONGLE_SCREEN_MODIFIER_ACTIVE
    zmk_widget_mod_status_init(&mod_widget, screen);
    lv_obj_set_size(zmk_widget_mod_status_obj(&mod_widget), 110, 100);
    lv_obj_align(zmk_widget_mod_status_obj(&mod_widget),
                 LV_ALIGN_RIGHT_MID, -8, 0);
#endif

#if CONFIG_DONGLE_SCREEN_BATTERY_ACTIVE
    zmk_widget_battery_bar_init(&battery_bar_widget, screen);
    lv_obj_set_size(zmk_widget_battery_bar_obj(&battery_bar_widget), 280, 45);
    lv_obj_align(zmk_widget_battery_bar_obj(&battery_bar_widget),
                 LV_ALIGN_BOTTOM_MID, 0, 0);
#endif

    return screen;
}
