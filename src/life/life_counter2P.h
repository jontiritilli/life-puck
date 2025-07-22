#include <lvgl.h>
#include "constants/constants.h"

#define ARC_OUTER_DIAMETER SCREEN_WIDTH // 1px margin all around for 360x360 screen

#define ARC_WIDTH 20
#define ARC_WIDTH_2P 16 // Slightly thinner for 2P mode

void init_life_counter_2P();
void reset_life();
void life_counter2p_loop();