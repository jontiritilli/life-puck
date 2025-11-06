// ============================================
// Own Header (first!)
// ============================================
#include "gestures.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>
#include <functional>
#include <map>
#include <stdio.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"

// ============================================
// Config
// ============================================
#include "config.h"

using GestureCallback = std::function<void()>;

static std::map<GestureType, GestureCallback> gesture_callbacks;

void register_gesture_callback(GestureType gesture, GestureCallback cb)
{
  gesture_callbacks[gesture] = cb;
}

void trigger_gesture(GestureType gesture)
{
  if (gesture_callbacks.count(gesture))
  {
    printf("[Gesture] Triggering: %d\n", (int)gesture);
    gesture_callbacks[gesture]();
  }
  else
  {
    printf("[Gesture] No callback for: %d\n", (int)gesture);
  }
}

void lvgl_gesture_event_handler(lv_event_t *e)
{
  static bool swipe_detected = false;
  static bool long_press_active = false;
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  lv_event_code_t code = lv_event_get_code(e);
  lv_point_t point = {0, 0};
  lv_indev_t *indev = lv_indev_get_act();
  if (indev)
  {
    lv_indev_get_point(indev, &point);
  }

  // Get player mode and gesture mode
  PlayerMode player_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
  bool is_two_player = (player_mode == PLAYER_MODE_TWO_PLAYER);

  // Get gesture control mode from NVS (defaults to config.h value)
  int gesture_mode = player_store.getInt(KEY_GESTURE_MODE, GESTURE_CONTROL_MODE);

  if (code == LV_EVENT_PRESSED)
  {
    swipe_detected = false;
    long_press_active = false;
  }
  else if (code == LV_EVENT_GESTURE)
  {
    lv_dir_t dir = lv_indev_get_gesture_dir(indev);

    if (gesture_mode == 0)
    {
      // CLASSIC MODE: Swipe up/down for large step adjustments
      if (dir == LV_DIR_BOTTOM)
      {
        if (is_two_player)
        {
          bool is_left = (point.x < SCREEN_WIDTH / 2);
          if (is_left)
          {
            trigger_gesture(GestureType::LongPressBottomLeft); // Big step -
          }
          else
          {
            trigger_gesture(GestureType::LongPressBottomRight);
          }
        }
        else
        {
          trigger_gesture(GestureType::LongPressBottom); // Big step -
        }
        swipe_detected = true;
      }
      else if (dir == LV_DIR_TOP)
      {
        if (is_two_player)
        {
          bool is_left = (point.x < SCREEN_WIDTH / 2);
          if (is_left)
          {
            trigger_gesture(GestureType::LongPressTopLeft); // Big step +
          }
          else
          {
            trigger_gesture(GestureType::LongPressTopRight);
          }
        }
        else
        {
          trigger_gesture(GestureType::LongPressTop); // Big step +
        }
        swipe_detected = true;
      }
    }
    else
    {
      // LONG PRESS MODE: Swipe down opens menu
      if (dir == LV_DIR_BOTTOM)
      {
        trigger_gesture(GestureType::SwipeDown);
        swipe_detected = true;
      }
      else if (dir == LV_DIR_TOP)
      {
        trigger_gesture(GestureType::SwipeUp);
        swipe_detected = true;
      }
    }
  }
  else if (code == LV_EVENT_LONG_PRESSED || code == LV_EVENT_LONG_PRESSED_REPEAT)
  {
    long_press_active = true;

    if (gesture_mode == 0)
    {
      // CLASSIC MODE: Long press center opens menu
      int center_x = SCREEN_WIDTH / 2;
      int center_y = SCREEN_HEIGHT / 2;
      int dx = point.x - center_x;
      int dy = point.y - center_y;
      float dist = sqrtf(dx * dx + dy * dy);

      if (dist < 80)
      {
        // Only trigger menu on first long press, not on repeats
        if (code == LV_EVENT_LONG_PRESSED)
        {
          trigger_gesture(GestureType::LongPressCenter);
        }
      }
    }
    else
    {
      // LONG PRESS MODE: Long press in zones for large step adjustments with repeat
      if (is_two_player)
      {
        bool is_left = (point.x < SCREEN_WIDTH / 2);
        bool is_top = (point.y < SCREEN_HEIGHT / 2);

        if (is_left && is_top)
        {
          trigger_gesture(GestureType::LongPressTopLeft); // Big step +
        }
        else if (is_left && !is_top)
        {
          trigger_gesture(GestureType::LongPressBottomLeft); // Big step -
        }
        else if (!is_left && is_top)
        {
          trigger_gesture(GestureType::LongPressTopRight); // Big step +
        }
        else
        {
          trigger_gesture(GestureType::LongPressBottomRight); // Big step -
        }
      }
      else
      {
        // 1-Player mode: Long press top/bottom
        if (point.y < SCREEN_HEIGHT / 2)
        {
          trigger_gesture(GestureType::LongPressTop); // Big step +
        }
        else
        {
          trigger_gesture(GestureType::LongPressBottom); // Big step -
        }
      }
    }
  }
  else if (code == LV_EVENT_CLICKED)
  {
    if (long_press_active)
    {
      long_press_active = false;
      return;
    }
    if (!swipe_detected)
    {
      if (is_two_player)
      {
        // 2-Spieler Modus: Links/Rechts unterscheiden
        bool is_left = (point.x < SCREEN_WIDTH / 2);
        bool is_top = (point.y < SCREEN_HEIGHT / 2);

        if (is_left && is_top)
        {
          trigger_gesture(GestureType::TapTopLeft); // Small +
        }
        else if (is_left && !is_top)
        {
          trigger_gesture(GestureType::TapBottomLeft); // Small -
        }
        else if (!is_left && is_top)
        {
          trigger_gesture(GestureType::TapTopRight); // Small +
        }
        else
        {
          trigger_gesture(GestureType::TapBottomRight); // Small -
        }
      }
      else
      {
        // 1-Spieler Modus
        if (point.y < SCREEN_HEIGHT / 2)
        {
          trigger_gesture(GestureType::TapTop); // Small +
        }
        else
        {
          trigger_gesture(GestureType::TapBottom); // Small -
        }
      }
    }
  }
}

