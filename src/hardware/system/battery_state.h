#pragma once
#include <Arduino.h>

/// ADC pin connected to battery voltage divider
#define BAT_ADC_PIN 8
/// Calibration offset for accurate voltage measurement
#define Measurement_offset 0.990476

/// Current battery voltage reading (updated by battery functions)
extern float BAT_analogVolts;

/**
 * @brief Initialize battery monitoring system
 * 
 * Configures ADC pin and sets up initial battery state.
 * Call once during system initialization.
 */
void battery_init(void);

/**
 * @brief Get current battery voltage
 * @return Battery voltage in volts (float)
 */
float battery_get_volts(void);

/**
 * @brief Get current battery charge percentage
 * @return Battery charge level (0.0-100.0%)
 */
float battery_get_percent(void);