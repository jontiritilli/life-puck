#pragma once
#include "ST77916.h"

#define PWR_KEY_Input_PIN GPIO_NUM_6
#define PWR_Control_PIN GPIO_NUM_7

#define Device_Sleep_Time 10
#define Device_Restart_Time 15
#define Device_Shutdown_Time 20

void Fall_Asleep(void);
void Shutdown(void);
void Restart(void);

void configure_GPIO(gpio_num_t pin, gpio_mode_t Mode);
void PWR_Init(void);
void PWR_Loop(void);