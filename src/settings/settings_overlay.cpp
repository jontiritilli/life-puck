#include "settings_overlay.h"
#include "constants/constants.h"
#include "battery/battery_state.h"
#include "menu/menu.h"
#include <lvgl.h>

// Gesture event callback for menu navigation
static void menu_gesture_event_cb(lv_event_t *e)
{
  printf("[menu_gesture_event_cb] Gesture event received\n");
  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
  if (dir == LV_DIR_RIGHT)
  {
    renderMenu(MENU_CONTEXTUAL);
  }
}

static void btn_back_event_cb(lv_event_t *e)
{
  renderMenu(MENU_CONTEXTUAL);
}

// Use a static callback instead of a lambda
static void btn_life_event_cb(lv_event_t *e)
{
  renderMenu(MENU_START_LIFE);
}

// Draw settings overlay using LVGL
void renderSettingsOverlay()
{
  extern lv_obj_t *settings_menu;
  settings_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(settings_menu, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(settings_menu, BLACK_COLOR, LV_PART_MAIN); // dark background
  lv_obj_set_style_bg_opa(settings_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_opa(settings_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(settings_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_radius(settings_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_pad_all(settings_menu, 16, LV_PART_MAIN);

  // Define grid: 9 rows, 1 column
  // Center the layout by using a single column and adjusting alignment
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 40, 40, 40, 40, 40, 40, 40, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(settings_menu, col_dsc, row_dsc);
  lv_obj_set_layout(settings_menu, LV_LAYOUT_GRID);
  lv_obj_set_scrollbar_mode(settings_menu, LV_SCROLLBAR_MODE_OFF); // Disable scrollbar

  // Back button
  lv_obj_t *btn_back = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_back, 80, 36);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_add_event_cb(btn_back, btn_back_event_cb, LV_EVENT_CLICKED, NULL);

  // Title
  lv_obj_t *lbl_title = lv_label_create(settings_menu);
  lv_label_set_text(lbl_title, "Settings");
  lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_24, 0);
  lv_obj_set_grid_cell(lbl_title, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  // Start Life button
  lv_obj_t *btn_life = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_life, 180, 40);
  lv_obj_set_grid_cell(btn_life, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_t *lbl_life = lv_label_create(btn_life);
  lv_label_set_text(lbl_life, "Start Life");
  lv_obj_set_style_text_font(lbl_life, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_life);
  lv_obj_add_event_cb(btn_life, btn_life_event_cb, LV_EVENT_CLICKED, NULL);

  // Brightness
  lv_obj_t *lbl_bright = lv_label_create(settings_menu);
  lv_label_set_text(lbl_bright, "Brightness:");
  lv_obj_set_style_text_font(lbl_bright, &lv_font_montserrat_16, 0);
  lv_obj_set_grid_cell(lbl_bright, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 3, 1);

  // Amp Counter
  lv_obj_t *lbl_amp = lv_label_create(settings_menu);
  lv_label_set_text(lbl_amp, "Amp Counter:");
  lv_obj_set_style_text_font(lbl_amp, &lv_font_montserrat_16, 0);
  lv_obj_set_grid_cell(lbl_amp, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 6, 1);

  // Battery
  lv_obj_t *lbl_batt = lv_label_create(settings_menu);
  float pct = battery_get_percent();
  char batt_str[32];
  snprintf(batt_str, sizeof(batt_str), "Battery: %.2f%%", pct);
  lv_label_set_text(lbl_batt, batt_str);
  lv_obj_set_style_text_font(lbl_batt, &lv_font_montserrat_16, 0);
  lv_obj_set_grid_cell(lbl_batt, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 7, 1);

  lv_obj_add_flag(settings_menu, LV_OBJ_FLAG_CLICKABLE);      // ensure overlay blocks clicks
  lv_obj_add_flag(settings_menu, LV_OBJ_FLAG_GESTURE_BUBBLE); // allow gestures to bubble up
  lv_obj_move_foreground(settings_menu);                      // bring to front

  // Add gesture event callback for settings menu only
  lv_obj_add_event_cb(settings_menu, menu_gesture_event_cb, LV_EVENT_GESTURE, NULL);
}