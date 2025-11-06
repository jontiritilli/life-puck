// ============================================
// Own Header (first!)
// ============================================
#include "timer.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>
#include <stdio.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "hardware/audio/simple_audio.h"

lv_obj_t *timer_container = nullptr;
static lv_obj_t *timer_label = nullptr;
static lv_timer_t *timer = nullptr;
static int elapsed_seconds = 0;
static bool timer_running = false;
static TimerMode current_timer_mode = TIMER_MODE_STOPWATCH;
static int round_time_seconds = DEFAULT_ROUND_TIME;

static void load_timer_settings()
{
  current_timer_mode = (TimerMode)player_store.getInt(KEY_TIMER_MODE, TIMER_MODE_STOPWATCH);
  round_time_seconds = player_store.getInt(KEY_ROUND_TIME, DEFAULT_ROUND_TIME);
  printf("[Timer] Loaded settings: mode=%d, round_time=%d\n", current_timer_mode, round_time_seconds);
}

static void update_timer_label()
{
  if (!timer_label)
    return;

  int display_seconds = elapsed_seconds;

  if (current_timer_mode == TIMER_MODE_COUNTDOWN)
  {
    display_seconds = round_time_seconds - elapsed_seconds;
    if (display_seconds < 0)
      display_seconds = 0;
  }

  int minutes = display_seconds / 60;
  int seconds = display_seconds % 60;
  char buf[8];
  snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);
  lv_label_set_text(timer_label, buf);

  if (timer_running)
  {
    lv_obj_set_style_text_color(timer_label, lv_color_white(), 0);
  }
  else
  {
    lv_obj_set_style_text_color(timer_label, GRAY_COLOR, 0);
  }

  if (current_timer_mode == TIMER_MODE_COUNTDOWN && elapsed_seconds >= round_time_seconds)
  {
    timer_running = false;
    lv_obj_set_style_text_color(timer_label, lv_color_hex(0xFF0000), 0);
  }
}

static void timer_tick_cb(lv_timer_t *t)
{
  if (timer_running)
  {
    elapsed_seconds++;

    if (current_timer_mode == TIMER_MODE_COUNTDOWN && elapsed_seconds >= round_time_seconds)
    {
      timer_running = false;

      // *** AUDIO NOTIFICATION: Timer reached zero! ***
      printf("[Timer] Countdown finished - playing notification sound\n");
      // Play selected timer finish sound
      simple_audio_play_sound(simple_audio_get_timer_sound());
    }

    update_timer_label();
  }
}

static void timer_click_cb(lv_event_t *e)
{
  timer_running = !timer_running;

  if (timer_running)
  {
    lv_obj_set_style_text_color(timer_label, lv_color_white(), 0);
  }
  else
  {
    lv_obj_set_style_text_color(timer_label, GRAY_COLOR, 0);
  }
  update_timer_label();
}

static void timer_long_press_cb(lv_event_t *e)
{
  reset_timer();

  // Kurzes visuelles Feedback beim Reset
  lv_obj_set_style_text_color(timer_label, lv_color_hex(0xFF0000), 0);

  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_time(&anim, 300);
  lv_anim_set_values(&anim, 255, 128);
  lv_anim_set_ready_cb(&anim, [](lv_anim_t *a)
                       {
    if (timer_label) {
      lv_obj_set_style_text_color(timer_label, GRAY_COLOR, 0);
    } });
  lv_anim_start(&anim);
}

void render_timer(lv_obj_t *parent)
{
  if (timer_container)
  {
    teardown_timer();
  }

  load_timer_settings();

  timer_container = lv_obj_create(parent);
  lv_obj_set_size(timer_container, 130, 45);
  lv_obj_set_style_bg_opa(timer_container, LV_OPA_TRANSP, 0);

  // White border
  lv_obj_set_style_border_width(timer_container, 2, 0);
  lv_obj_set_style_border_color(timer_container, lv_color_white(), 0);
  lv_obj_set_style_radius(timer_container, 10, 0);

  lv_obj_clear_flag(timer_container, LV_OBJ_FLAG_SCROLLABLE);

  timer_label = lv_label_create(timer_container);
  lv_obj_set_style_text_font(timer_label, &lv_font_montserrat_32, 0);
  lv_obj_center(timer_label);

  update_timer_label();

  lv_obj_add_event_cb(timer_container, timer_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(timer_container, timer_long_press_cb, LV_EVENT_LONG_PRESSED, NULL);

  if (!timer)
  {
    timer = lv_timer_create(timer_tick_cb, 1000, NULL);
  }
}

void reset_timer()
{
  elapsed_seconds = 0;
  timer_running = false;
  update_timer_label();
}

void teardown_timer()
{
  if (timer)
  {
    lv_timer_del(timer);
    timer = nullptr;
  }

  if (timer_container)
  {
    lv_obj_del(timer_container);
    timer_container = nullptr;
    timer_label = nullptr;
  }

  elapsed_seconds = 0;
  timer_running = false;
}

uint64_t toggle_show_timer()
{
  uint64_t current_state = player_store.getInt(KEY_SHOW_TIMER, 0);
  uint64_t new_state = !current_state;
  player_store.putInt(KEY_SHOW_TIMER, new_state);
  return new_state;
}

int get_elapsed_seconds()
{
  return elapsed_seconds;
}

bool get_is_timer_running()
{
  return timer_running;
}

bool toggle_timer_running()
{
  timer_running = !timer_running;
  update_timer_label();
  return timer_running;
}

TimerMode get_timer_mode()
{
  return current_timer_mode;
}

void set_timer_mode(TimerMode mode)
{
  current_timer_mode = mode;
  player_store.putInt(KEY_TIMER_MODE, (int)mode);
  reset_timer();
  printf("[Timer] Mode changed to: %d\n", mode);
}

int get_round_time()
{
  return round_time_seconds;
}

void set_round_time(int seconds)
{
  if (seconds > 0 && seconds <= 59940)
  {
    round_time_seconds = seconds;
    player_store.putInt(KEY_ROUND_TIME, seconds);
    reset_timer();
    printf("[Timer] Round time set to: %d seconds\n", seconds);
  }
}