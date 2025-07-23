#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <Arduino.h>

// Enum for life counter mode (if needed, add here)
// enum LifeCounterMode { ... } // Uncomment and define if you use an enum

extern int life_counter_mode;

BaseType_t create_task(TaskFunction_t task_function, const char *task_name, uint32_t stack_size, void *param, UBaseType_t priority, TaskHandle_t *task_handle = NULL);

#endif // MAIN_H
