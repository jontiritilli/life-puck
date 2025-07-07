
#include "gestures.h"
#include "life_counter.h"
#include <lvgl.h>

// --- Tap Gesture Regions ---
// Top 33% of screen: +1 life
// Bottom 33% of screen: -1 life
// Center: ignored (reserved for future features)
#define TAP_REGION_TOP    0.33
#define TAP_REGION_BOTTOM 0.67

// Handles tap events on the screen
static void tap_event_cb(lv_event_t* e) {
    lv_point_t p;
    lv_indev_t* indev = lv_indev_get_act();
    if (!indev) return;
    lv_indev_get_point(indev, &p);

    int screen_h = lv_obj_get_height(lv_scr_act());
    // Tap top region: increment life
    if (p.y < screen_h * TAP_REGION_TOP) {
        life_counter_increment(1); // Tap top: +1
    } 
    // Tap bottom region: decrement life
    else if (p.y > screen_h * TAP_REGION_BOTTOM) {
        life_counter_increment(-1); // Tap bottom: -1
    }
    // Center tap: ignored for now
}


// Handles swipe gesture events on the screen
// Up: +5, Down: -5
static void gesture_event_cb(lv_event_t* e) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_TOP) {
        life_counter_increment(5); // Swipe up: +5
    } else if (dir == LV_DIR_BOTTOM) {
        life_counter_increment(-5); // Swipe down: -5
    }
}


// Initializes gesture handling for tap and swipe on the main screen
void gestures_init() {
    // Attach tap event to the screen (for +1/-1)
    lv_obj_add_event_cb(lv_scr_act(), tap_event_cb, LV_EVENT_CLICKED, NULL);
    // Attach gesture event to the screen (for +5/-5)
    lv_obj_add_event_cb(lv_scr_act(), gesture_event_cb, LV_EVENT_GESTURE, NULL);
}


// Placeholder for future gesture polling or time-based logic
void gestures_update() {
    // No polling needed for LVGL event-based gestures
}
