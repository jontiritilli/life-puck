
#include "power_key.h"
#include "shutdown/shutdown.h"
#include <lvgl.h>
#include <esp_sleep.h>
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
      // printf("[wait_for_button_hold] Button held for %d ms, returning true\n", hold_ms);
      return true; // Button held long enough
    }
  }
  // printf("[wait_for_button_hold] Button released before %d ms, returning false\n", hold_ms);
  return false; // Released before required hold
}

void wake_up(void)
{
  pinMode(PWR_KEY_Input_PIN, INPUT);
  pinMode(PWR_Control_PIN, OUTPUT);
  digitalWrite(PWR_Control_PIN, LOW);
  vTaskDelay(100);
  if (!digitalRead(PWR_KEY_Input_PIN))
  {
    BAT_State = BAT_ON;
    digitalWrite(PWR_Control_PIN, HIGH);
    // printf("[wake_up] Waking up device\n");
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    printf("[wake_up] Wakeup reason: %d\n", wakeup_reason);
    vTaskDelay(300);
  }
  else
  {
    // USB plugged in without button - enter charging mode (stay powered but don't init UI)
    printf("[wake_up] USB charging mode - waiting for button press to boot\n");
    BAT_State = BAT_CHARGING;
    digitalWrite(PWR_Control_PIN, HIGH); // Keep power on for charging
  }
}

void fall_asleep(void)
{
  // printf("[fall_asleep] Preparing for deep sleep\n");
  // Power down display and touch
  if (board && board->getBacklight())
  {
    board->getBacklight()->off();
    // printf("[fall_asleep] Backlight OFF\n");
  }
  digitalWrite(PWR_Control_PIN, LOW);
  // printf("[fall_asleep] Display/touch power OFF\n");

  // Disable internal pullups/pulldowns on wake pin to reduce leakage
  pinMode(PWR_KEY_Input_PIN, INPUT);
  gpio_pulldown_dis((gpio_num_t)PWR_KEY_Input_PIN);
  gpio_pullup_dis((gpio_num_t)PWR_KEY_Input_PIN);
  // printf("[fall_asleep] Pullups/pulldowns disabled on wake pin\n");

  // Enable wakeup on external pin
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PWR_KEY_Input_PIN, HIGH);
  // printf("[fall_asleep] Wakeup enabled on pin %d\n", PWR_KEY_Input_PIN);

  // printf("[fall_asleep] Entering deep sleep NOW\n");
  esp_deep_sleep_start();
  // If we ever return here, something is wrong
  // printf("[fall_asleep] ERROR: Returned from esp_deep_sleep_start! Device did NOT sleep.\n");
}

void power_loop(void)
{
  if (BAT_State == BAT_CHARGING)
  {
    // In charging mode - check if button is pressed to boot
    if (!digitalRead(PWR_KEY_Input_PIN))
    {
      if (button_press_start == 0)
      {
        button_press_start = millis();
      }
      uint32_t held_time = millis() - button_press_start;
      // Require button hold to boot from charging mode
      if (held_time >= Device_Wake_Time)
      {
        printf("[power_loop] Button held while charging - booting device\n");
        BAT_State = BAT_ON;
        button_press_start = 0;
      }
    }
    else
    {
      button_press_start = 0;
    }
  }
  else if (BAT_State != BAT_OFF)
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
          // printf("[power_loop] Button held for sleep (%lu ms), going to sleep\n", held_time);
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

BatteryState get_battery_state(void)
{
  return BAT_State;
}
