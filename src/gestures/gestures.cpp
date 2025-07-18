// gestures.cpp
// LVGL gesture handling for LifePuck application
// Ready to be wired to CST816S touch controller

#include <lvgl.h>
#include <functional>
#include <map>
#include <stdio.h>

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
  // Track if a swipe was detected in this touch sequence
  static bool swipe_detected = false;
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  lv_event_code_t code = lv_event_get_code(e);
  lv_point_t point = {0, 0};
  lv_indev_t *indev = lv_indev_get_act();
  if (indev)
  {
    lv_indev_get_point(indev, &point);
    printf("[lvgl_gesture_event_handler] Event code=%d, point=(%d, %d)\n", code, point.x, point.y);
  }

  if (code == LV_EVENT_PRESSED)
  {
    swipe_detected = false;
  }
  else if (code == LV_EVENT_GESTURE)
  {
    lv_dir_t dir = lv_indev_get_gesture_dir(indev);
    if (dir == LV_DIR_TOP)
    {
      trigger_gesture(GestureType::SwipeUp);
      swipe_detected = true;
    }
    else if (dir == LV_DIR_BOTTOM)
    {
      trigger_gesture(GestureType::SwipeDown);
      swipe_detected = true;
    }
  }
  else if (code == LV_EVENT_CLICKED)
  {
    if (!swipe_detected)
    {
      if (point.y < LV_VER_RES / 2)
      {
        trigger_gesture(GestureType::TapTop);
      }
      else
      {
        trigger_gesture(GestureType::TapBottom);
      }
    }
  }
  else if (code == LV_EVENT_LONG_PRESSED)
  {
    trigger_gesture(GestureType::LongPressMenu);
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
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_ALL, NULL);
}

// Usage example (to be called from your app):
// register_gesture_callback(GestureType::TapTop, [](){ /* increment counter */ });
// register_gesture_callback(GestureType::SwipeDown, [](){ /* decrement by 5 */ });
// ...
// init_gesture_handling(lv_scr_act());
