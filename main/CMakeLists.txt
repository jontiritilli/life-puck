idf_component_register(
    SRCS 
        "./LifePuck.cpp" 
        "./EXIO/TCA9554PWR.cpp"
        "./LCD_Driver/ST77916.cpp"
        "./Touch_Driver/CST816.cpp"  
        "./LVGL_Driver/LVGL_Driver.cpp"
        "./SD_Card/SD_MMC.cpp" 
        "./I2C_Driver/I2C_Driver.cpp"
        "./PCF85063/PCF85063.cpp"
        "./QMI8658/QMI8658.cpp"
        "./BAT_Driver/BAT_Driver.cpp"
        "./PWR_Key/PWR_Key.cpp"
        "./boot_screen.cpp"
        "./life_counter.cpp"
        "./arc_animation.cpp"

    INCLUDE_DIRS 
        "./LCD_Driver" 
        "./Touch_Driver"
        "./LVGL_Driver" 
        "./EXIO"
        "./SD_Card" 
        "./I2C_Driver"
        "./PCF85063"
        "./QMI8658"
        "./BAT_Driver"
        "./PWR_Key"
        "."

    PRIV_REQUIRES spi_flash driver esp_wifi esp_event nvs_flash esp_lcd_st77916 esp_lcd_touch esp_adc fatfs
    REQUIRES unity
)
