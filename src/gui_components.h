#ifndef GUI_COMPONENTS_H
#define GUI_COMPONENTS_H

#include <lvgl.h>
#define SCREEN_WIDTH 360
#define SCREEN_HEIGHT 360

#define ARC_OUTER_DIAMETER 360 // 1px margin all around for 360x360 screen
#define ARC_INNER_DIAMETER 340
#define ARC_WIDTH (ARC_OUTER_DIAMETER - ARC_INNER_DIAMETER)
#define LIFE_STD_START 40

#define GREEN_COLOR lv_color_hex(0x00e31f)  // RGB for green
#define YELLOW_COLOR lv_color_hex(0xebf700) // RGB for yellow
#define RED_COLOR lv_color_hex(0xe80000)    // RGB for red

// Function to create the main GUI
void ui_init(void);
void show_life_counter(void);
void update_life_label(int value);
void init_touch(void);

#endif // GUI_COMPONENTS_H
