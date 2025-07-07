
#include "PWR_Key.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint8_t BAT_State = 0; 
static uint8_t Device_State = 0; 
static uint16_t Long_Press = 0;


void PWR_Loop(void)
{
  if(BAT_State){ 
    if(!gpio_get_level(PWR_KEY_Input_GPIO)){
      if(BAT_State == 2){         
        Long_Press ++;
        if(Long_Press >= Device_Sleep_Time){
          if(Long_Press >= Device_Sleep_Time && Long_Press < Device_Restart_Time)
            Device_State = 1;
          else if(Long_Press >= Device_Restart_Time && Long_Press < Device_Shutdown_Time)
            Device_State = 2;
          else if(Long_Press >= Device_Shutdown_Time)
            Shutdown(); 
        }
      }
    }
    else{
      if(BAT_State == 1)   
        BAT_State = 2;
      Long_Press = 0;
    }
  }
}
void Fall_Asleep(void)
{

}
void Restart(void)
{

}
void Shutdown(void)
{
  gpio_set_level(PWR_Control_GPIO, 0);
  LCD_Backlight = 0;           
}
void PWR_Init(void) {
  gpio_set_direction(PWR_KEY_Input_GPIO, GPIO_MODE_INPUT);
  gpio_set_direction(PWR_Control_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(PWR_Control_GPIO, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
  if(!gpio_get_level(PWR_KEY_Input_GPIO)) {
    BAT_State = 1;
    gpio_set_level(PWR_Control_GPIO, 1);
  }
}
