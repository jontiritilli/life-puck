#ifndef THEMES_H
#define THEMES_H

#include <lvgl.h>
#include "constants.h"

// ============================================================================
// THEME DATA STRUCTURE
// ============================================================================

/**
 * @brief Theme configuration structure
 * 
 * Defines visual properties for each theme:
 * - button_color: Used in Settings menus and submenus (NOT in life counters)
 * - text_color: Primary text color for buttons (currently not used)
 * - icon_color: Color for menu icons/symbols in contextual menu (menu.cpp)
 * - background_image: Background displayed in life counter screens
 * - background_opacity: Transparency level (0-255, 0=invisible, 255=solid)
 */
typedef struct {
    lv_color_t button_color;              // Button color for Settings/Submenus
    lv_color_t text_color;                // Text color for buttons (future use)
    lv_color_t icon_color;                // Icon/Symbol color for contextual menu
    const lv_image_dsc_t* background_image; // Background for life counter screens
    uint8_t background_opacity;           // Background transparency (0-255)
} Theme;

// ============================================================================
// THEME GETTER FUNCTIONS
// ============================================================================

/**
 * @brief Get Default theme (current blue with logo)
 * @return Pointer to Default theme configuration
 */
const Theme* getDefaultTheme();

/**
 * @brief Get Yugioh theme (gold with Millennium Puzzle)
 * @return Pointer to Yugioh theme configuration
 */
const Theme* getYugiohTheme();

/**
 * @brief Get Magic: The Gathering theme (purple with mana symbols)
 * @return Pointer to Magic theme configuration
 */
const Theme* getMagicTheme();

/**
 * @brief Get Pokemon theme (red with Pokeball)
 * @return Pointer to Pokemon theme configuration
 */
const Theme* getPokemonTheme();

/**
 * @brief Get Flesh & Blood theme (dark red with FAB artwork)
 * @return Pointer to FAB theme configuration
 */
const Theme* getFabTheme();

/**
 * @brief Get currently active theme based on NVS settings
 * 
 * Reads ThemeMode and ThemeSelection from NVS:
 * - If ThemeMode = OFF: Returns nullptr (no theme active)
 * - If ThemeMode = Automatic: Returns theme based on active preset (TODO: Phase 2)
 * - If ThemeMode = Manual: Returns user-selected theme
 * 
 * @return Pointer to active theme, or nullptr if themes are disabled
 */
const Theme* getCurrentTheme();

// ============================================================================
// UTILITY FUNCTIONS FOR EASY ACCESS
// ============================================================================

/**
 * @brief Get button color for Settings menus based on current theme
 * 
 * Returns the button color from the currently active theme.
 * If themes are OFF, returns default blue color.
 * Used by all Settings menu buttons (except Back buttons).
 * 
 * @return lv_color_t Button color for the current theme
 */
lv_color_t getThemeButtonColor();

/**
 * @brief Get icon color for menu symbols based on current theme
 * 
 * Returns the icon color from the currently active theme.
 * If themes are OFF, returns white (unchanged from original).
 * Used by menu.cpp icons/symbols in the contextual menu.
 * 
 * @return lv_color_t Icon color for the current theme
 */
lv_color_t getThemeIconColor();

/**
 * @brief Get text color for buttons based on current theme
 * 
 * Returns the text color from the currently active theme.
 * If themes are OFF, returns white (default).
 * Used by all themed buttons to ensure proper contrast.
 * 
 * Example: Gold buttons need black text for readability.
 * 
 * @return lv_color_t Text color for the current theme
 */
lv_color_t getThemeTextColor();

/**
 * @brief Get theme assigned to the currently active preset
 * 
 * Returns the theme selection assigned to the active preset.
 * Used by Automatic theme mode to determine which theme to apply.
 * 
 * @return ThemeSelection Theme assigned to the active preset
 */
ThemeSelection getPresetTheme();

/**
 * @brief Convert ThemeSelection enum to Theme pointer
 * 
 * Helper function to convert a ThemeSelection enum value
 * to the corresponding Theme object pointer.
 * 
 * @param selection ThemeSelection enum value
 * @return const Theme* Pointer to the corresponding theme
 */
const Theme* getThemeBySelection(ThemeSelection selection);

#endif // THEMES_H

