// ============================================
// Own Header (first!)
// ============================================
#include "preset_editor.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>
#include <cstring>
#include <cstdlib>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// UI Screens (same folder)
// ============================================
#include "menu.h"

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/ui_helpers.h"
#include "ui/helpers/ui_constants.h"

// ============================================
// Data Layer
// ============================================
#include "config.h"
#include "data/constants.h"
#include "data/tcg_presets.h"
#include "data/themes.h"

// ============================================
// Hardware Dependencies
// ============================================
#include "hardware/touch/Touch_CST816.h"

static lv_obj_t *preset_editor_menu = nullptr;
static lv_obj_t *preset_editor_swipe_layer = nullptr;
static lv_obj_t *name_popup = nullptr;
static lv_obj_t *values_popup = nullptr;
static lv_obj_t *ta_name = nullptr;
static lv_obj_t *keyboard = nullptr;

// Keyboard Mode Tracking
static bool keyboard_letter_mode = true;

struct SharedInputState
{
  lv_obj_t *ta;
  lv_obj_t *kb;
  int *current_var;
  lv_obj_t *current_label;
};

static int temp_life = 0;
static int temp_small = 0;
static int temp_large = 0;
static int temp_theme = 1; // Default theme
static int current_preset_idx = -1;

static void show_name_popup(int preset_idx);
static void show_values_popup(int preset_idx);
static void close_name_popup();
static void close_values_popup();
static const char *getThemeName(int theme_id);
static void save_preset_to_storage(int preset_idx);
static void load_presets_from_storage();

/**
 * @brief Get theme name from theme ID
 *
 * @param theme_id Theme ID (0=Off, 1=Yugioh, 2=Magic, 3=Pokemon, 4=Default, 5=FAB)
 * @return const char* Theme name string
 */
static const char *getThemeName(int theme_id)
{
  switch (theme_id)
  {
  case 0:
    return "Off";
  case 1:
    return "Yugioh";
  case 2:
    return "Magic";
  case 3:
    return "Pokemon";
  case 4:
    return "Default";
  case 5:
    return "FAB";
  default:
    return "Off";
  }
}

static const char *kb_map[] = {
    "7", "8", "9", "\n",
    "4", "5", "6", "\n",
    "1", "2", "3", "\n",
    LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_OK, NULL};

static const lv_buttonmatrix_ctrl_t kb_ctrl[] = {
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1};

// Keyboard Layouts
static const char *btnm_map_letters[] = {
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
    "z", "x", "c", "v", "b", "n", "m", "123", "\n",
    " ", LV_SYMBOL_BACKSPACE, LV_SYMBOL_OK, ""};

static const char *btnm_map_numbers[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
    "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "\n",
    "-", "_", "+", "=", "[", "]", "{", "}", "ABC", "\n",
    " ", LV_SYMBOL_BACKSPACE, LV_SYMBOL_OK, ""};

