#include "settings_overlay.h"
#include "constants/constants.h"
#include "battery/battery_state.h"
#include "menu/menu.h"
#include <lvgl.h>
#include <state/state_store.h>
#include <life/life_counter.h>
#include <life/life_counter2P.h>
#include <helpers/animation_helpers.h>
#include <timer/timer.h>

extern lv_obj_t *settings_menu;
extern lv_obj_t *life_counter_container;
extern lv_obj_t *life_counter_container_2p;

// Use a static callback instead of a lambda
static void btn_life_event_cb(lv_event_t *e)
{
  renderMenu(MENU_LIFE_CONFIG);
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
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {70, 60, 60, 60, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(settings_menu, col_dsc, row_dsc);
  lv_obj_set_layout(settings_menu, LV_LAYOUT_GRID);
  lv_obj_set_scrollbar_mode(settings_menu, LV_SCROLLBAR_MODE_OFF); // Disable scrollbar

  // Back button
  lv_obj_t *btn_back = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_back, 100, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { renderMenu(MENU_CONTEXTUAL, false); }, LV_EVENT_CLICKED, NULL);

  // Start Life button
  lv_obj_t *btn_life = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_life, 120, 50);
  lv_obj_set_style_bg_color(btn_life, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_life, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 1, 1);
  lv_obj_t *lbl_life = lv_label_create(btn_life);
  lv_label_set_text(lbl_life, "Start Life");
  lv_obj_set_style_text_font(lbl_life, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_life);
  lv_obj_add_event_cb(btn_life, btn_life_event_cb, LV_EVENT_CLICKED, NULL);

  // Brightness button
  lv_obj_t *btn_brightness = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_brightness, 120, 50);
  lv_obj_set_style_bg_color(btn_brightness, LIGHTNING_BLUE_COLOR, LV_PART_MAIN);

  lv_obj_set_grid_cell(btn_brightness, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 1, 1);
  lv_obj_add_event_cb(btn_brightness, [](lv_event_t *e)
                      { renderMenu(MENU_BRIGHTNESS); }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_brightness = lv_label_create(btn_brightness);
  lv_label_set_text(lbl_brightness, "Brightness");
  lv_obj_set_style_text_font(lbl_brightness, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_brightness);

  // Amp Counter
  int amp_mode = player_store.getInt(KEY_AMP_MODE, 0);
  lv_obj_t *btn_amp_toggle = lv_btn_create(settings_menu);
  lv_obj_set_style_bg_color(btn_amp_toggle, (amp_mode ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
  lv_obj_set_size(btn_amp_toggle, 120, 50);
  lv_obj_set_grid_cell(btn_amp_toggle, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
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
    lv_obj_set_style_bg_color(btn, (new_mode ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
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
      lv_obj_clear_flag(amp_button, LV_OBJ_FLAG_HIDDEN);
    } }, LV_EVENT_CLICKED, NULL);

  // Timer Toggle button
  lv_obj_t *btn_timer_toggle = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_timer_toggle, 120, 50);
  lv_obj_set_style_bg_color(btn_timer_toggle, (player_store.getInt(KEY_SHOW_TIMER, 1) ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_timer_toggle, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_t *lbl_timer = lv_label_create(btn_timer_toggle);
  uint64_t show_timer = player_store.getInt(KEY_SHOW_TIMER, 0);
  lv_label_set_text(lbl_timer, (show_timer ? "Timer On" : "Timer Off"));
  lv_obj_set_style_text_font(lbl_timer, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_timer);
  lv_obj_add_event_cb(btn_timer_toggle, [](lv_event_t *e)
                      { 
                        // update the label text based on current state
                        uint64_t show_timer = toggle_show_timer();
                        lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
                        lv_obj_t *label = lv_obj_get_child(btn, 0);
                        lv_obj_set_style_bg_color(btn, (show_timer ? LIGHTNING_BLUE_COLOR : GRAY_COLOR), LV_PART_MAIN);
                        lv_label_set_text(label, (show_timer ? "Timer On" : "Timer Off"));
                        uint64_t life_counter_mode = player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
                        if (!show_timer)
                        {
                          teardown_timer(); // Clean up the timer overlay
                        } else {
                          lv_label_set_text(label, "Timer On");
                          // get reference to active life counter
                          lv_obj_t *active_counter = (life_counter_mode == PLAYER_MODE_ONE_PLAYER) ?
                            life_counter_container : life_counter_container_2p;
                          if (!active_counter)
                          {
                            printf("[renderSettingsOverlay] No active life counter found, cannot render timer\n");
                            return;
                          }
                          // Render the timer overlay on the active life counter
                          render_timer(active_counter);
                          if(player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER) == PLAYER_MODE_ONE_PLAYER)
                          {
                            lv_obj_set_grid_cell(timer_container, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
                          } else {
                            lv_obj_set_grid_cell(timer_container, LV_GRID_ALIGN_CENTER, 0, 5, LV_GRID_ALIGN_START, 2, 1);
                          }
                        } }, LV_EVENT_CLICKED, NULL);

  // Restart Device button
  lv_obj_t *btn_restart = lv_btn_create(settings_menu);
  lv_obj_set_size(btn_restart, 180, 50);
  lv_obj_set_style_bg_color(btn_restart, RED_COLOR, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_restart, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_END, 3, 1);
  lv_obj_t *lbl_restart = lv_label_create(btn_restart);
  lv_label_set_text(lbl_restart, "Reboot");
  lv_obj_set_style_text_font(lbl_restart, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_restart);
  lv_obj_add_event_cb(btn_restart, [](lv_event_t *e)
                      { esp_restart(); }, LV_EVENT_CLICKED, NULL);

  // Battery
  lv_obj_t *lbl_batt = lv_label_create(settings_menu);
  float pct = battery_get_percent();
  char batt_str[32];
  const char *bat_symbol = (pct < 15) ? LV_SYMBOL_BATTERY_EMPTY : (pct < 30) ? LV_SYMBOL_BATTERY_1
                                                              : (pct < 55)   ? LV_SYMBOL_BATTERY_2
                                                              : (pct < 80)   ? LV_SYMBOL_BATTERY_3
                                                                             : LV_SYMBOL_BATTERY_FULL;
  snprintf(batt_str, sizeof(batt_str), "%s %d%%", bat_symbol, (int)(pct + 0.5f));
  lv_label_set_text(lbl_batt, batt_str);
  lv_obj_set_style_text_font(lbl_batt, &lv_font_montserrat_20, 0);
  lv_obj_set_grid_cell(lbl_batt, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 4, 1);
}

void teardownSettingsOverlay()
{
  if (settings_menu)
  {
    lv_obj_del(settings_menu);
    settings_menu = nullptr;
  }
}