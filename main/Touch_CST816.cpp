// ESP-IDF compatible implementation
#include "Touch_CST816.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

CST816_Touch touch_data = {0};
uint8_t Touch_interrupts=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I2C
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool I2C_Read_Touch(uint16_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
  // TODO: Replace with ESP-IDF I2C read implementation
  // Placeholder: always return true
  return true;
}
bool I2C_Write_Touch(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
  // TODO: Replace with ESP-IDF I2C write implementation
  // Placeholder: always return true
  return true;
}
/*!
    @brief  handle interrupts
*/
void Touch_CST816_ISR(void) {
  Touch_interrupts = true;
}

uint8_t Touch_Init(void) {
  // TODO: Initialize I2C using ESP-IDF
  CST816_Touch_Reset();
  uint16_t Verification = CST816_Read_cfg();
  CST816_AutoSleep(true);
  // Configure INT pin as input with pull-up
  gpio_set_direction((gpio_num_t)CST816_INT_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode((gpio_num_t)CST816_INT_PIN, GPIO_PULLUP_ONLY);
  // TODO: Attach interrupt using ESP-IDF gpio_isr_handler_add
  // Placeholder: not implemented
  return true;
}
/* Reset controller */
uint8_t CST816_Touch_Reset(void)
{
  Set_EXIO(EXIO_PIN1,Low);
  vTaskDelay(pdMS_TO_TICKS(10));
  Set_EXIO(EXIO_PIN1,High);
  vTaskDelay(pdMS_TO_TICKS(50));
  return true;
}
uint16_t CST816_Read_cfg(void) {
  uint8_t buf[3] = {0};
  I2C_Read_Touch(CST816_ADDR, CST816_REG_Version, buf, 1);
  printf("TouchPad_Version:0x%02x\r\n", buf[0]);
  I2C_Read_Touch(CST816_ADDR, CST816_REG_ChipID, buf, 3);
  printf("ChipID:0x%02x   ProjID:0x%02x   FwVersion:0x%02x \r\n",buf[0], buf[1], buf[2]);

  return true;
}
/*!
    @brief  Fall asleep automatically
*/
void CST816_AutoSleep(bool Sleep_State) {
  CST816_Touch_Reset();
  uint8_t Sleep_State_Set = (uint8_t)(!Sleep_State);
  I2C_Write_Touch(CST816_ADDR, CST816_REG_DisAutoSleep, &Sleep_State_Set, 1);
}

// reads sensor and touches
// updates Touch Points
uint8_t Touch_Read_Data(void) {
  uint8_t buf[6] = {0};
  uint8_t touchpad_cnt = 0;
  I2C_Read_Touch(CST816_ADDR, CST816_REG_GestureID, buf, 6);
  /* touched gesture */
  if (buf[0] != 0x00) 
    touch_data.gesture = (CST816_GESTURE)buf[0];
  if (buf[1] != 0x00) {        
    // TODO: Disable interrupts if needed (ESP-IDF)
    /* Number of touched points */
    touch_data.points = (uint8_t)buf[1];
    if(touch_data.points > CST816_LCD_TOUCH_MAX_POINTS)
        touch_data.points = CST816_LCD_TOUCH_MAX_POINTS;
    /* Fill coordinates */
    touch_data.x = ((buf[2] & 0x0F) << 8) + buf[3];               
    touch_data.y = ((buf[4] & 0x0F) << 8) + buf[5];
      
    // TODO: Enable interrupts if needed (ESP-IDF)
    // printf(" points=%d \r\n",touch_data.points);
  }
  return true;
}
void example_touchpad_read(void){
  Touch_Read_Data();
  if (touch_data.gesture != GESTURE_NONE ||  touch_data.points != 0x00) {
      printf("Touch : X=%u Y=%u points=%d\r\n",  touch_data.x , touch_data.y,touch_data.points);
  } else {
      // data->state = LV_INDEV_STATE_REL;
  }
}
void Touch_Loop(void){
  if(Touch_interrupts){
    Touch_interrupts = false;
    example_touchpad_read();
  }
}


/*!
    @brief  get the gesture event name
*/
const char* Touch_GestureName(void) {
  switch (touch_data.gesture) {
    case GESTURE_NONE:
      return "NONE";
    case GESTURE_SWIPE_DOWN:
      return "SWIPE DOWN";
    case GESTURE_SWIPE_UP:
      return "SWIPE UP";
    case GESTURE_SWIPE_LEFT:
      return "SWIPE LEFT";
    case GESTURE_SWIPE_RIGHT:
      return "SWIPE RIGHT";
    case GESTURE_SINGLE_CLICK:
      return "SINGLE CLICK";
    case GESTURE_DOUBLE_CLICK:
      return "DOUBLE CLICK";
    case GESTURE_LONG_PRESS:
      return "LONG PRESS";
    default:
      return "UNKNOWN";
  }
}