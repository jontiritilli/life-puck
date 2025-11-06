#include "counters_settings.h"
#include "config.h"
#include "data/constants.h"
#include "data/themes.h"
#include "ui/screens/menu/menu.h"
#include "ui/helpers/gestures.h"
#include "ui/helpers/ui_helpers.h"
#include "ui/helpers/ui_constants.h"
#include "core/state_manager.h"
#include "../life/simple_counters.h"
#include <lvgl.h>

static lv_obj_t *counters_menu = nullptr;

void teardownCountersSettings()
{
  if (counters_menu)
  {
    lv_obj_del(counters_menu);
    counters_menu = nullptr;
  }

  // Show all enabled counter UI again
  for (int i = 0; i < 4; i++)
  {
    if (SimpleCounters::counters[i].container != nullptr && SimpleCounters::counters[i].enabled)
    {
      lv_obj_clear_flag(SimpleCounters::counters[i].container, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

void renderCountersSettings()
{
  // Clean up any existing menu
  teardownCountersSettings();

  // Hide all counter UI while in settings
  for (int i = 0; i < 4; i++)
  {
    if (SimpleCounters::counters[i].container != nullptr)
    {
      lv_obj_add_flag(SimpleCounters::counters[i].container, LV_OBJ_FLAG_HIDDEN);
    }
  }

  // Create main container using standardized helper
  counters_menu = ui_create_menu_container(lv_scr_act());

  // Grid Layout: Full width buttons (1 column, 7 rows)
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(counters_menu, col_dsc, row_dsc);
  lv_obj_set_layout(counters_menu, LV_LAYOUT_GRID);

  // Title (Row 0)
  ui_create_title(counters_menu, "Counter Settings", 0);

  // Back button (Row 1)
  lv_obj_t *btn_back = ui_create_back_button(counters_menu, 1, 0, [](lv_event_t *e)
                                             { renderMenu(MENU_SETTINGS); });

  // Spacer at bottom for scrolling (Row 6)
  lv_obj_t *spacer = lv_obj_create(counters_menu);
  lv_obj_set_size(spacer, 280, 50);
  lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_grid_cell(spacer, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 6, 1);

  // Counter toggle buttons (Rows 2-5)
  const char *slot_names[] = {"Top-Left", "Top-Right", "Bottom-Left", "Bottom-Right"};

  for (int i = 0; i < 4; i++)
  {
    // Create toggle button - same style as timer toggle
    lv_obj_t *btn_toggle = lv_btn_create(counters_menu);
    lv_obj_set_size(btn_toggle, 280, UI_BTN_HEIGHT_STANDARD);

    // Load state and set colors
    bool enabled = SimpleCounters::is_enabled(i);
    lv_obj_set_style_bg_color(btn_toggle, (enabled ? getThemeButtonColor() : UI_COLOR_DISABLED), LV_PART_MAIN);
    lv_obj_set_grid_cell(btn_toggle, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, i + 2, 1);

    // Button label
    lv_obj_t *lbl_toggle = lv_label_create(btn_toggle);
    char label_text[32];
    snprintf(label_text, sizeof(label_text), "%s: %s", slot_names[i], enabled ? "ON" : "OFF");
    lv_label_set_text(lbl_toggle, label_text);
    lv_obj_set_style_text_font(lbl_toggle, UI_FONT_TOGGLE, 0);
    lv_obj_set_style_text_color(lbl_toggle, (enabled ? getThemeTextColor() : UI_COLOR_TEXT_PRIMARY), 0);
    lv_obj_center(lbl_toggle);

    // Button event handler
    lv_obj_add_event_cb(btn_toggle, [](lv_event_t *e)
                        {
      lv_event_code_t code = lv_event_get_code(e);
      if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        
        // Get slot from user data
        int slot = (int)(intptr_t)lv_event_get_user_data(e);
        
        // Toggle state
        bool current_state = SimpleCounters::is_enabled(slot);
        if (current_state) {
          SimpleCounters::disable_counter(slot);
        } else {
          SimpleCounters::enable_counter(slot);
        }
        
        // Update button appearance
        bool new_state = SimpleCounters::is_enabled(slot);
        lv_obj_set_style_bg_color(btn, (new_state ? getThemeButtonColor() : UI_COLOR_DISABLED), LV_PART_MAIN);
        lv_obj_set_style_text_color(label, (new_state ? getThemeTextColor() : UI_COLOR_TEXT_PRIMARY), 0);
        
        // Update label text
        const char* slot_names[] = {"Top-Left", "Top-Right", "Bottom-Left", "Bottom-Right"};
        char new_text[32];
        snprintf(new_text, sizeof(new_text), "%s: %s", slot_names[slot], new_state ? "ON" : "OFF");
        lv_label_set_text(label, new_text);
      } }, LV_EVENT_ALL, (void *)(intptr_t)i);
  }
}