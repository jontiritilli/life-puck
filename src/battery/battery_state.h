#pragma once
#include <Arduino.h>

#define BAT_ADC_PIN 8
#define Measurement_offset 0.990476

extern float BAT_analogVolts;

void battery_init(void);
float battery_get_volts(void);
float battery_get_percent(void);