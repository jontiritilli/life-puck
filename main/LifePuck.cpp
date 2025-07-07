#include "Display_ST77916.h"
#include "boot_screen.h"
#include "life_counter.h"
#include "gestures.h"
#include "arc_animation.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LCD_ST77916.h"
#include "I2C_Driver.h"
#include "Touch_CST816.h"
#include "Display_ST77916.h"
#include "Gyro_QMI8658.h"
#include "PWR_Key.h"
#include "life_counter.h"
#include "LVGL_Driver.h"
#include "Wireless.h"
#include "BAT_Driver.h"
#include "RTC_PCF85063.h"

void Psram_Inquiry() {
  char buffer[128];    /* Make sure buffer is enough for `sprintf` */
  sprintf(buffer, "   Biggest /     Free /    Total\n"
          "\t  SRAM : [%8d / %8d / %8d]\n"
          "\t PSRAM : [%8d / %8d / %8d]",
          heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
          heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
          heap_caps_get_total_size(MALLOC_CAP_INTERNAL),
          heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM),
          heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
          heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
  printf("MEM : %s\r\n", buffer);
}

void DriverTask(void *parameter) {
  Wireless_Test2();
  while(1){
    PWR_Loop();
    BAT_Get_Volts();
    RTC_Loop();
    QMI8658_Loop(); 
    // Psram_Inquiry();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void Driver_Loop() {
  xTaskCreatePinnedToCore(
    DriverTask,           
    "DriverTask",         
    4096,                 
    NULL,                 
    3,                    
    NULL,                 
    0                     
  );  
}

void setup() {
  I2C_Init();
  Touch_Init();
  LCD_Init();
  Lvgl_Init();
  boot_screen_show();
  // Life counter will be initialized after boot screen
}

int Time_Loop=0;
void loop() {
    boot_screen_update();
    Lvgl_Loop();
    if (!boot_screen_active()) {
        static bool app_shown = false;
        if (!app_shown) {
            life_counter_init(20, nullptr); // Show main life counter screen
            app_shown = true;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(5));
}

extern "C" void app_main(void)
{
    setup();
    while (1) {
        loop();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
