#include <lvgl.h>
#include "constants/constants.h"
#include <helpers/event_grouper.h>

#define ARC_OUTER_DIAMETER SCREEN_WIDTH // 1px margin all around for 360x360 screen

#define ARC_WIDTH 20
#define ARC_WIDTH_2P 16 // Slightly thinner for 2P mode

void init_life_counter_2P();
void reset_life_p1();
void reset_life_p2();
void life_counter2p_loop();

extern EventGrouper event_grouper_p1;
extern EventGrouper event_grouper_p2;