#include "power_management.h"
#include "config.h"
#include "battery_state.h"
#include "core/state_manager.h"
#include "hardware/display/display_st77916.h"
#include "hardware/peripherals/power_key.h"
#include "data/constants.h"
#include <Arduino.h>

// NVS Keys (defined in power_settings.cpp and constants.h)
#define KEY_AUTO_DIM_TIME "auto_dim_time"
#define KEY_SLEEP_TIME "sleep_time"
#define KEY_BATTERY_SAVER "battery_saver"

// Inactivity tracking
static unsigned long last_activity_time = 0;
static bool display_is_sleeping = false;
static bool display_is_dimmed = false;
static bool display_is_low_battery_dimmed = false; // Track if dimmed due to low battery
static int original_brightness = 50; // Default brightness (0-100)
static unsigned long last_wake_time = 0;  // Track when display was last woken
static const unsigned long TOUCH_IGNORE_DELAY = TOUCH_BLOCK_AFTER_WAKE_MS;
static bool power_management_suspended = false;  // Suspend power management during calibration

// Critical battery shutdown tracking
static bool low1 = false; // Track if battery is critically low
static unsigned long critical_battery_start_time = 0;
static const unsigned long CRITICAL_BATTERY_DELAY = 2000; // 2 seconds delay before shutdown
static const float CRITICAL_VOLTAGE_THRESHOLD = 3.3;//Voltage threshold for deep sleep (3.3
static unsigned long boot_time = 0;
static const unsigned long BOOT_GRACE_PERIOD = 10000; // 10 seconds grace period after boot
static unsigned long last_usb_detected_time = 0;
static const unsigned long USB_DETECTION_TIMEOUT = 30000; // 30 seconds - if USB was detected, assume it's still connected

void power_management_init()
{
  last_activity_time = millis();
  display_is_sleeping = false;
  display_is_dimmed = false;
  boot_time = millis(); // Track boot time
  
  original_brightness = player_store.getInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT); // Load user's brightness (0-100)
}

void power_reset_inactivity_timer()
{
  last_activity_time = millis();
  
  // Wake display if sleeping
  if (display_is_sleeping) {
    power_wake_display();
    last_wake_time = millis(); // Mark wake time to ignore touches briefly
  }
  
  // Restore brightness if dimmed (but not if low battery dimmed)
  if (display_is_dimmed && !display_is_low_battery_dimmed) {
    original_brightness = player_store.getInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT); // 0-100
    Set_Backlight(original_brightness);
    display_is_dimmed = false;
    last_wake_time = millis(); // Mark wake time to ignore touches briefly
  }
}

bool power_should_ignore_touch()
{
  // Ignore touches for a short time after waking from sleep/dim
  if (last_wake_time > 0 && (millis() - last_wake_time) < TOUCH_IGNORE_DELAY) {
    return true;
  }
  return false;
}

