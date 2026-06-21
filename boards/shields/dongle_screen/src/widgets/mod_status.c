#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/hid.h>
#include <lvgl.h>
#include "mod_status.h"
#include <fonts.h>
#include <palette.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static void update_mod_status(struct zmk_widget_mod_status *widget)
{
    uint8_t mods = zmk_hid_get_keyboard_report()->body.modifiers;

    lv_obj_set_style_text_color(widget->mods[0],
        lv_color_hex((mods & (MOD_LCTL | MOD_RCTL)) ? SNAZZY_WHITE : SNAZZY_DIM), 0);
    lv_obj_set_style_text_color(widget->mods[1],
        lv_color_hex((mods & (MOD_LSFT | MOD_RSFT)) ? SNAZZY_WHITE : SNAZZY_DIM), 0);
    lv_obj_set_style_text_color(widget->mods[2],
        lv_color_hex((mods & (MOD_LALT | MOD_RALT)) ? SNAZZY_WHITE : SNAZZY_DIM), 0);
    lv_obj_set_style_text_color(widget->mods[3],
        lv_color_hex((mods & (MOD_LGUI | MOD_RGUI)) ? SNAZZY_WHITE : SNAZZY_DIM), 0);
}

static void mod_status_timer_cb(struct k_timer *timer)
{
    struct zmk_widget_mod_status *widget = k_timer_user_data_get(timer);
    update_mod_status(widget);
}

static struct k_timer mod_status_timer;

int zmk_widget_mod_status_init(struct zmk_widget_mod_status *widget, lv_obj_t *parent)
{
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 110, 100);

    /* CTRL — top-left */
    widget->mods[0] = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->mods[0], &SamsungSans_Regular_40, 0);
    lv_label_set_text(widget->mods[0], "󰘴"); /* U+F0634 */
    lv_obj_align(widget->mods[0], LV_ALIGN_TOP_LEFT, 0, 0);

    /* SHIFT — top-right */
    widget->mods[1] = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->mods[1], &SamsungSans_Regular_40, 0);
    lv_label_set_text(widget->mods[1], "󰘶"); /* U+F0636 */
    lv_obj_align(widget->mods[1], LV_ALIGN_TOP_RIGHT, 0, 0);

    /* ALT — bottom-left */
    widget->mods[2] = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->mods[2], &SamsungSans_Regular_40, 0);
    lv_label_set_text(widget->mods[2], "󰘵"); /* U+F0635 */
    lv_obj_align(widget->mods[2], LV_ALIGN_BOTTOM_LEFT, 0, 0);

    /* GUI — bottom-right */
    widget->mods[3] = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->mods[3], &SamsungSans_Regular_40, 0);
#if CONFIG_DONGLE_SCREEN_SYSTEM_ICON == 1
    lv_label_set_text(widget->mods[3], "󰌽"); /* U+F033D */
#elif CONFIG_DONGLE_SCREEN_SYSTEM_ICON == 2
    lv_label_set_text(widget->mods[3], ""); /* U+E62A */
#else
    lv_label_set_text(widget->mods[3], "󰘳"); /* U+F0633 */
#endif
    lv_obj_align(widget->mods[3], LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    k_timer_init(&mod_status_timer, mod_status_timer_cb, NULL);
    k_timer_user_data_set(&mod_status_timer, widget);
    k_timer_start(&mod_status_timer, K_MSEC(100), K_MSEC(100));

    return 0;
}

lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget)
{
    return widget->obj;
}
