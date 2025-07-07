idf_component_register(
    SRCS
        "arc_animation.cpp"
        "BAT_Driver.cpp"
        "boot_screen.cpp"
        "Display_ST77916.cpp"
        "gestures.cpp"
        "Gyro_QMI8658.cpp"
        "I2C_Driver.cpp"
        "LCD_ST77916.c"
        "life_counter.cpp"
        "LifePuck.cpp"
        "LVGL_Driver.cpp"
        "menu.cpp"
        "PWR_Key.cpp"
        "RTC_PCF85063.cpp"
        "TCA9554PWR.cpp"
        "Touch_CST816.cpp"
        "Wireless.cpp"
    PRIV_REQUIRES spi_flash driver esp_wifi esp_event nvs_flash esp_lcd_st77916
    INCLUDE_DIRS "."
)
