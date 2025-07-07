#pragma once
#include "Display_ST77916.h"

#define PWR_KEY_Input_PIN   6
#define PWR_Control_PIN     7
// ESP-IDF expects gpio_num_t, so cast these macros when used as arguments
#define PWR_KEY_Input_GPIO   ((gpio_num_t)PWR_KEY_Input_PIN)
#define PWR_Control_GPIO     ((gpio_num_t)PWR_Control_PIN)

#define Device_Sleep_Time    10
#define Device_Restart_Time  15
#define Device_Shutdown_Time 20

void Fall_Asleep(void);
void Shutdown(void);
void Restart(void);

void PWR_Init(void);
void PWR_Loop(void);