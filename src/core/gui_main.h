#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include <lvgl.h>

/**
 * @brief Initialize the complete user interface
 * 
 * Sets up all LVGL screens, styles, and UI components.
 * Must be called after LVGL initialization.
 */
void ui_init(void);

/**
 * @brief Initialize touch input system
 * 
 * Configures touch driver and registers input device with LVGL.
 * Should be called during hardware initialization phase.
 */
void init_touch(void);

#endif // GUI_MAIN_H