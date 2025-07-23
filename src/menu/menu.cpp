#include <Arduino.h>
#include <lvgl.h>
#include <esp_display_panel.hpp>
#include "menu.h"
#include "settings/start_life.h"
#include "constants/constants.h"
#include <math.h>
#include <battery/battery_state.h>
#include <settings/settings_overlay.h>
#include <life/life_counter.h>
#include <life/life_counter2P.h>
#include <helpers/tap_layer.h>
#include <state/state_store.h>
#include <history/history.h>
#include <helpers/event_grouper.h>
#include "gui_main.h"

extern esp_panel::board::Board *board;

// Global menu objects
lv_obj_t *contextual_menu = nullptr;
lv_obj_t *settings_menu = nullptr;
lv_obj_t *settings_start_life_menu = nullptr;
lv_obj_t *history_menu = nullptr;
int circle_diameter = SCREEN_WIDTH;
int circle_radius = circle_diameter / 2;

// Forward declarations
static void togglePlayerMode();
static void resetActiveCounter();
static void showHistoryOverlay();
static bool is_in_center_cancel_area(lv_event_t *e);
void clearMenus();
void renderMenu(MenuState menuType);
bool is_in_quadrant(lv_event_t *e, int angle_start, int angle_end);

void handleContextualSelection(ContextualQuadrant quadrant)
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
    renderMenu(MENU_HISTORY);
    break;
  }
}

static void togglePlayerMode()
{
  // Toggle player mode (1P/2P)
  player_store.putInt(KEY_PLAYER_MODE, player_store.getInt(KEY_PLAYER_MODE, 0) == 1 ? 2 : 1);
  printf("[togglePlayerMode] Player mode toggled to %d\n", player_store.getInt(KEY_PLAYER_MODE, 0));
  // Rerender main GUI (life counter)
  ui_init();
  renderMenu(MENU_NONE);
}

static void resetActiveCounter()
{
  int player_mode = player_store.getInt(KEY_PLAYER_MODE, 0);
  if (player_mode == 1)
  {
    reset_life();
    event_grouper.resetHistory();
  }
  else if (player_mode == 2)
  {
    reset_life_p1();
    reset_life_p2();
    event_grouper_p1.resetHistory();
    event_grouper_p2.resetHistory();
  }
  printf("[resetActiveCounter] Reset life counter and history for player mode %d\n", player_mode);
  clearMenus();
}

void clearMenus()
{
  if (contextual_menu)
  {
    lv_obj_del(contextual_menu);
    contextual_menu = nullptr;
  }
  if (settings_menu)
  {
    lv_obj_del(settings_menu);
    settings_menu = nullptr;
  }
  if (settings_start_life_menu)
  {
    lv_obj_del(settings_start_life_menu);
    settings_start_life_menu = nullptr;
  }
  if (history_menu)
  {
    lv_obj_del(history_menu);
    history_menu = nullptr;
  }
}

static void contextual_btn_event_cb(lv_event_t *e)
{
  ContextualQuadrant quadrant = (ContextualQuadrant)(intptr_t)lv_event_get_user_data(e);
  handleContextualSelection(quadrant);
}

