/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>

#include <fonts.h>
#include <palette.h>

#include "output_status.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct output_status_state
{
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool ble_connected[2];
    bool ble_bonded[2];
    bool usb_is_hid_ready;
};

static struct output_status_state get_state(const zmk_event_t *_eh)
{
    struct output_status_state state = {
        .selected_endpoint = zmk_endpoints_selected(),          // 0 = USB, 1 = BLE
        .active_profile_index = zmk_ble_active_profile_index(), // active BLE profile index
        .usb_is_hid_ready = zmk_usb_is_hid_ready(),            // USB HID ready flag
    };
    for (uint8_t i = 0; i < 2; i++) {
        state.ble_connected[i] = zmk_ble_profile_is_connected(i);
        state.ble_bonded[i] = !zmk_ble_profile_is_open(i);
    }
    return state;
}

/* Map per-profile pairing/connection state to a color string.
 * color = pairing state (green=connected, blue=bonded-idle, dim=unpaired);
 * fill  = selected profile (filled glyph = active selection, outline = not selected). */
static const char *ble_state_color(bool connected, bool bonded) {
    if (connected) return SNAZZY_GREEN_STR; // connected
    if (bonded)    return SNAZZY_BLUE_STR;  // paired but idle
    return SNAZZY_DIM_STR;                  // unpaired / empty slot
}

static void set_status_symbol(struct zmk_widget_output_status *widget, struct output_status_state state)
{
    bool usb_selected = (state.selected_endpoint.transport == ZMK_TRANSPORT_USB);
    bool ble_selected = (state.selected_endpoint.transport == ZMK_TRANSPORT_BLE);
    int idx = state.active_profile_index;

    /* USB icon U+F0553: color = selected vs not */
    const char *c_usb = usb_selected ? SNAZZY_GREEN_STR : SNAZZY_DIM_STR;
    const char *usb_g = "\xF3\xB0\x95\x93";

    /* BLE profile 0: color = pairing state; filled U+F0CA0 if selected, outline U+F0CA1 otherwise */
    const char *c_b1 = ble_state_color(state.ble_connected[0], state.ble_bonded[0]);
    const char *b1_g = (ble_selected && idx == 0)
                           ? "\xF3\xB0\xB2\xA0"
                           : "\xF3\xB0\xB2\xA1";

    /* BLE profile 1: color = pairing state; filled U+F0CA2 if selected, outline U+F0CA3 otherwise */
    const char *c_b2 = ble_state_color(state.ble_connected[1], state.ble_bonded[1]);
    const char *b2_g = (ble_selected && idx == 1)
                           ? "\xF3\xB0\xB2\xA2"
                           : "\xF3\xB0\xB2\xA3";

    char text[64];
    snprintf(text, sizeof(text), "#%s %s# #%s %s# #%s %s#",
             c_usb, usb_g, c_b1, b1_g, c_b2, b2_g);
    lv_label_set_text(widget->label, text);
}

static void output_status_update_cb(struct output_status_state state)
{
    struct zmk_widget_output_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node)
    {
        set_status_symbol(widget, state);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);

// output_status.c
int zmk_widget_output_status_init(struct zmk_widget_output_status *widget, lv_obj_t *parent)
{
    widget->obj = lv_obj_create(parent);
    lv_obj_remove_style_all(widget->obj); /* transparent grouping container (no default bg/padding) */
    lv_obj_set_size(widget->obj, 170, 36);

    widget->label = lv_label_create(widget->obj);
    lv_label_set_recolor(widget->label, true);
    lv_obj_set_style_text_font(widget->label, &SamsungSans_Regular_28, 0);
    lv_obj_set_style_text_align(widget->label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(widget->label, LV_ALIGN_RIGHT_MID, 0, 0);

    sys_slist_append(&widgets, &widget->node);

    widget_output_status_init();
    return 0;
}

lv_obj_t *zmk_widget_output_status_obj(struct zmk_widget_output_status *widget)
{
    return widget->obj;
}
