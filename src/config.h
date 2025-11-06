// ============================================================================
// LIFEPUCK ESP32-S3 CONFIGURATION FILE
// ============================================================================
//
// SINGLE SOURCE OF TRUTH for all user-configurable settings
// ALL values defined here are used exclusively by the code
// Change values here and recompile to apply
//
// Hardware-specific values (pins, SPI, I2C) remain in board_config.h
//
// ============================================================================

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// ⚠️  FORCE RESET FLAGS - FOR TESTING AND DEVELOPMENT
// ============================================================================
// WARNING: These flags OVERRIDE all saved user settings!
// ⚠️  IMPORTANT: Set back to false for production releases!
// ============================================================================

#define FORCE_DEFAULT_CALIBRATION false // Force touch calibration to defaults
#define FORCE_DEFAULT_PRESETS false     // Force presets to factory defaults
#define FORCE_FACTORY_RESET false       // Reset ALL settings to defaults
#define FORCE_NVS_CLEAR false           // Erase ALL saved data (nuclear option)
#define SKIP_INITIAL_CALIBRATION false  // Skip calibration screen on first boot

// ============================================================================
// 🎨 THEME CONFIGURATION
// ============================================================================
// Theme colors, opacity, and visual appearance
// Used by: src/data/themes.cpp
// ============================================================================

// Theme Background Opacity (0-255: 0=transparent, 255=opaque)
#define THEME_OPACITY_DEFAULT 180
#define THEME_OPACITY_YUGIOH 180
#define THEME_OPACITY_MAGIC 180
#define THEME_OPACITY_POKEMON 180
#define THEME_OPACITY_FAB 180

// Theme Button Colors (RGB hex: 0xRRGGBB)
#define THEME_COLOR_DEFAULT 0x0070FF // Lightning Blue
#define THEME_COLOR_YUGIOH 0xFFD700  // Gold
#define THEME_COLOR_MAGIC 0x9B30FF   // Purple
#define THEME_COLOR_POKEMON 0xDC0A2D // Red
#define THEME_COLOR_FAB 0x8B0006     // Dark Red (Flesh & Blood)

// Theme Icon Colors
#define THEME_ICON_COLOR_DEFAULT 0xFFFFFF // White
#define THEME_ICON_COLOR_YUGIOH 0xFFD700  // Gold
#define THEME_ICON_COLOR_MAGIC 0x9B30FF   // Purple
#define THEME_ICON_COLOR_POKEMON 0xDC0A2D // Red
#define THEME_ICON_COLOR_FAB 0x8B0006     // Dark Red

// Theme Text Colors (ensures proper contrast)
#define THEME_TEXT_COLOR_DEFAULT 0xFFFFFF // White
#define THEME_TEXT_COLOR_YUGIOH 0x000000  // Black
#define THEME_TEXT_COLOR_MAGIC 0xFFFFFF   // White
#define THEME_TEXT_COLOR_POKEMON 0xFFFFFF // White
#define THEME_TEXT_COLOR_FAB 0xFFFFFF     // White

// Back Button Text Color
#define BACK_BUTTON_TEXT_COLOR 0x000000 // Black

// ============================================================================
// 🎰 DICE AND COIN ANIMATION SETTINGS
// ============================================================================
// Slot-machine style animations: Numbers/text cycle rapidly then slow to result
// Used by: src/ui/screens/menu/menu.cpp
// ============================================================================

#define ENABLE_DICE_ANIMATION true
#define ENABLE_COIN_ANIMATION true

// Animation timing
#define DICE_ANIMATION_DURATION_MS 800  // Total animation time (ms)
#define COIN_ANIMATION_DURATION_MS 1000 // Total animation time (ms)

// Animation behavior
#define DICE_CYCLES 15 // How many random numbers to show
#define COIN_CYCLES 12 // How many flips (Heads/Tails)

// Speed-up factor (each cycle gets longer by this amount)
#define ANIMATION_SLOWDOWN_MS 10 // Add 10ms to each subsequent cycle

// Visual effects
#define ANIMATION_SCALE_BOUNCE true // Scale up/down during animation
#define ANIMATION_SOUND_TICKS true  // Beep on each change (if audio on)

// ============================================================================
// 🪄 RESULT POPUP DISPLAY DURATIONS
// ============================================================================
// How long result stays visible after animation / normal popup
// Used by: src/ui/screens/menu/menu.cpp
// ============================================================================

#define DICE_RESULT_HOLD_MS 1000
#define COIN_RESULT_HOLD_MS 1000
#define GENERIC_RESULT_HOLD_MS 1000

