
#include "power_key.h"
#include "shutdown/shutdown.h"
#include <lvgl.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <state/state_store.h>
#include <constants/constants.h>

extern esp_panel::board::Board *board;

static BatteryState BAT_State = BAT_OFF;
// Device_State removed; only off/on supported
static uint32_t button_press_start = 0;
// Helper Functions
bool is_button_pressed(void)
{
  return (digitalRead(PWR_KEY_Input_PIN) == ButtonState::BUTTON_PRESSED);
}

// Helper function: Wait for button to be held for a specified duration (in ms)
bool wait_for_button_hold(uint16_t hold_ms)
{
  uint16_t wake_btn_hold_duration = 0;
  uint16_t required_count = hold_ms / 10;
  while (is_button_pressed())
  {
    vTaskDelay(10);
    wake_btn_hold_duration++;
    if (wake_btn_hold_duration >= required_count)
    {
      return true; // Button held long enough
    }
  }
  return false; // Released before required hold
}

void wake_up(void)
{
  pinMode(PWR_KEY_Input_PIN, INPUT);
  pinMode(PWR_Control_PIN, OUTPUT);
  digitalWrite(PWR_Control_PIN, LOW);
  vTaskDelay(100);

  // Check wake-up reason to distinguish button press from USB power
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  printf("[wake_up] Wakeup reason: %d\n", wakeup_reason);

  // Only boot if woken by button (EXT0) or first boot with button held
  // ESP_SLEEP_WAKEUP_UNDEFINED = power-on reset (USB plugged in)
  // ESP_SLEEP_WAKEUP_EXT0 = woken by power button
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0 ||
      (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED && !digitalRead(PWR_KEY_Input_PIN)))
  {
    BAT_State = BAT_ON;
    digitalWrite(PWR_Control_PIN, HIGH);
    vTaskDelay(300);
  }
  else
  {
    // USB plugged in or other wake source - go back to sleep immediately
    printf("[wake_up] USB charging detected, going back to sleep\n");
    fall_asleep();
  }
}

void fall_asleep(void)
{
  // Power down display and touch
  if (board && board->getBacklight())
  {
    board->getBacklight()->off();
  }

  // Disable peripherals to reduce power consumption
  // Turn off WiFi and Bluetooth (if enabled) - ignore errors if not initialized
  esp_wifi_stop();
  esp_bt_controller_disable();

  digitalWrite(PWR_Control_PIN, LOW);

  // Disable internal pullups/pulldowns on wake pin to reduce leakage
  pinMode(PWR_KEY_Input_PIN, INPUT);
  gpio_pulldown_dis((gpio_num_t)PWR_KEY_Input_PIN);
  gpio_pullup_dis((gpio_num_t)PWR_KEY_Input_PIN);

  // Disable all wakeup sources first
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  // Enable wakeup on external pin (power button only)
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PWR_KEY_Input_PIN, HIGH);

  // Configure power domains for minimum consumption during sleep
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);

  printf("[fall_asleep] Entering deep sleep\n");
  vTaskDelay(100); // Allow serial output to complete

  esp_deep_sleep_start();
}

void power_loop(void)
{
  if (BAT_State != BAT_OFF)
  {
    if (!digitalRead(PWR_KEY_Input_PIN))
    {
      if (BAT_State == BAT_READY_FOR_SLEEP)
      {
        if (button_press_start == 0)
        {
          button_press_start = millis();
        }
        uint32_t held_time = millis() - button_press_start;
        if (held_time >= Device_Sleep_Time)
        {
          fall_asleep();
        }
      }
    }
    else
    {
      if (BAT_State == BAT_ON)
        BAT_State = BAT_READY_FOR_SLEEP;
      button_press_start = 0;
    }
  }
}

void power_init(void)
{
  wake_up();
}
