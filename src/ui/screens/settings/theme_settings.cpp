// ============================================
// Own Header (first!)
// ============================================
#include "theme_settings.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// Hardware
// ============================================
#include "hardware/touch/Touch_CST816.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/menu/menu.h"

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/ui_helpers.h"
#include "ui/helpers/ui_constants.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "data/themes.h"

// Static variables for tracking theme changes
static const Theme *last_theme_settings_theme = nullptr;

// Static pointer to the theme menu container
static lv_obj_t *theme_menu = nullptr;

/**
 * @brief Get the text label for the current theme mode
 *
 * @param mode The theme mode enum value
 * @return const char* The corresponding text label
 */
static const char *getThemeModeText(ThemeMode mode)
{
  switch (mode)
  {
  case THEME_MODE_OFF:
    return "Themes: OFF";
  case THEME_MODE_AUTOMATIC:
    return "Themes: Automatic";
  case THEME_MODE_MANUAL:
    return "Themes: Manual";
  default:
    return "Themes: OFF";
  }
}

/**
 * @brief Get the text label for the current theme selection
 *
 * @param theme The theme selection enum value
 * @return const char* The corresponding text label
 */
static const char *getThemeSelectionText(ThemeSelection theme)
{
  switch (theme)
  {
  case THEME_OFF:
    return "Theme: Off";
  case THEME_FAB:
    return "Theme: Flesh & Blood";
  case THEME_YUGIOH:
    return "Theme: Yugioh";
  case THEME_MAGIC:
    return "Theme: Magic";
  case THEME_POKEMON:
    return "Theme: Pokemon";
  case THEME_DEFAULT:
    return "Theme: Default";
  default:
    return "Theme: Off";
  }
}

/**
 * @brief Cycle to the next theme mode
 *
 * Cycles through: OFF → Automatic → Manual → OFF
 *
 * @param current The current theme mode
 * @return ThemeMode The next theme mode in the cycle
 */
static ThemeMode getNextThemeMode(ThemeMode current)
{
  switch (current)
  {
  case THEME_MODE_OFF:
    return THEME_MODE_AUTOMATIC;
  case THEME_MODE_AUTOMATIC:
    return THEME_MODE_MANUAL;
  case THEME_MODE_MANUAL:
    return THEME_MODE_OFF;
  default:
    return THEME_MODE_OFF;
  }
}

/**
 * @brief Cycle to the next theme selection
 *
 * Cycles through all 5 themes: Off → Yugioh → Magic → Pokemon → Flesh & Blood → Default → Off
 *
 * @param current The current theme selection
 * @return ThemeSelection The next theme in the cycle
 */
static ThemeSelection getNextThemeSelection(ThemeSelection current)
{
  switch (current)
  {
  case THEME_OFF:
    return THEME_YUGIOH;
  case THEME_YUGIOH:
    return THEME_MAGIC;
  case THEME_MAGIC:
    return THEME_POKEMON;
  case THEME_POKEMON:
    return THEME_FAB;
  case THEME_FAB:
    return THEME_DEFAULT;
  case THEME_DEFAULT:
    return THEME_OFF;
  default:
    return THEME_OFF;
  }
}

