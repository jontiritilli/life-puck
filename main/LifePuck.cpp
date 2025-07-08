#include "LCD_Driver/ST77916.h"
#include "boot_screen.h"
#include "life_counter.h"
#include "gestures.h"
#include "arc_animation.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "I2C_Driver/I2C_Driver.h"
#include "Touch_Driver/CST816.h"
#include "QMI8658/QMI8658.h"
#include "PWR_Key/PWR_Key.h"
#include "life_counter.h"
#include "LVGL_Driver/LVGL_Driver.h"
#include "BAT_Driver/BAT_Driver.h"
#include "PCF85063/PCF85063.h"
#include "SD_Card/SD_MMC.h"

void Driver_Loop(void *parameter)
{
  while (1)
  {
    QMI8658_Loop();
    PCF85063_Loop();
    BAT_Get_Volts();
    PWR_Loop();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  vTaskDelete(NULL);
}
void Driver_Init(void)
{
  PWR_Init();
  BAT_Init();
  I2C_Init();
  EXIO_Init(); // Example Initialize EXIO
  Flash_Searching();
  PCF85063_Init();
  QMI8658_Init();
  xTaskCreatePinnedToCore(
      Driver_Loop,
      "Other Driver task",
      4096,
      NULL,
      3,
      NULL,
      0);
}
extern "C" void app_main(void)
{
  Driver_Init();

  SD_Init();
  LCD_Init();
  LVGL_Init(); // returns the screen object

  boot_screen_show();
  // lv_demo_widgets();
  // lv_demo_keypad_encoder();
  // lv_demo_benchmark();
  // lv_demo_stress();
  // lv_demo_music();

  while (1)
  {
    if (!boot_screen_active())
    {
      static bool app_shown = false;
      if (!app_shown)
      {
        life_counter_init(20, nullptr); // Show main life counter screen
        app_shown = true;
      }
    }
    // raise the task priority of LVGL and/or reduce the handler period can improve the performance
    vTaskDelay(pdMS_TO_TICKS(10));
    // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
    lv_timer_handler();
  }
}