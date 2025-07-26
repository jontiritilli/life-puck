#pragma once
#include "lvgl.h"

#define SCREEN_WIDTH 360 // Default, or set dynamically at runtime
#define SCREEN_HEIGHT 360
#define SCREEN_DIAMETER SCREEN_WIDTH
#define ARC_WIDTH 8

typedef struct
{
  int start_angle;
  int end_angle;
  lv_color_t color;
} arc_segment_t;

// Grouper constants
#define PLAYER_SINGLE 0
#define PLAYER_ONE 1
#define PLAYER_TWO 2
#define GROUPER_WINDOW 1500

// Enum for menu states
enum MenuState
{
  MENU_NONE,
  MENU_CONTEXTUAL,
  MENU_SETTINGS,
  MENU_LIFE_CONFIG,
  MENU_HISTORY,
  MENU_BRIGHTNESS
};

enum ContextualQuadrant
{
  QUADRANT_TL = 0,
  QUADRANT_TR = 1,
  QUADRANT_BL = 2,
  QUADRANT_BR = 3,
  QUADRANT_EXIT = 4
};

// Color Constants (RGB format)
#include "lvgl.h"
#define GREEN_COLOR lv_color_hex(0x00e31f)
#define YELLOW_COLOR lv_color_hex(0xebf700)
#define RED_COLOR lv_color_hex(0xe80000)
#define LIGHTNING_BLUE_COLOR lv_color_hex(0x0070ff) // lightning blue
#define WHITE_COLOR lv_color_hex(0xffffff)
#define BLACK_COLOR lv_color_hex(0x000000)
#define GRAY_COLOR lv_color_hex(0x808080)
#define DARK_GRAY_COLOR lv_color_hex(0x404040)

// Constants for StateStore
#define PLAYER_STORE "player_store"
#define KEY_LIFE_MAX "life"
#define KEY_PLAYER_MODE "player_mode"
#define KEY_BRIGHTNESS "brightness"
#define KEY_AMP_MODE "amp_mode"
#define KEY_LIFE_STEP_SMALL "life_step_small"
#define KEY_LIFE_STEP_LARGE "life_step_large"
// define for life increment levels small and large
#define DEFAULT_LIFE_INCREMENT_SMALL 1
#define DEFAULT_LIFE_INCREMENT_LARGE 5
#define DEFAULT_LIFE_MAX 40