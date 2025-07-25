#include <lvgl.h>
#include "constants/constants.h"
#include <helpers/event_grouper.h>

void init_life_counter();
void reset_life();
void clear_amp();
void life_counter_loop();

// Extern so we can access from main.cpp
extern EventGrouper event_grouper;