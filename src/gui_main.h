#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include <lvgl.h>

// Global input device reference
extern lv_indev_t *global_indev;

// Function to create the main GUI
void ui_init(lv_indev_t *indev);
lv_indev_t* init_touch(void);

#endif // GUI_MAIN_H
