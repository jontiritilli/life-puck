#pragma once
#include "Arduino.h"

#define LCD_BL_PIN 5
#define PWR_KEY_Input_PIN 6
#define PWR_Control_PIN 7

#define Device_Wake_Time 2 * 1000  // 2 seconds (assuming loop rate is 50Hz)
#define Device_Sleep_Time 3 * 1000 // 3 seconds (assuming loop rate is 50Hz)

// Button state enum for clarity
// represents state of voltage on PWR_KEY_Input_PIN
// 1 = button not pressed (HIGH), 0 = button pressed (LOW)
enum ButtonState
{
  BUTTON_NOT_PRESSED = 1,
  BUTTON_PRESSED = 0
};

void Fall_Asleep(void);
void Toggle_Screen(void);

void power_init(void);
void power_loop(void);