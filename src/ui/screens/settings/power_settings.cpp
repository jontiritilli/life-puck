#include "power_settings.h"
#include "config.h"
#include "data/constants.h"
#include "data/themes.h"
#include "ui/screens/menu/menu.h"
#include "ui/screens/settings/brightness.h"
#include "ui/helpers/ui_helpers.h"
#include "ui/helpers/ui_constants.h"
#include "core/state_manager.h"
#include "hardware/touch/Touch_CST816.h"
#include <Arduino.h>

// NVS Keys for power settings
#define KEY_AUTO_DIM_TIME "auto_dim_time"
#define KEY_SLEEP_TIME "sleep_time"
#define KEY_BATTERY_SAVER "battery_saver"

static lv_obj_t *power_settings_menu = nullptr;

// Static labels for updating button states
static lv_obj_t *lbl_auto_dim = nullptr;
static lv_obj_t *lbl_sleep = nullptr;
static lv_obj_t *lbl_battery_saver = nullptr;

// Auto-Dim Time options (in seconds): Off, 30s, 1min, 2min, 5min
static const int AUTO_DIM_OPTIONS[] = {0, 30, 60, 120, 300};
static const int AUTO_DIM_COUNT = 5;

// Sleep Time options (in seconds): Off, 2min, 5min, 10min
static const int SLEEP_OPTIONS[] = {0, 120, 300, 600};
static const int SLEEP_COUNT = 4;

// Helper function to format time
static String formatTime(int seconds)
{
  if (seconds == 0)
    return "OFF";
  if (seconds < 60)
    return String(seconds) + "s";
  return String(seconds / 60) + "min";
}

// Helper function to get current auto-dim index
static int getAutoDimIndex()
{
  int current = player_store.getInt(KEY_AUTO_DIM_TIME, AUTO_DIM_DEFAULT);
  for (int i = 0; i < AUTO_DIM_COUNT; i++)
  {
    if (AUTO_DIM_OPTIONS[i] == current)
      return i;
  }
  return 2; // Default to 1min if not found
}

// Helper function to get current sleep index
static int getSleepIndex()
{
  int current = player_store.getInt(KEY_SLEEP_TIME, SLEEP_DEFAULT);
  for (int i = 0; i < SLEEP_COUNT; i++)
  {
    if (SLEEP_OPTIONS[i] == current)
      return i;
  }
  return 2; // Default to 5min if not found
}

// Update button display
static void updateAutoDimButton(lv_obj_t *btn)
{
  int index = getAutoDimIndex();
  int value = AUTO_DIM_OPTIONS[index];
  String text = "Auto-Dim: " + formatTime(value);
  lv_label_set_text(lbl_auto_dim, text.c_str());

  // Gray out if OFF, theme color if ON
  if (value == 0)
  {
    lv_obj_set_style_bg_color(btn, UI_COLOR_DISABLED, 0);
    lv_obj_set_style_text_color(lbl_auto_dim, UI_COLOR_TEXT_PRIMARY, 0);
  }
  else
  {
    lv_obj_set_style_bg_color(btn, getThemeButtonColor(), 0);
    lv_obj_set_style_text_color(lbl_auto_dim, getThemeTextColor(), 0);
  }
}

static void updateSleepButton(lv_obj_t *btn)
{
  int index = getSleepIndex();
  int value = SLEEP_OPTIONS[index];
  String text = "Sleep: " + formatTime(value);
  lv_label_set_text(lbl_sleep, text.c_str());

  // Gray out if OFF, theme color if ON
  if (value == 0)
  {
    lv_obj_set_style_bg_color(btn, UI_COLOR_DISABLED, 0);
    lv_obj_set_style_text_color(lbl_sleep, UI_COLOR_TEXT_PRIMARY, 0);
  }
  else
  {
    lv_obj_set_style_bg_color(btn, getThemeButtonColor(), 0);
    lv_obj_set_style_text_color(lbl_sleep, getThemeTextColor(), 0);
  }
}

static void updateBatterySaverButton(lv_obj_t *btn)
{
  bool enabled = player_store.getInt(KEY_BATTERY_SAVER, LOW_BATTERY_DIM_DEFAULT) != 0;
  String text = "Low Battery Dimming: " + String(enabled ? "ON" : "OFF");
  lv_label_set_text(lbl_battery_saver, text.c_str());

  // Gray out if OFF, theme color if ON
  if (!enabled)
  {
    lv_obj_set_style_bg_color(btn, UI_COLOR_DISABLED, 0);
    lv_obj_set_style_text_color(lbl_battery_saver, UI_COLOR_TEXT_PRIMARY, 0);
  }
  else
  {
    lv_obj_set_style_bg_color(btn, getThemeButtonColor(), 0);
    lv_obj_set_style_text_color(lbl_battery_saver, getThemeTextColor(), 0);
  }
}

