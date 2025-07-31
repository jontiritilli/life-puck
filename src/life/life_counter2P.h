#include <lvgl.h>
#include "constants/constants.h"
#include <helpers/event_grouper.h>

void init_life_counter_2P();
void reset_life_2p();
void life_counter2p_loop();
void teardown_life_counter_2P();

extern EventGrouper event_grouper_p1;
extern EventGrouper event_grouper_p2;