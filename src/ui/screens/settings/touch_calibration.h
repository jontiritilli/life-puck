#pragma once

/**
 * @file touch_calibration.h
 * @brief Interactive touch calibration system
 * 
 * Since I couldn't find any similar approach in existing implementations,
 * I'm calling this "Suthe's Method" - a zone-based interactive calibration
 * technique with automatic parameter adjustment.
 * 
 * See docs/Suthes_Method_Touch_Calibration.md for full documentation.
 */

/**
 * @brief Start the interactive touch calibration process
 * 
 * Launches the zone-based calibration interface that automatically
 * adjusts offset_x, offset_y, scale_x, scale_y (and optionally rotation)
 * by detecting which zone the user touches and incrementally adjusting
 * the calibration matrix until convergence.
 */
void renderTouchCalibrationScreen();

/**
 * @brief Cleanup touch calibration screen
 */
void teardownTouchCalibrationScreen();

/**
 * @brief Reset touch calibration to factory defaults
 * 
 * Emergency function to restore default touch settings.
 */
void resetTouchCalibrationToDefaults();

/**
 * @brief Load and apply saved touch calibration from NVS
 * 
 * Called during boot to restore previously calibrated touch settings.
 */
void loadTouchCalibrationFromNVS();

/**
 * @brief Check if touch calibration needs confirmation after boot
 * 
 * @return true if confirmation dialog should be shown
 */
bool needsTouchCalibrationConfirmation();

/**
 * @brief Render touch calibration confirmation screen
 */
void renderTouchCalibrationConfirmation();

/**
 * @brief Cleanup touch calibration confirmation screen
 */
void teardownTouchCalibrationConfirmation();

/**
 * @brief Confirm the current touch calibration
 */
void confirmTouchCalibration();
