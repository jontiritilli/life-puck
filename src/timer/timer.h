#include <lvgl.h>

// Expose the timer container for positioning in other modules
extern lv_obj_t *timer_container;
#pragma once

// Renders the timer label and sets up the timer logic
void render_timer(lv_obj_t *parent);

// Resets and stops the timer
void reset_timer();
