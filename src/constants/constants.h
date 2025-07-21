#pragma once

#define SCREEN_WIDTH 360 // Default, or set dynamically at runtime
#define SCREEN_HEIGHT 360

#define LIFE_STD_START 40

// Enum for menu states
enum MenuState
{
  MENU_NONE,
  MENU_CONTEXTUAL,
  MENU_SETTINGS,
  MENU_START_LIFE,
  MENU_HISTORY
};

enum ContextualQuadrant
{
  QUADRANT_TL = 0,
  QUADRANT_TR = 1,
  QUADRANT_BL = 2,
  QUADRANT_BR = 3
};

// Color Constants (RGB format)
#include "lvgl.h"
#define GREEN_COLOR lv_color_hex(0x00e31f)
#define YELLOW_COLOR lv_color_hex(0xebf700)
#define RED_COLOR lv_color_hex(0xe80000)

// Constants for StateStore
#define PLAYER_STORE "player_store"
#define KEY_LIFE "life"
#define KEY_PLAYER_MODE "player_mode"