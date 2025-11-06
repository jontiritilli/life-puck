#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Render the Touch Settings menu
 *
 * Displays options for:
 * - Touch Calibration
 * - Swipe to Dismiss (ON/OFF)
 * - Gesture Control Mode (Classic/Hold)
 */
void renderTouchSettingsMenu();

/**
 * @brief Teardown the Touch Settings menu
 */
void teardownTouchSettingsMenu();

#ifdef __cplusplus
}
#endif
