#include "start_life.h"
#include <lvgl.h>
#include <menu/menu.h>
#include "state/state_store.h"

extern lv_obj_t *start_life_menu;

struct SaveBtnData
{
  lv_obj_t *slider;
  lv_obj_t *lbl_current;
};

static void slider_handle_change_cb(lv_event_t *e)
{
  SaveBtnData *data = (SaveBtnData *)lv_event_get_user_data(e);
  int value = lv_slider_get_value(data->slider);
  // Update label with current value
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", value);
  lv_label_set_text(data->lbl_current, buf);
}

// Save button event callback
static void save_btn_event_cb(lv_event_t *e)
{
  SaveBtnData *data = (SaveBtnData *)lv_event_get_user_data(e);
  int value = lv_slider_get_value(data->slider);
  player_store.putInt(KEY_LIFE_MAX, value);
  char buf[16];
  snprintf(buf, sizeof(buf), "Current: %d", value);
  lv_label_set_text(data->lbl_current, buf);
  renderMenu(MENU_SETTINGS);
}

void renderStartLifeScreen()
{
  teardownStartLifeScreen();
  int stored_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);

  // Create the main menu object
  start_life_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(start_life_menu, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(start_life_menu, BLACK_COLOR, LV_PART_MAIN); // dark background
  lv_obj_set_style_bg_opa(start_life_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_opa(start_life_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(start_life_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_radius(start_life_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_pad_all(start_life_menu, 16, LV_PART_MAIN);

  // Define grid: 5 rows, 1 column
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {60, 60, 60, 40, LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(start_life_menu, col_dsc, row_dsc);
  lv_obj_set_layout(start_life_menu, LV_LAYOUT_GRID);

  // Back button
  lv_obj_t *btn_back = lv_btn_create(start_life_menu);
  lv_obj_set_size(btn_back, 100, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { renderMenu(MENU_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  // Current value label
  lv_obj_t *lbl_current = lv_label_create(start_life_menu);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", stored_life);
  lv_label_set_text(lbl_current, buf);
  lv_obj_set_style_text_font(lbl_current, &lv_font_montserrat_36, 0);
  lv_obj_set_style_text_color(lbl_current, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_current, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  // Slider
  lv_obj_t *slider = lv_slider_create(start_life_menu);
  lv_slider_set_range(slider, 0, DEFAULT_LIFE_MAX);
  lv_slider_set_value(slider, stored_life, LV_ANIM_OFF);
  lv_obj_set_width(slider, LV_HOR_RES - 100);
  lv_obj_set_style_pad_left(slider, 20, LV_PART_KNOB);  // Add padding to left end
  lv_obj_set_style_pad_right(slider, 20, LV_PART_KNOB); // Add padding to right end
  lv_obj_set_style_width(slider, 40, LV_PART_KNOB);     // Make knob larger
  lv_obj_set_style_height(slider, 40, LV_PART_KNOB);    // Make knob larger
  lv_obj_add_event_cb(slider, slider_handle_change_cb, LV_EVENT_VALUE_CHANGED, new SaveBtnData{slider, lbl_current});
  lv_obj_set_grid_cell(slider, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  // Save button
  lv_obj_t *btn_save = lv_btn_create(start_life_menu);
  lv_obj_set_size(btn_save, 80, 40);
  lv_obj_set_grid_cell(btn_save, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_t *lbl_save = lv_label_create(btn_save);
  lv_label_set_text(lbl_save, "Save");
  lv_obj_set_style_text_font(lbl_save, &lv_font_montserrat_16, 0);
  lv_obj_set_style_bg_color(btn_save, GREEN_COLOR, LV_PART_MAIN); // Green background
  lv_obj_center(lbl_save);
  SaveBtnData *save_data = new SaveBtnData{slider, lbl_current};
  lv_obj_add_event_cb(btn_save, save_btn_event_cb, LV_EVENT_CLICKED, save_data);
}

void teardownStartLifeScreen()
{
  if (start_life_menu)
  {
    lv_obj_del(start_life_menu);
    start_life_menu = nullptr;
  }
}