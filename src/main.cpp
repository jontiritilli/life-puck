// ============================================
// System & Framework Headers
// ============================================
#include <Arduino.h>
#include <lvgl.h>
#include <ArduinoNvs.h>
#include <esp_err.h>
#include <esp_heap_caps.h>

// ============================================
// Config Headers
// ============================================
#include <board_config.h>
#include "config.h"

// ============================================
// Core System
// ============================================
#include "core/gui_main.h"
#include "core/main.h"
#include "core/state_manager.h"

// ============================================
// Hardware Layer
// ============================================
#include "hardware/peripherals/i2c_Driver.h"
#include "hardware/peripherals/tca9554_power.h"
#include "hardware/peripherals/power_key.h"
#include "hardware/display/display_st77916.h"
#include "hardware/display/lvgl_driver.h"
#include "hardware/touch/touch_cst816.h"
#include "hardware/system/battery_state.h"
#include "hardware/system/power_management.h"
#include "hardware/audio/simple_audio.h"

// ============================================
// UI Layer
// ============================================
#include "ui/screens/life/life_counter.h"
#include "ui/screens/life/life_counter_two_player.h"
#include "ui/screens/tools/dice_coin.h"
#include "ui/screens/settings/touch_calibration.h"
#include "ui/screens/menu/menu.h"  // For getCurrentMenu() and MENU_TOUCH_CALIBRATION
#include "ui/helpers/event_grouper.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "data/tcg_presets.h"


extern uint8_t Touch_interrupts;

PlayerMode life_counter_mode = PLAYER_MODE_ONE_PLAYER;
bool is_two_player_mode = false;
int player1_life = 20;
int player2_life = 20;

/**
 * @brief Update Player 1 display based on current mode
 * 
 * Updates the UI display for player 1 depending on whether we're in
 * single-player or two-player mode. The actual life value updates
 * are handled automatically by the EventGrouper system.
 */
void update_player1_display() {
    // Update Player 1 display based on current mode
    if (life_counter_mode == PLAYER_MODE_TWO_PLAYER) {
        // In 2-Player mode: Call 2P update function
        // The update_life_label function is called automatically via EventGrouper
        // Additional UI update logic could be added here
    } else {
        // In 1-Player mode: Call single player update
        // The update_life_label function is called automatically via EventGrouper  
        // Additional UI update logic could be added here
    }
}

/**
 * @brief Update Player 2 display (only relevant in 2-Player mode)
 * 
 * Updates the UI display for player 2. Only functional in two-player mode,
 * as single-player mode doesn't have a second player.
 */
void update_player2_display() {
    // Update Player 2 display (only relevant in 2-Player mode)
    if (life_counter_mode == PLAYER_MODE_TWO_PLAYER) {
        // In 2-Player mode: Player 2 update
        // The update_life_label function is called automatically via EventGrouper
        // Additional UI update logic could be added here
    }
    // In 1-Player mode there is no Player 2
}

/**
 * @brief Arduino setup function - called once at startup
 * 
 * Initializes all hardware components, LVGL GUI system, and game state.
 * Sets up the complete system including I2C, display, touch, battery monitoring,
 * and loads the initial TCG preset configuration.
 */
