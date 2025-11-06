#include "tcg_presets.h"
#include "core/state_manager.h"
#include "ui/screens/menu/menu.h"
#include "config.h"  // For preset definitions
#include <cstring>

TCGPreset TCG_PRESETS[10];
int TCG_PRESET_COUNT = 10;
int current_preset_index = 0;

// ============================================
// DEFAULT PRESETS - Only on FIRST startup
// ============================================
/**
 * @brief Initialize default TCG game presets
 * 
 * Sets up the default configurations for popular trading card games
 * including Magic: The Gathering, Pokemon, Yu-Gi-Oh!, and others.
 * These serve as factory defaults and fallback values.
 */
void init_default_presets() {
    // All preset values are now defined in config.h for easy editing
    
    // ---- SLOT 0: MTG Standard ----
    strncpy(TCG_PRESETS[0].name, PRESET_0_NAME, sizeof(TCG_PRESETS[0].name) - 1);
    TCG_PRESETS[0].starting_life = PRESET_0_STARTING_LIFE;
    TCG_PRESETS[0].small_step = PRESET_0_SMALL_STEP;
    TCG_PRESETS[0].large_step = PRESET_0_LARGE_STEP;

    // ---- SLOT 1: MTG Commander ----
    strncpy(TCG_PRESETS[1].name, PRESET_1_NAME, sizeof(TCG_PRESETS[1].name) - 1);
    TCG_PRESETS[1].starting_life = PRESET_1_STARTING_LIFE;
    TCG_PRESETS[1].small_step = PRESET_1_SMALL_STEP;
    TCG_PRESETS[1].large_step = PRESET_1_LARGE_STEP;

    // ---- SLOT 2: Pokemon TCG ----
    strncpy(TCG_PRESETS[2].name, PRESET_2_NAME, sizeof(TCG_PRESETS[2].name) - 1);
    TCG_PRESETS[2].starting_life = PRESET_2_STARTING_LIFE;
    TCG_PRESETS[2].small_step = PRESET_2_SMALL_STEP;
    TCG_PRESETS[2].large_step = PRESET_2_LARGE_STEP;

    // ---- SLOT 3: Yu-Gi-Oh! ----
    strncpy(TCG_PRESETS[3].name, PRESET_3_NAME, sizeof(TCG_PRESETS[3].name) - 1);
    TCG_PRESETS[3].starting_life = PRESET_3_STARTING_LIFE;
    TCG_PRESETS[3].small_step = PRESET_3_SMALL_STEP;
    TCG_PRESETS[3].large_step = PRESET_3_LARGE_STEP;
    
    // ---- SLOT 4: Flesh & Blood ----
    strncpy(TCG_PRESETS[4].name, PRESET_4_NAME, sizeof(TCG_PRESETS[4].name) - 1);
    TCG_PRESETS[4].starting_life = PRESET_4_STARTING_LIFE;
    TCG_PRESETS[4].small_step = PRESET_4_SMALL_STEP;
    TCG_PRESETS[4].large_step = PRESET_4_LARGE_STEP;

    // ---- SLOT 5: Lorcana ----
    strncpy(TCG_PRESETS[5].name, PRESET_5_NAME, sizeof(TCG_PRESETS[5].name) - 1);
    TCG_PRESETS[5].starting_life = PRESET_5_STARTING_LIFE;
    TCG_PRESETS[5].small_step = PRESET_5_SMALL_STEP;
    TCG_PRESETS[5].large_step = PRESET_5_LARGE_STEP;

    // ---- SLOT 6: One Piece TCG ----
    strncpy(TCG_PRESETS[6].name, PRESET_6_NAME, sizeof(TCG_PRESETS[6].name) - 1);
    TCG_PRESETS[6].starting_life = PRESET_6_STARTING_LIFE;
    TCG_PRESETS[6].small_step = PRESET_6_SMALL_STEP;
    TCG_PRESETS[6].large_step = PRESET_6_LARGE_STEP;

    // ---- SLOT 7: Custom 8 ----
    strncpy(TCG_PRESETS[7].name, PRESET_7_NAME, sizeof(TCG_PRESETS[7].name) - 1);
    TCG_PRESETS[7].starting_life = PRESET_7_STARTING_LIFE;
    TCG_PRESETS[7].small_step = PRESET_7_SMALL_STEP;
    TCG_PRESETS[7].large_step = PRESET_7_LARGE_STEP;

    // ---- SLOT 8: Custom 9 ----
    strncpy(TCG_PRESETS[8].name, PRESET_8_NAME, sizeof(TCG_PRESETS[8].name) - 1);
    TCG_PRESETS[8].starting_life = PRESET_8_STARTING_LIFE;
    TCG_PRESETS[8].small_step = PRESET_8_SMALL_STEP;
    TCG_PRESETS[8].large_step = PRESET_8_LARGE_STEP;

    // ---- SLOT 9: Custom 10 ----
    strncpy(TCG_PRESETS[9].name, PRESET_9_NAME, sizeof(TCG_PRESETS[9].name) - 1);
    TCG_PRESETS[9].starting_life = PRESET_9_STARTING_LIFE;
    TCG_PRESETS[9].small_step = PRESET_9_SMALL_STEP;
    TCG_PRESETS[9].large_step = PRESET_9_LARGE_STEP;
}

// ============================================
// INIT: Load from storage OR use defaults
// ============================================
/**
 * @brief Initialize preset system
 * 
 * Loads presets from persistent storage if available, otherwise
 * initializes with factory defaults. This is called during system startup.
 */
