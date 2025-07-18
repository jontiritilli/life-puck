#include "power_key.h"
#include "shutdown/shutdown.h"
#include <lvgl.h>
#include <esp_sleep.h>
#include <Arduino.h>
#include <esp_display_panel.hpp>

extern esp_panel::board::Board *board;

static uint8_t is_screen_on = false;
static uint16_t btn_hold_duration = 0;
static uint8_t prev_button_state = ButtonState::BUTTON_NOT_PRESSED; // Assume not pressed at start

// Helper Functions
bool is_button_pressed(void)
{
  return (digitalRead(PWR_KEY_Input_PIN) == ButtonState::BUTTON_PRESSED);
}

void set_screen_on(bool on)
{
  is_screen_on = on;
}

void toggle_screen(void)
{
  // Toggle the screen state
  digitalWrite(PWR_Control_PIN, is_screen_on ? LOW : HIGH);
  if (is_screen_on)
  {
    board->getBacklight()->off();
  }
  else
  {
    board->getBacklight()->on();
  }
  set_screen_on(!is_screen_on);
  printf("[toggle_screen] is_screen_on=%d\n", is_screen_on);
}

void wake_up(void)
{
  pinMode(PWR_Control_PIN, OUTPUT);
  digitalWrite(PWR_Control_PIN, HIGH);
  delay(5); // Give FET/IC time to latch
  pinMode(PWR_KEY_Input_PIN, INPUT);
  printf("[wake_up] Waking up device\n");
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  printf("[wake_up] Wakeup reason: %d\n", wakeup_reason);
  board->getBacklight()->on();
  set_screen_on(true);
  vTaskDelay(300);
  prev_button_state = is_button_pressed();
}

void fall_asleep(void)
{
  printf("[fall_asleep] Enabling wakeup on PWR_KEY_Input_PIN, entering deep sleep\n");
  digitalWrite(PWR_Control_PIN, LOW);
  board->getBacklight()->off();
  set_screen_on(false);
  while (is_button_pressed())
  {
    vTaskDelay(10);
  }
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PWR_KEY_Input_PIN, 0);
  esp_deep_sleep_start();
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

void power_loop(void)
{
  uint8_t button_state = is_button_pressed();
  static uint32_t last_toggle_time = 0;
  static uint32_t button_press_start = 0;
  const uint32_t debounce_ms = 100; // 100ms debounce
  uint32_t now = millis();

  if (button_state)
  {
    if (!prev_button_state)
    {
      // Button was just pressed
      button_press_start = now;
    }
    // If long press threshold reached, enter deep sleep immediately
    if ((now - button_press_start) >= Device_Sleep_Time)
    {
      printf("[power_loop] Button held, going to sleep\n");
      fall_asleep();
    }
  }
  else
  {
    if (prev_button_state)
    {
      if ((now - button_press_start) < Device_Sleep_Time && (now - last_toggle_time > debounce_ms))
      {
        printf("[power_loop] Toggling screen\n");
        toggle_screen();
        last_toggle_time = now;
      }
      button_press_start = 0;
    }
  }
  prev_button_state = button_state;
}

void power_init(void)
{
  wake_up();
}
