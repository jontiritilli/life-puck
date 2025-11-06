// ============================================
// Own Header (first!)
// ============================================
#include "gui_main.h"
// ============================================
// System & Framework Headers
// ============================================
#include <Arduino.h>
#include <lvgl.h>
#include <stdio.h>
#include <math.h>

// ============================================
// Core System
// ============================================
#include "core/main.h"
#include "core/state_manager.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/life/life_counter.h"
#include "ui/screens/life/life_counter_two_player.h"
#include "ui/screens/menu/menu.h"
#include "ui/screens/tools/timer.h"

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/animation_helpers.h"
#include "ui/helpers/gestures.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"

// ============================================
// Assets
// ============================================
#include "assets/images/logo.h"


void ui_init(void)
{
  teardown_life_counter_2P();
  teardown_life_counter();
  teardownAllMenus();

  printf("[lv_create_main_gui] Loading screen\n");
  lv_scr_load(lv_obj_create(NULL));
  init_gesture_handling(lv_scr_act());
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);
  
  // Create logo image (replaces "Life Puck" text) - show immediately
  printf("[GUI] Logo info: %dx%d, data_size=%d\n", logo.header.w, logo.header.h, logo.data_size);
  printf("[GUI] First few bytes: %02X %02X %02X %02X\n", 
         logo.data[0], logo.data[1], logo.data[2], logo.data[3]);
  
  // Create logo and store reference globally
  static lv_obj_t *logo_img = nullptr;
  logo_img = lv_image_create(lv_scr_act());
  lv_image_set_src(logo_img, &logo);
  lv_obj_align(logo_img, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_image_opa(logo_img, LV_OPA_COVER, 0);
  
  printf("[GUI] Logo created and should be visible at %lu ms\n", millis());
  
  // Force LVGL to render the logo immediately
  lv_refr_now(NULL);
  printf("[GUI] Forced logo render at %lu ms\n", millis());
  
  // Delay to let logo render before starting life counter
  delay(1500); // 1.5 seconds to show logo
  printf("[GUI] Starting life counter after delay at %lu ms\n", millis());
  
  PlayerMode player_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
  life_counter_mode = player_mode;
  if (player_mode == PLAYER_MODE_ONE_PLAYER) {
    printf("[GUI] Starting 1-player life counter at %lu ms\n", millis());
    init_life_counter();
  } else {
    printf("[GUI] Starting 2-player life counter at %lu ms\n", millis());
    init_life_counter_2P();
  }
  printf("[GUI] Life counter started at %lu ms\n", millis());
}