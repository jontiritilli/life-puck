#pragma once
#include "driver/i2c_master.h"
#include "esp_mac.h"
#include "TCA9554PWR.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CST816_ADDR           0x15
#define CST816_SDA_PIN        1
#define CST816_SCL_PIN        3
#define CST816_INT_PIN        4
#define CST816_RST_PIN        -1 // EXIO1
#define I2C_MASTER_FREQ_HZ    400000 // I2C master clock frequency

#define CST816_LCD_TOUCH_MAX_POINTS (1)

// CST816 gesture codes
typedef enum {
    GESTURE_NONE = 0x00,
    GESTURE_SWIPE_UP = 0x01,
    GESTURE_SWIPE_DOWN = 0x02,
    GESTURE_SWIPE_LEFT = 0x03,
    GESTURE_SWIPE_RIGHT = 0x04,
    GESTURE_SINGLE_CLICK = 0x05,
    GESTURE_DOUBLE_CLICK = 0x0B,
    GESTURE_LONG_PRESS = 0x0C
} CST816_GESTURE;

// Register addresses
#define CST816_REG_GestureID      0x01
#define CST816_REG_Version        0x15
#define CST816_REG_ChipID         0xA7
#define CST816_REG_ProjID         0xA8
#define CST816_REG_FwVersion      0xA9
#define CST816_REG_AutoSleepTime  0xF9
#define CST816_REG_DisAutoSleep   0xFE

// Touch data structure
typedef struct {
    uint8_t points;           // Number of touch points
    CST816_GESTURE gesture;   // Gesture code
    uint16_t x;               // X coordinate
    uint16_t y;               // Y coordinate
} CST816_Touch;

extern CST816_Touch touch_data;

// API functions
uint8_t Touch_Init(void);
void Touch_Loop(void);
uint8_t CST816_Touch_Reset(void);
void CST816_AutoSleep(bool Sleep_State);
uint16_t CST816_Read_cfg(void);
const char* Touch_GestureName(void); // Changed to const char* for ESP-IDF compatibility
uint8_t Touch_Read_Data(void);
void example_touchpad_read(void);
void Touch_CST816_ISR(void);
#ifdef __cplusplus
}
#endif
