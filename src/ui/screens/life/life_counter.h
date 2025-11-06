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


void init_life_counter();
void reset_life();
void life_counter_loop();
void teardown_life_counter();
void refresh_life_counter_theme();
void resetLastLoadedTheme();  // Reset theme tracker for preset changes  // NEW: Refresh theme without full re-render

// *** PERSISTENT LIFE STORAGE ***
void saveLifeToNVS(int life_value, int player = 1);
int loadLifeFromNVS(int player = 1);
void clearSavedLife();

// Extern so we can access from main.cpp
extern EventGrouper event_grouper;