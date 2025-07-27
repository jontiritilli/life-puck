#include <lvgl.h>
#include <stdio.h>
#include "constants/constants.h"

static lv_obj_t *timer_label = nullptr;
static lv_timer_t *timer = nullptr;
static int elapsed_seconds = 0;
static bool timer_running = false;

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

// Click event callback to start timer
static void timer_click_cb(lv_event_t *e)
{
  timer_running = !timer_running; // Toggle timer state
  if (timer_running)
  {
    lv_obj_set_style_text_color(timer_label, lv_color_white(), 0); // Change color to indicate running
  }
  else
  {
    lv_obj_set_style_text_color(timer_label, RED_COLOR, 0); // Change color to indicate paused
  }
}

// Render the timer on screen
void render_timer()
{
  // Create label if not already created
  if (!timer_label)
  {
    timer_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(timer_label, &lv_font_montserrat_32, 0);
    lv_obj_align(timer_label, LV_ALIGN_TOP_RIGHT, -20, 20); // Top right corner, adjust as needed
    lv_obj_add_event_cb(timer_label, timer_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(timer_label, [](lv_event_t *e)
                        {
                          reset_timer();                                            // Reset timer on long press
                          timer_running = false;                                    // Stop the timer
                          lv_obj_set_style_text_color(timer_label, GREEN_COLOR, 0); // Change color
                        },
                        LV_EVENT_LONG_PRESSED, NULL);
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
