#pragma once
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include <lvgl.h>
#include "demos/lv_demos.h"

#include "../LCD_Driver/ST77916.h"
#include "../Touch_Driver/CST816.h"

#define LVGL_BUF_LEN  (EXAMPLE_LCD_WIDTH * EXAMPLE_LCD_HEIGHT / 20)
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2

extern lv_display_t *disp;    

void example_lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, unsigned char *color_map);
void example_increase_lvgl_tick(void *arg);
void Lvgl_Loop(void);

void LVGL_Init(void);                     // Call this function to initialize the screen (must be called in the main function) !!!!!