#include "Arduino.h"
#include <lvgl.h>
#include <stdio.h>
#include <math.h>
#include <esp_display_panel.hpp>
#include "life/life_counter.h"
#include "life/life_counter2P.h"
#include "menu/menu.h"
#include "gui_main.h"
#include "main.h"
#include "touch/touch.h"
#include "helpers/animation_helpers.h"
#include "gestures/gestures.h"
#include "constants/constants.h"
#include "state/state_store.h"
#include "timer/timer.h"
#include "images/logo.h"

void ui_init(void)
{
  teardown_life_counter_2P();
  teardown_life_counter();
  teardownAllMenus();

  // Now load the screen (all objects are hidden/transparent)
  printf("[lv_create_main_gui] Loading screen\n");
  lv_scr_load(lv_obj_create(NULL));
  init_gesture_handling(lv_scr_act());
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);
  // Disable scrollbars on screen
  lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

  // Create logo image (360x358 native resolution)
  lv_obj_t *logo_img = lv_img_create(lv_scr_act());
  lv_img_set_src(logo_img, &logo);
  lv_obj_align(logo_img, LV_ALIGN_CENTER, 0, 0);

  // Start fade-in for logo image (fade_in_obj handles initial transparency)
  fade_in_obj(logo_img, 500, 500, [](lv_anim_t *a)
              {
    // Fade out title label, then show life counter
    if (a && a->var) {
      fade_out_obj((lv_obj_t *)a->var, 500, 1500, [](lv_anim_t *anim) {
        if (anim && anim->var) {
          lv_obj_add_flag((lv_obj_t *)anim->var, LV_OBJ_FLAG_HIDDEN);
        }
        PlayerMode player_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
        // Now show the life counter (arc, label, animation)
        life_counter_mode = player_mode;
        if (player_mode == PLAYER_MODE_ONE_PLAYER) {
          init_life_counter();
        } else {
          init_life_counter_2P();
        }
      });
    } });
}
