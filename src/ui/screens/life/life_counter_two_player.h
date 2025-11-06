// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/event_grouper.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"

void init_life_counter_2P();
void reset_life_2p();
void life_counter2p_loop();
void teardown_life_counter_2P();
void refresh_life_counter_2p_theme();
void resetLastLoadedTheme2P();  // Reset theme tracker for preset changes  // NEW: Refresh theme without full re-render

extern EventGrouper event_grouper_p1;
extern EventGrouper event_grouper_p2;