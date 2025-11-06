#pragma once

/**
 * @brief Display the starting life configuration screen
 * 
 * Shows UI for selecting initial life total for new games.
 * Typically used before starting a new TCG match.
 */
void renderLifeConfigScreen();

/**
 * @brief Clean up and close the life configuration screen
 * 
 * Removes the life config UI and returns to previous screen.
 * Frees associated LVGL objects and memory.
 */
void teardownStartLifeScreen();