// ============================================================================
// 🎮 PRESET DEFINITIONS
// ============================================================================
// TCG game presets: name, starting life, steps, theme assignment
// Used by: src/data/tcg_presets.cpp
// Theme values: 0=Off, 1=Yugioh, 2=Magic, 3=Pokemon, 4=Default, 5=FAB
// ============================================================================

// PRESET 0: Flesh & Blood CC (Classic Constructed)
#define PRESET_0_NAME "FAB CC"
#define PRESET_0_STARTING_LIFE 40
#define PRESET_0_SMALL_STEP 1
#define PRESET_0_LARGE_STEP 5
#define PRESET_0_THEME 5 // FAB

// PRESET 1: Flesh & Blood Blitz
#define PRESET_1_NAME "FAB Blitz"
#define PRESET_1_STARTING_LIFE 20
#define PRESET_1_SMALL_STEP 1
#define PRESET_1_LARGE_STEP 5
#define PRESET_1_THEME 5 // FAB

// PRESET 2: MTG Standard
#define PRESET_2_NAME "MTG Standard"
#define PRESET_2_STARTING_LIFE 20
#define PRESET_2_SMALL_STEP 1
#define PRESET_2_LARGE_STEP 5
#define PRESET_2_THEME 2 // Magic

// PRESET 3: MTG Commander
#define PRESET_3_NAME "MTG Commander"
#define PRESET_3_STARTING_LIFE 40
#define PRESET_3_SMALL_STEP 1
#define PRESET_3_LARGE_STEP 10
#define PRESET_3_THEME 2 // Magic

// PRESET 4: Pokemon TCG
#define PRESET_4_NAME "Pokemon TCG"
#define PRESET_4_STARTING_LIFE 60
#define PRESET_4_SMALL_STEP 10
#define PRESET_4_LARGE_STEP 30
#define PRESET_4_THEME 3 // Pokemon

// PRESET 5: Yu-Gi-Oh!
#define PRESET_5_NAME "Yu-Gi-Oh!"
#define PRESET_5_STARTING_LIFE 8000
#define PRESET_5_SMALL_STEP 50
#define PRESET_5_LARGE_STEP 500
#define PRESET_5_THEME 1 // Yugioh

// PRESET 6: Lorcana
#define PRESET_6_NAME "Lorcana"
#define PRESET_6_STARTING_LIFE 20
#define PRESET_6_SMALL_STEP 1
#define PRESET_6_LARGE_STEP 5
#define PRESET_6_THEME 4 // Default

// PRESET 7: One Piece TCG
#define PRESET_7_NAME "One Piece TCG"
#define PRESET_7_STARTING_LIFE 5
#define PRESET_7_SMALL_STEP 1
#define PRESET_7_LARGE_STEP 2
#define PRESET_7_THEME 4 // Default

// PRESET 8-9: Custom Presets
#define PRESET_8_NAME "Custom 9"
#define PRESET_8_STARTING_LIFE 20
#define PRESET_8_SMALL_STEP 1
#define PRESET_8_LARGE_STEP 5
#define PRESET_8_THEME 4

#define PRESET_9_NAME "Custom 10"
#define PRESET_9_STARTING_LIFE 20
#define PRESET_9_SMALL_STEP 1
#define PRESET_9_LARGE_STEP 5
#define PRESET_9_THEME 4

