#pragma once

#include <lvgl.h>
#include <zmk/display.h>

struct zmk_widget_mod_status
{
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *mods[4]; /* 0=CTRL, 1=SHIFT, 2=ALT, 3=GUI */
};

int zmk_widget_mod_status_init(struct zmk_widget_mod_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget);
