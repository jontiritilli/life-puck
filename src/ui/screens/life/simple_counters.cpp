#include "simple_counters.h"
#include "core/state_manager.h"
#include "data/themes.h"
#include "config.h"

extern StateStore player_store;

// Forward declaration
static void on_counter_event(lv_event_t* e);

// Initialize static array
SimpleCounter SimpleCounters::counters[SIMPLE_COUNTER_COUNT] = {};

void SimpleCounters::init() {
    // Initialize all 4 slots
    for (int i = 0; i < SIMPLE_COUNTER_COUNT; i++) {
        counters[i].slot = i;
        counters[i].value = 0;
        counters[i].enabled = false;
        counters[i].container = nullptr;
        counters[i].circle = nullptr;
        counters[i].label = nullptr;
        counters[i].press_start_time = 0;
    }
    
    // Load saved values from NVS
    load_all_from_nvs();
    
    // Create UI for enabled counters
    for (int i = 0; i < SIMPLE_COUNTER_COUNT; i++) {
        if (counters[i].enabled) {
            create_counter_ui(i);
        }
    }
}

void SimpleCounters::shutdown() {
    // Clean up all counter UIs
    for (int i = 0; i < SIMPLE_COUNTER_COUNT; i++) {
        if (counters[i].container != nullptr) {
            lv_obj_del(counters[i].container);
            counters[i].container = nullptr;
            counters[i].circle = nullptr;
            counters[i].label = nullptr;
        }
    }
}

void SimpleCounters::enable_counter(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;

    counters[slot].enabled = true;
    save_to_nvs(slot);

    // Create UI immediately if not already created
    if (counters[slot].container == nullptr) {
        create_counter_ui(slot);
    }
}

void SimpleCounters::disable_counter(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;
    
    counters[slot].enabled = false;
    
    if (counters[slot].container != nullptr) {
        destroy_counter_ui(slot);
    }
    
    save_to_nvs(slot);
}

void SimpleCounters::increment(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;
    
    counters[slot].value++;
    
    // Cap at maximum value to prevent overflow
    if (counters[slot].value > COUNTER_MAX_VALUE) {
        counters[slot].value = COUNTER_MAX_VALUE;
    }
    
    if (counters[slot].label != nullptr) {
        update_display(slot);
    }
    
    save_to_nvs(slot);
}

void SimpleCounters::decrement(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;
    
    counters[slot].value--;
    
    if (counters[slot].value < 0) {
        counters[slot].value = 0;
    }
    
    if (counters[slot].label != nullptr) {
        update_display(slot);
    }
    
    save_to_nvs(slot);
}

void SimpleCounters::reset(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;
    
    counters[slot].value = 0;
    
    if (counters[slot].label != nullptr) {
        update_display(slot);
    }
    
    save_to_nvs(slot);
}

int SimpleCounters::get_value(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return 0;
    
    return counters[slot].value;
}

bool SimpleCounters::is_enabled(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return false;
    
    return counters[slot].enabled;
}

void SimpleCounters::update_display(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;
    
    lv_label_set_text_fmt(counters[slot].label, "%d", counters[slot].value);
}

void SimpleCounters::destroy_counter_ui(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;
    
    if (counters[slot].container != nullptr) {
        lv_obj_del(counters[slot].container);
        counters[slot].container = nullptr;
        counters[slot].circle = nullptr;
        counters[slot].label = nullptr;
    }
}

