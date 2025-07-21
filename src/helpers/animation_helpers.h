#include <stdio.h>
#include <lvgl.h>

void text_fade_anim_cb(void *label_obj, int32_t opa);
void fade_in_obj(lv_obj_t *obj, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb = NULL);
void fade_out_obj(lv_obj_t *obj, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb = NULL);
