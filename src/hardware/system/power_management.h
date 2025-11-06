#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <Arduino.h>

// Power management functions
void power_management_init();
void power_check_inactivity();
void power_reset_inactivity_timer();
bool power_should_ignore_touch(); // Check if touches should be ignored after wake
void power_set_brightness(int level); // Level 0-100 (for compatibility)
void power_sleep_display();
void power_wake_display();
void power_apply_battery_saver();
void power_suspend(bool suspend); // Suspend/resume power management (for calibration)

#endif // POWER_MANAGEMENT_H

