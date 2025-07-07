#include "driver/gpio.h"
#include "esp_lcd_st77916.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

// All display pin macros and parameters are now defined in Display_ST77916.h
#include "Display_ST77916.h"


void LCD_FillWhite(void); // Forward declaration
esp_lcd_panel_handle_t panel_handle = NULL;


// Implementation remains unchanged, but now uses EXAMPLE_LCD_PIN_NUM_BK_LIGHT macro from header
void Display_Backlight_Init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << (gpio_num_t)LCD_Backlight_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)LCD_Backlight_PIN, 1); // 1 = backlight ON, 0 = OFF
}

void LCD_Init(void) {

    // Manual QSPI bus config (match your ESP-IDF's spi_bus_config_t definition)
    const spi_bus_config_t buscfg = {
        .data0_io_num = ESP_PANEL_LCD_SPI_IO_DATA0,
        .data1_io_num = ESP_PANEL_LCD_SPI_IO_DATA1,
        .sclk_io_num = ESP_PANEL_LCD_SPI_IO_SCK,
        .data2_io_num = ESP_PANEL_LCD_SPI_IO_DATA2,
        .data3_io_num = ESP_PANEL_LCD_SPI_IO_DATA3,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .data_io_default_level = 0,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * 80 * sizeof(uint16_t),
        .flags = 0,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
        .intr_flags = 0,
    };
    esp_err_t err;
    err = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        printf("[DEBUG] spi_bus_initialize failed: %d\n", err);
        return;
    }

    esp_lcd_panel_io_handle_t io_handle = NULL;
    // Manual QSPI IO config (match your ESP-IDF's esp_lcd_panel_io_spi_config_t definition)
    const esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = EXAMPLE_LCD_PIN_NUM_CS,
        .dc_gpio_num = -1,
        .spi_mode = 0,
        .pclk_hz = 40 * 1000 * 1000,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 32,
        .lcd_param_bits = 8,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .flags = {
            .dc_high_on_cmd = 0,
            .dc_low_on_data = 0,
            .dc_low_on_param = 0,
            .octal_mode = 0,
            .quad_mode = 1,
            .sio_mode = 0,
            .lsb_first = 0,
            .cs_high_active = 0,
        },
    };
    err = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle);
    if (err != ESP_OK) {
        printf("[DEBUG] esp_lcd_new_panel_io_spi failed: %d\n", err);
        return;
    }

    static st77916_vendor_config_t vendor_config = {
        .init_cmds = NULL,
        .init_cmds_size = 0,
        .flags = { .use_qspi_interface = 1 },
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_LCD_PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        .bits_per_pixel = EXAMPLE_LCD_BIT_PER_PIXEL,
        .flags = { .reset_active_high = 0 },
        .vendor_config = (void *)&vendor_config,
    };
    err = esp_lcd_new_panel_st77916(io_handle, &panel_config, &panel_handle);
    if (err != ESP_OK) {
        printf("[DEBUG] esp_lcd_new_panel_st77916 failed: %d\n", err);
        return;
    }
    err = esp_lcd_panel_reset(panel_handle);
    if (err != ESP_OK) {
        printf("[DEBUG] esp_lcd_panel_reset failed: %d\n", err);
        return;
    }
    err = esp_lcd_panel_init(panel_handle);
    if (err != ESP_OK) {
        printf("[DEBUG] esp_lcd_panel_init failed: %d\n", err);
        return;
    }
    err = esp_lcd_panel_disp_on_off(panel_handle, true);
    if (err != ESP_OK) {
        printf("[DEBUG] esp_lcd_panel_disp_on_off failed: %d\n", err);
        return;
    }

    // Initialize backlight
    Display_Backlight_Init();
    printf("[DEBUG] Backlight initialized.\n");
    // LCD_FillWhite(); // Fill the screen with white for testing
    printf("[LCD] Initialized with resolution %dx%d\n", EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);
}

// Direct display test: fill the screen with solid white (bypassing LVGL)
void LCD_FillWhite(void) {
    if (!panel_handle) {
        printf("[DirectTest] panel_handle is NULL, cannot draw.\n");
        return;
    }
    size_t line_pixels = EXAMPLE_LCD_H_RES;
    uint16_t *line_buf = (uint16_t *)heap_caps_malloc(line_pixels * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!line_buf) {
        printf("[DirectTest] Failed to allocate line_buf.\n");
        return;
    }
    for (size_t i = 0; i < line_pixels; ++i) line_buf[i] = 0xFFFF; // RGB565 white
    for (size_t y = 0; y < EXAMPLE_LCD_V_RES; ++y) {
        esp_lcd_panel_draw_bitmap(panel_handle, 0, y, EXAMPLE_LCD_H_RES, y + 1, line_buf);
    }
    printf("[DirectTest] Filled screen with white (line by line).\n");
    heap_caps_free(line_buf);
}
