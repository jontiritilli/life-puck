#pragma once
#include "lvgl.h"
#include "config.h"

#define SCREEN_WIDTH 360
#define SCREEN_HEIGHT 360
#define SCREEN_DIAMETER SCREEN_WIDTH
#define ARC_WIDTH 8

// *** SMOOTH ANIMATION CONSTANTS ***
#define SMOOTH_ARC_STEPS 1000        // High-resolution animation steps for smooth arcs
// ARC_ANIMATION_DURATION now defined in config.h

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
// GROUPER_WINDOW now defined in config.h

enum PlayerMode
{
  PLAYER_MODE_ONE_PLAYER = 0,
  PLAYER_MODE_TWO_PLAYER = 1
};

// Enum for menu states
enum MenuState
{
  MENU_NONE,
  MENU_CONTEXTUAL,
  MENU_SETTINGS,
  MENU_LIFE_CONFIG,
  MENU_HISTORY,
  MENU_BRIGHTNESS,
  MENU_TCG, 
  MENU_DICE_LIST,
  MENU_PRESET_LIST,
  MENU_PRESET_EDITOR,    // Preset Editor
  MENU_TOUCH_CALIBRATION, // NEW: Touch Calibration
  MENU_AUDIO_SETTINGS,   // NEW: Audio Settings Submenu
  MENU_TIMER_SETTINGS,   // NEW: Timer Settings Submenu
  MENU_POWER_SETTINGS,   // NEW: Power Settings Submenu
  MENU_THEME_SETTINGS,   // NEW: Theme Settings Submenu
  MENU_TOUCH_SETTINGS    // NEW: Touch Settings Submenu
};

enum ContextualQuadrant
{
  QUADRANT_TL = 0,
  QUADRANT_TR = 1,
  QUADRANT_BL = 2,
  QUADRANT_BR = 3,
  QUADRANT_EXIT = 4
};

// Color Constants
#define GREEN_COLOR lv_color_hex(0x00e31f)
#define YELLOW_COLOR lv_color_hex(0xebf700)
#define RED_COLOR lv_color_hex(0xe80000)
#define LIGHTNING_BLUE_COLOR lv_color_hex(0x0070ff)
#define WHITE_COLOR lv_color_hex(0xffffff)
#define BLACK_COLOR lv_color_hex(0x000000)
#define GRAY_COLOR lv_color_hex(0x808080)
#define DARK_GRAY_COLOR lv_color_hex(0x404040)

// Constants for StateStore
#define PLAYER_STORE "player_store"
#define KEY_LIFE_MAX "life"
#define KEY_PLAYER_MODE "player_mode"
#define KEY_BRIGHTNESS "brightness"
#define KEY_LIFE_STEP_SMALL "life_step_small"
#define KEY_LIFE_STEP_LARGE "life_step_large"
#define KEY_SHOW_TIMER "show_timer"
#define KEY_SWIPE_TO_CLOSE "swipe_close"     // Swipe to dismiss menus/overlays
#define KEY_GESTURE_MODE "gesture_mode"      // Gesture control mode (0=Classic, 1=Long Press)
#define KEY_THEME_MODE "theme_mode"          // Theme Mode (0=OFF, 1=Automatic, 2=Manual)
#define KEY_THEME_SELECTION "theme_sel"      // Theme Selection (0=Default, 1=Yugioh, 2=Magic, 3=Pokemon)

// Custom Preset Theme NVS Keys
#define KEY_PRESET_7_THEME "preset_7_theme"  // Custom 8 theme
#define KEY_PRESET_8_THEME "preset_8_theme"  // Custom 9 theme
#define KEY_PRESET_9_THEME "preset_9_theme"  // Custom 10 theme

// *** PERSISTENT LIFE VALUES ***
#define KEY_SAVED_LIFE_P1 "saved_life_p1"    // Single player or Player 1 life (legacy)
#define KEY_SAVED_LIFE_P2 "saved_life_p2"    // Player 2 life (two-player mode) (legacy)
#define KEY_LIFE_SAVE_VALID "life_save_valid" // Flag: 1 if saved values are valid (legacy)

// *** NEW IMPROVED LIFE SYSTEM ***
#define KEY_CURRENT_LIFE_P1 "cur_life_p1" // Player 1 current life (max 15 chars for NVS)
#define KEY_CURRENT_LIFE_P2 "cur_life_p2" // Player 2 current life  
#define KEY_CURRENT_LIFE_VALID "cur_life_valid" // Flag: 1 if current life is valid

// define for life increment levels small and large
#define DEFAULT_LIFE_INCREMENT_SMALL 1
#define DEFAULT_LIFE_INCREMENT_LARGE 5
#define DEFAULT_LIFE_MAX 40

enum step_size_t
{
  STEP_SIZE_SMALL = 1,
  STEP_SIZE_LARGE = 2
};

// Theme System Enums
enum ThemeMode
{
  THEME_MODE_OFF = 0,
  THEME_MODE_AUTOMATIC = 1,
  THEME_MODE_MANUAL = 2
};

enum ThemeSelection
{
  THEME_OFF = 0,        // No theme - standard blue buttons, no background
  THEME_YUGIOH = 1,     // Gold theme
  THEME_MAGIC = 2,      // Purple theme (MTG)
  THEME_POKEMON = 3,    // Red theme
  THEME_DEFAULT = 4,    // Blue theme with logo background
  THEME_FAB = 5         // Dark red theme (Flesh & Blood)
};