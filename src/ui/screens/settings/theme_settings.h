#pragma once

extern "C"
{
  /**
   * @brief Renders the Theme Settings submenu
   * 
   * This function creates a submenu for configuring theme settings:
   * - Theme Mode: Toggle between OFF, Automatic, and Manual
   * - Theme Selection: Choose between Default, Yugioh, Magic, and Pokemon themes
   * 
   * When Theme Mode is OFF, the Theme Selection button is disabled (grayed out).
   * All settings are persisted to NVS storage.
   */
  void renderThemeSettingsMenu();

  /**
   * @brief Cleans up and removes the Theme Settings menu
   * 
   * Deletes all LVGL objects associated with the Theme Settings menu
   * and resets the internal pointer to nullptr.
   */
  void teardownThemeSettingsMenu();

  /**
   * @brief Refresh theme colors in theme settings menu
   * 
   * Updates button and text colors when returning to theme settings menu
   * after theme changes. Called from settings_overlay.cpp.
   */
  void refreshThemeSettingsColors();
}

