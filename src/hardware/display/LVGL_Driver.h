#pragma once

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>
#include <esp_heap_caps.h>

// ============================================
// Config
// ============================================
#include "config/lv_conf.h"
#include <board_config.h> 

// ============================================
// Hardware Drivers (same level/nearby)
// ============================================
#include "display_st77916.h"
#include "../touch/touch_cst816.h"


#ifndef LCD_WIDTH
    #define LCD_WIDTH   360  // Fallback
#endif

#ifndef LCD_HEIGHT
    #define LCD_HEIGHT    360  // Fallback
#endif

#define LVGL_BUF_LEN  (LCD_WIDTH * 80)
#define EXAMPLE_LVGL_TICK_PERIOD_MS  10

// Corrected function signatures for LVGL v9
void Lvgl_Display_Flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void Lvgl_Touchpad_Read(lv_indev_t *indev, lv_indev_data_t *data);

void Lvgl_Init(void);
void Lvgl_Loop(void);


// === 7-Parameter Touch Calibration Matrix API ===
extern float g_cal_matrix[7];
void updateTouchCalibrationMatrix(const float* matrix);
const float* getDefaultCalibrationMatrix();