// ============================================================================
// 👆 TOUCH CALIBRATION DEFAULTS
// ============================================================================
// 7-parameter affine transformation matrix for touch coordinate mapping
// Used by: src/hardware/display/lvgl_driver.cpp
// Tested values: scale_x=0.85, scale_y=0.90 for ESP32-S3-Touch-LCD-1.85C
//
// 🔧 CALIBRATION ADJUSTMENT GUIDE:
// ============================================================================
//
// OFFSET_X: Horizontal position shift
//   ➤ INCREASE value → Touch points move RIGHT
//   ➤ DECREASE value → Touch points move LEFT
//   ➤ Range: typically -50.0 to +50.0
//
// OFFSET_Y: Vertical position shift
//   ➤ INCREASE value → Touch points move DOWN
//   ➤ DECREASE value → Touch points move UP
//   ➤ Range: typically -50.0 to +50.0
//
// SCALE_X: Horizontal scaling (width adjustment)
//   ➤ INCREASE value → Touch area becomes WIDER (stretches horizontally)
//   ➤ DECREASE value → Touch area becomes NARROWER (compresses horizontally)
//   ➤ Range: typically 0.5 to 1.5 (1.0 = no scaling)
//
// SCALE_Y: Vertical scaling (height adjustment)
//   ➤ INCREASE value → Touch area becomes TALLER (stretches vertically)
//   ➤ DECREASE value → Touch area becomes SHORTER (compresses vertically)
//   ➤ Range: typically 0.5 to 1.5 (1.0 = no scaling)
//
// SHEAR_XY: Horizontal skew (tilt left/right)
//   ➤ POSITIVE value → Touch area tilts RIGHT (top moves right, bottom moves left)
//   ➤ NEGATIVE value → Touch area tilts LEFT (top moves left, bottom moves right)
//   ➤ Range: typically -0.5 to +0.5 (0.0 = no skew)
//
// SHEAR_YX: Vertical skew (tilt up/down)
//   ➤ POSITIVE value → Touch area tilts DOWN (left moves down, right moves up)
//   ➤ NEGATIVE value → Touch area tilts UP (left moves up, right moves down)
//   ➤ Range: typically -0.5 to +0.5 (0.0 = no skew)
//
// DIVISOR: Overall scaling factor
//   ➤ INCREASE value → Entire touch area becomes SMALLER
//   ➤ DECREASE value → Entire touch area becomes LARGER
//   ➤ Range: typically 0.5 to 2.0 (1.0 = no change)
//
// 🎯 COMMON CALIBRATION SCENARIOS:
// ============================================================================
//
// Problem: "Touch points are too far to the right"
// Solution: DECREASE OFFSET_X (try -10.0, -20.0, etc.)
//
// Problem: "Touch points are too high up"
// Solution: INCREASE OFFSET_Y (try +10.0, +20.0, etc.)
//
// Problem: "Touch area is too wide horizontally"
// Solution: DECREASE SCALE_X (try 0.8, 0.7, etc.)
//
// Problem: "Touch area is too tall vertically"
// Solution: DECREASE SCALE_Y (try 0.8, 0.7, etc.)
//
// Problem: "Touch area is rotated clockwise"
// Solution: ADJUST SHEAR_XY and SHEAR_YX (try small values like ±0.1)
//
// Problem: "Touch area is too big overall"
// Solution: INCREASE DIVISOR (try 1.2, 1.5, etc.)
//
// Problem: "Touch area is too small overall"
// Solution: DECREASE DIVISOR (try 0.8, 0.6, etc.)
// ============================================================================

#define TOUCH_CAL_DEFAULT_OFFSET_X 23.5f
#define TOUCH_CAL_DEFAULT_SCALE_X 0.90f
#define TOUCH_CAL_DEFAULT_SHEAR_XY 0.0f
#define TOUCH_CAL_DEFAULT_OFFSET_Y -7.0f
#define TOUCH_CAL_DEFAULT_SHEAR_YX 0.0f
#define TOUCH_CAL_DEFAULT_SCALE_Y 0.95f
#define TOUCH_CAL_DEFAULT_DIVISOR 1.0f

// ============================================================================
// 🖥️  DISPLAY & POWER DEFAULTS
// ============================================================================
// Default settings for screen, auto-dim, sleep, battery management
// Used by various files - will be centralized to use these values
// ============================================================================

// Screen Brightness (0-255)
#define BRIGHTNESS_DEFAULT 100 // Default: 100 (39%)

// Auto-Dim Settings (in seconds)
// Available UI options: 0, 30, 60, 120, 300
#define AUTO_DIM_DEFAULT 60 // Default: 60s (1 minute)

// Sleep Settings (in seconds)
// Available UI options: 0, 120, 300, 600
#define SLEEP_DEFAULT 300 // Default: 300s (5 minutes)

// Low Battery Dimming
#define LOW_BATTERY_DIM_DEFAULT 1 // Default: 1 (ON)

// ============================================================================
// 🔊 AUDIO DEFAULTS
// ============================================================================
// Audio configuration
// Used by: src/hardware/audio/simple_audio.h and simple_audio.cpp
// ============================================================================

#define AUDIO_ENABLED_DEFAULT 1 // 1=ON, 0=OFF
#define AUDIO_VOLUME_DEFAULT 10 // Volume: 0-100
#define TIMER_SOUND_DEFAULT 1   // 1=ON, 0=OFF

// ============================================================================
// ⏱️  TIMER DEFAULTS
// ============================================================================
// Timer configuration
// Used by: src/ui/screens/settings/timer_settings.cpp
// ============================================================================

#define TIMER_ENABLED_DEFAULT 0  // 0=OFF, 1=ON
#define TIMER_DEFAULT_MINUTES 10 // Default round time in minutes

// ============================================================================
// 🔢 SIMPLE COUNTERS CONFIGURATION
// ============================================================================
// Simple counter system settings for both 1P and 2P modes
// Used by: src/ui/screens/life/simple_counters.cpp
// ============================================================================

// Counter System Settings
#define SIMPLE_COUNTER_COUNT 4 // Number of available counter slots (fixed at 4)
#define COUNTER_MAX_VALUE 999  // Maximum counter value (prevents overflow)