void handle_menu_quadrant(int x, int y)
{
  if (x < LV_HOR_RES / 2 && y < LV_VER_RES / 2)
  {
    trigger_gesture(GestureType::MenuTL);
  }
  else if (x >= LV_HOR_RES / 2 && y < LV_VER_RES / 2)
  {
    trigger_gesture(GestureType::MenuTR);
  }
  else if (x < LV_HOR_RES / 2 && y >= LV_VER_RES / 2)
  {
    trigger_gesture(GestureType::MenuBL);
  }
  else
  {
    trigger_gesture(GestureType::MenuBR);
  }
}

void init_gesture_handling(lv_obj_t *root_obj)
{
  // Only listen to specific events we care about instead of LV_EVENT_ALL for better performance
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_LONG_PRESSED, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_LONG_PRESSED_REPEAT, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_GESTURE, NULL);
  lv_obj_add_event_cb(root_obj, lvgl_gesture_event_handler, LV_EVENT_RELEASED, NULL);

  // Get the first (and typically only) input device for touch
  lv_indev_t *indev = lv_indev_get_next(NULL);
  if (indev != NULL)
  {
    lv_indev_set_long_press_time(indev, LONG_PRESS_TIME_MS);
    lv_indev_set_long_press_repeat_time(indev, LONG_PRESS_REPEAT_TIME_MS);
    printf("[Gestures] Long press time set to %dms, repeat time set to %dms\n", LONG_PRESS_TIME_MS, LONG_PRESS_REPEAT_TIME_MS);
  }
  else
  {
    printf("[Gestures] WARNING: Could not get input device reference\n");
  }
}

void clear_gesture_callbacks()
{
  gesture_callbacks.clear();
}