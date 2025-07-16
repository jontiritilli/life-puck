/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "gui_components.h"
#include "power_key/power_key.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

// Limit tasks to run on the ESP32’s application CPU (CPU1)
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

esp_panel::board::Board *board = new esp_panel::board::Board();

// Function to create a FreeRTOS task
BaseType_t create_task(TaskFunction_t task_function, const char *task_name, uint32_t stack_size, void *param, UBaseType_t priority, TaskHandle_t *task_handle = NULL);

/* LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes */
#define BUFFER_SIZE (SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint16_t) / 8)

/* Forward declaration for flush_cb */
void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map);

/* Forward declaration for gui_task */
void gui_task(void *pvParameters);

void setup()
{
  Serial.begin(115200);

  Serial.println("Initializing board");
  power_init();
  board->init();
  assert(board->begin());
  create_task(gui_task, "gui_task", 4096, NULL, 1, NULL);
}

void loop()
{
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  power_loop();
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
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }

  // Step 3: Set display buffers and flush callback
  lv_display_set_buffers(display, buf1, buf2, BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display, flush_cb);

  Serial.println("Creating UI");

  ui_init();

  // Step 4: Main GUI loop (LVGL 9.3)
  while (1)
  {
    uint32_t time_till_next = 5;

    // Timer handler needs to be called periodically to handle the tasks of LVGL
    time_till_next = lv_timer_handler(); // lv_task_handler() is aparently lvgl v8 only
    // ui_tick();

    if (time_till_next != LV_NO_TIMER_READY)           // Handle LV_NO_TIMER_READY (-1)
      vTaskDelay(time_till_next / portTICK_PERIOD_MS); // Delay to avoid unnecessary polling
  }
}

void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
  int width = area->x2 - area->x1 + 1;
  int height = area->y2 - area->y1 + 1;
  board->getLCD()->drawBitmap(area->x1, area->y1, width, height, px_map);
  lv_display_flush_ready(display);
}