void renderPowerSettingsMenu()
{
  if (power_settings_menu)
  {
    teardownPowerSettingsMenu();
  }

  // Create main container using standardized helper
  power_settings_menu = ui_create_menu_container(lv_scr_act());

  // Grid Layout: Full width buttons (1 column, 6 rows)
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(power_settings_menu, col_dsc, row_dsc);
  lv_obj_set_layout(power_settings_menu, LV_LAYOUT_GRID);

  // Title (Row 0)
  ui_create_title(power_settings_menu, "Power Settings", 0);

  // Back button (Row 1)
  lv_obj_t *btn_back = ui_create_back_button(power_settings_menu, 1, 0, [](lv_event_t *e)
                                             { renderMenu(MENU_SETTINGS); });

  // Brightness Button (Row 2, Full Width)
  lv_obj_t *btn_brightness = lv_btn_create(power_settings_menu);
  lv_obj_set_size(btn_brightness, 280, UI_BTN_HEIGHT_STANDARD);
  lv_obj_set_style_bg_color(btn_brightness, getThemeButtonColor(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_brightness, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_t *lbl_brightness = lv_label_create(btn_brightness);
  lv_label_set_text(lbl_brightness, "Brightness");
  lv_obj_set_style_text_font(lbl_brightness, UI_FONT_TOGGLE, 0);
  lv_obj_set_style_text_color(lbl_brightness, getThemeTextColor(), 0);
  lv_obj_center(lbl_brightness);
  lv_obj_add_event_cb(btn_brightness, [](lv_event_t *e)
                      { renderBrightnessOverlay(MENU_POWER_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  // Auto-Dim Button (Row 3, Full Width)
  lv_obj_t *btn_auto_dim = lv_btn_create(power_settings_menu);
  lv_obj_set_size(btn_auto_dim, 280, UI_BTN_HEIGHT_STANDARD);
  lv_obj_set_style_bg_color(btn_auto_dim, getThemeButtonColor(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_auto_dim, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 3, 1);
  lbl_auto_dim = lv_label_create(btn_auto_dim);
  lv_obj_set_style_text_font(lbl_auto_dim, UI_FONT_TOGGLE, 0);
  lv_obj_center(lbl_auto_dim);

  // Text color set by updateAutoDimButton()
  updateAutoDimButton(btn_auto_dim);

  lv_obj_add_event_cb(btn_auto_dim, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
    
    // Cycle through options
    int index = getAutoDimIndex();
    index = (index + 1) % AUTO_DIM_COUNT;
    player_store.putInt(KEY_AUTO_DIM_TIME, AUTO_DIM_OPTIONS[index]);
    
    updateAutoDimButton(btn);
    printf("[PowerSettings] Auto-Dim set to %d seconds\n", AUTO_DIM_OPTIONS[index]); }, LV_EVENT_CLICKED, NULL);

  // Sleep Button (Row 4, Full Width)
  lv_obj_t *btn_sleep = lv_btn_create(power_settings_menu);
  lv_obj_set_size(btn_sleep, 280, UI_BTN_HEIGHT_STANDARD);
  lv_obj_set_style_bg_color(btn_sleep, getThemeButtonColor(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_sleep, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 4, 1);
  lbl_sleep = lv_label_create(btn_sleep);
  lv_obj_set_style_text_font(lbl_sleep, UI_FONT_TOGGLE, 0);
  lv_obj_center(lbl_sleep);

  // Text color set by updateSleepButton()
  updateSleepButton(btn_sleep);

  lv_obj_add_event_cb(btn_sleep, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
    
    // Cycle through options
    int index = getSleepIndex();
    index = (index + 1) % SLEEP_COUNT;
    player_store.putInt(KEY_SLEEP_TIME, SLEEP_OPTIONS[index]);
    
    updateSleepButton(btn);
    printf("[PowerSettings] Sleep set to %d seconds\n", SLEEP_OPTIONS[index]); }, LV_EVENT_CLICKED, NULL);

  // Battery Saver Button (Row 5, Full Width)
  lv_obj_t *btn_battery_saver = lv_btn_create(power_settings_menu);
  lv_obj_set_size(btn_battery_saver, 280, UI_BTN_HEIGHT_STANDARD);
  lv_obj_set_style_bg_color(btn_battery_saver, getThemeButtonColor(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_battery_saver, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 5, 1);
  lbl_battery_saver = lv_label_create(btn_battery_saver);
  lv_obj_set_style_text_font(lbl_battery_saver, UI_FONT_TOGGLE, 0);
  lv_obj_center(lbl_battery_saver);

  // Text color set by updateBatterySaverButton()
  updateBatterySaverButton(btn_battery_saver);

  lv_obj_add_event_cb(btn_battery_saver, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
    
    // Toggle on/off
    bool current = player_store.getInt(KEY_BATTERY_SAVER, LOW_BATTERY_DIM_DEFAULT) != 0;
    player_store.putInt(KEY_BATTERY_SAVER, current ? 0 : 1);
    
    updateBatterySaverButton(btn);
    printf("[PowerSettings] Battery Saver %s\n", current ? "OFF" : "ON"); }, LV_EVENT_CLICKED, NULL);

  // Extra Spacer for Scrolling (Row 6)
  lv_obj_t *extra_spacer = lv_obj_create(power_settings_menu);
  lv_obj_set_size(extra_spacer, 10, 50);
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 6, 1);
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);

  // Add swipe-to-close gesture (same as audio/timer settings menu)
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled)
  {
    extern CST816_Touch touch_data; // Access RAW touch data for accurate swipe detection

    lv_obj_add_event_cb(power_settings_menu, [](lv_event_t *e)
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
            renderMenu(MENU_SETTINGS);
            return;
          }
        }
        is_swiping = false;
      } }, LV_EVENT_ALL, NULL);
  }

  printf("[PowerSettings] Menu rendered\n");
}

void teardownPowerSettingsMenu()
{
  if (power_settings_menu)
  {
    lv_obj_del(power_settings_menu);
    power_settings_menu = nullptr;
    lbl_auto_dim = nullptr;
    lbl_sleep = nullptr;
    lbl_battery_saver = nullptr;
    printf("[PowerSettings] Menu torn down\n");
  }
}
