#include <lvgl.h>

#pragma once

// Timer mode constants
#define KEY_TIMER_MODE "timer_mode"
#define KEY_ROUND_TIME "round_time"
#define DEFAULT_ROUND_TIME 300  // 5 minutes in seconds

enum TimerMode {
  TIMER_MODE_STOPWATCH = 0,
  TIMER_MODE_COUNTDOWN = 1
};

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

// Timer mode functions
TimerMode get_timer_mode();
void set_timer_mode(TimerMode mode);

// Round time functions
int get_round_time();
void set_round_time(int seconds);