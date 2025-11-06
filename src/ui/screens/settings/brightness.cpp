// ============================================
// Own Header (first!)
// ============================================
#include "brightness.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>

// ============================================
// Config
// ============================================
#include "config.h"

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// Hardware
// ============================================
#include "hardware/display/display_st77916.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/menu/menu.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"

// The old 'board' object is no longer needed
// extern esp_panel::board::Board *board;

extern lv_obj_t *brightness_control;

struct ChangeData
{
  lv_obj_t *button;
  lv_obj_t *val_current;
};

// Global brightness value - loaded dynamically on first use
int brightness = -1; // -1 means not loaded yet

// Load brightness from NVS on first access
static void ensure_brightness_loaded()
{
  if (brightness == -1)
  {
    brightness = player_store.getInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT);
    printf("[Brightness] Loaded from NVS: %d%%\n", brightness);
  }
}

// This function is now much simpler
static void set_brightness()
{
  player_store.putInt(KEY_BRIGHTNESS, brightness);
  Set_Backlight(brightness); // Use the new driver function
}

static void brightness_up_event_handler(lv_event_t *e)
{
  ensure_brightness_loaded(); // Make sure brightness is loaded from NVS
  if (brightness < 100)
  {
    brightness += 5;
    lv_obj_t *value_label = (lv_obj_t *)lv_event_get_user_data(e);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", brightness);
    lv_label_set_text(value_label, buf);
    set_brightness();
  }
}

static void brightness_down_event_handler(lv_event_t *e)
{
  ensure_brightness_loaded(); // Make sure brightness is loaded from NVS
  if (brightness > 5)
  {
    brightness -= 5;
    lv_obj_t *value_label = (lv_obj_t *)lv_event_get_user_data(e);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", brightness);
    lv_label_set_text(value_label, buf);
    set_brightness();
  }
}

void teardownBrightnessOverlay()
{
  if (brightness_control)
  {
    lv_obj_del(brightness_control);
    brightness_control = nullptr;
  }
}

void renderBrightnessOverlay(MenuState return_menu)
{
  ensure_brightness_loaded(); // Make sure brightness is loaded from NVS

  teardownBrightnessOverlay();
  brightness_control = lv_obj_create(lv_scr_act());
  lv_obj_set_size(brightness_control, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(brightness_control, BLACK_COLOR, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(brightness_control, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_radius(brightness_control, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_border_opa(brightness_control, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(brightness_control, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_pad_all(brightness_control, 16, LV_PART_MAIN);

  // Define grid: 3 columns, 3 rows
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {60, 100, 40, LV_GRID_TEMPLATE_LAST}; // row
  lv_obj_set_grid_dsc_array(brightness_control, col_dsc, row_dsc);
  lv_obj_set_layout(brightness_control, LV_LAYOUT_GRID);

  // Add back button
  lv_obj_t *btn_back = lv_btn_create(brightness_control);
  lv_obj_set_size(btn_back, 100, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 3, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { 
                        MenuState *return_menu_ptr = (MenuState*)lv_event_get_user_data(e);
                        renderMenu(*return_menu_ptr); }, LV_EVENT_CLICKED, (void *)&return_menu);

  // Top label
  lv_obj_t *label = lv_label_create(brightness_control);
  lv_label_set_text(label, "Brightness");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
  lv_obj_set_grid_cell(label, LV_GRID_ALIGN_CENTER, 0, 3, LV_GRID_ALIGN_CENTER, 1, 1);

  // Brightness value label
  lv_obj_t *value_label = lv_label_create(brightness_control);
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", brightness);
  lv_label_set_text(value_label, buf);
  lv_obj_set_style_text_font(value_label, &lv_font_montserrat_36, 0);
  lv_obj_set_grid_cell(value_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  // Left arrow button (down)
  lv_obj_t *btn_left = lv_btn_create(brightness_control);
  lv_obj_set_size(btn_left, 80, 80);
  lv_obj_set_grid_cell(btn_left, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_t *left_label = lv_label_create(btn_left);
  lv_label_set_text(left_label, LV_SYMBOL_LEFT);
  lv_obj_center(left_label);
  lv_obj_add_event_cb(btn_left, brightness_down_event_handler, LV_EVENT_CLICKED, value_label);

  // Right arrow button (up)
  lv_obj_t *btn_right = lv_btn_create(brightness_control);
  lv_obj_set_size(btn_right, 80, 80);
  lv_obj_set_grid_cell(btn_right, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_t *right_label = lv_label_create(btn_right);
  lv_label_set_text(right_label, LV_SYMBOL_RIGHT);
  lv_obj_center(right_label);
  lv_obj_add_event_cb(btn_right, brightness_up_event_handler, LV_EVENT_CLICKED, value_label);
}