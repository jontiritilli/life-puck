#include <lvgl.h>
#include <stdio.h>
#include "constants/constants.h"

lv_obj_t *timer_container = nullptr; // Container for the timer label
static lv_obj_t *timer_label = nullptr;
static lv_timer_t *timer = nullptr;
int elapsed_seconds = 0;
static bool timer_running = false;

// Forward declarations
void reset_timer();

// Format and update the timer label
static void update_timer_label()
{
  if (!timer_label)
    return;
  int minutes = elapsed_seconds / 60;
  int seconds = elapsed_seconds % 60;
  char buf[8];
  snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);
  lv_label_set_text(timer_label, buf);
}

// Timer callback to increment time
static void timer_tick_cb(lv_timer_t *t)
{
  if (timer_running)
  {
    elapsed_seconds++;
    update_timer_label();
  }
}

// Click event callback to start/pause timer (stopwatch style)
static void timer_click_cb(lv_event_t *e)
{
  timer_running = !timer_running;
  if (timer_running)
  {
    lv_obj_set_style_text_color(timer_label, lv_color_white(), 0); // Change color to indicate running
  }
  else
  {
    lv_obj_set_style_text_color(timer_label, GRAY_COLOR, 0); // Change color to indicate paused
  }
}

// Render the timer on screen
void render_timer(lv_obj_t *parent)
{
  if (!timer_container)
  {
    // Create timer container as a child of the given parent
    timer_container = lv_obj_create(parent);
    lv_obj_set_size(timer_container, 140, 50);
    lv_obj_add_flag(timer_container, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_style_bg_color(timer_container, lv_color_black(), 0);
    lv_obj_set_style_border_opa(timer_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(timer_container, WHITE_COLOR, 0);
    lv_obj_set_style_border_width(timer_container, 2, 0);
    lv_obj_remove_flag(timer_container, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling
    lv_obj_add_event_cb(timer_container, timer_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(timer_container, [](lv_event_t *e)
                        {
        reset_timer();
        timer_running = false;
        lv_obj_set_style_text_color(timer_label, RED_COLOR, 0); }, LV_EVENT_LONG_PRESSED, NULL);
  }
  // Create timer label if not already created
  if (!timer_label)
  {
    timer_label = lv_label_create(timer_container);
    lv_obj_set_style_text_font(timer_label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(timer_label, GRAY_COLOR, 0);
    lv_obj_set_style_text_align(timer_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(timer_label, LV_ALIGN_BOTTOM_MID, 0, 8); // Align label nearly flush with bottom
    update_timer_label();
  }
  // Create LVGL timer if not already created
  if (!timer)
  {
    timer = lv_timer_create(timer_tick_cb, 1000, NULL); // 1 second interval
  }
}

// Optionally, add a function to reset or stop the timer
void reset_timer()
{
  elapsed_seconds = 0;
  timer_running = false;
  update_timer_label();
}

// Fully teardown the timer and its state
void teardown_timer()
{
  if (timer_container)
  {
    lv_obj_del(timer_container);
    timer_container = nullptr;
  }
  if (timer)
  {
    lv_timer_del(timer);
    timer = nullptr;
  }
  timer_label = nullptr;
  elapsed_seconds = 0;
  timer_running = false;
}