void power_check_inactivity()
{
  // Skip if power management is suspended (e.g., during touch calibration)
  if (power_management_suspended) {
    return;
  }
  
  // Skip if display is already sleeping
  if (display_is_sleeping) {
    return;
  }
  
  unsigned long current_time = millis();
  unsigned long inactive_time = (current_time - last_activity_time) / 1000; // Convert to seconds
  
  // Get settings from NVS
  int auto_dim_time = player_store.getInt(KEY_AUTO_DIM_TIME, AUTO_DIM_DEFAULT);
  int sleep_time = player_store.getInt(KEY_SLEEP_TIME, SLEEP_DEFAULT);
  bool battery_saver = player_store.getInt(KEY_BATTERY_SAVER, LOW_BATTERY_DIM_DEFAULT) != 0;
  
  // Check for sleep FIRST (works even during low battery dim)
  if (sleep_time > 0 && inactive_time >= sleep_time) {
    power_sleep_display();
    return;
  }
  
  // Low Battery Dimming: Force dim at critical battery (if enabled)
  if (battery_saver) {
    float battery_volts = battery_get_volts();
    int battery_percent = (int)battery_get_percent();
    
    // Detect USB connection (voltage < 1V indicates USB charging or switch toggling)
    if (battery_volts < 1.0) {
      last_usb_detected_time = millis(); // Mark USB as detected
      critical_battery_start_time = 0; // Reset shutdown timer immediately
      
      // If we were dimmed due to low battery, restore brightness immediately
      if (display_is_low_battery_dimmed) {
        original_brightness = player_store.getInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT);
        Set_Backlight(original_brightness);
        display_is_low_battery_dimmed = false;
      }
      // Continue with normal auto-dim logic
    }
    else {
      // Check if USB was recently detected (within last 30 seconds)
      bool usb_recently_detected = (millis() - last_usb_detected_time) < USB_DETECTION_TIMEOUT;
      
      // Skip critical battery shutdown during boot grace period or if USB detected
      if (millis() - boot_time < BOOT_GRACE_PERIOD || usb_recently_detected) {
        low1 = false;
        critical_battery_start_time = 0;
      }
      // Critical battery shutdown at ≤2%
      else if (battery_percent <= 2) {
        // First detection: Set low1 flag and start timer
        if (!low1) {
          low1 = true;
          critical_battery_start_time = millis();
        }
        
        // After 2 seconds: Re-check voltage and decide
        if (millis() - critical_battery_start_time >= CRITICAL_BATTERY_DELAY) {
          float current_voltage = battery_get_volts();
          
          // Deep sleep if voltage is below 3.4V and above 1V
          // Below 3.4V = real low battery, above 1V = not USB charging
          if (current_voltage > 1.0 && current_voltage < CRITICAL_VOLTAGE_THRESHOLD) {
            Set_Backlight(0);
            vTaskDelay(100);
            fall_asleep(); // Enter deep sleep to protect battery
          } else {
            // Voltage out of range - reset (USB was connected or battery recovered)
            low1 = false;
            critical_battery_start_time = 0;
          }
        }
      } else {
        // Battery above 2% - reset
        low1 = false;
        critical_battery_start_time = 0;
      }
      
      // Force dim at low battery (≤15%)
      if (battery_percent <= 15 && !display_is_low_battery_dimmed) {
        original_brightness = player_store.getInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT);
        int dimmed_brightness = 5; // Force to 5% at critical battery
        Set_Backlight(dimmed_brightness);
        display_is_low_battery_dimmed = true;
        display_is_dimmed = false; // Clear auto-dim flag
        return; // Skip normal auto-dim logic (but sleep already checked above)
      }
      
      // Restore brightness immediately if battery is back above 15%
      if (battery_percent > 15 && display_is_low_battery_dimmed) {
        original_brightness = player_store.getInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT);
        Set_Backlight(original_brightness);
        display_is_low_battery_dimmed = false;
        return;
      }
      
      // If low battery dimmed, skip normal auto-dim logic (but sleep already checked above)
      if (display_is_low_battery_dimmed) {
        return;
      }
    }
  }
  
  // Check for auto-dim (if enabled)
  if (auto_dim_time > 0 && inactive_time >= auto_dim_time && !display_is_dimmed) {
    // Dim to 25% of current brightness (minimum 5%)
    original_brightness = player_store.getInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT); // 0-100
    int dimmed_brightness = original_brightness / 4; // 25% of current
    if (dimmed_brightness < 5) dimmed_brightness = 5; // Minimum 5%
    Set_Backlight(dimmed_brightness);
    display_is_dimmed = true;
  }
}

void power_set_brightness(int level)
{
  // This function is for compatibility - brightness is managed by brightness.cpp
  // Level is 0-100
  if (level < 0) level = 0;
  if (level > 100) level = 100;
  
  Set_Backlight(level);
  original_brightness = level;
  player_store.putInt(KEY_BRIGHTNESS, level);
}

void power_sleep_display()
{
  if (display_is_sleeping) return;
  
  // Turn off backlight
  Set_Backlight(0);
  display_is_sleeping = true;
  display_is_dimmed = false;
}

void power_wake_display()
{
  if (!display_is_sleeping) return;
  
  // Restore original brightness
  original_brightness = player_store.getInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT); // 0-100
  Set_Backlight(original_brightness);
  display_is_sleeping = false;
  display_is_dimmed = false;
}

/**
 * @brief Suspend or resume power management
 * 
 * Use this to prevent auto-dim and sleep during critical operations
 * like touch calibration where user interaction is required.
 * 
 * @param suspend true to suspend power management, false to resume
 */
void power_suspend(bool suspend)
{
  power_management_suspended = suspend;
  
  if (suspend) {
    printf("[Power] Power management SUSPENDED (auto-dim and sleep disabled)\n");
    // Wake display if sleeping or dimmed
    if (display_is_sleeping) {
      power_wake_display();
    }
    if (display_is_dimmed && !display_is_low_battery_dimmed) {
      // Restore from auto-dim (but keep low battery dim if active)
      original_brightness = player_store.getInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT);
      Set_Backlight(original_brightness);
      display_is_dimmed = false;
    }
    // DON'T reset inactivity timer - keep current state during suspension
  } else {
    printf("[Power] Power management RESUMED (auto-dim and sleep re-enabled)\n");
    // Reset inactivity timer to give user a grace period
    power_reset_inactivity_timer();
  }
}

void power_apply_battery_saver()
{
  // This function is now integrated into power_check_inactivity()
  // Kept for compatibility, but does nothing
  // Low Battery Dimming is checked every cycle in power_check_inactivity()
}