void renderThemeSettingsMenu()
{
  teardownThemeSettingsMenu();

  // Create main container using standardized helper
  theme_menu = ui_create_menu_container(lv_scr_act());

  // Grid Layout: Full width buttons (1 column, 5 rows)
  // Row 0: Title, Row 1: Back, Row 2: Theme Mode, Row 3: Theme Selection, Row 4: Spacer
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {40, 60, 50, 50, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(theme_menu, col_dsc, row_dsc);
  lv_obj_set_layout(theme_menu, LV_LAYOUT_GRID);

  // Title (Row 0)
  ui_create_title(theme_menu, "Theme Settings", 0);

  // Back button (Row 1)
  lv_obj_t *btn_back = ui_create_back_button(theme_menu, 1, 0, [](lv_event_t *e)
                                             { renderMenu(MENU_SETTINGS); });

  // Load current theme settings from NVS
  int theme_mode_int = player_store.getInt(KEY_THEME_MODE, THEME_MODE_OFF);
  int theme_sel_int = player_store.getInt(KEY_THEME_SELECTION, THEME_DEFAULT);
  ThemeMode current_theme_mode = static_cast<ThemeMode>(theme_mode_int);
  ThemeSelection current_theme_sel = static_cast<ThemeSelection>(theme_sel_int);

  // Theme Mode Toggle Button (Row 2, Full Width)
  lv_obj_t *btn_theme_mode = lv_btn_create(theme_menu);
  lv_obj_set_size(btn_theme_mode, 280, UI_BTN_HEIGHT_STANDARD);
  // Color: Gray when OFF, Theme color when not OFF
  bool theme_mode_active = (current_theme_mode != THEME_MODE_OFF);
  lv_obj_set_style_bg_color(btn_theme_mode,
                            (theme_mode_active ? getThemeButtonColor() : UI_COLOR_DISABLED),
                            LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_theme_mode, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_t *lbl_theme_mode = lv_label_create(btn_theme_mode);
  lv_label_set_text(lbl_theme_mode, getThemeModeText(current_theme_mode));
  lv_obj_set_style_text_font(lbl_theme_mode, UI_FONT_TOGGLE, 0);
  lv_obj_set_style_text_color(lbl_theme_mode, (theme_mode_active ? getThemeTextColor() : UI_COLOR_TEXT_PRIMARY), 0);
  lv_obj_center(lbl_theme_mode);

  // Theme Selection Button (Row 3, Full Width)
  lv_obj_t *btn_theme_sel = lv_btn_create(theme_menu);
  lv_obj_set_size(btn_theme_sel, 280, UI_BTN_HEIGHT_STANDARD);
  // Color: Gray when Theme Mode is OFF or Automatic, Theme color only when Manual
  bool theme_sel_active = (current_theme_mode == THEME_MODE_MANUAL);
  lv_obj_set_style_bg_color(btn_theme_sel,
                            (theme_sel_active ? getThemeButtonColor() : UI_COLOR_DISABLED),
                            LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_theme_sel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 3, 1);
  lv_obj_t *lbl_theme_sel = lv_label_create(btn_theme_sel);
  lv_label_set_text(lbl_theme_sel, getThemeSelectionText(current_theme_sel));
  lv_obj_set_style_text_font(lbl_theme_sel, UI_FONT_TOGGLE, 0);
  lv_obj_set_style_text_color(lbl_theme_sel, (theme_sel_active ? getThemeTextColor() : UI_COLOR_TEXT_PRIMARY), 0);
  lv_obj_center(lbl_theme_sel);

  // Event callback for Theme Mode button
  // We need to pass both buttons to update the Theme Selection button color
  struct ButtonPair
  {
    lv_obj_t *mode_btn;
    lv_obj_t *sel_btn;
  };
  static ButtonPair btn_pair;
  btn_pair.mode_btn = btn_theme_mode;
  btn_pair.sel_btn = btn_theme_sel;

  lv_obj_add_event_cb(btn_theme_mode, [](lv_event_t *e)
                      {
                        ButtonPair *pair = (ButtonPair *)lv_event_get_user_data(e);

                        // Get current theme mode and cycle to next
                        int theme_mode_int = player_store.getInt(KEY_THEME_MODE, THEME_MODE_OFF);
                        ThemeMode current = static_cast<ThemeMode>(theme_mode_int);
                        ThemeMode next = getNextThemeMode(current);

                        // Save to NVS
                        player_store.putInt(KEY_THEME_MODE, static_cast<int>(next));
                        printf("[ThemeSettings] Theme Mode changed: %s\n", getThemeModeText(next));

                        // Update button text only (colors will update when leaving menu)
                        lv_obj_t *label = lv_obj_get_child(pair->mode_btn, 0);
                        lv_label_set_text(label, getThemeModeText(next));

                        bool is_active = (next != THEME_MODE_OFF);
                        lv_obj_set_style_bg_color(pair->mode_btn,
                                                  (is_active ? getThemeButtonColor() : UI_COLOR_DISABLED),
                                                  LV_PART_MAIN);
                        // Text color will be updated when leaving theme menu

                        // Update Theme Selection button color (only active when Manual mode)
                        bool sel_active = (next == THEME_MODE_MANUAL);
                        lv_obj_set_style_bg_color(pair->sel_btn,
                                                  (sel_active ? getThemeButtonColor() : UI_COLOR_DISABLED),
                                                  LV_PART_MAIN);
                        // Text color will be updated when leaving theme menu

                        // TODO: Apply theme changes when theme system is implemented
                      },
                      LV_EVENT_CLICKED, &btn_pair);

  // Event callback for Theme Selection button
  // Only allow clicks when Theme Mode is Manual
  lv_obj_add_event_cb(btn_theme_sel, [](lv_event_t *e)
                      {
                        // Check if theme mode is Manual - only allow selection in Manual mode
                        int theme_mode_int = player_store.getInt(KEY_THEME_MODE, THEME_MODE_OFF);
                        ThemeMode mode = static_cast<ThemeMode>(theme_mode_int);

                        if (mode != THEME_MODE_MANUAL)
                        {
                          printf("[ThemeSettings] Theme Selection disabled - only available in Manual mode\n");
                          return; // Button is disabled, don't process click
                        }

                        // Get current theme selection and cycle to next
                        int theme_sel_int = player_store.getInt(KEY_THEME_SELECTION, THEME_DEFAULT);
                        ThemeSelection current = static_cast<ThemeSelection>(theme_sel_int);
                        ThemeSelection next = getNextThemeSelection(current);

                        // Save to NVS
                        player_store.putInt(KEY_THEME_SELECTION, static_cast<int>(next));
                        printf("[ThemeSettings] Theme Selection changed: %s\n", getThemeSelectionText(next));

                        // Update button text only (colors will update when leaving menu)
                        lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
                        lv_obj_t *label = lv_obj_get_child(btn, 0);
                        lv_label_set_text(label, getThemeSelectionText(next));
                        // Text color will be updated when leaving theme menu

                        // TODO: Apply selected theme when theme system is implemented
                      },
                      LV_EVENT_CLICKED, NULL);

  // Extra Spacer for Scrolling (Row 4)
  lv_obj_t *extra_spacer = lv_obj_create(theme_menu);
  lv_obj_set_size(extra_spacer, 10, 50);
  lv_obj_set_grid_cell(extra_spacer, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 4, 1);
  lv_obj_set_style_bg_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_opa(extra_spacer, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(extra_spacer, LV_OBJ_FLAG_CLICKABLE);

  // Add swipe-to-close gesture (same as other settings submenus)
  bool swipe_enabled = (player_store.getInt(KEY_SWIPE_TO_CLOSE, 0) != 0);
  if (swipe_enabled)
  {
    extern CST816_Touch touch_data; // Access RAW touch data for accurate swipe detection

    lv_obj_add_event_cb(theme_menu, [](lv_event_t *e)
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

void teardownThemeSettingsMenu()
{
  if (theme_menu)
  {
    lv_obj_del(theme_menu);
    theme_menu = nullptr;
  }
  last_theme_settings_theme = nullptr;
}

/**
 * @brief Refresh theme colors in theme settings menu
 *
 * Updates button and text colors when returning to theme settings menu
 * after theme changes. Called from settings_overlay.cpp.
 */
void refreshThemeSettingsColors()
{
  if (theme_menu == nullptr)
    return;

  const Theme *current_theme = getCurrentTheme();
  if (current_theme == last_theme_settings_theme)
    return;

  lv_color_t btn_color = getThemeButtonColor();
  lv_color_t txt_color = getThemeTextColor();

  // Update all buttons in the theme menu
  // Find buttons by iterating through children
  uint32_t child_count = lv_obj_get_child_cnt(theme_menu);
  for (uint32_t i = 0; i < child_count; i++)
  {
    lv_obj_t *child = lv_obj_get_child(theme_menu, i);

    // Check if this is a button (has clickable flag)
    if (lv_obj_has_flag(child, LV_OBJ_FLAG_CLICKABLE))
    {
      // Skip back button (white background)
      lv_color_t bg_color = lv_obj_get_style_bg_color(child, LV_PART_MAIN);
      if (lv_color_eq(bg_color, lv_color_white()))
      {
        continue; // Skip back button
      }

      // Update button background color
      lv_obj_set_style_bg_color(child, btn_color, LV_PART_MAIN);

      // Update text color if button has label
      lv_obj_t *label = lv_obj_get_child(child, 0);
      if (label)
      {
        lv_obj_set_style_text_color(label, txt_color, 0);
      }
    }
  }

  last_theme_settings_theme = current_theme;
}
