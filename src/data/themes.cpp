// ============================================
// Own Header (first!)
// ============================================
#include "themes.h"

// ============================================
// Project Headers
// ============================================
#include "constants.h"
#include "config.h"
#include "core/state_manager.h"
#include "tcg_presets.h"

// ============================================
// Asset Headers
// ============================================
#include "../assets/images/logo.h"
#include "../assets/images/bg_yugioh.h"
#include "../assets/images/bg_magic.h"
#include "../assets/images/bg_pokemon.h"
#include "../assets/images/bg_fab.h"

// ============================================================================
// THEME DEFINITIONS
// ============================================================================

/**
 * Default Theme
 * - Button Color: Lightning Blue (from config.h)
 * - Text Color: White (from config.h)
 * - Icon Color: White (unchanged from original)
 * - Background: Existing logo
 * - Used when no specific theme is selected or when themes are OFF
 */
static const Theme theme_default = {
    .button_color = lv_color_hex(THEME_COLOR_DEFAULT),
    .text_color = lv_color_hex(THEME_TEXT_COLOR_DEFAULT),
    .icon_color = lv_color_hex(THEME_ICON_COLOR_DEFAULT),
    .background_image = &logo,
    .background_opacity = THEME_OPACITY_DEFAULT};

/**
 * Yugioh Theme
 * - Button Color: Ancient Egyptian Gold (from config.h)
 * - Text Color: BLACK (from config.h) - much better contrast on gold!
 * - Icon Color: Gold to match buttons
 * - Background: Millennium Puzzle design (currently placeholder)
 */
static const Theme theme_yugioh = {
    .button_color = lv_color_hex(THEME_COLOR_YUGIOH),
    .text_color = lv_color_hex(THEME_TEXT_COLOR_YUGIOH),
    .icon_color = lv_color_hex(THEME_ICON_COLOR_YUGIOH),
    .background_image = &bg_yugioh, // TODO: Replace with Millennium Puzzle
    .background_opacity = THEME_OPACITY_YUGIOH};

/**
 * Magic: The Gathering Theme
 * - Button Color: Mystical Purple (from config.h)
 * - Text Color: White (from config.h)
 * - Icon Color: Purple to match buttons
 * - Background: Five mana symbols design (currently placeholder)
 */
static const Theme theme_magic = {
    .button_color = lv_color_hex(THEME_COLOR_MAGIC),
    .text_color = lv_color_hex(THEME_TEXT_COLOR_MAGIC),
    .icon_color = lv_color_hex(THEME_ICON_COLOR_MAGIC),
    .background_image = &bg_magic, // TODO: Replace with 5 mana symbols
    .background_opacity = THEME_OPACITY_MAGIC};

/**
 * Pokemon Theme
 * - Button Color: Pokemon Red (from config.h)
 * - Text Color: White (from config.h)
 * - Icon Color: Red to match buttons
 * - Background: Pokeball design (currently placeholder)
 */
static const Theme theme_pokemon = {
    .button_color = lv_color_hex(THEME_COLOR_POKEMON),
    .text_color = lv_color_hex(THEME_TEXT_COLOR_POKEMON),
    .icon_color = lv_color_hex(THEME_ICON_COLOR_POKEMON),
    .background_image = &bg_pokemon, // TODO: Replace with Pokeball design
    .background_opacity = THEME_OPACITY_POKEMON};

/**
 * Flesh & Blood Theme
 * - Button Color: Dark Red #8b0006 (from config.h)
 * - Text Color: White (from config.h)
 * - Icon Color: Dark Red to match buttons
 * - Background: FAB artwork
 */
static const Theme theme_fab = {
    .button_color = lv_color_hex(THEME_COLOR_FAB),
    .text_color = lv_color_hex(THEME_TEXT_COLOR_FAB),
    .icon_color = lv_color_hex(THEME_ICON_COLOR_FAB),
    .background_image = &bg_fab,
    .background_opacity = THEME_OPACITY_FAB};

// ============================================================================
// THEME GETTER FUNCTIONS
// ============================================================================

const Theme *getDefaultTheme()
{
  return &theme_default;
}

const Theme *getYugiohTheme()
{
  return &theme_yugioh;
}

const Theme *getMagicTheme()
{
  return &theme_magic;
}

const Theme *getPokemonTheme()
{
  return &theme_pokemon;
}

const Theme *getFabTheme()
{
  return &theme_fab;
}

/**
 * @brief Get currently active theme based on NVS settings
 *
 * Implementation:
 * 1. Read ThemeMode from NVS (OFF/Automatic/Manual)
 * 2. If OFF: Return nullptr (no theme)
 * 3. If Automatic: Return theme based on preset (TODO: Phase 2)
 * 4. If Manual: Return user-selected theme
 */
