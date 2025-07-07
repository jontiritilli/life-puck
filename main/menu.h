#ifndef MENU_H
#define MENU_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and show the quadrant menu overlay.
 *
 * @param parent The parent LVGL object (usually the main screen or a container).
 * @param on_action Callback for when a menu action is selected (can be NULL).
 *                  Signature: void on_action(int action_id)
 * @return lv_obj_t* The menu container object.
 */
lv_obj_t* menu_create(lv_obj_t* parent, void (*on_action)(int));

// Action IDs for the 4 quadrants
#define MENU_ACTION_SETTINGS 0
#define MENU_ACTION_1P2P     1
#define MENU_ACTION_RESET    2
#define MENU_ACTION_HISTORY  3

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MENU_H
