#pragma once
#include <lvgl.h>

void arc_animation_init(int min_value, int max_value, int start_value, lv_obj_t* parent = nullptr);
void arc_animation_set(int value);
void arc_animation_update();
void arc_animation_set_color(lv_color_t color);
void arc_animation_set_thickness(int thickness);