void SimpleCounters::create_counter_ui(int slot) {
    // Validate slot and check if already exists
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;
    if (counters[slot].container != nullptr) return;
    
    // Create container as child of screen - size automatically adjusts to COUNTER_SIZE
    // Check if we're in 2P mode and use the appropriate container
    lv_obj_t* parent = lv_scr_act();
    
    // Try to find the 2P container if it exists
    extern lv_obj_t* life_counter_container_2p;
    if (life_counter_container_2p != nullptr) {
        parent = life_counter_container_2p;
    }
    
    counters[slot].container = lv_obj_create(parent);
    lv_obj_set_size(counters[slot].container, COUNTER_SIZE + 20, COUNTER_SIZE + 20); // Auto-adjusts to COUNTER_SIZE
    lv_obj_add_flag(counters[slot].container, LV_OBJ_FLAG_IGNORE_LAYOUT);
    
    // Completely disable all scrolling and scrollbars
    lv_obj_clear_flag(counters[slot].container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(counters[slot].container, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(counters[slot].container, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_clear_flag(counters[slot].container, LV_OBJ_FLAG_SCROLL_ONE);
    lv_obj_clear_flag(counters[slot].container, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_clear_flag(counters[slot].container, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_scrollbar_mode(counters[slot].container, LV_SCROLLBAR_MODE_OFF);
    
    // Force no scrollbars - already disabled above
    
    // Set position based on slot - much closer to center
    switch (slot) {
        case 0: // Top-Left
            lv_obj_align(counters[slot].container, LV_ALIGN_TOP_LEFT, COUNTER_1_X, COUNTER_1_Y);
            break;
        case 1: // Top-Right
            lv_obj_align(counters[slot].container, LV_ALIGN_TOP_RIGHT, -COUNTER_2_X, COUNTER_2_Y);
            break;
        case 2: // Bottom-Left
            lv_obj_align(counters[slot].container, LV_ALIGN_BOTTOM_LEFT, COUNTER_3_X, -COUNTER_3_Y);
            break;
        case 3: // Bottom-Right
            lv_obj_align(counters[slot].container, LV_ALIGN_BOTTOM_RIGHT, -COUNTER_4_X, -COUNTER_4_Y);
            break;
        default:
            return;
    }
    
    // Configure container style - completely transparent (scrolling already disabled above)
    
    // Make container completely transparent
    lv_obj_set_style_bg_opa(counters[slot].container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(counters[slot].container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(counters[slot].container, 0, 0);
    lv_obj_set_style_pad_gap(counters[slot].container, 0, 0);
    
    // Ensure container can receive events
    lv_obj_clear_flag(counters[slot].container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(counters[slot].container, LV_OBJ_FLAG_CLICKABLE);
    
    // Create circle inside container - bigger but still smaller than container
    counters[slot].circle = lv_obj_create(counters[slot].container);
    lv_obj_set_size(counters[slot].circle, COUNTER_SIZE + 10, COUNTER_SIZE + 10);
    lv_obj_center(counters[slot].circle);
    lv_obj_set_style_radius(counters[slot].circle, (COUNTER_SIZE + 10) / 2, 0);
    
    // Disable scrolling on circle too
    lv_obj_clear_flag(counters[slot].circle, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(counters[slot].circle, LV_SCROLLBAR_MODE_OFF);
    
    // Use theme colors with configurable transparency and gradient
    lv_color_t theme_color = getThemeButtonColor();
    
    // Fallback to bright yellow if theme color is too dark
    if (lv_color_brightness(theme_color) < 50) {
        theme_color = lv_color_hex(0xFFFF00); // Bright yellow fallback
    }
    
    lv_obj_set_style_bg_color(counters[slot].circle, theme_color, 0);
    lv_obj_set_style_bg_opa(counters[slot].circle, COUNTER_OPACITY, 0); // Configurable opacity
    
    // Add subtle gradient effect (top to bottom) with configurable darkening
    lv_obj_set_style_bg_grad_color(counters[slot].circle, lv_color_darken(getThemeButtonColor(), COUNTER_GRADIENT_DARKEN), 0);
    lv_obj_set_style_bg_grad_dir(counters[slot].circle, LV_GRAD_DIR_VER, 0);
    
    // Softer border with configurable opacity
    lv_obj_set_style_border_color(counters[slot].circle, getThemeButtonColor(), 0);
    lv_obj_set_style_border_opa(counters[slot].circle, COUNTER_BORDER_OPACITY, 0); // Configurable border opacity
    lv_obj_set_style_border_width(counters[slot].circle, 2, 0);
    
    // Create label inside circle - bigger font
    counters[slot].label = lv_label_create(counters[slot].circle);
    lv_label_set_text_fmt(counters[slot].label, "%d", counters[slot].value);
    lv_obj_set_style_text_font(counters[slot].label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(counters[slot].label, lv_color_white(), 0);  // Always white for maximum visibility
    lv_obj_center(counters[slot].label);
    
    // Move counter to foreground to ensure visibility
    lv_obj_move_foreground(counters[slot].container);
    lv_obj_move_foreground(counters[slot].circle);
    
    // Add event callbacks on the circle (visible element) - include PRESSING for color changes
    lv_obj_add_event_cb(counters[slot].circle, on_counter_event, LV_EVENT_PRESSED, (void*)(intptr_t)slot);
    lv_obj_add_event_cb(counters[slot].circle, on_counter_event, LV_EVENT_PRESSING, (void*)(intptr_t)slot);
    lv_obj_add_event_cb(counters[slot].circle, on_counter_event, LV_EVENT_RELEASED, (void*)(intptr_t)slot);
}

void SimpleCounters::save_to_nvs(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;
    
    char key_enabled[32];
    char key_value[32];
    
    sprintf(key_enabled, NVS_KEY_COUNTER_ENABLED, slot);
    sprintf(key_value, NVS_KEY_COUNTER_VALUE, slot);
    
    player_store.putInt(key_enabled, counters[slot].enabled ? 1 : 0);
    player_store.putInt(key_value, counters[slot].value);
}

void SimpleCounters::load_from_nvs(int slot) {
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;
    
    char key_enabled[32];
    char key_value[32];
    
    sprintf(key_enabled, NVS_KEY_COUNTER_ENABLED, slot);
    sprintf(key_value, NVS_KEY_COUNTER_VALUE, slot);
    
    counters[slot].enabled = (player_store.getInt(key_enabled, 0) != 0);
    counters[slot].value = player_store.getInt(key_value, 0);
}

void SimpleCounters::save_all() {
    for (int i = 0; i < SIMPLE_COUNTER_COUNT; i++) {
        save_to_nvs(i);
    }
}

void SimpleCounters::load_all_from_nvs() {
    for (int i = 0; i < SIMPLE_COUNTER_COUNT; i++) {
        load_from_nvs(i);
    }
}

void SimpleCounters::create_all_enabled_ui() {
    for (int i = 0; i < SIMPLE_COUNTER_COUNT; i++) {
        if (counters[i].enabled && counters[i].container == nullptr) {
            create_counter_ui(i);
        }
    }
}

void SimpleCounters::reset_all_counters() {
    for (int i = 0; i < SIMPLE_COUNTER_COUNT; i++) {
        if (counters[i].enabled) {
            counters[i].value = 0;
            if (counters[i].label != nullptr) {
                update_display(i);
            }
            save_to_nvs(i);
        }
    }
}

// Event handler for counter touch events
static void on_counter_event(lv_event_t* e) {
    int slot = (int)(intptr_t)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    
    // Only handle events for valid slots
    if (slot < 0 || slot >= SIMPLE_COUNTER_COUNT) return;
    
    printf("[Counter] Event %d for slot %d\n", code, slot);
    
    if (code == LV_EVENT_PRESSED) {
        // Store press start time
        SimpleCounters::counters[slot].press_start_time = millis();
        printf("[Counter] Pressed slot %d\n", slot);
        
    } else if (code == LV_EVENT_PRESSING) {
        // Change text color based on press duration
        uint32_t duration = millis() - SimpleCounters::counters[slot].press_start_time;
        
        if (duration > COUNTER_LONG_PRESS_THRESHOLD) {
            // Reset color (red)
            lv_obj_set_style_text_color(SimpleCounters::counters[slot].label, 
                                      lv_color_hex(COUNTER_RESET_COLOR), 0);
        } else if (duration > COUNTER_TAP_THRESHOLD) {
            // Decrement color (yellow)
            lv_obj_set_style_text_color(SimpleCounters::counters[slot].label, 
                                      lv_color_hex(COUNTER_DECREMENT_COLOR), 0);
        }
        
    } else if (code == LV_EVENT_RELEASED) {
        // Calculate duration
        uint32_t duration = millis() - SimpleCounters::counters[slot].press_start_time;
        printf("[Counter] Released slot %d, duration: %d\n", slot, duration);
        
        // Reset text color to white for maximum visibility
        lv_obj_set_style_text_color(SimpleCounters::counters[slot].label, 
                                  lv_color_white(), 0);
        
        // Perform action based on duration
        if (duration < COUNTER_TAP_THRESHOLD) {
            // Short tap: increment
            printf("[Counter] Incrementing slot %d\n", slot);
            SimpleCounters::increment(slot);
        } else if (duration < COUNTER_LONG_PRESS_THRESHOLD) {
            // Medium press: decrement
            printf("[Counter] Decrementing slot %d\n", slot);
            SimpleCounters::decrement(slot);
        } else {
            // Long press: reset
            printf("[Counter] Resetting slot %d\n", slot);
            SimpleCounters::reset(slot);
        }
        
    }
}