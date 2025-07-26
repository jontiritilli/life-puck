#include "settings_overlay.h"
#include "constants/constants.h"
#include "battery/battery_state.h"
#include "menu/menu.h"
#include <lvgl.h>
#include <state/state_store.h>
#include <life/life_counter.h>
#include <helpers/animation_helpers.h>

extern lv_obj_t *settings_menu;

// Use a static callback instead of a lambda
static void btn_life_event_cb(lv_event_t *e)
{
  renderMenu(MENU_START_LIFE);
}

// Draw settings overlay using LVGL
void renderSettingsOverlay()
{
  teardownSettingsOverlay();
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
  static lv_coord_t row_dsc[] = {60, 40, 40, 40, 40, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(settings_menu, col_dsc, row_dsc);
  lv_obj_set_layout(settings_menu, LV_LAYOUT_GRID);
  lv_obj_set_scrollbar_mode(settings_menu, LV_SCROLLBAR_MODE_OFF); // Disable scrollbar

  // Back button
  lv_obj_t *btn_back = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_back, 100, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { renderMenu(MENU_CONTEXTUAL); }, LV_EVENT_CLICKED, NULL);

  // Start Life button
  lv_obj_t *btn_life = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_life, 180, 40);
  lv_obj_set_grid_cell(btn_life, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_t *lbl_life = lv_label_create(btn_life);
  lv_label_set_text(lbl_life, "Start Life");
  lv_obj_set_style_text_font(lbl_life, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_life);
  lv_obj_add_event_cb(btn_life, btn_life_event_cb, LV_EVENT_CLICKED, NULL);

  // Brightness button
  lv_obj_t *btn_brightness = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_brightness, 180, 40);
  lv_obj_set_grid_cell(btn_brightness, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_add_event_cb(btn_brightness, [](lv_event_t *e)
                      { renderMenu(MENU_BRIGHTNESS); }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_brightness = lv_label_create(btn_brightness);
  lv_label_set_text(lbl_brightness, "Brightness");
  lv_obj_set_style_text_font(lbl_brightness, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_brightness);

  // Amp Counter
  int amp_mode = player_store.getInt(KEY_AMP_MODE, 0);
  lv_obj_t *btn_amp_toggle = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_amp_toggle, 180, 40);
  lv_obj_set_grid_cell(btn_amp_toggle, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_t *lbl_amp_label = lv_label_create(btn_amp_toggle);
  lv_label_set_text(lbl_amp_label, (amp_mode ? "Amp On" : "Amp Off")); // shows current state
  lv_obj_set_style_text_font(lbl_amp_label, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_amp_label);
  // Store the label pointer as user data for the callback
  lv_obj_add_event_cb(btn_amp_toggle, [](lv_event_t *e)
                      {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    int current = player_store.getInt(KEY_AMP_MODE, 0);
    int new_mode = !current;
    player_store.putInt(KEY_AMP_MODE, new_mode);
    lv_label_set_text(label, (new_mode ? "Amp On" : "Amp Off"));
    extern lv_obj_t *amp_button;
    if (amp_button && !new_mode)
    {
      // Clear amp state if amp mode is turned off
      clear_amp();
      // hide the amp button if it exists
      lv_obj_add_flag(amp_button, LV_OBJ_FLAG_HIDDEN);
    } else if (amp_button && new_mode)
    {
      // show the amp button if it exists
      lv_obj_set_style_opa(amp_button, LV_OPA_TRANSP, 0); // Start fully transparent
      lv_obj_clear_flag(amp_button, LV_OBJ_FLAG_HIDDEN);
      fade_in_obj(amp_button, 1000, 0, NULL); // Animate to visible
    } }, LV_EVENT_CLICKED, NULL);

  // Battery
  lv_obj_t *lbl_batt = lv_label_create(settings_menu);
  float pct = battery_get_percent();
  char batt_str[32];
  snprintf(batt_str, sizeof(batt_str), "Battery: %d%%", (int)(pct + 0.5f));
  lv_label_set_text(lbl_batt, batt_str);
  lv_obj_set_style_text_font(lbl_batt, &lv_font_montserrat_20, 0);
  lv_obj_set_grid_cell(lbl_batt, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
}

void teardownSettingsOverlay()
{
  if (settings_menu)
  {
    lv_obj_del(settings_menu);
    settings_menu = nullptr;
  }
}