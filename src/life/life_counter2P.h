#include <lvgl.h>
#include "constants/constants.h"
#include <helpers/event_grouper.h>

void init_life_counter_2P();
void reset_life_p1();
void reset_life_p2();
void life_counter2p_loop();

extern EventGrouper event_grouper_p1;
extern EventGrouper event_grouper_p2;