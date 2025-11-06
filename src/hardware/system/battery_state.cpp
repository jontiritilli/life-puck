/**
 * @file battery_state.cpp
 * @brief Battery voltage monitoring and percentage calculation
 */

#include "battery_state.h"

/// Current battery voltage reading (updated by measurement functions)
float BAT_analogVolts = 0;

void battery_init(void)
{
  // Set ADC resolution to 12 bits (0-4095) for precise voltage measurement
  analogReadResolution(12);
}

float battery_get_volts(void)
{
  // Read raw ADC value in millivolts
  int Volts = analogReadMilliVolts(BAT_ADC_PIN);
  // Apply voltage divider calculation and calibration offset
  BAT_analogVolts = (float)(Volts * 3.0 / 1000.0) / Measurement_offset;
  return BAT_analogVolts;
}

float battery_get_percent()
{
  float volts = battery_get_volts();
  // Li-Po battery voltage range: 3.0V (empty) to 4.16V (full measured)
  // Updated from 4.2V to 4.16V for more accurate percentage calculation
  // based on actual measured max voltage of this hardware
  float min_voltage = 3.0;
  float max_voltage = 4.16;
  // Calculate linear percentage based on voltage range
  float percent = ((volts - min_voltage) / (max_voltage - min_voltage)) * 100.0;
  // Clamp to valid percentage range
  if (percent > 100.0)
    percent = 100.0;
  if (percent < 0.0)
    percent = 0.0;
  return percent;
}