// Draw contextual menu overlay with 4 quadrants using LVGL
void renderContextualMenuOverlay()
{
  clearMenus();
  // Make the overlay a true circle, centered on the screen
  int circle_diameter = (SCREEN_WIDTH < SCREEN_HEIGHT ? SCREEN_WIDTH : SCREEN_HEIGHT) + 5; // Increase size by 5 pixels
  int circle_radius = circle_diameter / 2;
  int circle_x = (SCREEN_WIDTH - circle_diameter) / 2;  // Center the circle horizontally
  int circle_y = (SCREEN_HEIGHT - circle_diameter) / 2; // Center the circle vertically
  contextual_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(contextual_menu, circle_diameter, circle_diameter);
  lv_obj_set_style_bg_color(contextual_menu, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(contextual_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(contextual_menu, 0, LV_PART_MAIN);
  lv_obj_set_style_outline_width(contextual_menu, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(contextual_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_align(contextual_menu, LV_ALIGN_CENTER, 0, 0);
  lv_obj_remove_flag(contextual_menu, LV_OBJ_FLAG_SCROLLABLE);

  // Use a slightly smaller ring for the menu ring inside the overlay circle
  int ring_diameter = circle_diameter;
  int ring_radius = ring_diameter / 2;
  int cx = circle_radius;
  int cy = circle_radius;

  // Add quadrant labels directly to the overlay for visual feedback
  lv_obj_t *lbl_tl = lv_label_create(contextual_menu);
  lv_label_set_text(lbl_tl, LV_SYMBOL_SETTINGS);
  lv_obj_set_style_text_font(lbl_tl, &lv_font_montserrat_30, 0);
  lv_obj_align(lbl_tl, LV_ALIGN_CENTER, -ring_radius / 2, -ring_radius / 2);

  lv_obj_t *lbl_tr = lv_label_create(contextual_menu);
  String lbl_text = player_store.getInt(KEY_PLAYER_MODE, 0) == 1 ? "2P" : "1P";
  lv_label_set_text(lbl_tr, lbl_text.c_str());
  lv_obj_set_style_text_font(lbl_tr, &lv_font_montserrat_20, 0);
  lv_obj_align(lbl_tr, LV_ALIGN_CENTER, ring_radius / 2, -ring_radius / 2);

  lv_obj_t *lbl_bl = lv_label_create(contextual_menu);
  lv_label_set_text(lbl_bl, LV_SYMBOL_REFRESH);
  lv_obj_set_style_text_font(lbl_bl, &lv_font_montserrat_30, 0);
  lv_obj_align(lbl_bl, LV_ALIGN_CENTER, -ring_radius / 2, ring_radius / 2);

  lv_obj_t *lbl_br = lv_label_create(contextual_menu);
  lv_label_set_text(lbl_br, LV_SYMBOL_LIST);
  lv_obj_set_style_text_font(lbl_br, &lv_font_montserrat_30, 0);
  lv_obj_align(lbl_br, LV_ALIGN_CENTER, ring_radius / 2, ring_radius / 2);

  // Make the overlay itself clickable for quadrant hit detection
  lv_obj_add_flag(contextual_menu, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(contextual_menu, [](lv_event_t *e)
                      {
    // Prevent quadrant actions if tap is in the center cancel area
    if (is_in_center_cancel_area(e)) {
      // Tap is in the center cancel area; let the button handle it
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
    } }, LV_EVENT_CLICKED, NULL);

  // Center cancel area (empty circle)
  int hole_diameter = (ring_radius / 3) * 2; // half the previous size
  lv_obj_t *center_cancel = lv_btn_create(contextual_menu);
  lv_obj_set_size(center_cancel, hole_diameter, hole_diameter);
  lv_obj_align(center_cancel, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_radius(center_cancel, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(center_cancel, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(center_cancel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_opa(center_cancel, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_color(center_cancel, lv_color_white(), 0);
  lv_obj_add_event_cb(center_cancel, [](lv_event_t *e)
                      {
    // Close menu on tap in center
    renderMenu(MENU_NONE); }, LV_EVENT_CLICKED, NULL);
  // Label for center cancel
  lv_obj_t *lbl_cancel = lv_label_create(center_cancel);
  lv_label_set_text(lbl_cancel, LV_SYMBOL_OK);
  lv_obj_set_style_text_font(lbl_cancel, &lv_font_montserrat_30, 0);
  lv_obj_center(lbl_cancel); // Center the label in the cancel button
}

// Update renderMenu to use LVGL overlays
void renderMenu(MenuState menuType)
{
  clearMenus();
  switch (menuType)
  {
  case MENU_CONTEXTUAL:
    renderContextualMenuOverlay();
    break;
  case MENU_SETTINGS:
    renderSettingsOverlay();
    break;
  case MENU_START_LIFE:
    renderStartLifeScreen();
    break;
  case MENU_HISTORY:
    renderHistoryOverlay();
    break;
  default:
    break;
  }
}
// Helper for center cancel area hit detection
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

// Helper for quadrant hit detection
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