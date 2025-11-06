#include "lvgl_driver.h"
#include "hardware/system/power_management.h"
#include "ui/screens/menu/menu.h"
#include "config.h"
#include <cstring>

static lv_display_t *display;
static lv_indev_t *indev;

// ============================================
// === ZENTRALE DEFAULT-KALIBRIERUNG ===
// ============================================
const float DEFAULT_CAL_MATRIX[7] = {
    TOUCH_CAL_DEFAULT_OFFSET_X,
    TOUCH_CAL_DEFAULT_SCALE_X,
    TOUCH_CAL_DEFAULT_SHEAR_XY,
    TOUCH_CAL_DEFAULT_OFFSET_Y,
    TOUCH_CAL_DEFAULT_SHEAR_YX,
    TOUCH_CAL_DEFAULT_SCALE_Y,
    TOUCH_CAL_DEFAULT_DIVISOR
};

float g_cal_matrix[7];

void updateTouchCalibrationMatrix(const float* matrix) {
    if (matrix) {
        memcpy(g_cal_matrix, matrix, sizeof(g_cal_matrix));
        
        printf("\n========================================\n");
        printf("[CALIBRATION] g_cal_matrix updated:\n");
        printf("  [0] offset_x  = %.3f\n", g_cal_matrix[0]);
        printf("  [1] scale_x   = %.3f\n", g_cal_matrix[1]);
        printf("  [2] shear_xy  = %.3f\n", g_cal_matrix[2]);
        printf("  [3] offset_y  = %.3f\n", g_cal_matrix[3]);
        printf("  [4] shear_yx  = %.3f\n", g_cal_matrix[4]);
        printf("  [5] scale_y   = %.3f\n", g_cal_matrix[5]);
        printf("  [6] divisor   = %.3f\n", g_cal_matrix[6]);
        printf("========================================\n\n");
    }
}

const float* getDefaultCalibrationMatrix() {
    return DEFAULT_CAL_MATRIX;
}

static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_LEN * sizeof(lv_color_t), MALLOC_CAP_DMA);
static lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_LEN * sizeof(lv_color_t), MALLOC_CAP_DMA);

void Lvgl_Display_Flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    LCD_addWindow(area->x1, area->y1, area->x2, area->y2, (uint16_t *)px_map);
    lv_display_flush_ready(disp);
}

void Lvgl_Touchpad_Read(lv_indev_t *indev, lv_indev_data_t *data) {
    Touch_Read_Data();
    
    if (getCurrentMenu() == MENU_TOUCH_CALIBRATION) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    
    if (touch_data.points != 0) {
        power_reset_inactivity_timer();
        
        if (power_should_ignore_touch()) {
            data->state = LV_INDEV_STATE_RELEASED;
            return;
        }
    }
    
    if (touch_data.points != 0) {
        int raw_x = touch_data.x;
        int raw_y = touch_data.y;

        // Apply 7-parameter matrix transformation
        float cal_x = (g_cal_matrix[1] * raw_x) + (g_cal_matrix[2] * raw_y) + g_cal_matrix[0];
        float cal_y = (g_cal_matrix[4] * raw_x) + (g_cal_matrix[5] * raw_y) + g_cal_matrix[3];
        cal_x /= g_cal_matrix[6];
        cal_y /= g_cal_matrix[6];

        int final_x = (int)(cal_x + 0.5f);
        int final_y = (int)(cal_y + 0.5f);

        // Edge correction for origin-based scaling effect
        // Left edge (X < 120): Correct up to 40px rightward
        if (final_x < 120) {
            float correction_factor = (120.0f - final_x) / 120.0f;
            final_x += (int)(correction_factor * 20.0f);
        }
        
        // Top edge (Y < 120): Correct up to 40px downward
        if (final_y < 120) {
            float correction_factor = (120.0f - final_y) / 120.0f;
            final_y += (int)(correction_factor * 20.0f);
        }

        // Clamp to screen bounds
        if (final_x < 0) final_x = 0;
        if (final_y < 0) final_y = 0;
        if (final_x >= LCD_WIDTH) final_x = LCD_WIDTH - 1;
        if (final_y >= LCD_HEIGHT) final_y = LCD_HEIGHT - 1;
        
        data->point.x = final_x;
        data->point.y = final_y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    
    touch_data.points = 0;
    touch_data.gesture = NONE;
}

static void lv_tick_task(void *arg) {
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void Lvgl_Init(void) {
    lv_init();

    if (!buf1 || !buf2) {
        printf("[LVGL_ERROR] Buffer allocation FAILED! Not enough DMA memory!\n");
        return;
    }
    
    display = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_buffers(display, buf1, buf2, LVGL_BUF_LEN * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, Lvgl_Display_Flush);
    
    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, Lvgl_Touchpad_Read);
    lv_indev_set_display(indev, display);

    const esp_timer_create_args_t lvgl_tick_timer_args = { .callback = &lv_tick_task, .name = "lvgl_tick" };
    esp_timer_handle_t lvgl_tick_timer;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);

    printf("[LVGL_Init] COMPLETE with 3-round calibration + edge correction\n");
}

void Lvgl_Loop(void) {
    lv_timer_handler();
}
