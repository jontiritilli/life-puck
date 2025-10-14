// gestures.cpp
// LVGL gesture handling for LifePuck application
// Ready to be wired to CST816S touch controller

#include <lvgl.h>
#include <functional>
#include <map>
#include <stdio.h>
#include "gestures.h"
#include <constants/constants.h>

// Callback type for gestures
using GestureCallback = std::function<void()>;

// Gesture callback registry
static std::map<GestureType, GestureCallback> gesture_callbacks;

// Register a callback for a gesture
void register_gesture_callback(GestureType gesture, GestureCallback cb)
{
  gesture_callbacks[gesture] = cb;
}

// Call the callback for a gesture
void trigger_gesture(GestureType gesture)
{
  if (gesture_callbacks.count(gesture))
  {
    gesture_callbacks[gesture]();
  }
}

// Example LVGL event handler for tap/swipe (to be wired to touch events)
void lvgl_gesture_event_handler(lv_event_t *e)
{
  // Track if a swipe or long press was detected in this touch sequence
  static bool swipe_detected = false;
  static bool long_press_active = false;
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  lv_event_code_t code = lv_event_get_code(e);
  lv_point_t point = {0, 0};
  lv_indev_t *indev = lv_indev_get_act();
  if (indev)
  {
    lv_indev_get_point(indev, &point);
    // printf("[lvgl_gesture_event_handler] Event code=%d, point=(%d, %d)\n", code, point.x, point.y);
  }

  if (code == LV_EVENT_PRESSED)
  {
    swipe_detected = false;
    long_press_active = false;
  }
  else if (code == LV_EVENT_GESTURE)
  {
    lv_dir_t dir = lv_indev_get_gesture_dir(indev);
    if (dir == LV_DIR_TOP)
    {
      // Generic swipe up
      trigger_gesture(GestureType::SwipeUp);
      swipe_detected = true;
    }
    else if (dir == LV_DIR_BOTTOM)
    {
      // Generic swipe down
      trigger_gesture(GestureType::SwipeDown);
      swipe_detected = true;
    }
  }
  else if (code == LV_EVENT_CLICKED)
  {
    if (long_press_active)
    {
      // Ignore click after long press
      long_press_active = false;
      return;
    }
    if (!swipe_detected)
    {
      if (point.y < SCREEN_HEIGHT / 2)
      {
        if (point.x < SCREEN_WIDTH / 2)
        {
          trigger_gesture(GestureType::TapTopLeft);
        }
        else
        {
          trigger_gesture(GestureType::TapTopRight);
        }
        trigger_gesture(GestureType::TapTop);
      }
      else
      {
        if (point.x < SCREEN_WIDTH / 2)
        {
          trigger_gesture(GestureType::TapBottomLeft);
        }
        else
        {
          trigger_gesture(GestureType::TapBottomRight);
        }
        trigger_gesture(GestureType::TapBottom);
      }
    }
  }
  else if (code == LV_EVENT_LONG_PRESSED)
  {
    long_press_active = true;
    // Determine if long press is in top or bottom half
    if (point.y < SCREEN_HEIGHT / 2)
    {
      if (point.x < SCREEN_WIDTH / 2)
      {
        trigger_gesture(GestureType::LongPressTopLeft);
      }
      else
      {
        trigger_gesture(GestureType::LongPressTopRight);
      }
      trigger_gesture(GestureType::LongPressTop);
    }
    else
    {
      if (point.x < SCREEN_WIDTH / 2)
      {
        trigger_gesture(GestureType::LongPressBottomLeft);
      }
      else
      {
        trigger_gesture(GestureType::LongPressBottomRight);
      }
      trigger_gesture(GestureType::LongPressBottom);
    }
  }
  else if (code == LV_EVENT_LONG_PRESSED_REPEAT)
  {
    // Continue triggering long press actions while held
    if (point.y < SCREEN_HEIGHT / 2)
    {
      if (point.x < SCREEN_WIDTH / 2)
      {
        trigger_gesture(GestureType::LongPressTopLeft);
      }
      else
      {
        trigger_gesture(GestureType::LongPressTopRight);
      }
      trigger_gesture(GestureType::LongPressTop);
    }
    else
    {
      if (point.x < SCREEN_WIDTH / 2)
      {
        trigger_gesture(GestureType::LongPressBottomLeft);
      }
      else
      {
        trigger_gesture(GestureType::LongPressBottomRight);
      }
      trigger_gesture(GestureType::LongPressBottom);
    }
  }
}

// Example: contextual menu quadrant selection (to be implemented)
void handle_menu_quadrant(int x, int y)
{
  // Assume screen divided into 4 quadrants
  if (x < LV_HOR_RES / 2 && y < LV_VER_RES / 2)
  {
    trigger_gesture(GestureType::MenuTL); // Top-left
  }
  else if (x >= LV_HOR_RES / 2 && y < LV_VER_RES / 2)
  {
    trigger_gesture(GestureType::MenuTR); // Top-right
  }
  else if (x < LV_HOR_RES / 2 && y >= LV_VER_RES / 2)
  {
    trigger_gesture(GestureType::MenuBL); // Bottom-left
  }
  else
  {
    trigger_gesture(GestureType::MenuBR); // Bottom-right
  }
}

// Initialization function to attach event handler to LVGL objects
void init_gesture_handling(lv_obj_t *root_obj)
{
  // Only listen to specific events we care about instead of LV_EVENT_ALL for better performance
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_LONG_PRESSED, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_LONG_PRESSED_REPEAT, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_GESTURE, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_RELEASED, NULL);
  lv_indev_set_long_press_time(lv_indev_get_act(), 500);
  lv_indev_set_long_press_repeat_time(lv_indev_get_act(), 400); // Repeat every 400ms while held (~2.5x per second)
}

// Clear all registered gesture callbacks
void clear_gesture_callbacks()
{
  gesture_callbacks.clear();
}
// Usage example (to be called from your app):
// register_gesture_callback(GestureType::TapTop, [](){ /* increment counter */ });
// register_gesture_callback(GestureType::SwipeDown, [](){ /* decrement by 5 */ });
// ...
// init_gesture_handling(lv_scr_act());
