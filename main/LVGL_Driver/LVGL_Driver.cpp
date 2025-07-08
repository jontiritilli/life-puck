#include "LVGL_Driver.h"

static const char *TAG_LVGL = "LVGL";

void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void example_lvgl_flush_cb(lv_display_t *disp_drv, const lv_area_t *area, unsigned char *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) lv_display_get_user_data(disp_drv);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 +1, offsety2 + 1, color_map);
    lv_display_flush_ready(disp_drv);
}

/*Read the touchpad*/
void example_touchpad_read(lv_indev_t * indev_drv, lv_indev_data_t * data )
{
    uint16_t touchpad_x[5] = {0};
    uint16_t touchpad_y[5] = {0};
    uint8_t touchpad_cnt = 0;

    /* Read touch controller data */
    esp_lcd_touch_read_data((esp_lcd_touch_handle_t)lv_indev_get_user_data(indev_drv));

    /* Get coordinates */
    bool touchpad_pressed = esp_lcd_touch_get_coordinates((esp_lcd_touch_handle_t)lv_indev_get_user_data(indev_drv), touchpad_x, touchpad_y, NULL, &touchpad_cnt, 5);

    // printf("CCCCCCCCCCCCC=%d  \r\n",touchpad_cnt);
    if (touchpad_pressed && touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
        // printf("X=%u Y=%u num=%d \r\n", data->point.x, data->point.y,touchpad_cnt);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
   
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
lv_display_t *disp;
void LVGL_Init(void)
{
    ESP_LOGI(TAG_LVGL, "Initialize LVGL library");
    lv_init();
    
    // Create display object
    disp = lv_display_create(EXAMPLE_LCD_WIDTH, EXAMPLE_LCD_HEIGHT);
    
    // Allocate draw buffers
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_LEN * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf1);
    lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_LEN * sizeof(lv_color_t) , MALLOC_CAP_SPIRAM);    
    assert(buf2);
    
    // Set the buffers for the display
    lv_display_set_buffers(disp, buf1, buf2, LVGL_BUF_LEN * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);

    ESP_LOGI(TAG_LVGL, "Register display driver to LVGL");
    // Set display flush callback and user data
    lv_display_set_flush_cb(disp, example_lvgl_flush_cb);
    lv_display_set_user_data(disp, panel_handle);
    
    ESP_LOGI(TAG_LVGL,"Register touch input device to LVGL");                                                 
    // Create input device for touch
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, example_touchpad_read);
    lv_indev_set_user_data(indev, tp);

    /********************* LVGL *********************/
    ESP_LOGI(TAG_LVGL, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

}

void Lvgl_Loop(void)
{
    lv_timer_handler();
}
