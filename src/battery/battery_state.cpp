#include "battery_state.h"

float BAT_analogVolts = 0;

void battery_init(void)
{
  // set the resolution to 12 bits (0-4095)
  analogReadResolution(12);
}

float battery_get_volts(void)
{
  int Volts = analogReadMilliVolts(BAT_ADC_PIN); // millivolts
  BAT_analogVolts = (float)(Volts * 3.0 / 1000.0) / Measurement_offset;
  return BAT_analogVolts;
}

float battery_get_percent()
{
  float volts = battery_get_volts();
  float min_voltage = 3.0;
  float max_voltage = 4.2;
  float percent = ((volts - min_voltage) / (max_voltage - min_voltage)) * 100.0;
  if (percent > 100.0)
    percent = 100.0;
  if (percent < 0.0)
    percent = 0.0;
  return percent;
}