#pragma once
#include <lvgl.h>

// ============================================================================
// UI STANDARDIZATION CONSTANTS
// ============================================================================
// These constants ensure visual consistency across all menu screens
// ============================================================================

// ============================================================================
// BUTTON SIZES
// ============================================================================
#define UI_BTN_WIDTH_STANDARD 120 // Standard button width
#define UI_BTN_HEIGHT_STANDARD 50 // Standard button height
#define UI_BTN_WIDTH_WIDE 260     // Wide button (spans 2 columns)
#define UI_BTN_HEIGHT_TALL 60     // Tall button for emphasis

// ============================================================================
// FONT SIZES
// ============================================================================
#define UI_FONT_TITLE &lv_font_montserrat_20  // Screen titles
#define UI_FONT_BUTTON &lv_font_montserrat_20 // Button labels
#define UI_FONT_TOGGLE &lv_font_montserrat_18 // Toggle/value labels
#define UI_FONT_INFO &lv_font_montserrat_16   // Info text (battery, etc.)
#define UI_FONT_SMALL &lv_font_montserrat_14  // Small text

// ============================================================================
// COLORS
// ============================================================================
#define UI_COLOR_BACK_BUTTON lv_color_white()          // Back button background
#define UI_COLOR_BACK_TEXT lv_color_black()            // Back button text
#define UI_COLOR_DISABLED lv_color_hex(0x444444)       // Disabled/OFF state
#define UI_COLOR_TITLE lv_color_white()                // Title text
#define UI_COLOR_TEXT_PRIMARY lv_color_white()         // Primary text
#define UI_COLOR_TEXT_SECONDARY lv_color_hex(0xAAAAAA) // Secondary text

// ============================================================================
// GRID LAYOUT CONSTANTS
// ============================================================================
#define UI_GRID_COLS 2        // Standard 2-column grid
#define UI_GRID_ROW_HEIGHT 60 // Standard row height
#define UI_GRID_GAP 10        // Gap between items

// ============================================================================
// POSITIONS
// ============================================================================
#define UI_BACK_BTN_ROW 1      // Back button always in row 1
#define UI_BACK_BTN_COL 0      // Back button always in col 0
#define UI_TITLE_ROW 0         // Title always in row 0
#define UI_CONTENT_START_ROW 2 // Content starts at row 2

// ============================================================================
// TEXT LABELS
// ============================================================================
#define UI_TEXT_BACK LV_SYMBOL_LEFT " Back" // Standard back button text

// ============================================================================
// MENU CONTAINER SETTINGS
// ============================================================================
#define UI_MENU_WIDTH (SCREEN_WIDTH - 20)       // Menu width
#define UI_MENU_HEIGHT (SCREEN_HEIGHT - 30)     // Menu height
#define UI_MENU_RADIUS 15                       // Menu corner radius
#define UI_MENU_BG_COLOR lv_color_hex(0x000000) // Menu background color
