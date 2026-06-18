/*
 * Copyright (c) 2024 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#include "custom_status_screen.h"

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
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, 255, LV_PART_MAIN);

    lv_style_init(&global_style);
    lv_style_set_text_color(&global_style, lv_color_white());
    lv_style_set_text_letter_space(&global_style, 1);
    lv_style_set_text_line_space(&global_style, 1);
    lv_obj_add_style(screen, &global_style, LV_PART_MAIN);

    /* --- Layout for 280x240 (landscape: 280px wide, 240px tall) ---
     *
     *  y=0  ┌─────────────────────────────┐
     *       │   output_status (top, 35px) │  TOP_MID, y_offset=5
     *  y=35 ├─────────────────────────────┤
     *       │                             │
     *       │   layer_roller (120px)      │  CENTER, y_offset=-35
     *       │   (3 rows × 40px = 120px)   │   width=240, height=120
     *       │                             │
     * y=155 ├─────────────────────────────┤
     *       │   mod_status (40px)         │  CENTER, y_offset=55
     * y=195 ├─────────────────────────────┤
     *       │   battery_bar (45px)        │  BOTTOM_MID, y_offset=0
     * y=240 └─────────────────────────────┘
     *
     * Fine-tune y_offsets after first on-device flash.
     */

#if CONFIG_DONGLE_SCREEN_OUTPUT_ACTIVE
    zmk_widget_output_status_init(&output_status_widget, screen);
    lv_obj_align(zmk_widget_output_status_obj(&output_status_widget),
                 LV_ALIGN_TOP_MID, 0, 5);
#endif

#if CONFIG_DONGLE_SCREEN_LAYER_ACTIVE
    zmk_widget_layer_roller_init(&layer_roller_widget, screen);
    lv_obj_set_size(zmk_widget_layer_roller_obj(&layer_roller_widget), 240, 120);
    lv_obj_align(zmk_widget_layer_roller_obj(&layer_roller_widget),
                 LV_ALIGN_CENTER, 0, -35);
#endif

#if CONFIG_DONGLE_SCREEN_MODIFIER_ACTIVE
    zmk_widget_mod_status_init(&mod_widget, screen);
    lv_obj_set_size(zmk_widget_mod_status_obj(&mod_widget), 240, 40);
    lv_obj_align(zmk_widget_mod_status_obj(&mod_widget),
                 LV_ALIGN_CENTER, 0, 55);
#endif

#if CONFIG_DONGLE_SCREEN_BATTERY_ACTIVE
    zmk_widget_battery_bar_init(&battery_bar_widget, screen);
    lv_obj_set_size(zmk_widget_battery_bar_obj(&battery_bar_widget), 280, 45);
    lv_obj_align(zmk_widget_battery_bar_obj(&battery_bar_widget),
                 LV_ALIGN_BOTTOM_MID, 0, 0);
#endif

    return screen;
}
