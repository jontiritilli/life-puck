// gestures.h
// Gesture type definitions and registration for LifePuck

#pragma once
#include <functional>

// Gesture types
enum class GestureType
{
  TapTop,
  TapBottom,
  SwipeUp,
  SwipeDown,
  LongPressMenu,
  MenuTL,
  MenuTR,
  MenuBL,
  MenuBR
};

using GestureCallback = std::function<void()>;

void register_gesture_callback(GestureType gesture, GestureCallback cb);
void init_gesture_handling(lv_obj_t *screen);
void lvgl_gesture_event_handler(lv_event_t *e);