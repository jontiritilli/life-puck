// ============================================
// Own Header (first!)
// ============================================
#include "menu.h"

// ============================================
// System & Framework Headers
// ============================================
#include <Arduino.h>
#include <lvgl.h>
#include <math.h>

// ============================================
// Core System
// ============================================
#include "core/gui_main.h"
#include "core/state_manager.h"

// ============================================
// Hardware
// ============================================
#include "hardware/system/battery_state.h"
#include "hardware/touch/Touch_CST816.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/life/life_counter.h"
#include "ui/screens/life/life_counter_two_player.h"
#include "ui/screens/life/simple_counters.h"
#include "ui/screens/settings/settings_overlay.h"
#include "ui/screens/settings/brightness.h"
#include "ui/screens/settings/touch_calibration.h"
#include "ui/screens/settings/audio_settings.h"
#include "ui/screens/settings/timer_settings.h"
#include "ui/screens/settings/power_settings.h"
#include "ui/screens/settings/theme_settings.h"
#include "ui/screens/settings/touch_settings.h"
#include "ui/screens/settings/counters_settings.h"
#include "ui/screens/tools/timer.h"
#include "ui/screens/tools/dice_coin.h"
#include "ui/screens/menu/preset_editor.h"

// ============================================
// UI Components
// ============================================
#include "ui/components/start_life.h"

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/animation_helpers.h"
#include "ui/helpers/tap_layer.h"
#include "ui/helpers/event_grouper.h"
#include "ui/helpers/ui_helpers.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "data/history.h"
#include "data/tcg_presets.h"
#include "data/themes.h"
// Audio and config for animations
#include "hardware/audio/simple_audio.h"
#include "config.h"
// Counter system
#include "ui/screens/life/simple_counters.h"

lv_obj_t *contextual_menu = nullptr;
lv_obj_t *settings_menu = nullptr;
lv_obj_t *life_config_menu = nullptr;
lv_obj_t *history_menu = nullptr;
lv_obj_t *brightness_control = nullptr;
lv_obj_t *dice_list_menu = nullptr;
lv_obj_t *preset_list_menu = nullptr;

static MenuState currentMenu = MENU_NONE;
static int current_menu_page = 0;
static const int TOTAL_MENU_PAGES = 2;

static void togglePlayerMode();
void resetActiveCounter();
static void showHistoryOverlay();
static void showLifeScreen();
void hideLifeScreen();
void teardownContextualMenuOverlay();
static bool is_in_center_cancel_area(lv_event_t *e);
void renderMenu(MenuState menuType);
void renderMenu(MenuState menuType, bool animate_menu);
bool is_in_quadrant(lv_event_t *e, int angle_start, int angle_end);
void renderDiceListMenu();
void renderPresetListMenu();
void teardownDiceListMenu();
void teardownPresetListMenu();
void show_tcg_result_popup(const char *title, const char *result);

int circle_diameter = SCREEN_WIDTH;
int circle_radius = circle_diameter / 2;

MenuState getCurrentMenu()
{
  return currentMenu;
}

void handleContextualSelection(ContextualQuadrant quadrant)
{
  if (current_menu_page == 0)
  {
    switch (quadrant)
    {
    case QUADRANT_TL:
      renderMenu(MENU_SETTINGS);
      break;
    case QUADRANT_TR:
      togglePlayerMode();
      break;
    case QUADRANT_BL:
      resetActiveCounter();
      break;
    case QUADRANT_BR:
      renderMenu(MENU_PRESET_LIST);
      break;
    }
  }
  else
  {
    switch (quadrant)
    {
    case QUADRANT_TL:
      renderMenu(MENU_DICE_LIST);
      break;
    case QUADRANT_TR:
    {
      bool heads = flip_coin();
      if (ENABLE_COIN_ANIMATION)
      {
        extern void animate_coin_flip(bool is_heads);
        animate_coin_flip(heads);
      }
      else
      {
        show_tcg_result_popup("Coin flip", heads ? "HEADS" : "TAILS");
      }
    }
    break;
    case QUADRANT_BL:
      renderMenu(MENU_HISTORY);
      break;
    case QUADRANT_BR:
      break;
    }
  }
}

static void togglePlayerMode()
{
  PlayerMode current_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
  if (current_mode != PLAYER_MODE_ONE_PLAYER && current_mode != PLAYER_MODE_TWO_PLAYER)
    current_mode = PLAYER_MODE_ONE_PLAYER;
  PlayerMode new_mode = (current_mode == PLAYER_MODE_ONE_PLAYER) ? PLAYER_MODE_TWO_PLAYER : PLAYER_MODE_ONE_PLAYER;
  player_store.putInt(KEY_PLAYER_MODE, (int)new_mode);

  // *** RESET EVERYTHING: Use centralized reset function ***
  // resetActiveCounter() resets life, counters, timer, and marks saved life as invalid
  resetActiveCounter();

  printf("[togglePlayerMode] Player mode toggled to %d, everything reset\n", new_mode);
  ui_init();
  renderMenu(MENU_NONE);
}

