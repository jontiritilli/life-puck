#pragma once

/**
 * @brief Display the game history overlay screen
 * 
 * Creates and shows a modal overlay containing the complete
 * game history with all life changes and timestamps.
 */
void renderHistoryOverlay();

/**
 * @brief Close and cleanup the history overlay
 * 
 * Removes the history overlay from screen and frees
 * associated LVGL objects and memory.
 */
void teardownHistoryOverlay();
