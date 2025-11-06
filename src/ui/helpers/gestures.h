/**
 * @file gestures.h
 * @brief Gesture type definitions and registration for LifePuck
 *
 * Provides a comprehensive gesture recognition system that maps
 * screen areas to specific touch gestures and callbacks.
 */

#pragma once
#include <functional>
#include <lvgl.h>

/**
 * @brief Enumeration of all supported gesture types
 *
 * Defines various tap, swipe, and long press gestures
 * mapped to different screen regions for intuitive control.
 */
enum class GestureType
{
  TapTop,               ///< Single tap on top half of screen
  TapBottom,            ///< Single tap on bottom half of screen
  TapTopLeft,           ///< Tap on top-left quadrant
  TapTopRight,          ///< Tap on top-right quadrant
  TapBottomLeft,        ///< Tap on bottom-left quadrant
  TapBottomRight,       ///< Tap on bottom-right quadrant
  SwipeUp,              ///< Upward swipe gesture
  SwipeDown,            ///< Downward swipe gesture
  LongPressTop,         ///< Long press on top half
  LongPressBottom,      ///< Long press on bottom half
  LongPressTopLeft,     ///< Long press on top-left quadrant
  LongPressBottomLeft,  ///< Long press on bottom-left quadrant
  LongPressTopRight,    ///< Long press on top-right quadrant
  LongPressBottomRight, ///< Long press on bottom-right quadrant
  LongPressCenter,      ///< Long press on center area
  MenuTL,               ///< Menu access from top-left
  MenuTR,               ///< Menu access from top-right
  MenuBL,               ///< Menu access from bottom-left
  MenuBR                ///< Menu access from bottom-right
};

/// Callback function type for gesture events
using GestureCallback = std::function<void()>;

/**
 * @brief Register a callback for a specific gesture type
 * @param gesture The gesture type to register
 * @param cb Callback function to execute when gesture is detected
 */
void register_gesture_callback(GestureType gesture, GestureCallback cb);

/**
 * @brief Initialize gesture handling for a screen
 * @param screen LVGL screen object to attach gesture detection
 */
void init_gesture_handling(lv_obj_t *screen);

/**
 * @brief LVGL event handler for processing gesture events
 * @param e LVGL event structure containing gesture data
 */
void lvgl_gesture_event_handler(lv_event_t *e);

/**
 * @brief Clear all registered gesture callbacks
 */
void clear_gesture_callbacks();