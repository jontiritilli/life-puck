#include <lvgl.h>
#pragma once

// Expose the timer container for positioning in other modules
extern lv_obj_t *timer_container;

// Renders the timer label and sets up the timer logic
void render_timer(lv_obj_t *parent);

// Resets and stops the timer
void reset_timer();

// Cleans up the timer overlay
void teardown_timer();

// Toggles the visibility of the timer
uint64_t toggle_show_timer();

// Returns the current elapsed seconds
int get_elapsed_seconds();

// Returns whether the timer is currently running
bool get_is_timer_running();

// Toggles the running state of the timer (start/pause)
bool toggle_timer_running();