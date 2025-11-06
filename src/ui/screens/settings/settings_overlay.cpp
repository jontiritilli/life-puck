// ============================================
// Own Header (first!)
// ============================================
#include "settings_overlay.h"
#include "counters_settings.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// Hardware
// ============================================
#include "hardware/system/battery_state.h"
#include "hardware/touch/Touch_CST816.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/menu/menu.h"
#include "ui/screens/life/life_counter.h"
#include "ui/screens/life/life_counter_two_player.h"
#include "ui/screens/tools/timer.h"

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/animation_helpers.h"
#include "ui/helpers/ui_helpers.h"
#include "ui/helpers/ui_constants.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "data/themes.h"
#include "hardware/audio/simple_audio.h"

extern lv_obj_t *settings_menu;
extern lv_obj_t *life_counter_container;
extern lv_obj_t *life_counter_container_2p;

// Theme tracking for Settings menu
static const Theme *last_settings_theme = nullptr;

static void btn_life_event_cb(lv_event_t *e)
{
  renderMenu(MENU_LIFE_CONFIG);
}

void renderSettingsOverlay()
{
  teardownSettingsOverlay();
  hideLifeScreen();

  settings_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(settings_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(settings_menu);
  lv_obj_set_style_bg_color(settings_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(settings_menu, 15, 0);
  lv_obj_set_style_border_width(settings_menu, 0, 0);
  lv_obj_set_flex_flow(settings_menu, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(settings_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scrollbar_mode(settings_menu, LV_SCROLLBAR_MODE_AUTO);

  // SWIPE TO CLOSE - Always add handler, but check setting inside
  extern CST816_Touch touch_data; // Access RAW touch data for accurate swipe detection

  lv_obj_add_event_cb(settings_menu, [](lv_event_t *e)
                      {
    extern CST816_Touch touch_data; // Re-declare inside lambda
    static lv_point_t start_point;
    static bool is_swiping = false;
    
    // Check if swipe is enabled for this specific event
    bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
    if (!swipe_enabled) return; // Exit early if disabled
    
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
          renderMenu(MENU_CONTEXTUAL, false);
          return;
        }
      }
      is_swiping = false;
    } }, LV_EVENT_ALL, NULL);

  // Grid Layout: 2 columns, 6 rows (Touch Settings replaces toggle buttons)
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(settings_menu, col_dsc, row_dsc);
  lv_obj_set_layout(settings_menu, LV_LAYOUT_GRID);
  lv_obj_set_scrollbar_mode(settings_menu, LV_SCROLLBAR_MODE_AUTO);

  // Battery indicator (Row 0 - top, centered) - Shows percentage and voltage
  lv_obj_t *battery_label = lv_label_create(settings_menu);
  lv_obj_set_grid_cell(battery_label, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_style_text_font(battery_label, UI_FONT_INFO, 0);
  lv_obj_set_style_text_color(battery_label, UI_COLOR_TEXT_PRIMARY, 0);

  int battery_percentage = (int)(battery_get_percent() + 0.5f);
  float battery_voltage = battery_get_volts();
  char battery_text[32];
  snprintf(battery_text, sizeof(battery_text), LV_SYMBOL_BATTERY_FULL " %d%% (%.2fV)",
           battery_percentage, battery_voltage);
  lv_label_set_text(battery_label, battery_text);

  // Back button (Row 1) - centered, closes all menus
  lv_obj_t *btn_back = ui_create_back_button(settings_menu, 1, 0, [](lv_event_t *e)
                                             { renderMenu(MENU_CONTEXTUAL); });
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 1, 1);

  // Menu buttons using UI helpers
  ui_create_button(settings_menu, "Timer", 2, 0, [](lv_event_t *e)
                   { renderMenu(MENU_TIMER_SETTINGS); });
  ui_create_button(settings_menu, "Power", 2, 1, [](lv_event_t *e)
                   { renderMenu(MENU_POWER_SETTINGS); });
  ui_create_button(settings_menu, "Touch", 3, 0, [](lv_event_t *e)
                   { renderMenu(MENU_TOUCH_SETTINGS); });
  ui_create_button(settings_menu, "Themes", 3, 1, [](lv_event_t *e)
                   { renderMenu(MENU_THEME_SETTINGS); });
  ui_create_button(settings_menu, "Counters", 4, 0, [](lv_event_t *e)
                   { renderCountersSettings(); });
  ui_create_button(settings_menu, "Presets", 4, 1, [](lv_event_t *e)
                   { renderMenu(MENU_PRESET_EDITOR); });
  ui_create_button(settings_menu, "Start Life", 5, 0, btn_life_event_cb);
#ifdef BOARD_1_85C
  // Audio settings only available on C model (has speaker)
  ui_create_button(settings_menu, "Audio", 5, 1, [](lv_event_t *e)
                   { renderMenu(MENU_AUDIO_SETTINGS); });
#endif

  // Extra Spacer for Scrolling (Row 6, spanning both columns)
  lv_obj_t *extra_spacer = lv_obj_create(settings_menu);
  lv_obj_set_size(extra_spacer, 10, 50);
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 6, 1);
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);
}

void teardownSettingsOverlay()
{
  if (settings_menu)
  {
    lv_obj_del(settings_menu);
    settings_menu = nullptr;
  }
  last_settings_theme = nullptr; // Reset theme tracking
}

/**
 * @brief Reset theme tracker to force refresh on next render
 *
 * Used when preset changes in Automatic mode to ensure
 * Settings menu theme updates immediately.
 */
void resetLastSettingsTheme()
{
  last_settings_theme = nullptr; // Force refresh on next render
}