void resetActiveCounter()
{
  PlayerMode player_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
  if (player_mode == PLAYER_MODE_ONE_PLAYER)
  {
    reset_life();
  }
  else if (player_mode == PLAYER_MODE_TWO_PLAYER)
  {
    reset_life_2p();
  }
  printf("[resetActiveCounter] Reset life counter and history for player mode %d\n", player_mode);
  reset_timer();
  showLifeScreen();
  teardownAllMenus();
}

void show_tcg_result_popup(const char *title, const char *result)
{
  lv_obj_t *popup = lv_obj_create(lv_scr_act());
  lv_obj_set_size(popup, 180, 150);
  lv_obj_center(popup);
  lv_obj_set_style_bg_color(popup, lv_color_hex(0x202020), 0);
  lv_obj_set_style_border_width(popup, 3, 0);
  lv_obj_set_style_border_color(popup, getThemeButtonColor(), 0); // Use theme color
  lv_obj_set_style_radius(popup, 20, 0);

  lv_obj_t *lbl_title = lv_label_create(popup);
  lv_label_set_text(lbl_title, title);
  lv_obj_set_style_text_color(lbl_title, getThemeButtonColor(), 0); // Use theme color
  lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 15);

  lv_obj_t *lbl_result = lv_label_create(popup);
  lv_label_set_text(lbl_result, result);
  lv_obj_set_style_text_font(lbl_result, &lv_font_montserrat_40, 0);
  lv_obj_set_style_text_color(lbl_result, getThemeButtonColor(), 0); // Match theme color
  lv_obj_center(lbl_result);

  lv_timer_t *timer = lv_timer_create([](lv_timer_t *t)
                                      {
    lv_obj_t *obj = (lv_obj_t*)lv_timer_get_user_data(t);
    lv_obj_del(obj);
    lv_timer_del(t); }, GENERIC_RESULT_HOLD_MS, nullptr);
  lv_timer_set_user_data(timer, popup);
  lv_timer_set_repeat_count(timer, 1);
}

// ================================
// Slot-machine style animations
// ================================

static lv_obj_t *anim_popup = nullptr;
static lv_obj_t *anim_lbl_title = nullptr;
static lv_obj_t *anim_lbl_result = nullptr;

static void ensure_anim_popup(const char *title, const char *text)
{
  if (anim_popup == nullptr)
  {
    anim_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(anim_popup, 180, 150);
    lv_obj_center(anim_popup);
    lv_obj_set_style_bg_color(anim_popup, lv_color_hex(0x202020), 0);
    lv_obj_set_style_border_width(anim_popup, 3, 0);
    lv_obj_set_style_border_color(anim_popup, getThemeButtonColor(), 0);
    lv_obj_set_style_radius(anim_popup, 20, 0);

    anim_lbl_title = lv_label_create(anim_popup);
    lv_obj_set_style_text_color(anim_lbl_title, getThemeButtonColor(), 0);
    lv_obj_align(anim_lbl_title, LV_ALIGN_TOP_MID, 0, 15);

    anim_lbl_result = lv_label_create(anim_popup);
    lv_obj_set_style_text_font(anim_lbl_result, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(anim_lbl_result, getThemeButtonColor(), 0);
    lv_obj_center(anim_lbl_result);
  }

  if (title)
    lv_label_set_text(anim_lbl_title, title);
  if (text)
    lv_label_set_text(anim_lbl_result, text);
}

static void close_anim_popup_after(uint32_t delay_ms)
{
  if (!anim_popup)
    return;
  lv_timer_t *timer = lv_timer_create([](lv_timer_t *t)
                                      {
    if (anim_popup) {
      lv_obj_del(anim_popup);
    }
    anim_popup = nullptr;
    anim_lbl_title = nullptr;
    anim_lbl_result = nullptr;
    lv_timer_del(t); }, delay_ms, nullptr);
  lv_timer_set_repeat_count(timer, 1);
}

static inline void update_popup_text(const char *text)
{
  if (anim_lbl_result)
    lv_label_set_text(anim_lbl_result, text);
}

static void animate_popup_bounce()
{
  if (!anim_popup || !ANIMATION_SCALE_BOUNCE)
    return;
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, anim_popup);
  lv_anim_set_time(&a, 180);
  lv_anim_set_values(&a, 256, 296); // 100% -> ~116%
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_zoom);
  lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
  lv_anim_start(&a);
}

