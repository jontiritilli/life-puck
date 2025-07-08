
#pragma once
#include <lvgl.h>

void life_counter_init(int start_value = 20, lv_obj_t *parent = nullptr);
void life_counter_set(int value);
int life_counter_get();
void life_counter_increment(int delta);
void life_counter_update(); // Call in loop for UI refresh/animation
