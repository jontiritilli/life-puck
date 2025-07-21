#pragma once
#include <lvgl.h>

#ifdef __cplusplus
extern "C"
{
#endif

  // Creates a transparent tap/gesture layer on the given parent and registers the given callback for both tap and gesture events.
  lv_obj_t *create_tap_layer(lv_obj_t *parent, lv_event_cb_t cb);

#ifdef __cplusplus
}
#endif