void renderPresetEditorMenu()
{
  if (preset_editor_menu)
  {
    lv_obj_del(preset_editor_menu);
  }

  if (preset_editor_swipe_layer)
  {
    lv_obj_del(preset_editor_swipe_layer);
    preset_editor_swipe_layer = nullptr;
  }

  load_presets_from_storage();

  preset_editor_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(preset_editor_menu, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 30);
  lv_obj_center(preset_editor_menu);
  lv_obj_set_style_bg_color(preset_editor_menu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(preset_editor_menu, 15, 0);
  lv_obj_set_style_border_width(preset_editor_menu, 0, 0);
  // Grid Layout: 1 column, 13 rows (title, back, 10 presets, spacer)
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(preset_editor_menu, col_dsc, row_dsc);
  lv_obj_set_layout(preset_editor_menu, LV_LAYOUT_GRID);
  lv_obj_set_scrollbar_mode(preset_editor_menu, LV_SCROLLBAR_MODE_AUTO);

  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled)
  {
    extern struct CST816_Touch touch_data; // Access RAW touch data for accurate swipe detection

    lv_obj_add_event_cb(preset_editor_menu, [](lv_event_t *e)
                        {
      extern struct CST816_Touch touch_data; // Re-declare inside lambda
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

  // Title (Row 0)
  ui_create_title(preset_editor_menu, "PRESETS", 0);

  // Back button (Row 1)
  ui_create_back_button(preset_editor_menu, 1, 0, [](lv_event_t *e)
                        { renderMenu(MENU_SETTINGS); });

  // Preset rows (Rows 2-11)
  for (int i = 0; i < 10; i++)
  {
    lv_obj_t *row = lv_obj_create(preset_editor_menu);
    lv_obj_set_size(row, 280, UI_BTN_HEIGHT_STANDARD);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(0x333333), 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_grid_cell(row, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, i + 2, 1);
    lv_obj_set_user_data(row, (void *)(intptr_t)i);

    lv_obj_t *lbl = lv_label_create(row);
    char buf[64];
    snprintf(buf, sizeof(buf), "%s (%d)", TCG_PRESETS[i].name, TCG_PRESETS[i].starting_life);
    lv_label_set_text(lbl, buf);
    lv_obj_set_style_text_font(lbl, UI_FONT_TOGGLE, 0);
    lv_obj_center(lbl);

    lv_obj_add_event_cb(row, [](lv_event_t *e)
                        {
      if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *target = (lv_obj_t*)lv_event_get_target(e);
        int idx = (int)(intptr_t)lv_obj_get_user_data(target);
        show_name_popup(idx);
      } }, LV_EVENT_CLICKED, NULL);
  }

  // Extra Spacer for Scrolling (Row 12)
  lv_obj_t *extra_spacer = lv_obj_create(preset_editor_menu);
  lv_obj_set_size(extra_spacer, 10, 50);
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 12, 1);
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);
}

