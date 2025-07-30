#include "animation_helpers.h"
#include <lvgl.h>
#include <stdio.h>

// Animation callback for label fade-in
void text_fade_anim_cb(void *label_obj, int32_t opa)
{
  lv_obj_set_style_text_opa((lv_obj_t *)label_obj, opa, 0);
}

// Called when the fade-in animation finishes
// Helper: fade in a label or arc
void fade_in_obj(lv_obj_t *obj, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb)
{
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, obj);
  printf("[fade_in_obj] Fading in obj=%p, duration=%u, delay=%u\n", obj, duration, delay);
  if (lv_obj_check_type(obj, &lv_label_class))
  {
    lv_anim_set_exec_cb(&anim, text_fade_anim_cb);
    lv_obj_set_style_text_opa(obj, LV_OPA_TRANSP, 0);
  }
  else if (lv_obj_check_type(obj, &lv_arc_class))
  {
    lv_anim_set_exec_cb(&anim, [](void *arc_obj, int32_t opa)
                        { lv_obj_set_style_arc_opa((lv_obj_t *)arc_obj, opa, LV_PART_INDICATOR); });
    lv_obj_set_style_arc_opa(obj, LV_OPA_TRANSP, LV_PART_INDICATOR);
  }
  else
  {
    // For buttons and general objects, animate LV_STYLE_OPA on LV_PART_MAIN
    lv_anim_set_exec_cb(&anim, [](void *o, int32_t opa)
                        { lv_obj_set_style_opa((lv_obj_t *)o, opa, LV_PART_MAIN); });
    lv_obj_set_style_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
  }
  lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
  lv_anim_set_time(&anim, duration);
  lv_anim_set_delay(&anim, delay);
  if (ready_cb)
    lv_anim_set_ready_cb(&anim, ready_cb);
  lv_anim_start(&anim);
}

// Helper: fade out a label or arc
void fade_out_obj(lv_obj_t *obj, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb)
{
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, obj);
  if (lv_obj_check_type(obj, &lv_label_class))
  {
    lv_anim_set_exec_cb(&anim, text_fade_anim_cb);
    lv_obj_set_style_text_opa(obj, LV_OPA_COVER, 0);
  }
  else if (lv_obj_check_type(obj, &lv_arc_class))
  {
    lv_anim_set_exec_cb(&anim, [](void *arc_obj, int32_t opa)
                        { lv_obj_set_style_arc_opa((lv_obj_t *)arc_obj, opa, LV_PART_INDICATOR); });
    lv_obj_set_style_arc_opa(obj, LV_OPA_COVER, LV_PART_INDICATOR);
  }
  else
  {
    // For buttons and general objects, animate LV_STYLE_OPA on LV_PART_MAIN
    lv_anim_set_exec_cb(&anim, [](void *o, int32_t opa)
                        { lv_obj_set_style_opa((lv_obj_t *)o, opa, LV_PART_MAIN); });
    lv_obj_set_style_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
  }
  lv_anim_set_values(&anim, LV_OPA_COVER, LV_OPA_TRANSP);
  lv_anim_set_time(&anim, duration);
  lv_anim_set_delay(&anim, delay);
  if (ready_cb)
    lv_anim_set_ready_cb(&anim, ready_cb);
  lv_anim_start(&anim);
}

// slide in animation for menus or side panels
void slide_in_obj_horizontal(lv_obj_t *obj, lv_coord_t start_x, lv_coord_t end_x, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb)
{
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, obj);
  lv_anim_set_exec_cb(&anim, [](void *o, int32_t x)
                      { lv_obj_set_x((lv_obj_t *)o, x); });
  lv_obj_set_x(obj, start_x); // Start position
  lv_anim_set_values(&anim, start_x, end_x);
  lv_anim_set_time(&anim, duration);
  lv_anim_set_delay(&anim, delay);
  if (ready_cb)
    lv_anim_set_ready_cb(&anim, ready_cb);
  lv_anim_start(&anim);
}

// slide in animation for vertical movement (Y axis)
void slide_in_obj_vertical(lv_obj_t *obj, lv_coord_t start_y, lv_coord_t end_y, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb)
{
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, obj);
  lv_anim_set_exec_cb(&anim, [](void *o, int32_t y)
                      { lv_obj_set_y((lv_obj_t *)o, y); });
  lv_obj_set_y(obj, start_y); // Start position
  lv_anim_set_values(&anim, start_y, end_y);
  lv_anim_set_time(&anim, duration);
  lv_anim_set_delay(&anim, delay);
  if (ready_cb)
    lv_anim_set_ready_cb(&anim, ready_cb);
  lv_anim_start(&anim);
}