#include "start_life.h"
#include <lvgl.h>
#include <menu/menu.h>
#include "state/state_store.h"

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
  snprintf(buf, sizeof(buf), "Current: %d", value);
  lv_label_set_text(data->lbl_current, buf);
}

static void back_gesture_event_cb(lv_event_t *e)
{
  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
  if (dir == LV_DIR_LEFT)
  {
    // Go back to settings overlay
    renderMenu(MENU_SETTINGS);
  }
}

static void exit_btn_event_cb(lv_event_t *e)
{
  renderMenu(MENU_SETTINGS);
}

// Save button event callback
static void save_btn_event_cb(lv_event_t *e)
{
  SaveBtnData *data = (SaveBtnData *)lv_event_get_user_data(e);
  int value = lv_slider_get_value(data->slider);
  player_store.setLife(value);
  char buf[16];
  snprintf(buf, sizeof(buf), "Current: %d", value);
  lv_label_set_text(data->lbl_current, buf);
  renderMenu(MENU_SETTINGS);
}

void renderStartLifeScreen()
{
  extern lv_obj_t *settings_start_life_menu;
  settings_start_life_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(settings_start_life_menu, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(settings_start_life_menu, BLACK_COLOR, LV_PART_MAIN); // dark background
  lv_obj_add_event_cb(settings_start_life_menu, back_gesture_event_cb, LV_EVENT_GESTURE, NULL);

  // Define grid: 5 rows, 1 column
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {50, 40, 60, 60, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(settings_start_life_menu, col_dsc, row_dsc);
  lv_obj_set_layout(settings_start_life_menu, LV_LAYOUT_GRID);

  // Title label
  lv_obj_t *lbl_title = lv_label_create(settings_start_life_menu);
  lv_label_set_text(lbl_title, "Set Starting Life");
  lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_24, 0);
  lv_obj_set_grid_cell(lbl_title, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  // Current value label
  lv_obj_t *lbl_current = lv_label_create(settings_start_life_menu);
  int stored_life = player_store.getLife(LIFE_STD_START);
  char buf[16];
  snprintf(buf, sizeof(buf), "Current: %d", stored_life);
  lv_label_set_text(lbl_current, buf);
  lv_obj_set_style_text_font(lbl_current, &lv_font_montserrat_16, 0);
  lv_obj_set_grid_cell(lbl_current, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  // Slider
  lv_obj_t *slider = lv_slider_create(settings_start_life_menu);
  lv_slider_set_range(slider, 0, LIFE_STD_START);
  lv_slider_set_value(slider, stored_life, LV_ANIM_OFF);
  lv_obj_set_width(slider, LV_HOR_RES - 150);
  // onchange callback to update label
  lv_obj_add_event_cb(slider, slider_handle_change_cb, LV_EVENT_VALUE_CHANGED, new SaveBtnData{slider, lbl_current});
  lv_obj_set_style_text_font(lbl_current, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(lbl_current, lv_color_white(), 0);
  lv_obj_set_grid_cell(slider, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  // Save button
  lv_obj_t *btn_save = lv_btn_create(settings_start_life_menu);
  lv_obj_set_size(btn_save, 120, 40);
  lv_obj_set_grid_cell(btn_save, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_t *lbl_save = lv_label_create(btn_save);
  lv_label_set_text(lbl_save, "Save");
  lv_obj_set_style_text_font(lbl_save, &lv_font_montserrat_16, 0);
  lv_obj_set_style_bg_color(btn_save, GREEN_COLOR, LV_PART_MAIN); // Green background
  lv_obj_center(lbl_save);
  SaveBtnData *save_data = new SaveBtnData{slider, lbl_current};
  lv_obj_add_event_cb(btn_save, save_btn_event_cb, LV_EVENT_CLICKED, save_data);

  // Exit button
  lv_obj_t *btn_exit = lv_btn_create(settings_start_life_menu);
  lv_obj_set_size(btn_exit, 120, 40);
  lv_obj_set_grid_cell(btn_exit, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
  lv_obj_t *lbl_exit = lv_label_create(btn_exit);
  lv_label_set_text(lbl_exit, "Back");
  lv_obj_set_style_text_font(lbl_exit, &lv_font_montserrat_16, 0);
  lv_obj_set_style_bg_color(btn_exit, RED_COLOR, LV_PART_MAIN); // Red background
  lv_obj_center(lbl_exit);

  // exit button callback
  lv_obj_add_event_cb(btn_exit, exit_btn_event_cb, LV_EVENT_CLICKED, NULL);
}