static void show_name_popup(int preset_idx)
{
  close_name_popup();
  current_preset_idx = preset_idx;
  keyboard_letter_mode = true; // Reset to letters

  name_popup = lv_obj_create(lv_scr_act());
  lv_obj_set_size(name_popup, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_align(name_popup, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_set_style_bg_color(name_popup, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_width(name_popup, 0, 0);
  lv_obj_set_style_radius(name_popup, 0, 0);
  lv_obj_clear_flag(name_popup, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *lbl_title = lv_label_create(name_popup);
  lv_label_set_text(lbl_title, "Name:");
  lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(lbl_title, LIGHTNING_BLUE_COLOR, 0);
  lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 10);

  ta_name = lv_textarea_create(name_popup);
  lv_obj_set_size(ta_name, SCREEN_WIDTH - 40, 50);
  lv_obj_align(ta_name, LV_ALIGN_TOP_MID, 0, 45);
  lv_textarea_set_text(ta_name, TCG_PRESETS[preset_idx].name);
  lv_textarea_set_max_length(ta_name, 20);
  lv_textarea_set_one_line(ta_name, true);
  lv_obj_set_style_text_font(ta_name, &lv_font_montserrat_24, 0);

  // Create Button Matrix Keyboard
  keyboard = lv_btnmatrix_create(name_popup);
  lv_btnmatrix_set_map(keyboard, btnm_map_letters); // Start with letters
  lv_btnmatrix_set_btn_width(keyboard, 30, 2);      // Space wider
  lv_btnmatrix_set_btn_width(keyboard, 31, 2);      // Backspace wider
  lv_btnmatrix_set_btn_width(keyboard, 32, 2);      // Enter wider

  lv_obj_set_size(keyboard, SCREEN_WIDTH, 180);
  lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, -50);
  lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x2C2C2C), 0);

  // Event Handler for Button Matrix
  lv_obj_add_event_cb(keyboard, [](lv_event_t *e)
                      {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
      lv_obj_t *btnm = (lv_obj_t*)lv_event_get_target(e);
      uint16_t btn_id = lv_btnmatrix_get_selected_btn(btnm);
      const char *btn_text = lv_btnmatrix_get_btn_text(btnm, btn_id);
      
      if (btn_text) {
        if (strcmp(btn_text, LV_SYMBOL_BACKSPACE) == 0) {
          // Backspace: Delete last character
          const char *current_text = lv_textarea_get_text(ta_name);
          size_t len = strlen(current_text);
          if (len > 0) {
            char new_text[21];
            strncpy(new_text, current_text, len - 1);
            new_text[len - 1] = '\0';
            lv_textarea_set_text(ta_name, new_text);
          }
        } else if (strcmp(btn_text, LV_SYMBOL_OK) == 0) {
          // Enter: Go to next step
          close_name_popup();
          show_values_popup(current_preset_idx);
        } else if (strcmp(btn_text, "123") == 0) {
          // Switch to numbers mode
          keyboard_letter_mode = false;
          lv_btnmatrix_set_map(btnm, btnm_map_numbers);
          lv_btnmatrix_set_btn_width(btnm, 30, 2);  // Space
          lv_btnmatrix_set_btn_width(btnm, 31, 2);  // Backspace
          lv_btnmatrix_set_btn_width(btnm, 32, 2);  // Enter
          printf("[ButtonMatrix] Switched to numbers mode\n");
        } else if (strcmp(btn_text, "ABC") == 0) {
          // Switch back to letters mode
          keyboard_letter_mode = true;
          lv_btnmatrix_set_map(btnm, btnm_map_letters);
          lv_btnmatrix_set_btn_width(btnm, 30, 2);  // Space
          lv_btnmatrix_set_btn_width(btnm, 31, 2);  // Backspace
          lv_btnmatrix_set_btn_width(btnm, 32, 2);  // Enter
          printf("[ButtonMatrix] Switched to letters mode\n");
        } else if (strcmp(btn_text, " ") == 0) {
          // Add space
          const char *current_text = lv_textarea_get_text(ta_name);
          if (strlen(current_text) < 20) {
            char new_text[22];
            snprintf(new_text, sizeof(new_text), "%s ", current_text);
            lv_textarea_set_text(ta_name, new_text);
          }
        } else {
          // Add normal character
          const char *current_text = lv_textarea_get_text(ta_name);
          if (strlen(current_text) < 20) {
            char new_text[22];
            snprintf(new_text, sizeof(new_text), "%s%s", current_text, btn_text);
            lv_textarea_set_text(ta_name, new_text);
          }
        }
      }
    } }, LV_EVENT_ALL, NULL);

  lv_obj_add_event_cb(ta_name, [](lv_event_t *ev)
                      {
    lv_event_code_t code = lv_event_get_code(ev);
    if (code == LV_EVENT_VALUE_CHANGED) {
      lv_obj_t *ta_src = (lv_obj_t*)lv_event_get_target(ev);
      const char *text = lv_textarea_get_text(ta_src);
      if (strlen(text) > 20) {
        char limited[21];
        strncpy(limited, text, 20);
        limited[20] = '\0';
        lv_textarea_set_text(ta_src, limited);
      }
    }
    else if (code == LV_EVENT_READY) {
      lv_obj_t *ta_src = (lv_obj_t*)lv_event_get_target(ev);
      const char *name = lv_textarea_get_text(ta_src);
      char limited[21];
      strncpy(limited, name, 20);
      limited[20] = '\0';
      strncpy(TCG_PRESETS[current_preset_idx].name, limited, sizeof(TCG_PRESETS[current_preset_idx].name) - 1);
      TCG_PRESETS[current_preset_idx].name[sizeof(TCG_PRESETS[current_preset_idx].name) - 1] = '\0';
      close_name_popup();
      show_values_popup(current_preset_idx);
    } }, LV_EVENT_ALL, NULL);

  lv_obj_t *btn_apply = lv_btn_create(name_popup);
  lv_obj_set_size(btn_apply, 120, 45);
  lv_obj_align(btn_apply, LV_ALIGN_BOTTOM_MID, 0, -3);
  lv_obj_set_style_bg_color(btn_apply, lv_color_hex(0x00AA00), 0);
  lv_obj_set_user_data(btn_apply, (void *)(intptr_t)preset_idx);
  lv_obj_add_event_cb(btn_apply, [](lv_event_t *e)
                      {
    lv_obj_t *target = (lv_obj_t*)lv_event_get_target(e);
    int idx = (int)(intptr_t)lv_obj_get_user_data(target);
    
    const char *name = lv_textarea_get_text(ta_name);
    char limited[21];
    strncpy(limited, name, 20);
    limited[20] = '\0';
    strncpy(TCG_PRESETS[idx].name, limited, sizeof(TCG_PRESETS[idx].name) - 1);
    TCG_PRESETS[idx].name[sizeof(TCG_PRESETS[idx].name) - 1] = '\0';
    
    close_name_popup();
    show_values_popup(idx); }, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_apply = lv_label_create(btn_apply);
  lv_label_set_text(lbl_apply, "Apply");
  lv_obj_set_style_text_font(lbl_apply, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_apply);
}

