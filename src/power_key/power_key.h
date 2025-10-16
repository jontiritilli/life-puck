#pragma once
#include "Arduino.h"

#define LCD_BL_PIN 5
#define PWR_KEY_Input_PIN 6
#define PWR_Control_PIN 7

#define Device_Wake_Time 2 * 1000  // 2 seconds (assuming loop rate is 50Hz)
#define Device_Sleep_Time 3 * 1000 // 3 seconds (assuming loop rate is 50Hz)

// USB detection voltage threshold
// USB charging: ~4.16-4.19V, Battery only: ~3.7-4.16V, Threshold: 4.15V
// Set high enough to reliably detect USB charging vs battery
// Uses hysteresis: higher to detect USB, lower to confirm disconnect
#define USB_VOLTAGE_THRESHOLD 4.15f
#define USB_DISCONNECT_THRESHOLD 4.12f

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
  BAT_ON = 1
};

void fall_asleep(void);
void wake_up(void);
void power_init(void);
void power_loop(void);
BatteryState get_battery_state(void);