const Theme *getCurrentTheme()
{
  // Read theme mode from NVS
  int theme_mode_int = player_store.getInt(KEY_THEME_MODE, THEME_MODE_OFF);
  ThemeMode theme_mode = static_cast<ThemeMode>(theme_mode_int);

  // If themes are OFF, return nullptr (no background)
  if (theme_mode == THEME_MODE_OFF)
  {
    return nullptr;
  }

  // If Automatic mode: Return theme based on active preset
  if (theme_mode == THEME_MODE_AUTOMATIC)
  {
    // Get theme assigned to the currently active preset
    ThemeSelection preset_theme = getPresetTheme();
    return getThemeBySelection(preset_theme);
  }

  // Manual mode: Return user-selected theme
  int theme_sel_int = player_store.getInt(KEY_THEME_SELECTION, THEME_DEFAULT);
  ThemeSelection theme_sel = static_cast<ThemeSelection>(theme_sel_int);
  return getThemeBySelection(theme_sel);
}

// ============================================================================
// UTILITY FUNCTIONS FOR EASY ACCESS
// ============================================================================

/**
 * @brief Get button color for Settings menus
 *
 * Returns the button color from the currently active theme.
 * If themes are OFF, returns default blue color.
 */
lv_color_t getThemeButtonColor()
{
  const Theme *theme = getCurrentTheme();
  if (theme != nullptr)
  {
    return theme->button_color;
  }
  // Fallback to default blue if theme is OFF
  return lv_color_hex(THEME_COLOR_DEFAULT);
}

/**
 * @brief Get icon color for menu symbols
 *
 * Returns the icon color from the currently active theme.
 * If themes are OFF, returns white (unchanged from original).
 */
lv_color_t getThemeIconColor()
{
  const Theme *theme = getCurrentTheme();
  if (theme != nullptr)
  {
    return theme->icon_color;
  }
  // Fallback to white if theme is OFF
  return lv_color_white();
}

/**
 * @brief Get text color for buttons
 *
 * Returns the text color from the currently active theme.
 * If themes are OFF, returns white (default).
 * Ensures proper contrast (e.g., black text on gold buttons).
 */
lv_color_t getThemeTextColor()
{
  const Theme *theme = getCurrentTheme();
  if (theme != nullptr)
  {
    return theme->text_color;
  }
  // Fallback to white if theme is OFF
  return lv_color_white();
}

/**
 * @brief Get theme assigned to the currently active preset
 *
 * Returns the theme selection assigned to the active preset.
 * Used by Automatic theme mode to determine which theme to apply.
 *
 * Priority:
 * 1. User override (stored in NVS for this preset)
 * 2. Default from config.h
 *
 * @return ThemeSelection Theme assigned to the active preset
 */
ThemeSelection getPresetTheme()
{
  // Get current preset index from NVS
  int active_preset_index = player_store.getInt("preset_idx", 0);

  // Check for user override in NVS
  char theme_key[20];
  snprintf(theme_key, sizeof(theme_key), "preset_%d_theme", active_preset_index);

  // Get default theme from config.h for this preset
  int default_theme;
  switch (active_preset_index)
  {
  case 0:
    default_theme = PRESET_0_THEME;
    break; // FAB CC → FAB (5)
  case 1:
    default_theme = PRESET_1_THEME;
    break; // FAB Blitz → FAB (5)
  case 2:
    default_theme = PRESET_2_THEME;
    break; // MTG Standard → Magic (2)
  case 3:
    default_theme = PRESET_3_THEME;
    break; // MTG Commander → Magic (2)
  case 4:
    default_theme = PRESET_4_THEME;
    break; // Pokemon TCG → Pokemon (3)
  case 5:
    default_theme = PRESET_5_THEME;
    break; // Yu-Gi-Oh! → Yugioh (1)
  case 6:
    default_theme = PRESET_6_THEME;
    break; // Lorcana → (from config.h)
  case 7:
    default_theme = PRESET_7_THEME;
    break; // One Piece TCG → (from config.h)
  case 8:
    default_theme = PRESET_8_THEME;
    break; // Custom 9 → (from config.h)
  case 9:
    default_theme = PRESET_9_THEME;
    break; // Custom 10 → (from config.h)
  default:
    default_theme = THEME_YUGIOH;
    break; // Fallback to Yugioh (1)
  }

  // Read from NVS (user override) or use config.h default
  int theme_value = player_store.getInt(theme_key, default_theme);
  return (ThemeSelection)theme_value;
}

/**
 * @brief Convert ThemeSelection enum to Theme pointer
 *
 * Helper function to convert a ThemeSelection enum value
 * to the corresponding Theme object pointer.
 *
 * @param selection ThemeSelection enum value
 * @return const Theme* Pointer to the corresponding theme, or nullptr if OFF
 */
const Theme *getThemeBySelection(ThemeSelection selection)
{
  switch (selection)
  {
  case THEME_OFF:
    return nullptr; // No theme
  case THEME_YUGIOH:
    return getYugiohTheme();
  case THEME_MAGIC:
    return getMagicTheme();
  case THEME_POKEMON:
    return getPokemonTheme();
  case THEME_DEFAULT:
    return getDefaultTheme();
  case THEME_FAB:
    return getFabTheme();
  default:
    return nullptr; // Fallback: No theme
  }
}