static void open_input_cb(lv_event_t *e)
{
  SharedInputState *state = (SharedInputState *)lv_event_get_user_data(e);
  lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
  int *var = (int *)lv_obj_get_user_data(btn);
  lv_obj_t *lbl = lv_obj_get_child(btn, 0);

  lv_obj_add_flag(state->ta, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(state->kb, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_textarea(state->kb, NULL);
  state->current_var = nullptr;
  state->current_label = nullptr;

  char buf[16];
  snprintf(buf, sizeof(buf), "%d", *var);
  lv_textarea_set_text(state->ta, buf);
  state->current_var = var;
  state->current_label = lbl;
  lv_obj_clear_flag(state->ta, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(state->kb, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_textarea(state->kb, state->ta);
  lv_obj_move_foreground(state->kb);
}

static void shared_ta_event_cb(lv_event_t *e)
{
  SharedInputState *state = (SharedInputState *)lv_event_get_user_data(e);
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED && state->current_var && state->current_label)
  {
    int value = atoi(lv_textarea_get_text(state->ta));
    if (value >= 0 && value < 10000)
    {
      if (state->current_var == &temp_life)
      {
        if (value > 9999)
        {
          value = 9999;
          char buf[16];
          snprintf(buf, sizeof(buf), "%d", value);
          lv_textarea_set_text(state->ta, buf);
        }
      }
      else if (state->current_var == &temp_small || state->current_var == &temp_large)
      {
        if (value > 9999)
        {
          value = 9999;
          char buf[16];
          snprintf(buf, sizeof(buf), "%d", value);
          lv_textarea_set_text(state->ta, buf);
        }
      }

      *(state->current_var) = value;
      char buf[16];
      snprintf(buf, sizeof(buf), "%d", value);
      lv_label_set_text(state->current_label, buf);
    }
  }
  else if (code == LV_EVENT_READY && state->current_var && state->current_label)
  {
    int value = atoi(lv_textarea_get_text(state->ta));
    if (value >= 0)
    {
      if (state->current_var == &temp_life)
      {
        if (value > 9999)
        {
          value = 9999;
        }
      }
      else if (state->current_var == &temp_small || state->current_var == &temp_large)
      {
        if (value > 9999)
        {
          value = 9999;
        }
      }

      *(state->current_var) = value;
      char buf[16];
      snprintf(buf, sizeof(buf), "%d", value);
      lv_label_set_text(state->current_label, buf);
    }
  }
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
  {
    lv_obj_add_flag(state->ta, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(state->kb, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(state->kb, NULL);
    state->current_var = nullptr;
    state->current_label = nullptr;
  }
}

static void show_values_popup(int preset_idx)
{
  close_values_popup();
  current_preset_idx = preset_idx;

  temp_life = TCG_PRESETS[preset_idx].starting_life;
  temp_small = TCG_PRESETS[preset_idx].small_step;
  temp_large = TCG_PRESETS[preset_idx].large_step;

  // Load theme from NVS (user override) or config.h default
  char theme_key[20];
  snprintf(theme_key, sizeof(theme_key), "preset_%d_theme", preset_idx);

  // Get default theme for this preset from config.h
  int default_theme;
  switch (preset_idx)
  {
  case 0:
    default_theme = PRESET_0_THEME;
    break; // MTG Standard → Magic (2)
  case 1:
    default_theme = PRESET_1_THEME;
    break; // MTG Commander → Magic (2)
  case 2:
    default_theme = PRESET_2_THEME;
    break; // Pokemon TCG → Pokemon (3)
  case 3:
    default_theme = PRESET_3_THEME;
    break; // Yu-Gi-Oh! → Yugioh (1)
  case 4:
    default_theme = PRESET_4_THEME;
    break; // Flesh & Blood → (from config.h)
  case 5:
    default_theme = PRESET_5_THEME;
    break; // Lorcana → (from config.h)
  case 6:
    default_theme = PRESET_6_THEME;
    break; // One Piece TCG → (from config.h)
  case 7:
    default_theme = PRESET_7_THEME;
    break; // Custom 8 → (from config.h)
  case 8:
    default_theme = PRESET_8_THEME;
    break; // Custom 9 → (from config.h)
  case 9:
    default_theme = PRESET_9_THEME;
    break; // Custom 10 → (from config.h)
  default:
    default_theme = THEME_YUGIOH;
    break; // Fallback: Yugioh (1)
  }

  // Load from NVS (user override) or use config.h default
  temp_theme = player_store.getInt(theme_key, default_theme);

  values_popup = lv_obj_create(lv_scr_act());
  lv_obj_set_size(values_popup, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(values_popup, BLACK_COLOR, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(values_popup, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_opa(values_popup, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(values_popup, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_radius(values_popup, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_pad_all(values_popup, 16, LV_PART_MAIN);

  // Enable vertical scrolling for Theme button visibility
  lv_obj_set_scroll_dir(values_popup, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(values_popup, LV_SCROLLBAR_MODE_AUTO);

  // Reduced row heights to fit better: 60px rows + extra spacer row for scrolling
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {60, 60, 60, 60, 60, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // 5 rows + spacer
  lv_obj_set_grid_dsc_array(values_popup, col_dsc, row_dsc);
  lv_obj_set_layout(values_popup, LV_LAYOUT_GRID);

  lv_obj_t *btn_apply = lv_btn_create(values_popup);
  lv_obj_set_size(btn_apply, 100, 60);
  lv_obj_set_style_bg_color(btn_apply, lv_color_hex(0x00AA00), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_apply, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_t *lbl_apply = lv_label_create(btn_apply);
  lv_label_set_text(lbl_apply, LV_SYMBOL_OK " Apply");
  lv_obj_set_style_text_font(lbl_apply, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_apply);
  lv_obj_set_user_data(btn_apply, (void *)(intptr_t)preset_idx);
  lv_obj_add_event_cb(btn_apply, [](lv_event_t *e)
                      { 
                        lv_obj_t *target = (lv_obj_t*)lv_event_get_target(e);
                        int idx = (int)(intptr_t)lv_obj_get_user_data(target);
                        
                        TCG_PRESETS[idx].starting_life = temp_life;
                        TCG_PRESETS[idx].small_step = temp_small;
                        TCG_PRESETS[idx].large_step = temp_large;
                        
                        // Save theme for ALL presets (0-9)
                        char theme_key[20];
                        snprintf(theme_key, sizeof(theme_key), "preset_%d_theme", idx);
                        player_store.putInt(theme_key, temp_theme);

                        save_preset_to_storage(idx);

                        // If this is the currently active preset AND theme mode is Automatic, refresh the life counter theme
                        int active_preset = player_store.getInt("preset_idx", 0);
                        int theme_mode = player_store.getInt(KEY_THEME_MODE, THEME_MODE_OFF);
                        if (idx == active_preset && theme_mode == THEME_MODE_AUTOMATIC) {
                            extern void refresh_life_counter_theme();
                            refresh_life_counter_theme();
                        }

                        close_values_popup();
                        renderPresetEditorMenu(); }, LV_EVENT_CLICKED, NULL);

  lv_obj_t *lbl_life_max = lv_label_create(values_popup);
  lv_label_set_text(lbl_life_max, "Life Start");
  lv_obj_set_style_text_font(lbl_life_max, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lbl_life_max, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_life_max, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_t *btn_max_life = lv_btn_create(values_popup);
  lv_obj_set_size(btn_max_life, 80, 40);
  lv_obj_set_style_bg_color(btn_max_life, lv_color_hex(0x0070FF), LV_PART_MAIN); // Blue
  lv_obj_set_style_text_font(btn_max_life, &lv_font_montserrat_24, 0);
  lv_obj_set_grid_cell(btn_max_life, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_t *lbl_max_life_val = lv_label_create(btn_max_life);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", temp_life);
  lv_label_set_text(lbl_max_life_val, buf);
  lv_obj_center(lbl_max_life_val);

  lv_obj_t *lbl_small_step = lv_label_create(values_popup);
  lv_label_set_text(lbl_small_step, "Small Step");
  lv_obj_set_style_text_font(lbl_small_step, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lbl_small_step, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_small_step, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_t *btn_small_step = lv_btn_create(values_popup);
  lv_obj_set_size(btn_small_step, 80, 40);
  lv_obj_set_style_bg_color(btn_small_step, lv_color_hex(0x0070FF), LV_PART_MAIN); // Blue
  lv_obj_set_style_text_font(btn_small_step, &lv_font_montserrat_24, 0);
  lv_obj_set_grid_cell(btn_small_step, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_t *lbl_small_step_val = lv_label_create(btn_small_step);
  snprintf(buf, sizeof(buf), "%d", temp_small);
  lv_label_set_text(lbl_small_step_val, buf);
  lv_obj_center(lbl_small_step_val);

  lv_obj_t *lbl_large_step = lv_label_create(values_popup);
  lv_label_set_text(lbl_large_step, "Big Step");
  lv_obj_set_style_text_font(lbl_large_step, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lbl_large_step, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_large_step, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);

  lv_obj_t *btn_large_step = lv_btn_create(values_popup);
  lv_obj_set_size(btn_large_step, 80, 40);
  lv_obj_set_style_bg_color(btn_large_step, lv_color_hex(0x0070FF), LV_PART_MAIN); // Blue
  lv_obj_set_style_text_font(btn_large_step, &lv_font_montserrat_24, 0);
  lv_obj_set_grid_cell(btn_large_step, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_t *lbl_large_step_val = lv_label_create(btn_large_step);
  snprintf(buf, sizeof(buf), "%d", temp_large);
  lv_label_set_text(lbl_large_step_val, buf);
  lv_obj_center(lbl_large_step_val);

  // Theme Button (available for ALL presets)
  lv_obj_t *lbl_theme = lv_label_create(values_popup);
  lv_label_set_text(lbl_theme, "Theme");
  lv_obj_set_style_text_font(lbl_theme, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lbl_theme, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_theme, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);

  lv_obj_t *btn_theme = lv_btn_create(values_popup);
  lv_obj_set_size(btn_theme, 120, 40);
  lv_obj_set_style_bg_color(btn_theme, lv_color_hex(0x0070FF), LV_PART_MAIN); // Blue like other buttons
  lv_obj_set_style_text_font(btn_theme, &lv_font_montserrat_20, 0);
  lv_obj_set_grid_cell(btn_theme, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 4, 1);
  lv_obj_t *lbl_theme_val = lv_label_create(btn_theme);
  lv_label_set_text(lbl_theme_val, getThemeName(temp_theme));
  lv_obj_set_style_text_color(lbl_theme_val, lv_color_white(), 0); // White text
  lv_obj_center(lbl_theme_val);

  lv_obj_add_event_cb(btn_theme, [](lv_event_t *e)
                      {
    // Cycle through all 6 themes: Off → Yugioh → Magic → Pokemon → Default → FAB → Off
    temp_theme = (temp_theme + 1) % 6;

    lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text(label, getThemeName(temp_theme)); }, LV_EVENT_CLICKED, NULL);

  // Extra Spacer for Scrolling (Row 5, spanning both columns)
  // This ensures Theme button is fully visible when scrolling down
  lv_obj_t *extra_spacer = lv_obj_create(values_popup);
  lv_obj_set_size(extra_spacer, 10, 150);                                                    // 250px high for ample scrolling space
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 5, 1); // Row 5
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);                                   // Invisible
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);                               // No border
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);                                    // Not clickable

  static SharedInputState shared_input_state = {nullptr, nullptr, nullptr, nullptr};

  shared_input_state.ta = lv_textarea_create(values_popup);
  lv_textarea_set_one_line(shared_input_state.ta, true);
  lv_obj_set_style_text_font(shared_input_state.ta, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(shared_input_state.ta, lv_color_white(), 0);
  lv_obj_set_size(shared_input_state.ta, SCREEN_WIDTH - 120, 60);
  lv_obj_align(shared_input_state.ta, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_add_flag(shared_input_state.ta, LV_OBJ_FLAG_HIDDEN);

  shared_input_state.kb = lv_keyboard_create(values_popup);
  lv_keyboard_set_mode(shared_input_state.kb, LV_KEYBOARD_MODE_NUMBER);
  lv_obj_add_flag(shared_input_state.kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_text_font(shared_input_state.kb, &lv_font_montserrat_24, 0);
  lv_keyboard_set_map(shared_input_state.kb, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
  lv_keyboard_set_mode(shared_input_state.kb, LV_KEYBOARD_MODE_USER_1);
  lv_obj_set_size(shared_input_state.kb, SCREEN_WIDTH, SCREEN_HEIGHT - 150);
  lv_obj_align(shared_input_state.kb, LV_ALIGN_TOP_MID, 0, 80);
  lv_obj_clear_flag(shared_input_state.kb, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_user_data(btn_max_life, &temp_life);
  lv_obj_set_user_data(btn_small_step, &temp_small);
  lv_obj_set_user_data(btn_large_step, &temp_large);

  lv_obj_add_event_cb(btn_max_life, open_input_cb, LV_EVENT_CLICKED, &shared_input_state);
  lv_obj_add_event_cb(btn_small_step, open_input_cb, LV_EVENT_CLICKED, &shared_input_state);
  lv_obj_add_event_cb(btn_large_step, open_input_cb, LV_EVENT_CLICKED, &shared_input_state);

  lv_obj_add_event_cb(shared_input_state.ta, shared_ta_event_cb, LV_EVENT_ALL, &shared_input_state);
}

static void save_preset_to_storage(int preset_idx)
{
  char key_name[32], key_life[32], key_small[32], key_large[32];

  snprintf(key_name, sizeof(key_name), "preset_%d_name", preset_idx);
  snprintf(key_life, sizeof(key_life), "preset_%d_life", preset_idx);
  snprintf(key_small, sizeof(key_small), "preset_%d_small", preset_idx);
  snprintf(key_large, sizeof(key_large), "preset_%d_large", preset_idx);

  player_store.putString(key_name, TCG_PRESETS[preset_idx].name);
  player_store.putInt(key_life, TCG_PRESETS[preset_idx].starting_life);
  player_store.putInt(key_small, TCG_PRESETS[preset_idx].small_step);
  player_store.putInt(key_large, TCG_PRESETS[preset_idx].large_step);

  printf("[Preset] Saved preset %d: %s (%d/%d/%d)\n", preset_idx,
         TCG_PRESETS[preset_idx].name,
         TCG_PRESETS[preset_idx].starting_life,
         TCG_PRESETS[preset_idx].small_step,
         TCG_PRESETS[preset_idx].large_step);
}

static void load_presets_from_storage()
{
  for (int i = 0; i < 10; i++)
  {
    char key_name[32], key_life[32], key_small[32], key_large[32];

    snprintf(key_name, sizeof(key_name), "preset_%d_name", i);
    snprintf(key_life, sizeof(key_life), "preset_%d_life", i);
    snprintf(key_small, sizeof(key_small), "preset_%d_small", i);
    snprintf(key_large, sizeof(key_large), "preset_%d_large", i);

    String stored_name = player_store.getString(key_name, "");
    if (stored_name.length() > 0)
    {
      strncpy(TCG_PRESETS[i].name, stored_name.c_str(), sizeof(TCG_PRESETS[i].name) - 1);
      TCG_PRESETS[i].name[sizeof(TCG_PRESETS[i].name) - 1] = '\0';

      TCG_PRESETS[i].starting_life = player_store.getInt(key_life, TCG_PRESETS[i].starting_life);
      TCG_PRESETS[i].small_step = player_store.getInt(key_small, TCG_PRESETS[i].small_step);
      TCG_PRESETS[i].large_step = player_store.getInt(key_large, TCG_PRESETS[i].large_step);
    }
  }
  printf("[Preset] Loaded all presets from storage\n");
}

static void close_name_popup()
{
  if (name_popup)
  {
    lv_obj_del(name_popup);
    name_popup = nullptr;
  }
  ta_name = nullptr;
  keyboard = nullptr;
}

static void close_values_popup()
{
  if (values_popup)
  {
    lv_obj_del(values_popup);
    values_popup = nullptr;
  }
}

void teardownPresetEditorMenu()
{
  if (preset_editor_menu)
  {
    lv_obj_del(preset_editor_menu);
    preset_editor_menu = nullptr;
  }
  if (preset_editor_swipe_layer)
  {
    lv_obj_del(preset_editor_swipe_layer);
    preset_editor_swipe_layer = nullptr;
  }
  close_name_popup();
  close_values_popup();
}