static inline void play_tick_sound()
{
  if (ANIMATION_SOUND_TICKS && simple_audio_is_enabled())
  {
    simple_audio_beep(2000, 30);
  }
}

static inline void play_result_sound()
{
  if (ANIMATION_SOUND_TICKS && simple_audio_is_enabled())
  {
    simple_audio_play_sound(SOUND_SUCCESS);
  }
}

// Dice animation state
static lv_timer_t *dice_animation_timer = nullptr;
static int dice_animation_cycle = 0;
static int dice_final_result = 0;
static int dice_max_sides = 6;
static uint32_t dice_cycle_delay = 0;

static void dice_animation_tick(lv_timer_t *timer)
{
  dice_animation_cycle++;

  if (dice_animation_cycle >= DICE_CYCLES)
  {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", dice_final_result);
    update_popup_text(buf);
    animate_popup_bounce();
    play_result_sound();
    lv_timer_del(dice_animation_timer);
    dice_animation_timer = nullptr;
    close_anim_popup_after(DICE_RESULT_HOLD_MS);
    return;
  }

  int display_num;
  if (dice_animation_cycle >= DICE_CYCLES - 2)
  {
    display_num = dice_final_result;
  }
  else
  {
    display_num = (esp_random() % dice_max_sides) + 1;
  }
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", display_num);
  update_popup_text(buf);
  play_tick_sound();

  dice_cycle_delay += ANIMATION_SLOWDOWN_MS;
  lv_timer_set_period(timer, dice_cycle_delay);
}

static void animate_dice_roll(int final_result, int max_sides)
{
  dice_final_result = final_result;
  dice_max_sides = max_sides;
  dice_animation_cycle = 0;
  dice_cycle_delay = 30;

  int first = (esp_random() % max_sides) + 1;
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", first);
  ensure_anim_popup("Rolling...", buf);

  if (dice_animation_timer)
  {
    lv_timer_del(dice_animation_timer);
    dice_animation_timer = nullptr;
  }
  dice_animation_timer = lv_timer_create(dice_animation_tick, dice_cycle_delay, nullptr);
}

// Coin animation state
static lv_timer_t *coin_animation_timer = nullptr;
static int coin_animation_cycle = 0;
static bool coin_final_result = false; // true=heads
static uint32_t coin_cycle_delay = 0;

static void coin_animation_tick(lv_timer_t *timer)
{
  coin_animation_cycle++;

  if (coin_animation_cycle >= COIN_CYCLES)
  {
    const char *txt = coin_final_result ? "HEADS" : "TAILS";
    update_popup_text(txt);
    animate_popup_bounce();
    play_result_sound();
    lv_timer_del(coin_animation_timer);
    coin_animation_timer = nullptr;
    close_anim_popup_after(COIN_RESULT_HOLD_MS);
    return;
  }

  const char *display_txt;
  if (coin_animation_cycle >= COIN_CYCLES - 2)
  {
    display_txt = coin_final_result ? "HEADS" : "TAILS";
  }
  else
  {
    display_txt = (coin_animation_cycle % 2 == 0) ? "HEADS" : "TAILS";
  }
  update_popup_text(display_txt);
  play_tick_sound();

  coin_cycle_delay += ANIMATION_SLOWDOWN_MS;
  lv_timer_set_period(timer, coin_cycle_delay);
}

void animate_coin_flip(bool is_heads)
{
  coin_final_result = is_heads;
  coin_animation_cycle = 0;
  coin_cycle_delay = 40;

  const char *first = (esp_random() % 2) == 0 ? "HEADS" : "TAILS";
  ensure_anim_popup("Flipping...", first);

  if (coin_animation_timer)
  {
    lv_timer_del(coin_animation_timer);
    coin_animation_timer = nullptr;
  }
  coin_animation_timer = lv_timer_create(coin_animation_tick, coin_cycle_delay, nullptr);
}

