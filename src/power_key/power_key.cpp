
#include "power_key.h"
#include "shutdown/shutdown.h"
#include <lvgl.h>
#include <esp_sleep.h>
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <state/state_store.h>
#include <constants/constants.h>

extern esp_panel::board::Board *board;

static uint8_t BAT_State = 0;
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
      printf("[wait_for_button_hold] Button held for %d ms, returning true\n", hold_ms);
      return true; // Button held long enough
    }
  }
  printf("[wait_for_button_hold] Button released before %d ms, returning false\n", hold_ms);
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
    BAT_State = 1;
    digitalWrite(PWR_Control_PIN, HIGH);
    printf("[wake_up] Waking up device\n");
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    printf("[wake_up] Wakeup reason: %d\n", wakeup_reason);
    vTaskDelay(300);
  }
}

void fall_asleep(void)
{
  printf("[fall_asleep] Enabling wakeup on PWR_KEY_Input_PIN, entering deep sleep\n");
  pinMode(PWR_KEY_Input_PIN, INPUT);
  board->getBacklight()->off(); // Manually turn off LCD backlight before sleep
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PWR_KEY_Input_PIN, HIGH);
  esp_deep_sleep_start();
}

void power_loop(void)
{
  if (BAT_State)
  {
    if (!digitalRead(PWR_KEY_Input_PIN))
    {
      if (BAT_State == 2)
      {
        if (button_press_start == 0)
        {
          button_press_start = millis();
        }
        uint32_t held_time = millis() - button_press_start;
        if (held_time >= Device_Sleep_Time)
        {
          printf("[power_loop] Button held for sleep (%lu ms), going to sleep\n", held_time);
          fall_asleep();
        }
      }
    }
    else
    {
      if (BAT_State == 1)
        BAT_State = 2;
      button_press_start = 0;
    }
  }
}

void power_init(void)
{
  wake_up();
}