void init_presets() {
    printf("[Presets] Starting init_presets()\n");
    
    // First load defaults (fallback)
    printf("[Presets] Calling init_default_presets()\n");
    init_default_presets();
    printf("[Presets] init_default_presets() completed\n");
    
    // Then try to overwrite from storage
    printf("[Presets] Starting storage loop\n");
    for (int i = 0; i < 10; i++) {
        printf("[Presets] Processing preset %d\n", i);
        char key_name[32], key_life[32], key_small[32], key_large[32];
        
        snprintf(key_name, sizeof(key_name), "preset_%d_name", i);
        snprintf(key_life, sizeof(key_life), "preset_%d_life", i);
        snprintf(key_small, sizeof(key_small), "preset_%d_small", i);
        snprintf(key_large, sizeof(key_large), "preset_%d_large", i);
        
        printf("[Presets] Getting stored name for preset %d\n", i);
        String stored_name = player_store.getString(key_name, "");
        printf("[Presets] Stored name: '%s'\n", stored_name.c_str());
        
        if (stored_name.length() > 0) {
            printf("[Presets] Stored preset found for %d, overwriting default\n", i);
            // Stored preset found - overwrite default
            strncpy(TCG_PRESETS[i].name, stored_name.c_str(), sizeof(TCG_PRESETS[i].name) - 1);
            TCG_PRESETS[i].name[sizeof(TCG_PRESETS[i].name) - 1] = '\0';
            
            printf("[Presets] Getting stored values for preset %d\n", i);
            TCG_PRESETS[i].starting_life = player_store.getInt(key_life, TCG_PRESETS[i].starting_life);
            TCG_PRESETS[i].small_step = player_store.getInt(key_small, TCG_PRESETS[i].small_step);
            TCG_PRESETS[i].large_step = player_store.getInt(key_large, TCG_PRESETS[i].large_step);
            printf("[Presets] Preset %d loaded: %s, life=%d, small=%d, large=%d\n", 
                   i, TCG_PRESETS[i].name, TCG_PRESETS[i].starting_life, 
                   TCG_PRESETS[i].small_step, TCG_PRESETS[i].large_step);
        } else {
            printf("[Presets] No stored preset for %d, keeping default\n", i);
        }
        // If NOT stored, keep the default from init_default_presets()
    }
    printf("[Presets] init_presets() completed successfully\n");
}

// ============================================
// PRESET ACCESS FUNCTIONS - DO NOT MODIFY
// ============================================
/**
 * @brief Load active preset index from storage
 * 
 * Retrieves the currently selected preset index from persistent storage.
 * Defaults to preset 0 if no valid index is found.
 */
void load_preset() {
    printf("[Presets] Starting load_preset()\n");
    printf("[Presets] Getting preset_idx from player_store\n");
    current_preset_index = player_store.getInt("preset_idx", 0);
    printf("[Presets] Got preset_idx: %d\n", current_preset_index);
    
    if (current_preset_index < 0 || current_preset_index >= TCG_PRESET_COUNT) {
        printf("[Presets] Invalid preset_idx %d, resetting to 0\n", current_preset_index);
        current_preset_index = 0;
    }
    printf("[Presets] Final preset_idx: %d\n", current_preset_index);
    printf("[Presets] load_preset() completed successfully\n");
}

/**
 * @brief Save active preset index to storage
 * 
 * @param index Index of the preset to make active (0-9)
 * 
 * Stores the preset index in persistent storage if valid.
 */
void save_preset(int index) {
    if (index >= 0 && index < TCG_PRESET_COUNT) {
        current_preset_index = index;
        player_store.putInt("preset_idx", index);
        
        // Note: resetActiveCounter() is called by the caller after updating NVS values
        // Don't call it here to avoid double-reset
        
        printf("[Preset] Applied preset %s with %d starting life\n", TCG_PRESETS[index].name, TCG_PRESETS[index].starting_life);
    }
}

/**
 * @brief Get the currently active preset
 * 
 * @return TCGPreset The current preset configuration
 * 
 * Returns the preset at the current index, or preset 0 as fallback.
 */
TCGPreset get_preset() {
    printf("[Presets] Starting get_preset()\n");
    printf("[Presets] current_preset_index: %d, TCG_PRESET_COUNT: %d\n", current_preset_index, TCG_PRESET_COUNT);
    
    if (current_preset_index < 0 || current_preset_index >= TCG_PRESET_COUNT) {
        printf("[Presets] Invalid index, returning TCG_PRESETS[0]\n");
        printf("[Presets] TCG_PRESETS[0] address: %p\n", &TCG_PRESETS[0]);
        return TCG_PRESETS[0];
    }
    
    printf("[Presets] Returning TCG_PRESETS[%d]\n", current_preset_index);
    printf("[Presets] TCG_PRESETS[%d] address: %p\n", current_preset_index, &TCG_PRESETS[current_preset_index]);
    printf("[Presets] TCG_PRESETS[%d] name: '%s'\n", current_preset_index, TCG_PRESETS[current_preset_index].name);
    printf("[Presets] TCG_PRESETS[%d] starting_life: %d\n", current_preset_index, TCG_PRESETS[current_preset_index].starting_life);
    printf("[Presets] TCG_PRESETS[%d] small_step: %d\n", current_preset_index, TCG_PRESETS[current_preset_index].small_step);
    printf("[Presets] TCG_PRESETS[%d] large_step: %d\n", current_preset_index, TCG_PRESETS[current_preset_index].large_step);
    
    TCGPreset result = TCG_PRESETS[current_preset_index];
    printf("[Presets] Copied preset - name: '%s', starting_life: %d\n", result.name, result.starting_life);
    return result;
}