void renderDiceListMenu()
{
  teardownDiceListMenu();
  hideLifeScreen();

  dice_list_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(dice_list_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(dice_list_menu);
  lv_obj_set_style_bg_color(dice_list_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(dice_list_menu, 15, 0);
  lv_obj_set_style_border_width(dice_list_menu, 0, 0);
  lv_obj_set_scrollbar_mode(dice_list_menu, LV_SCROLLBAR_MODE_AUTO);

  // Grid Layout: 1 column, 10 rows (title + back + 7 dice + spacer)
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 50, 40, 40, 40, 40, 40, 40, 40, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(dice_list_menu, col_dsc, row_dsc);
  lv_obj_set_layout(dice_list_menu, LV_LAYOUT_GRID);

  // SWIPE TO CLOSE (horizontal right swipe like in Settings)
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled)
  {
    extern CST816_Touch touch_data; // Access RAW touch data for accurate swipe detection

    lv_obj_add_event_cb(dice_list_menu, [](lv_event_t *e)
                        {
      extern CST816_Touch touch_data; // Re-declare inside lambda
      static lv_point_t start_point;
      static bool is_swiping = false;
      
      lv_event_code_t code = lv_event_get_code(e);
      
      if (code == LV_EVENT_PRESSED) {
        // Use RAW coordinates for swipe detection
        start_point.x = touch_data.x;
        start_point.y = touch_data.y;
        is_swiping = false;
        printf("[Dice] PRESSED at RAW (%d, %d)\n", start_point.x, start_point.y);
      }
      else if (code == LV_EVENT_PRESSING) {
        // Use RAW coordinates for swipe detection
        lv_point_t point;
        point.x = touch_data.x;
        point.y = touch_data.y;
        int dx = point.x - start_point.x;
        int dy = point.y - start_point.y;
        
        // Detect horizontal swipe
        if (abs(dx) > 30 && abs(dx) > abs(dy) * 1.5) {
          is_swiping = true;
          printf("[Dice] SWIPING detected! RAW dx=%d dy=%d\n", dx, dy);
        }
      }
      else if (code == LV_EVENT_RELEASED) {
        printf("[Dice] RELEASED, is_swiping=%d\n", is_swiping);
        if (is_swiping) {
          // Use RAW coordinates for final check
          lv_point_t point;
          point.x = touch_data.x;
          point.y = touch_data.y;
          int dx = point.x - start_point.x;
          int dy = point.y - start_point.y;
          
          printf("[Dice] Swipe check RAW: dx=%d dy=%d (need dx>80 and dx>dy)\n", dx, dy);
          
          // Swipe right threshold - relaxed for round display
          if (dx > 80 && abs(dx) > abs(dy)) {
            printf("[Dice] SWIPE CONFIRMED! Closing menu.\n");
            current_menu_page = 1;
            renderMenu(MENU_CONTEXTUAL, false);
            return;
          }
        }
        is_swiping = false;
      } }, LV_EVENT_ALL, NULL);
  }

  // Title (Row 0)
  ui_create_title(dice_list_menu, "DICE", 0);

  // Back button (Row 1)
  ui_create_back_button(dice_list_menu, 1, 0, [](lv_event_t *e)
                        {
    current_menu_page = 1;
    renderMenu(MENU_CONTEXTUAL, false); });

  // Dice buttons (Rows 2-8)
  for (int i = 0; i < DICE_TYPE_COUNT; i++)
  {
    lv_obj_t *btn = lv_btn_create(dice_list_menu);
    lv_obj_set_size(btn, 160, 40);
    lv_obj_set_style_bg_color(btn, getThemeButtonColor(), 0); // Use theme color
    lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, i + 2, 1);
    lv_obj_set_user_data(btn, (void *)(intptr_t)i);
    lv_obj_add_event_cb(btn, [](lv_event_t *e)
                        {
      lv_obj_t *target = (lv_obj_t*)lv_event_get_target(e);
      int idx = (int)(intptr_t)lv_obj_get_user_data(target);
      int result = DICE_TYPES[idx].roll_func();
      char buf[16];
      snprintf(buf, sizeof(buf), "%d", result);
      if (ENABLE_DICE_ANIMATION) {
        int sides = 6;
        switch (idx) {
          case 0: sides = 4; break;  // D4
          case 1: sides = 6; break;  // D6
          case 2: sides = 8; break;  // D8
          case 3: sides = 10; break; // D10
          case 4: sides = 12; break; // D12
          case 5: sides = 20; break; // D20
          case 6: sides = 100; break; // D100
        }
        ensure_anim_popup(DICE_TYPES[idx].name, buf);
        animate_dice_roll(result, sides);
      } else {
        show_tcg_result_popup(DICE_TYPES[idx].name, buf);
      } }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, DICE_TYPES[i].name);
    lv_obj_set_style_text_color(lbl, getThemeTextColor(), 0); // Theme text color
    lv_obj_center(lbl);
  }

  // Extra spacer for scrolling (Row 9)
  lv_obj_t *extra_spacer = lv_obj_create(dice_list_menu);
  lv_obj_set_size(extra_spacer, 10, 50);
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 9, 1);
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);

  currentMenu = MENU_DICE_LIST;
}