void setup()
{
    Serial.begin(115200);
    Serial.println("--- Starting Life-Puck with Custom Demo Drivers ---");

    // Hardware initialization sequence
    I2C_Init();
    TCA9554PWR_Init();
    
    // Power button initialization (MUST be called early!)
    power_init();
    printf("[MAIN] Power button initialized\n");
    
    // Note: Emergency reset via BOOT button removed - conflicts with download mode
    
    LCD_Init();
    Backlight_Init();
    
    // Load and apply saved brightness from NVS (or force to default)
    #if FORCE_BRIGHTNESS
        Set_Backlight(FORCED_BRIGHTNESS_VALUE);
        printf("[MAIN] ⚠️  FORCE_BRIGHTNESS active - brightness: %d\n", FORCED_BRIGHTNESS_VALUE);
    #elif FORCE_FACTORY_RESET
        Set_Backlight(BRIGHTNESS_DEFAULT);
        printf("[MAIN] ⚠️  FORCE_FACTORY_RESET - brightness: %d (default)\n", BRIGHTNESS_DEFAULT);
    #else
        int saved_brightness = player_store.getInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT);
        Set_Backlight(saved_brightness);
        printf("[MAIN] Applied saved brightness: %d%%\n", saved_brightness);
    #endif

    // GUI system initialization
    Lvgl_Init();
    
    // ============================================================================
    // FORCE RESET FLAGS - Apply settings resets
    // ============================================================================
    #if FORCE_NVS_CLEAR
        Serial.println("⚠️⚠️⚠️  FORCE_NVS_CLEAR ACTIVE - ERASING ALL NVS DATA  ⚠️⚠️⚠️");
        NVS.eraseAll();
        Serial.println("⚠️  NVS CLEARED! Set FORCE_NVS_CLEAR back to false in config.h!");
        delay(2000); // Give user time to see the warning
    #endif
    
    #if FORCE_FACTORY_RESET
        Serial.println("⚠️  FORCE_FACTORY_RESET active - resetting all settings to config.h defaults");
        // Reset all settings to config.h values
        player_store.putInt(KEY_BRIGHTNESS, BRIGHTNESS_DEFAULT);
        player_store.putInt("audio_enabled", AUDIO_ENABLED_DEFAULT);
        player_store.putInt("audio_volume", AUDIO_VOLUME_DEFAULT);
        player_store.putInt(KEY_SHOW_TIMER, TIMER_ENABLED_DEFAULT);
        player_store.putInt(KEY_THEME_MODE, 0); // THEME_MODE_OFF
        player_store.putInt(KEY_AUTO_DIM_TIME, AUTO_DIM_DEFAULT);
        player_store.putInt(KEY_SLEEP_TIME, SLEEP_DEFAULT);
        player_store.putInt(KEY_BATTERY_SAVER, LOW_BATTERY_DIM_DEFAULT);
        Serial.println("✅ All settings reset to defaults!");
    #endif
    
    #if FORCE_THEME_MODE
        player_store.putInt(KEY_THEME_MODE, FORCED_THEME_MODE_VALUE);
        Serial.printf("⚠️  FORCE_THEME_MODE active - mode: %d\n", FORCED_THEME_MODE_VALUE);
    #endif
    
    #if FORCE_THEME_SELECTION
        player_store.putInt(KEY_THEME_SELECTION, FORCED_THEME_VALUE);
        Serial.printf("⚠️  FORCE_THEME_SELECTION active - theme: %d\n", FORCED_THEME_VALUE);
    #endif
    
    #if FORCE_AUDIO_SETTINGS
        player_store.putInt("audio_enabled", FORCED_AUDIO_ENABLED ? 1 : 0);
        player_store.putInt("audio_volume", FORCED_AUDIO_VOLUME);
        Serial.printf("⚠️  FORCE_AUDIO_SETTINGS active - enabled: %d, volume: %d\n", 
                      FORCED_AUDIO_ENABLED, FORCED_AUDIO_VOLUME);
    #endif
    
    #if FORCE_DEFAULT_CALIBRATION
        Serial.println("⚠️  FORCE_DEFAULT_CALIBRATION active - using config.h touch defaults");
        resetTouchCalibrationToDefaults();
    #else
        // Load saved touch calibration from NVS (after LVGL init)
        loadTouchCalibrationFromNVS();
    #endif

    // System monitoring initialization  
    battery_init();
    
    // Audio system initialization
    simple_audio_init();
    printf("[MAIN] Audio system initialized\n");

    // Initialize presets BEFORE ui_init()! (This also initializes NVS)
    #if FORCE_DEFAULT_PRESETS
        Serial.println("⚠️  FORCE_DEFAULT_PRESETS active - ignoring saved presets");
        init_default_presets(); // Skip NVS loading, use config.h only
    #else
        init_presets(); // Normal: Load from NVS or use defaults
    #endif
    load_preset();
    
    // Set initial life values from current preset
    printf("[MAIN] Getting preset from get_preset()\n");
    TCGPreset preset = get_preset();
    printf("[MAIN] Got preset: name='%s', starting_life=%d\n", preset.name, preset.starting_life);
    printf("[MAIN] Setting player1_life to %d\n", preset.starting_life);
    player1_life = preset.starting_life;
    printf("[MAIN] Setting player2_life to %d\n", preset.starting_life);
    player2_life = preset.starting_life;
    printf("[MAIN] Life values set successfully\n");
    
    // Power management initialization (after NVS is initialized)
    power_management_init();

    // Wait for display to fully initialize before showing UI
    delay(10); // Give display time to settle completely
    printf("[MAIN] Display settled, initializing UI 10ms Delay\n");
    
    // Initialize user interface
    ui_init();
    
    // Check if touch calibration confirmation is needed after boot
    #if !SKIP_INITIAL_CALIBRATION
        if (needsTouchCalibrationConfirmation()) {
            printf("[MAIN] Touch calibration confirmation required - showing dialog\n");
            renderTouchCalibrationConfirmation();
        }
    #else
        Serial.println("⚠️  SKIP_INITIAL_CALIBRATION active - skipping calibration confirmation");
        confirmTouchCalibration(); // Auto-confirm any pending calibration
    #endif
    
    // PSRAM initialization with error handling
    if (psramFound()) {
        printf("PSRAM: %d bytes\n", ESP.getPsramSize());
        heap_caps_malloc_extmem_enable(4096);  // Use PSRAM for large allocations
        printf("[OK] PSRAM enabled for allocations >= 4096 bytes\n");
    } else {
        printf("[WARNING] No PSRAM found - using internal RAM only\n");
    }
}

/**
 * @brief Arduino main loop - called continuously
 * 
 * Handles the main application loop including GUI updates, touch processing,
 * power management, and game mode-specific logic. Runs the appropriate
 * life counter loop based on current player mode.
 */
void loop()
{
    // Core system loops
    Lvgl_Loop();
    Touch_Loop();
    power_loop();
    power_check_inactivity(); // Check and apply power modes
    
    // Game mode specific processing
    // Disable life counter during touch calibration to prevent event conflicts
    if (getCurrentMenu() != MENU_TOUCH_CALIBRATION) {
        if (life_counter_mode == PLAYER_MODE_ONE_PLAYER) {
            life_counter_loop();
        } else if (life_counter_mode == PLAYER_MODE_TWO_PLAYER) {
            life_counter2p_loop();
        }
    }
    
    // Clear touch interrupt flag if set
    if (Touch_interrupts) {
        Touch_interrupts = 0;
    }
    
    // lv_timer_handler() already called in Lvgl_Loop() - avoid double call
    vTaskDelay(5 / portTICK_PERIOD_MS);
}

/**
 * @brief Apply current preset settings to the game
 * 
 * @param preset_index Index of the preset to apply (currently unused, uses active preset)
 * 
 * Loads the current TCG preset and applies its starting life values to both players.
 * Updates the display for the active player mode.
 */
void apply_preset_to_game(int preset_index) {
    TCGPreset preset = get_preset();
    
    // Set both players to preset starting life
    player1_life = preset.starting_life;
    player2_life = preset.starting_life;
    
    // Update displays based on current mode
    update_player1_display();
    if (is_two_player_mode) {
        update_player2_display();
    }
}