#pragma once
#include "Arduino.h"

#define LCD_BL_PIN 5
#define PWR_KEY_Input_PIN 6
#define PWR_Control_PIN 7

#define Device_Wake_Time 2 * 1000  // 2 seconds (assuming loop rate is 50Hz)
#define Device_Sleep_Time 3 * 1000 // 3 seconds (assuming loop rate is 50Hz)

// Button state enum for clarity
enum ButtonState
{
  BUTTON_NOT_PRESSED = 1,
  BUTTON_PRESSED = 0
};

// Battery state enum for power management
enum BatteryState
{
  BAT_OFF = 0,
  BAT_ON = 1,
  BAT_READY_FOR_SLEEP = 2,
  BAT_CHARGING = 3
};

void fall_asleep(void);
void wake_up(void);
void power_init(void);
void power_loop(void);
BatteryState get_battery_state(void);