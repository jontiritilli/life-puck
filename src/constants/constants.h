#pragma once
#include "lvgl.h"

#define SCREEN_WIDTH 360 // Default, or set dynamically at runtime
#define SCREEN_HEIGHT 360
#define SCREEN_DIAMETER SCREEN_WIDTH
#define ARC_WIDTH 5
#define LIFE_STD_START 40

typedef struct
{
  int start_angle;
  int end_angle;
  lv_color_t color;
} arc_segment_t;

// Enum for menu states
enum MenuState
{
  MENU_NONE,
  MENU_CONTEXTUAL,
  MENU_SETTINGS,
  MENU_START_LIFE,
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
#define WHITE_COLOR lv_color_hex(0xffffff)
#define BLACK_COLOR lv_color_hex(0x000000)
#define GRAY_COLOR lv_color_hex(0x808080)
#define DARK_GRAY_COLOR lv_color_hex(0x404040)

// Constants for StateStore
#define PLAYER_STORE "player_store"
#define KEY_LIFE "life"
#define KEY_PLAYER_MODE "player_mode"
#define KEY_BRIGHTNESS "brightness"