// Counter Visual Appearance
#define COUNTER_SIZE 50            // Base size of counter circles in pixels
#define COUNTER_OPACITY 180        // Counter background opacity (180 = 70% opaque)
#define COUNTER_BORDER_OPACITY 153 // Counter border opacity (153 = 60% opacity)
#define COUNTER_GRADIENT_DARKEN 30 // How much to darken gradient color (0-100)

// Counter Positioning (4 corners)
// IMPORTANT: These values are OFFSETS from screen edges, not center coordinates!
// - Higher values = further from edges (more towards center)
// - Lower values = closer to edges
// - All values are in pixels from the respective screen edge
// - Screen size is 360x360px, so max safe values are around 300px
#define COUNTER_1_X 50 // Top-Left: 50px from left edge, 50px from top edge
#define COUNTER_1_Y 50 // Top-Left: 50px from left edge, 50px from top edge
#define COUNTER_2_X 50 // Top-Right: 50px from right edge, 50px from top edge
#define COUNTER_2_Y 50 // Top-Right: 50px from right edge, 50px from top edge
#define COUNTER_3_X 50 // Bottom-Left: 50px from left edge, 50px from bottom edge
#define COUNTER_3_Y 50 // Bottom-Left: 50px from left edge, 50px from bottom edge
#define COUNTER_4_X 50 // Bottom-Right: 50px from right edge, 50px from bottom edge
#define COUNTER_4_Y 50 // Bottom-Right: 50px from right edge, 50px from bottom edge

// Counter Interaction Timing (milliseconds)
#define COUNTER_TAP_THRESHOLD 500         // Short tap duration for increment
#define COUNTER_LONG_PRESS_THRESHOLD 1500 // Long press duration for reset

// Counter Visual Feedback Colors (hex RGB)
#define COUNTER_NORMAL_COLOR 0x555555    // Default counter background color
#define COUNTER_DECREMENT_COLOR 0xFFFF00 // Yellow color during decrement press
#define COUNTER_RESET_COLOR 0xFF0000     // Red color during reset press

// Counter NVS Storage Keys
#define NVS_KEY_COUNTER_ENABLED "cnt_en_%d" // NVS key for counter enabled state
#define NVS_KEY_COUNTER_VALUE "cnt_val_%d"  // NVS key for counter value

// ============================================================================
// 🎨 UI TIMING & BEHAVIOR
// ============================================================================
// Animation speeds, gesture detection, interaction timing
// Used throughout the UI code
// ============================================================================

// Animation Durations (milliseconds)
#define ARC_ANIMATION_DURATION 600 // Life counter arc animation
#define GROUPER_WINDOW 1500        // Life change grouping window for auto-save

// Touch & Gesture Control Mode
// ============================================================================
// Choose between two gesture control schemes:
// 0 = CLASSIC MODE (Original behavior):
//     - Swipe up/down = Large step adjustments (±5)
//     - Long press center = Open menu
// 1 = LONG PRESS MODE (New behavior):
//     - Long press top/bottom = Large step adjustments (±5, repeats while held)
//     - Swipe down = Open menu
// ============================================================================
#define GESTURE_CONTROL_MODE 1 // 0 = Classic (swipe), 1 = Long Press (hold)

// Touch & Gesture Timing
#define TOUCH_BLOCK_AFTER_WAKE_MS 300 // Block touches after wake from sleep
#define SWIPE_THRESHOLD_PX 80         // Pixels to trigger swipe gesture
#define LONG_PRESS_TIME_MS 500        // Time to trigger long press (milliseconds)
#define LONG_PRESS_REPEAT_TIME_MS 500 // Repeat interval while holding long press (milliseconds)

// Touch Calibration Parameters
#define CALIBRATION_TIMEOUT_MS 10000 // Confirmation timeout (10 seconds)
#define CALIBRATION_LOCK_FRAMES 10   // Frames to lock calibration step

// ============================================================================
// 🔧 ADVANCED FORCE FLAGS (Development only)
// ============================================================================

#define FORCE_THEME_MODE false
#define FORCED_THEME_MODE_VALUE 0 // 0=OFF, 1=Automatic, 2=Manual

#define FORCE_THEME_SELECTION false
#define FORCED_THEME_VALUE 0 // 0=Off, 1=Yugioh, 2=Magic, 3=Pokemon, 4=Default

#define FORCE_AUDIO_SETTINGS false
#define FORCED_AUDIO_ENABLED 1
#define FORCED_AUDIO_VOLUME 75

#define FORCE_BRIGHTNESS false
#define FORCED_BRIGHTNESS_VALUE 200

// ============================================================================

#endif // CONFIG_H
