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

// Enhanced wake-up: latch power as soon as button is pressed
void wake_up(void)
{
  // Latch power ON immediately if button is pressed
  if (is_button_pressed())
  {
    digitalWrite(PWR_Control_PIN, HIGH);
    delay(5); // Give FET/IC time to latch
    // Wait for button release to avoid bouncing
    while (is_button_pressed())
    {
      vTaskDelay(10);
    }
  }
  else
  {
    // If not pressed, just ensure power is ON
    digitalWrite(PWR_Control_PIN, HIGH);
    delay(5);
  }
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
  set_screen_on(false);
  digitalWrite(PWR_Control_PIN, LOW);
  board->getBacklight()->off();
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
  static uint8_t last_button_state = ButtonState::BUTTON_NOT_PRESSED;

  if (button_state && !last_button_state)
  {
    // Button just pressed, check hold durations
    if (wait_for_button_hold(Device_Sleep_Time))
    {
      printf("[power_loop] Button held for sleep, going to sleep\n");
      fall_asleep();
    }
    else if (wait_for_button_hold(Device_Wake_Time))
    {
      printf("[power_loop] Button held for wake, waking up\n");
      wake_up();
    }
    else
    {
      // Short press, toggle screen
      printf("[power_loop] Short press, toggling screen\n");
      toggle_screen();
    }
  }
  last_button_state = button_state;
}

void power_init(void)
{
  pinMode(PWR_KEY_Input_PIN, INPUT); // Ensure IO 6 is set as GPIO at the top
  pinMode(PWR_Control_PIN, OUTPUT);
  wake_up();
}