void renderPresetListMenu()
{
  teardownPresetListMenu();
  hideLifeScreen();

  preset_list_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(preset_list_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(preset_list_menu);
  lv_obj_set_style_bg_color(preset_list_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(preset_list_menu, 15, 0);
  lv_obj_set_style_border_width(preset_list_menu, 0, 0);
  lv_obj_set_scrollbar_mode(preset_list_menu, LV_SCROLLBAR_MODE_AUTO);

  // Grid Layout: 1 column, 13 rows (title + back + 10 presets + spacer)
  static lv_coord_t col_dsc_preset[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc_preset[] = {40, 50, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(preset_list_menu, col_dsc_preset, row_dsc_preset);
  lv_obj_set_layout(preset_list_menu, LV_LAYOUT_GRID);

  // SWIPE TO CLOSE (horizontal right swipe like in Settings)
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled)
  {
    extern CST816_Touch touch_data; // Access RAW touch data for accurate swipe detection

    lv_obj_add_event_cb(preset_list_menu, [](lv_event_t *e)
                        {
      extern CST816_Touch touch_data; // Re-declare inside lambda
      static lv_point_t start_point;
      static bool is_swiping = false;
      
      lv_event_code_t code = lv_event_get_code(e);
      
      if (code == LV_EVENT_PRESSED) {
        // Use RAW coordinates for swipe detection
        start_point.x = touch_data.x;
        start_point.y = touch_data.y;
        is_swiping = false;
      }
      else if (code == LV_EVENT_PRESSING) {
        // Use RAW coordinates for swipe detection
        lv_point_t point;
        point.x = touch_data.x;
        point.y = touch_data.y;
        int dx = point.x - start_point.x;
        int dy = point.y - start_point.y;
        
        // Detect horizontal swipe
        if (abs(dx) > 30 && abs(dx) > abs(dy) * 1.5) {
          is_swiping = true;
        }
      }
      else if (code == LV_EVENT_RELEASED) {
        if (is_swiping) {
          // Use RAW coordinates for final check
          lv_point_t point;
          point.x = touch_data.x;
          point.y = touch_data.y;
          int dx = point.x - start_point.x;
          int dy = point.y - start_point.y;
          
          // Swipe right threshold - relaxed for round display
          if (dx > 80 && abs(dx) > abs(dy)) {
            current_menu_page = 0;
            renderMenu(MENU_CONTEXTUAL, false);
            return;
          }
        }
        is_swiping = false;
      } }, LV_EVENT_ALL, NULL);
  }

  // Title (Row 0)
  ui_create_title(preset_list_menu, "PRESETS", 0);

  // Back button (Row 1)
  ui_create_back_button(preset_list_menu, 1, 0, [](lv_event_t *e)
                        {
    current_menu_page = 0;
    renderMenu(MENU_CONTEXTUAL, false); });

  // Preset buttons (Rows 2-11)
  for (int i = 0; i < TCG_PRESET_COUNT; i++)
  {
    lv_obj_t *btn = lv_btn_create(preset_list_menu);
    lv_obj_set_size(btn, 180, 45);
    lv_obj_set_style_bg_color(btn, getThemeButtonColor(), 0); // Use theme color
    lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, i + 2, 1);
    lv_obj_set_user_data(btn, (void *)(intptr_t)i);
    lv_obj_add_event_cb(btn, [](lv_event_t *e)
                        {
      lv_obj_t *target = (lv_obj_t*)lv_event_get_target(e);
      int idx = (int)(intptr_t)lv_obj_get_user_data(target);
      save_preset(idx);
      
      TCGPreset preset = get_preset();
      player_store.putInt(KEY_LIFE_MAX, preset.starting_life);
      player_store.putInt(KEY_LIFE_STEP_SMALL, preset.small_step);
      player_store.putInt(KEY_LIFE_STEP_LARGE, preset.large_step);
      
      // Save active preset index for theme system
      player_store.putInt("preset_idx", idx);
      
      // *** RESET EVERYTHING: Use centralized reset function ***
      // This resets life, counters, timer, and marks saved life as invalid
      resetActiveCounter();
      
      // If Automatic mode is active, reset theme trackers to force refresh
      int theme_mode = player_store.getInt(KEY_THEME_MODE, THEME_MODE_OFF);
      if (theme_mode == THEME_MODE_AUTOMATIC) {
        extern void resetLastLoadedTheme();     // Single player
        extern void resetLastLoadedTheme2P();   // Two player  
        extern void resetLastSettingsTheme();   // Settings menu
        resetLastLoadedTheme();
        resetLastLoadedTheme2P();
        resetLastSettingsTheme();
      }
      
      ui_init();
      renderMenu(MENU_NONE); }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl = lv_label_create(btn);
    char buf[64];
    snprintf(buf, sizeof(buf), "%s (%d)", TCG_PRESETS[i].name, TCG_PRESETS[i].starting_life);
    lv_label_set_text(lbl, buf);
    lv_obj_set_style_text_color(lbl, getThemeTextColor(), 0); // Theme text color
    lv_obj_center(lbl);
  }

  // Extra spacer for scrolling (Row 12)
  lv_obj_t *extra_spacer = lv_obj_create(preset_list_menu);
  lv_obj_set_size(extra_spacer, 10, 50);
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 12, 1);
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);

  currentMenu = MENU_PRESET_LIST;
}

void renderContextualMenuOverlay(bool animate_menu)
{
  teardownContextualMenuOverlay();
  int circle_diameter = (SCREEN_WIDTH < SCREEN_HEIGHT ? SCREEN_WIDTH : SCREEN_HEIGHT);
  int circle_radius = circle_diameter / 2;

  contextual_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(contextual_menu, circle_diameter, circle_diameter);
  lv_obj_set_style_bg_color(contextual_menu, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(contextual_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_opa(contextual_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(contextual_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_radius(contextual_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_align(contextual_menu, LV_ALIGN_CENTER, 0, 0);
  lv_obj_clear_flag(contextual_menu, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(contextual_menu, LV_OBJ_FLAG_GESTURE_BUBBLE);

  int ring_radius = circle_radius;

  if (current_menu_page == 0)
  {
    // Get theme icon color
    lv_color_t icon_color = getThemeIconColor();

    lv_obj_t *lbl_tl = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_tl, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_font(lbl_tl, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(lbl_tl, icon_color, 0); // Apply theme icon color
    lv_obj_align(lbl_tl, LV_ALIGN_CENTER, -ring_radius / 2, -ring_radius / 2);

    lv_obj_t *lbl_tr = lv_label_create(contextual_menu);
    const char *lbl_text = player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER) == PLAYER_MODE_ONE_PLAYER ? "2P" : "1P";
    lv_label_set_text(lbl_tr, lbl_text);
    lv_obj_set_style_text_font(lbl_tr, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(lbl_tr, icon_color, 0); // Apply theme icon color
    lv_obj_align(lbl_tr, LV_ALIGN_CENTER, ring_radius / 2, -ring_radius / 2);

    lv_obj_t *lbl_bl = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_bl, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_font(lbl_bl, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(lbl_bl, icon_color, 0); // Apply theme icon color
    lv_obj_align(lbl_bl, LV_ALIGN_CENTER, -ring_radius / 2, ring_radius / 2);

    lv_obj_t *lbl_br = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_br, LV_SYMBOL_LIST);
    lv_obj_set_style_text_font(lbl_br, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(lbl_br, icon_color, 0); // Apply theme icon color
    lv_obj_align(lbl_br, LV_ALIGN_CENTER, ring_radius / 2, ring_radius / 2);
  }
  else
  {
    // Get theme icon color (Page 2: Dice/Coin/History)
    lv_color_t icon_color = getThemeIconColor();

    lv_obj_t *lbl_tl = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_tl, "Dice");
    lv_obj_set_style_text_font(lbl_tl, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(lbl_tl, icon_color, 0); // Apply theme icon color
    lv_obj_align(lbl_tl, LV_ALIGN_CENTER, -ring_radius / 2, -ring_radius / 2);

    lv_obj_t *lbl_tr = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_tr, "Coin");
    lv_obj_set_style_text_font(lbl_tr, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(lbl_tr, icon_color, 0); // Apply theme icon color
    lv_obj_align(lbl_tr, LV_ALIGN_CENTER, ring_radius / 2, -ring_radius / 2);

    lv_obj_t *lbl_bl = lv_label_create(contextual_menu);
    lv_label_set_text(lbl_bl, LV_SYMBOL_DOWNLOAD);
    lv_obj_set_style_text_font(lbl_bl, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(lbl_bl, icon_color, 0); // Apply theme icon color
    lv_obj_align(lbl_bl, LV_ALIGN_CENTER, -ring_radius / 2, ring_radius / 2);
  }

  static bool swipe_detected = false;

  lv_obj_add_event_cb(contextual_menu, [](lv_event_t *e)
                      {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
      swipe_detected = false;
    }
    else if (code == LV_EVENT_GESTURE) {
      lv_indev_t *indev = lv_indev_get_act();
      lv_dir_t dir = lv_indev_get_gesture_dir(indev);
      if (dir == LV_DIR_LEFT) {
        current_menu_page = (current_menu_page + 1) % TOTAL_MENU_PAGES;
        renderMenu(MENU_CONTEXTUAL, false);
        swipe_detected = true;
      } else if (dir == LV_DIR_RIGHT) {
        current_menu_page = (current_menu_page - 1 + TOTAL_MENU_PAGES) % TOTAL_MENU_PAGES;
        renderMenu(MENU_CONTEXTUAL, false);
        swipe_detected = true;
      }
    }
    else if (code == LV_EVENT_CLICKED && !swipe_detected) {
      if (is_in_center_cancel_area(e)) {
        return;
      }
      if (is_in_quadrant(e, -180, -90)) {
        handleContextualSelection(QUADRANT_TL);
      } else if (is_in_quadrant(e, -90, 0)) {
        handleContextualSelection(QUADRANT_TR);
      } else if (is_in_quadrant(e, 0, 90)) {
        handleContextualSelection(QUADRANT_BR);
      } else if (is_in_quadrant(e, 90, 180)) {
        handleContextualSelection(QUADRANT_BL);
      }
    } }, LV_EVENT_ALL, NULL);

  lv_obj_add_flag(contextual_menu, LV_OBJ_FLAG_CLICKABLE);

  int hole_diameter = (ring_radius / 3) * 2;
  lv_obj_t *center_cancel = lv_btn_create(contextual_menu);
  lv_obj_set_size(center_cancel, hole_diameter, hole_diameter);
  lv_obj_align(center_cancel, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_radius(center_cancel, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(center_cancel, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(center_cancel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_opa(center_cancel, LV_OPA_TRANSP, 0);
  lv_obj_add_event_cb(center_cancel, [](lv_event_t *e)
                      { renderMenu(MENU_NONE); }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_cancel = lv_label_create(center_cancel);
  lv_label_set_text(lbl_cancel, LV_SYMBOL_CLOSE);
  lv_obj_set_style_text_font(lbl_cancel, &lv_font_montserrat_48, 0);
  lv_obj_center(lbl_cancel);

  int dot_size = 8;
  int dot_spacing = 20;
  int start_x = (circle_diameter / 2) - 10;
  int y_pos = circle_radius + 75;

  for (int i = 0; i < TOTAL_MENU_PAGES; i++)
  {
    lv_obj_t *dot = lv_obj_create(contextual_menu);
    lv_obj_set_size(dot, dot_size, dot_size);
    lv_obj_set_pos(dot, start_x + (i * dot_spacing) - (TOTAL_MENU_PAGES * dot_spacing / 2), y_pos);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);

    if (i == current_menu_page)
    {
      lv_obj_set_style_bg_color(dot, getThemeButtonColor(), 0); // Use theme color for active page indicator
      lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    }
    else
    {
      lv_obj_set_style_bg_color(dot, lv_color_hex(0x666666), 0);
      lv_obj_set_style_bg_opa(dot, LV_OPA_50, 0);
    }
  }

  if (animate_menu)
  {
    lv_obj_set_y(contextual_menu, -SCREEN_HEIGHT);
    slide_in_obj_vertical(contextual_menu, -SCREEN_HEIGHT, 0, 250, 0, nullptr);
  }
}

void renderMenu(MenuState menuType)
{
  renderMenu(menuType, true);
}

void renderMenu(MenuState menuType, bool animate_menu)
{
  teardownAllMenus();
  hideLifeScreen();
  switch (menuType)
  {
  case MENU_CONTEXTUAL:
    renderContextualMenuOverlay(animate_menu);
    currentMenu = MENU_CONTEXTUAL;
    break;
  case MENU_SETTINGS:
    renderSettingsOverlay();
    currentMenu = MENU_SETTINGS;
    break;
  case MENU_LIFE_CONFIG:
    renderLifeConfigScreen();
    currentMenu = MENU_LIFE_CONFIG;
    break;
  case MENU_HISTORY:
    renderHistoryOverlay();
    currentMenu = MENU_HISTORY;
    break;
  case MENU_BRIGHTNESS:
    renderBrightnessOverlay();
    currentMenu = MENU_BRIGHTNESS;
    break;
  case MENU_DICE_LIST:
    renderDiceListMenu();
    break;
  case MENU_PRESET_LIST:
    renderPresetListMenu();
    break;
  case MENU_PRESET_EDITOR:
    renderPresetEditorMenu();
    currentMenu = MENU_PRESET_EDITOR;
    break;
  case MENU_TOUCH_CALIBRATION:
    renderTouchCalibrationScreen();
    currentMenu = MENU_TOUCH_CALIBRATION;
    break;
  case MENU_AUDIO_SETTINGS:
    teardownAudioSettingsMenu();
    renderAudioSettingsMenu();
    currentMenu = MENU_AUDIO_SETTINGS;
    break;
  case MENU_TIMER_SETTINGS:
    teardownTimerSettingsMenu();
    renderTimerSettingsMenu();
    currentMenu = MENU_TIMER_SETTINGS;
    break;
  case MENU_POWER_SETTINGS:
    teardownPowerSettingsMenu();
    renderPowerSettingsMenu();
    currentMenu = MENU_POWER_SETTINGS;
    break;
  case MENU_THEME_SETTINGS:
    refreshThemeSettingsColors(); // Update colors when returning to theme menu
    teardownThemeSettingsMenu();
    renderThemeSettingsMenu();
    currentMenu = MENU_THEME_SETTINGS;
    break;
  case MENU_TOUCH_SETTINGS:
    teardownTouchSettingsMenu();
    renderTouchSettingsMenu();
    currentMenu = MENU_TOUCH_SETTINGS;
    break;
  case MENU_NONE:
  default:
    showLifeScreen();
    currentMenu = MENU_NONE;
    break;
  }
}

static bool is_in_center_cancel_area(lv_event_t *e)
{
  lv_point_t p;
  lv_indev_get_point(lv_indev_get_act(), &p);
  int x = p.x, y = p.y;
  int circle_x = (SCREEN_WIDTH - circle_diameter) / 2;
  int circle_y = (SCREEN_HEIGHT - circle_diameter) / 2;
  int cx = circle_x + circle_radius;
  int cy = circle_y + circle_radius;
  int ring_radius = circle_radius;
  int dx = x - cx, dy = y - cy;
  float dist = sqrtf(dx * dx + dy * dy);
  int hole_radius = ring_radius / 3;
  return (dist < hole_radius);
}

bool is_in_quadrant(lv_event_t *e, int angle_start, int angle_end)
{
  lv_point_t p;
  lv_indev_get_point(lv_indev_get_act(), &p);
  int x = p.x, y = p.y;
  int circle_x = (SCREEN_WIDTH - circle_diameter) / 2;
  int circle_y = (SCREEN_HEIGHT - circle_diameter) / 2;
  int cx = circle_x + circle_radius;
  int cy = circle_y + circle_radius;
  int ring_diameter = circle_diameter;
  int ring_radius = ring_diameter / 2;
  int dx = x - cx, dy = y - cy;
  float angle = atan2f(dy, dx) * 180.0f / M_PI;
  float dist = sqrtf(dx * dx + dy * dy);
  int hole_radius = ring_radius / 3;
  return (dist >= hole_radius && dist <= ring_radius && angle >= angle_start && angle < angle_end);
}

extern lv_obj_t *life_counter_container;
extern lv_obj_t *life_counter_container_2p;

void hideLifeScreen()
{
  if (life_counter_container)
    lv_obj_add_flag(life_counter_container, LV_OBJ_FLAG_HIDDEN);
  if (life_counter_container_2p)
    lv_obj_add_flag(life_counter_container_2p, LV_OBJ_FLAG_HIDDEN);

  // Hide all counter UI during menu animations
  for (int i = 0; i < 4; i++)
  {
    if (SimpleCounters::counters[i].container != nullptr)
    {
      lv_obj_add_flag(SimpleCounters::counters[i].container, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

void showLifeScreen()
{
  if (life_counter_container)
  {
    lv_obj_clear_flag(life_counter_container, LV_OBJ_FLAG_HIDDEN);
    // Refresh theme if changed while in settings
    refresh_life_counter_theme();
    // Create counter UI for enabled counters
    SimpleCounters::create_all_enabled_ui();
  }
  if (life_counter_container_2p)
  {
    lv_obj_clear_flag(life_counter_container_2p, LV_OBJ_FLAG_HIDDEN);
    // Refresh theme if changed while in settings
    refresh_life_counter_2p_theme();
  }

  // Show all enabled counter UI when returning to life screen
  for (int i = 0; i < 4; i++)
  {
    if (SimpleCounters::counters[i].container != nullptr && SimpleCounters::counters[i].enabled)
    {
      lv_obj_clear_flag(SimpleCounters::counters[i].container, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

void teardownAllMenus()
{
  currentMenu = MENU_NONE;
  teardownContextualMenuOverlay();
  teardownSettingsOverlay();
  teardownStartLifeScreen();
  teardownHistoryOverlay();
  teardownBrightnessOverlay();
  teardownDiceListMenu();
  teardownPresetListMenu();
  teardownPresetEditorMenu();
  teardownTouchCalibrationScreen();
  teardownAudioSettingsMenu();
  teardownTimerSettingsMenu();
  teardownPowerSettingsMenu();
  teardownThemeSettingsMenu();
  teardownTouchSettingsMenu();
  teardownCountersSettings();
}

void teardownContextualMenuOverlay()
{
  if (contextual_menu)
  {
    lv_obj_del(contextual_menu);
    contextual_menu = nullptr;
  }
}

void teardownDiceListMenu()
{
  if (dice_list_menu)
  {
    lv_obj_del(dice_list_menu);
    dice_list_menu = nullptr;
  }
}

void teardownPresetListMenu()
{
  if (preset_list_menu)
  {
    lv_obj_del(preset_list_menu);
    preset_list_menu = nullptr;
  }
}