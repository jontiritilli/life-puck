#include <lvgl.h>
#include "constants/constants.h"
#include <helpers/event_grouper.h>

#define ARC_OUTER_DIAMETER SCREEN_WIDTH // 1px margin all around for 360x360 screen

#define ARC_WIDTH 20

void init_life_counter();
void reset_life();
void life_counter_loop();

// Extern so we can access from main.cpp
extern EventGrouper event_grouper;