
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
#include <battery/battery_state.h>

extern esp_panel::board::Board *board;

static BatteryState BAT_State = BAT_OFF;
static uint32_t button_press_start = 0;

// Helper Functions
bool is_button_pressed(void)
{
  return (digitalRead(PWR_KEY_Input_PIN) == ButtonState::BUTTON_PRESSED);
}

// Check if USB is connected by detecting charging voltage
bool is_usb_connected(void)
{
  float voltage = battery_get_volts();
  return (voltage > USB_VOLTAGE_THRESHOLD);
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

  // Check wake-up reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  float voltage = battery_get_volts();
  printf("[wake_up] Wakeup reason: %d, Battery voltage: %.2fV\n", wakeup_reason, voltage);

  // ESP_SLEEP_WAKEUP_EXT0 = woken by power button
  // ESP_SLEEP_WAKEUP_UNDEFINED = power-on reset (USB plugged in or first boot)
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
  {
    // Woken by button press - boot the device
    printf("[wake_up] Button wake - booting device\n");
    digitalWrite(PWR_Control_PIN, HIGH);
    vTaskDelay(100);
    BAT_State = BAT_ON;
  }
  else
  {
    // Non-button wake (USB plug, spurious wake, etc.)
    printf("[wake_up] Non-button wake detected\n");

    // Check if this might be USB by checking voltage
    // Note: This threshold may become less accurate as battery ages
    float voltage = battery_get_volts();
    bool likely_usb = (voltage > USB_VOLTAGE_THRESHOLD);
    printf("[wake_up] Voltage: %.2fV, likely USB: %s\n", voltage, likely_usb ? "YES" : "NO");

    if (likely_usb)
    {
      // Likely USB - stay in stable idle to prevent flash loop
      printf("[wake_up] USB detected - entering stable idle mode\n");
      digitalWrite(PWR_Control_PIN, HIGH);
      vTaskDelay(100);

      uint32_t last_check = 0;
      while (true)
      {
        vTaskDelay(100);

        // Check for button press to boot
        if (is_button_pressed())
        {
          vTaskDelay(50); // Debounce
          if (is_button_pressed() && wait_for_button_hold(Device_Wake_Time))
          {
            printf("[wake_up] Button held - booting device\n");
            BAT_State = BAT_ON;
            return; // Exit to boot
          }
        }

        // Check voltage every 2 seconds
        uint32_t now = millis();
        if (now - last_check > 2000)
        {
          float current_voltage = battery_get_volts();
          if (current_voltage < USB_VOLTAGE_THRESHOLD) // USB disconnected
          {
            printf("[wake_up] USB disconnected (%.2fV) - entering deep sleep\n", current_voltage);
            break; // Exit to deep sleep below
          }
          last_check = now;
        }
      }
    }

    // No USB detected - power down and deep sleep
    printf("[wake_up] No USB - powering down\n");
    digitalWrite(PWR_Control_PIN, LOW);
    vTaskDelay(100);

    // Configure and enter deep sleep
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PWR_KEY_Input_PIN, HIGH);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);

    printf("[wake_up] Entering deep sleep\n");
    vTaskDelay(100);
    esp_deep_sleep_start();

    // Never reaches here
    BAT_State = BAT_OFF;
  }
}

void fall_asleep(void)
{
  printf("[fall_asleep] Shutting down\n");

  // Turn off display
  if (board && board->getBacklight())
  {
    board->getBacklight()->off();
  }

  // Disable peripherals
  esp_wifi_stop();
  esp_bt_controller_disable();

  // Check if USB is connected
  float voltage = battery_get_volts();
  bool usb_connected = (voltage > USB_VOLTAGE_THRESHOLD);
  printf("[fall_asleep] USB connected: %s (%.2fV)\n", usb_connected ? "YES" : "NO", voltage);

  if (usb_connected)
  {
    // USB connected - keep power on but display off
    printf("[fall_asleep] USB connected - entering low power mode with display off\n");
    digitalWrite(PWR_Control_PIN, HIGH);
    vTaskDelay(100);

    // Simple loop: wait for USB disconnect or button press, then deep sleep
    uint32_t last_check = 0;
    while (true)
    {
      vTaskDelay(100);

      // Check for button press to wake
      if (is_button_pressed())
      {
        vTaskDelay(50); // Debounce
        if (is_button_pressed() && wait_for_button_hold(Device_Wake_Time))
        {
          printf("[fall_asleep] Button held - waking device\n");
          BAT_State = BAT_ON;

          // Re-enable display
          if (board && board->getBacklight())
          {
            board->getBacklight()->on();
            int brightness = player_store.getInt(KEY_BRIGHTNESS, 100);
            board->getBacklight()->setBrightness(brightness);
          }

          return; // Exit to main loop
        }
      }

      // Check voltage every 2 seconds
      uint32_t now = millis();
      if (now - last_check > 2000) // Check every 2 seconds
      {
        float current_voltage = battery_get_volts();

        if (current_voltage < USB_VOLTAGE_THRESHOLD) // USB disconnected
        {
          printf("[fall_asleep] USB disconnected (%.2fV) - entering deep sleep\n", current_voltage);
          digitalWrite(PWR_Control_PIN, LOW);
          vTaskDelay(100);
          break; // Exit loop to deep sleep below
        }

        last_check = now;
      }
    }
  }
  else
  {
    // No USB - power off immediately
    printf("[fall_asleep] No USB - powering off\n");
    digitalWrite(PWR_Control_PIN, LOW);
    vTaskDelay(100);
  }

  // Configure deep sleep
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PWR_KEY_Input_PIN, HIGH);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);

  printf("[fall_asleep] Entering deep sleep\n");
  vTaskDelay(100);
  esp_deep_sleep_start();
}

void power_loop(void)
{
  if (BAT_State == BAT_ON)
  {
    // Check for button hold to sleep
    if (is_button_pressed())
    {
      if (button_press_start == 0)
      {
        button_press_start = millis();
      }

      uint32_t held_time = millis() - button_press_start;
      if (held_time >= Device_Sleep_Time)
      {
        printf("[power_loop] Button held - entering sleep\n");
        button_press_start = 0;
        fall_asleep();
        // After deep sleep wake, execution starts from setup()
        // This line only reached if we return from USB charging loop
      }
    }
    else
    {
      // Button released - reset timer
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
