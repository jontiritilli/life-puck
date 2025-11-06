/**
 * @file touch_cst816.h
 * @brief CST816 capacitive touch controller driver
 * 
 * Provides touch input detection and gesture recognition
 * for the CST816 touch controller chip.
 */

#pragma once

// ============================================
// System & Framework Headers
// ============================================
#include <Arduino.h>
#include <Wire.h>

// ============================================
// Hardware Dependencies
// ============================================
#include "../peripherals/tca9554_power.h"

/// I2C address of CST816 touch controller
#define CST816_ADDR           0x15
/// GPIO pin connected to touch interrupt
#define CST816_INT_PIN        4
/// Touch controller reset pin (handled via GPIO expander)
#define CST816_RST_PIN        -1
/// I2C bus frequency for touch communication
#define I2C_MASTER_FREQ_HZ    400000


/// Maximum number of simultaneous touch points supported
#define CST816_LCD_TOUCH_MAX_POINTS             (1)

/**
 * @brief Hardware gesture types recognized by CST816
 * 
 * These are raw gesture codes returned by the touch controller.
 * Higher-level gesture processing is handled by the UI layer.
 */
enum GESTURE {
  NONE = 0x00,         ///< No gesture detected
  SWIPE_UP = 0x01,     ///< Upward swipe
  SWIPE_DOWN = 0x02,   ///< Downward swipe
  SWIPE_LEFT = 0x03,   ///< Left swipe
  SWIPE_RIGHT = 0x04,  ///< Right swipe
  SINGLE_CLICK = 0x05, ///< Single tap
  DOUBLE_CLICK = 0x0B, ///< Double tap
  LONG_PRESS = 0x0C    ///< Long press
};
// ============================================
// CST816 Register Definitions
// ============================================
/// Register containing current gesture ID
#define CST816_REG_GestureID      0x01
/// Hardware version register
#define CST816_REG_Version        0x15
/// Chip identification register
#define CST816_REG_ChipID         0xA7
/// Project ID register
#define CST816_REG_ProjID         0xA8
/// Firmware version register
#define CST816_REG_FwVersion      0xA9
/// Auto-sleep timeout configuration
#define CST816_REG_AutoSleepTime  0xF9
/// Disable auto-sleep feature
#define CST816_REG_DisAutoSleep   0xFE


/// Global touch data structure
extern struct CST816_Touch touch_data;

/**
 * @brief Touch data structure containing current touch state
 */
struct CST816_Touch{
  uint8_t points;     ///< Number of active touch points (0 or 1)
  GESTURE gesture;    ///< Current gesture type detected
  uint16_t x;         ///< X coordinate of touch point
  uint16_t y;         ///< Y coordinate of touch point
};

/**
 * @brief Initialize CST816 touch controller
 * @return 0 on success, error code on failure
 */
uint8_t Touch_Init();

/**
 * @brief Main touch processing loop function
 * 
 * Should be called regularly to update touch state
 * and process new touch events.
 */
void Touch_Loop(void);

/**
 * @brief Hardware reset of CST816 touch controller
 * @return 0 on success, error code on failure
 */
uint8_t CST816_Touch_Reset(void);

/**
 * @brief Configure auto-sleep behavior
 * @param Sleep_State true to enable auto-sleep, false to disable
 */
void CST816_AutoSleep(bool Sleep_State);

/**
 * @brief Read configuration from CST816
 * @return Configuration value read from device
 */
uint16_t CST816_Read_cfg(void);

/**
 * @brief Get human-readable name of current gesture
 * @return String containing gesture name
 */
String Touch_GestureName(void);

/**
 * @brief Read raw touch data from CST816
 * @return 0 on success, error code on failure
 */
uint8_t Touch_Read_Data(void);

/**
 * @brief Example function for reading touchpad data
 * 
 * Demonstrates basic touch data acquisition.
 * Used for testing and debugging.
 */
void example_touchpad_read(void);

/**
 * @brief Interrupt service routine for touch events
 * 
 * IRAM_ATTR ensures function is stored in RAM for fast execution.
 * Called when touch interrupt pin is triggered.
 */
void IRAM_ATTR Touch_CST816_ISR(void);
