/*****************************************************************************
  | File        :   LVGL_Driver.c
  
  | help        : 
    The provided LVGL library file must be installed first
******************************************************************************/


#include "LVGL_Driver.h"
#include "Display_sT77916.h"
#include "esp_lcd_panel_ops.h"
#include <esp_timer.h>



static lv_draw_buf_t draw_buf;
static lv_color_t *buf1 = NULL;
// static lv_color_t buf2[ LVGL_BUF_LEN]; // Unused buffer removed to avoid warning
// static lv_color_t* buf1 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN , MALLOC_CAP_SPIRAM);
// static lv_color_t* buf2 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN , MALLOC_CAP_SPIRAM);
    
/*  Display flushing 
    Displays LVGL content on the LCD
    This function implements associating LVGL data to the LCD screen
*/
// Updated flush callback for LVGL v9+ (expects uint8_t *color_p)
void Lvgl_Display_LCD(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *color_p)
{
    extern esp_lcd_panel_handle_t panel_handle;
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (void *)color_p);
    lv_disp_flush_ready(disp_drv);
}
/*Read the touchpad*/
void Lvgl_Touchpad_Read( lv_indev_t * indev_drv, lv_indev_data_t * data )
{
  Touch_Read_Data();
  if (touch_data.points != 0x00) {
    data->point.x = touch_data.x;
    data->point.y = touch_data.y;
    data->state = LV_INDEV_STATE_PR;
    // printf("LVGL : X=%u Y=%u points=%d\r\n",  touch_data.x , touch_data.y,touch_data.points);
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
  if (touch_data.gesture != GESTURE_NONE ) {    
  }
  touch_data.x = 0;
  touch_data.y = 0;
  touch_data.points = 0;
  touch_data.gesture = GESTURE_NONE;
}
void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}
void example_increase_lvgl_Loop_tick(void *arg)
{
  lv_timer_handler(); /* let the GUI do its work */
}
void Lvgl_Init(void)
{

    lv_init();

    // Allocate a partial LVGL buffer in internal RAM (DMA capable)
    buf1 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if (!buf1) {
        printf("[LVGL] ERROR: Failed to allocate display buffer in internal RAM!\n");
        abort();
    }
    // Initialize the draw buffer for LVGL v9+ (partial buffer, RGB565, stride auto)
    lv_draw_buf_init(
        &draw_buf,
        LCD_WIDTH,
        LCD_HEIGHT, // Use LVGL_BUF_SIZE for height
        LV_COLOR_FORMAT_RGB565, // 16-bit color format
        0,                      // stride: 0 for auto
        buf1,                   // pointer to buffer
        LVGL_BUF_SIZE * sizeof(lv_color_t) // size of buffer in bytes
    );

    /* Initialize the display */
    lv_display_t *disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_flush_cb(disp, Lvgl_Display_LCD);
    lv_display_set_draw_buffers(disp, &draw_buf, NULL);

    /* Initialize the (dummy) input device driver */
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, Lvgl_Touchpad_Read);

    // Removed default label to allow boot screen to show

    /* Timer for LVGL tick (requires <esp_timer.h>) */
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lvgl_tick",
        .skip_unhandled_events = false
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);
}
void Lvgl_Loop(void)
{
  lv_timer_handler(); /* let the GUI do its work */
}
