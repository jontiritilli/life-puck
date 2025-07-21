#include "tap_layer.h"
#include <lvgl.h>
#include <constants/constants.h>
#include <cmath>

lv_obj_t *create_tap_layer(lv_obj_t *parent, lv_event_cb_t cb)
{
  lv_obj_t *tap_layer = lv_btn_create(parent);
  lv_obj_set_size(tap_layer, LV_HOR_RES, LV_VER_RES);
  lv_obj_align(tap_layer, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(tap_layer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(tap_layer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_outline_opa(tap_layer, LV_OPA_TRANSP, 0);
  lv_obj_add_flag(tap_layer, LV_OBJ_FLAG_GESTURE_BUBBLE);
  lv_obj_add_flag(tap_layer, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(tap_layer, cb, LV_EVENT_GESTURE, NULL);
  lv_obj_add_event_cb(tap_layer, cb, LV_EVENT_CLICKED, NULL);
  return tap_layer;
}
