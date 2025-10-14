#include <Arduino.h>
#include <lvgl.h>
#include "ArduinoNvs.h"
#include "gui_main.h"
#include <esp_display_panel.hpp>
#include "power_key/power_key.h"
#include "constants/constants.h"
#include "battery/battery_state.h"
#include <life/life_counter.h>
#include <life/life_counter2P.h>
#include <helpers/event_grouper.h>
#include "main.h"
#include "state/state_store.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

// Limit tasks to run on the ESP32’s application CPU (CPU1)
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

esp_panel::board::Board *board = new esp_panel::board::Board();

// Global input device reference for gesture configuration
lv_indev_t *global_indev = nullptr;

// Function to create a FreeRTOS task
BaseType_t create_task(TaskFunction_t task_function, const char *task_name, uint32_t stack_size, void *param, UBaseType_t priority, TaskHandle_t *task_handle);

/* LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes */
#define BUFFER_SIZE (SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint16_t) / 10)

/* Forward declaration for flush_cb */
void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map);

/* Forward declaration for gui_task */
void gui_task(void *pvParameters);

// Global mode variable (should be updated by UI logic)
PlayerMode life_counter_mode = PLAYER_MODE_ONE_PLAYER;

void setup()
{
  Serial.begin(115200);
  Serial.println("[setup] Serial initialized");

  pinMode(PWR_KEY_Input_PIN, INPUT);
  pinMode(PWR_Control_PIN, OUTPUT);
  Serial.println("[setup] Initializing board");
  board->init();
  assert(board->begin());
  board->getBacklight()->off();
  battery_init();
  create_task(gui_task, "gui_task", 16384, NULL, 1, NULL);
  power_init();
}

void loop()
{
  vTaskDelay(10 / portTICK_PERIOD_MS);
  power_loop();
  if (life_counter_mode == PLAYER_MODE_ONE_PLAYER) // Single player mode
  {
    life_counter_loop();
  }
  else if (life_counter_mode == PLAYER_MODE_TWO_PLAYER) // Two player mode
  {
    life_counter2p_loop();
  }
}

BaseType_t create_task(TaskFunction_t task_function, const char *task_name, uint32_t stack_size, void *param, UBaseType_t priority, TaskHandle_t *task_handle)
{
  return xTaskCreatePinnedToCore(
      task_function, // Function to be called
      task_name,     // Name of task
      stack_size,    // Stack size (bytes in ESP32)
      param,         // Parameter to pass to function
      priority,      // Task priority (0 to configMAX_PRIORITIES - 1)
      task_handle,   // Task handle
      app_cpu        // Run on core 1 (change to 0 for core 0)
  );
}

void gui_task(void *pvParameters)
{
  // Wait for device to be actually powered on (not just charging)
  Serial.println("[gui_task] Waiting for device to boot...");
  while (get_battery_state() != BAT_ON)
  {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  Serial.println("Initializing LVGL");
  lv_init();
  lv_tick_set_cb(xTaskGetTickCount);

  // Step 1: Create display object (LVGL 9.3)
  lv_display_t *display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);

  // Step 2: Allocate display buffers
  uint32_t *buf1 = (uint32_t *)malloc(BUFFER_SIZE / 2 * sizeof(uint32_t));
  uint32_t *buf2 = (uint32_t *)malloc(BUFFER_SIZE / 2 * sizeof(uint32_t));
  if (!buf1 || !buf2)
  {
    Serial.println("[LVGL] ERROR: Display buffer allocation failed!");
    while (1)
    {
      vTaskDelay(20 / portTICK_PERIOD_MS);
    }
  }

  // Step 3: Set display buffers and flush callback
  lv_display_set_buffers(display, buf1, buf2, BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display, flush_cb);

  // Clear display to black before creating UI to prevent static flash
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);

  Serial.println("Creating UI");

  // Initialize touch first so we have the indev reference
  global_indev = init_touch();
  ui_init(global_indev);

  // Render black screen first to eliminate static flash
  lv_refr_now(display);
  vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for display to fully update

  // Now turn on backlight with clean black screen showing
  board->getBacklight()->on();
  board->getBacklight()->setBrightness(player_store.getInt(KEY_BRIGHTNESS, 100));
  // Step 4: Main GUI loop (LVGL 9.3)
  while (1)
  {
    uint32_t time_till_next = 5;

    // Timer handler needs to be called periodically to handle the tasks of LVGL
    time_till_next = lv_timer_handler();

    if (time_till_next != LV_NO_TIMER_READY)           // Handle LV_NO_TIMER_READY (-1)
      vTaskDelay(time_till_next / portTICK_PERIOD_MS); // Delay to avoid unnecessary polling
  }
}

void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
  lv_draw_sw_rgb565_swap(
      px_map, (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1));

  const int offsetx1 = area->x1;
  const int offsetx2 = area->x2;
  const int offsety1 = area->y1;
  const int offsety2 = area->y2;
  int width = offsetx2 - offsetx1 + 1;
  int height = offsety2 - offsety1 + 1;
  board->getLCD()->drawBitmap(offsetx1, offsety1, width, height, px_map);
  lv_display_flush_ready(display);
}