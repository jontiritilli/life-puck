#include "touch_settings.h"
#include "config.h"
#include "data/constants.h"
#include "data/themes.h"
#include "ui/screens/menu/menu.h"
#include "ui/helpers/ui_helpers.h"
#include "ui/helpers/ui_constants.h"
#include "core/state_manager.h"
#include "hardware/touch/Touch_CST816.h"
#include <lvgl.h>

static lv_obj_t *touch_settings_menu = nullptr;

void renderTouchSettingsMenu()
{
  teardownTouchSettingsMenu();

  // Create main container using standardized helper
  touch_settings_menu = ui_create_menu_container(lv_scr_act());

  // Grid Layout: Single column for wide buttons
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(touch_settings_menu, col_dsc, row_dsc);
  lv_obj_set_layout(touch_settings_menu, LV_LAYOUT_GRID);

  // Title (Row 0)
  ui_create_title(touch_settings_menu, "Touch Settings", 0, 2);

  // Back button (Row 1) - centered across both columns
  lv_obj_t *btn_back = ui_create_back_button(touch_settings_menu, 1, 0, [](lv_event_t *e)
                                             { renderMenu(MENU_SETTINGS); });
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 1, 1);

  // Touch Calibration Button (Row 2, Full Width)
  ui_create_wide_button(touch_settings_menu, "Touch Calibration", 2, [](lv_event_t *e)
                        { renderMenu(MENU_TOUCH_CALIBRATION); });

  // Menu Swipe Toggle (Row 3, Full Width)
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  ui_create_wide_toggle_button(touch_settings_menu, "Menu Swipe: ON", "Menu Swipe: OFF", swipe_enabled, 3, [](lv_event_t *e)
                               {
    int state = player_store.getInt(KEY_SWIPE_TO_CLOSE, 0);
    state = (state == 0) ? 1 : 0;
    player_store.putInt(KEY_SWIPE_TO_CLOSE, state);
    printf("[TouchSettings] Swipe to dismiss toggled to: %d\n", state);
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    bool enabled = (state != 0);
    lv_label_set_text(label, enabled ? "Menu Swipe: ON" : "Menu Swipe: OFF");
    lv_obj_set_style_bg_color(btn, (enabled ? getThemeButtonColor() : UI_COLOR_DISABLED), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, (enabled ? getThemeTextColor() : UI_COLOR_TEXT_PRIMARY), 0); });

  // Life Gesture Mode Button (Row 4, Full Width) - Mode selector, not toggle
  int gesture_mode = player_store.getInt(KEY_GESTURE_MODE, GESTURE_CONTROL_MODE);
  lv_obj_t *btn_gesture_mode = ui_create_wide_button(touch_settings_menu,
                                                     (gesture_mode == 0) ? "Life Step: Swipe" : "Life Step: Long Press",
                                                     4,
                                                     [](lv_event_t *e)
                                                     {
    int mode = player_store.getInt(KEY_GESTURE_MODE, GESTURE_CONTROL_MODE);
    mode = (mode == 0) ? 1 : 0;
    player_store.putInt(KEY_GESTURE_MODE, mode);
    printf("[TouchSettings] Gesture control mode changed to: %d (%s)\n", mode, (mode == 0) ? "Life Step: Swipe" : "Life Step: Long Press");
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text(label, (mode == 0) ? "Life Step: Swipe" : "Life Step: Long Press"); });

  // Extra Spacer for Scrolling (Row 5)
  lv_obj_t *extra_spacer = lv_obj_create(touch_settings_menu);
  lv_obj_set_size(extra_spacer, 10, 50);
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 5, 1);
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);

  // Add swipe-to-close gesture (same as other settings submenus)
  bool swipe_to_close_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_to_close_enabled)
  {
    extern CST816_Touch touch_data; // Access RAW touch data for accurate swipe detection

    lv_obj_add_event_cb(touch_settings_menu, [](lv_event_t *e)
                        {
      extern CST816_Touch touch_data; // Re-declare inside lambda
      static lv_point_t start_point;
      static bool is_swiping = false;

      lv_event_code_t code = lv_event_get_code(e);

      if (code == LV_EVENT_PRESSED)
      {
        // Use RAW coordinates for swipe detection
        start_point.x = touch_data.x;
        start_point.y = touch_data.y;
        is_swiping = false;
      }
      else if (code == LV_EVENT_PRESSING)
      {
        // Use RAW coordinates for swipe detection
        lv_point_t point;
        point.x = touch_data.x;
        point.y = touch_data.y;
        int dx = point.x - start_point.x;
        int dy = point.y - start_point.y;

        // Detect horizontal swipe
        if (abs(dx) > 30 && abs(dx) > abs(dy) * 1.5)
        {
          is_swiping = true;
        }
      }
      else if (code == LV_EVENT_RELEASED)
      {
        if (is_swiping)
        {
          // Use RAW coordinates for final check
          lv_point_t point;
          point.x = touch_data.x;
          point.y = touch_data.y;
          int dx = point.x - start_point.x;
          int dy = point.y - start_point.y;

          // Swipe right threshold - relaxed for round display
          if (dx > 80 && abs(dx) > abs(dy))
          {
            renderMenu(MENU_SETTINGS);
            return;
          }
        }
        is_swiping = false;
      } }, LV_EVENT_ALL, NULL);
  }
}

void teardownTouchSettingsMenu()
{
  if (touch_settings_menu)
  {
    lv_obj_del(touch_settings_menu);
    touch_settings_menu = nullptr;
  }
}
