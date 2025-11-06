// ============================================
// Own Header (first!)
// ============================================
#include "life_counter_two_player.h"

// ============================================
// System & Framework Headers
// ============================================
#include <Arduino.h>
#include <lvgl.h>
#include <math.h>
#include <stdio.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/menu/menu.h"
#include "ui/screens/tools/timer.h"
#include "ui/screens/life/simple_counters.h"

// ============================================
// UI Helpers
// ============================================
#include "ui/helpers/animation_helpers.h"
#include "ui/helpers/gestures.h"
#include "ui/helpers/event_grouper.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "data/themes.h"

// ============================================
// Config
// ============================================
#include "config.h"

// ============================================
// Hardware/Storage
// ============================================
#include <ArduinoNvs.h>


// --- Two Player Life Counter GUI State ---
#define ARC_GAP_DEGREES 60
lv_obj_t *life_counter_container_2p = nullptr; // Global for menu access
static lv_obj_t *theme_background_2p = nullptr;  // Theme background image
static const Theme* last_loaded_theme_2p = nullptr;  // Track last loaded theme for change detection
static lv_obj_t *life_arc_p1 = nullptr;
static lv_obj_t *life_arc_p2 = nullptr;
static lv_obj_t *life_label_p1 = nullptr;
static lv_obj_t *life_label_p2 = nullptr;
static lv_obj_t *center_line = nullptr;
// Persistent points for center line
static lv_point_precise_t center_line_points[2];

// --- Forward Declarations ---
void update_life_label(int player, int value);
static void arc_sweep_anim_cb_p1(void *var, int32_t value);
static void arc_sweep_anim_cb_p2(void *var, int32_t value);
static void arc_sweep_anim_ready_cb(lv_anim_t *a);
static void life_counter_gesture_event_handler(lv_event_t *e);
static lv_color_t interpolate_color(lv_color_t c1, lv_color_t c2, uint8_t t);
void increment_life(int player, int value);
void decrement_life(int player, int value);
void reset_life(int player);
void queue_life_change_2p(int player, int value);

// *** PERSISTENT LIFE STORAGE (from life_counter.cpp) ***
extern void saveLifeToNVS(int life_value, int player);
extern int loadLifeFromNVS(int player);
extern void clearSavedLife();

// Suppress tap after gesture
static bool gesture_active = false;

// Event grouping for 2P mode
EventGrouper event_grouper_p1(GROUPER_WINDOW, player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX), PLAYER_ONE);
EventGrouper event_grouper_p2(GROUPER_WINDOW, player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX), PLAYER_TWO);

// Define grouped_change_label and is_initializing for 2P context
static lv_obj_t *grouped_change_label_p1 = nullptr;
static lv_obj_t *grouped_change_label_p2 = nullptr;
static bool is_initializing_2p = false;

// *** Animation targets for blink prevention ***
static int target_life_p1 = 0;
static int target_life_p2 = 0;

// Call this after boot animation to show the two-player life counter
void init_life_counter_2P()
{
  // Hide logo when 2P life counter starts
  printf("[LifeCounter2P] Starting at %lu ms\n", millis());
  lv_obj_t *screen = lv_screen_active();
  lv_obj_t *logo_img = lv_obj_get_child(screen, -1);
  if (logo_img && lv_obj_has_class(logo_img, &lv_image_class)) {
    lv_obj_add_flag(logo_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_del(logo_img);
    printf("[LifeCounter2P] Logo hidden and deleted at %lu ms\n", millis());
  } else {
    printf("[LifeCounter2P] No logo found at %lu ms\n", millis());
  }
  
  is_initializing_2p = true;  // Set flag to indicate initialization is active
  teardown_life_counter_2P(); // Clean up any previous state
  
  // *** LOAD SAVED LIFE VALUES FIRST - no temporary max life display ***
  target_life_p1 = loadLifeFromNVS(1);  // Player 1
  target_life_p2 = loadLifeFromNVS(2);  // Player 2
  event_grouper_p1.resetHistory(target_life_p1);
  event_grouper_p2.resetHistory(target_life_p2);
  if (!life_counter_container_2p)
  {
    life_counter_container_2p = lv_obj_create(lv_scr_act());
    lv_obj_set_size(life_counter_container_2p, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_radius(life_counter_container_2p, LV_RADIUS_CIRCLE, 0);
    lv_obj_clear_flag(life_counter_container_2p, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(life_counter_container_2p, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(life_counter_container_2p, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(life_counter_container_2p, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(life_counter_container_2p, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(life_counter_container_2p, 0, 0);
    lv_obj_set_style_border_width(life_counter_container_2p, 0, 0);

    // Set up grid: 3 columns, 1 row
    static lv_coord_t col_dsc[] = {10, LV_GRID_FR(1), 5, LV_GRID_FR(1), 10, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {150, 150, 60, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(life_counter_container_2p, col_dsc, row_dsc);
    lv_obj_set_layout(life_counter_container_2p, LV_LAYOUT_GRID);
  }
  
  // *** INITIALIZE COUNTERS AFTER CONTAINER IS CREATED for 2P mode ***
  SimpleCounters::init();
  SimpleCounters::create_all_enabled_ui();
  
  // *** THEME BACKGROUND INTEGRATION WITH LIVE UPDATE ***
  // Get current theme and check if it changed
  const Theme* current_theme = getCurrentTheme();
  
  // Detect theme change: either pointer changed or theme properties changed
  bool theme_changed = (current_theme != last_loaded_theme_2p);
  
  if (theme_changed)
  {
    printf("[LifeCounter2P] Theme change detected - reloading background\n");
    
    // Clean up old background if exists
    if (theme_background_2p)
    {
      lv_obj_del(theme_background_2p);
      theme_background_2p = nullptr;
      printf("[LifeCounter2P] Old theme background removed\n");
    }
    
    // Create new background if theme is active
    if (current_theme != nullptr && current_theme->background_image != nullptr)
    {
      // Create background image as FIRST child (lowest z-order)
      theme_background_2p = lv_image_create(life_counter_container_2p);
      lv_image_set_src(theme_background_2p, current_theme->background_image);
      
      // Scale to fill screen (360x360)
      lv_obj_set_size(theme_background_2p, SCREEN_WIDTH, SCREEN_HEIGHT);
      lv_obj_align(theme_background_2p, LV_ALIGN_CENTER, 0, 0);
      
      // Set opacity from theme configuration
      lv_obj_set_style_opa(theme_background_2p, current_theme->background_opacity, 0);
      
      // Ensure background is not clickable and doesn't interfere with UI
      lv_obj_clear_flag(theme_background_2p, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_flag(theme_background_2p, LV_OBJ_FLAG_IGNORE_LAYOUT);
      
      // Move to background (lowest z-order) - ensures all UI elements are on top
      lv_obj_move_background(theme_background_2p);
      
      printf("[LifeCounter2P] New theme background loaded with opacity %d\n", current_theme->background_opacity);
      
      // Recreate counters after theme loading to ensure they're on top
      SimpleCounters::create_all_enabled_ui();
    }
    else
    {
      printf("[LifeCounter2P] Theme OFF - no background displayed\n");
    }
    
    // Update last loaded theme
    last_loaded_theme_2p = current_theme;
  }
  if (!life_arc_p1)
  {
    life_arc_p1 = lv_arc_create(life_counter_container_2p);
    lv_obj_add_flag(life_arc_p1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(life_arc_p1, SCREEN_DIAMETER, SCREEN_DIAMETER);
    lv_obj_align(life_arc_p1, LV_ALIGN_CENTER, 0, 0);
    int arc1_bg_start = 90 + ARC_GAP_DEGREES / 2;
    int arc1_bg_end = 270;
    lv_arc_set_bg_angles(life_arc_p1, arc1_bg_start, arc1_bg_end);
    lv_arc_set_angles(life_arc_p1, arc1_bg_start, arc1_bg_start); // Start at min
    lv_obj_set_style_arc_color(life_arc_p1, GREEN_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(life_arc_p1, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc_p1, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc_p1, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_remove_style(life_arc_p1, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(life_arc_p1, LV_OBJ_FLAG_CLICKABLE);
  }
  if (!life_label_p1)
  {
    life_label_p1 = lv_label_create(life_counter_container_2p);
    lv_obj_add_flag(life_label_p1, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(life_label_p1, "0");
    if (event_grouper_p1.getLifeTotal() > 999)
      lv_obj_set_style_text_font(life_label_p1, &lv_font_montserrat_48, 0);
    else
      lv_obj_set_style_text_font(life_label_p1, &lv_font_montserrat_72, 0);
    lv_obj_set_style_text_color(life_label_p1, lv_color_white(), 0);
    lv_obj_set_style_text_opa(life_label_p1, LV_OPA_TRANSP, 0);
    lv_obj_set_grid_cell(life_label_p1, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 1, 1);
  }
  if (!grouped_change_label_p1)
  {
    grouped_change_label_p1 = lv_label_create(life_counter_container_2p);
    lv_obj_add_flag(grouped_change_label_p1, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(grouped_change_label_p1, "0");
    lv_obj_set_style_text_font(grouped_change_label_p1, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(grouped_change_label_p1, lv_color_white(), 0);
    lv_obj_set_grid_cell(grouped_change_label_p1, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_END, 0, 1);
  }
  if (!life_arc_p2)
  {
    life_arc_p2 = lv_arc_create(life_counter_container_2p);
    lv_obj_add_flag(life_arc_p2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(life_arc_p2, SCREEN_DIAMETER, SCREEN_DIAMETER);
    lv_obj_align(life_arc_p2, LV_ALIGN_CENTER, 0, 0);
    int arc2_bg_start = 270;
    int arc2_bg_end = 90 - ARC_GAP_DEGREES / 2;
    lv_arc_set_bg_angles(life_arc_p2, arc2_bg_start, arc2_bg_end);
    lv_arc_set_angles(life_arc_p2, arc2_bg_start, arc2_bg_start); // Start at min
    lv_arc_set_mode(life_arc_p2, LV_ARC_MODE_REVERSE);
    lv_obj_set_style_arc_color(life_arc_p2, GREEN_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(life_arc_p2, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc_p2, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc_p2, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_remove_style(life_arc_p2, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(life_arc_p2, LV_OBJ_FLAG_CLICKABLE);
  }
  if (!life_label_p2)
  {
    life_label_p2 = lv_label_create(life_counter_container_2p);
    lv_obj_add_flag(life_label_p2, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(life_label_p2, "0");
    if (event_grouper_p2.getLifeTotal() > 999)
      lv_obj_set_style_text_font(life_label_p2, &lv_font_montserrat_48, 0);
    else
      lv_obj_set_style_text_font(life_label_p2, &lv_font_montserrat_72, 0);
    lv_obj_set_style_text_color(life_label_p2, lv_color_white(), 0);
    lv_obj_set_style_text_opa(life_label_p2, LV_OPA_TRANSP, 0);
    lv_obj_set_grid_cell(life_label_p2, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_START, 1, 1);
  }
  if (!grouped_change_label_p2)
  {
    grouped_change_label_p2 = lv_label_create(life_counter_container_2p);
    lv_obj_add_flag(grouped_change_label_p2, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(grouped_change_label_p2, "0");
    lv_obj_set_style_text_font(grouped_change_label_p2, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(grouped_change_label_p2, lv_color_white(), 0);
    lv_obj_set_grid_cell(grouped_change_label_p2, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_END, 0, 1);
  }

  if (life_arc_p1)
  {
    lv_obj_clear_flag(life_arc_p1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_arc_opa(life_arc_p1, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_anim_t anim1;
    lv_anim_init(&anim1);
    lv_anim_set_var(&anim1, NULL);
    lv_anim_set_exec_cb(&anim1, arc_sweep_anim_cb_p1);
    lv_anim_set_values(&anim1, 0, SMOOTH_ARC_STEPS);  // Smooth animation with 1000 steps
    lv_anim_set_time(&anim1, ARC_ANIMATION_DURATION);  // Faster, responsive animation
    lv_anim_set_delay(&anim1, 0);
    lv_anim_set_ready_cb(&anim1, arc_sweep_anim_ready_cb);
    lv_anim_start(&anim1);
  }

  if (life_arc_p2)
  {
    lv_obj_clear_flag(life_arc_p2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_arc_opa(life_arc_p2, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_anim_t anim2;
    lv_anim_init(&anim2);
    lv_anim_set_var(&anim2, NULL);
    lv_anim_set_exec_cb(&anim2, arc_sweep_anim_cb_p2);
    lv_anim_set_values(&anim2, 0, SMOOTH_ARC_STEPS);  // Smooth animation with 1000 steps
    lv_anim_set_time(&anim2, ARC_ANIMATION_DURATION);  // Faster, responsive animation
    lv_anim_set_delay(&anim2, 0);
    lv_anim_set_ready_cb(&anim2, arc_sweep_anim_ready_cb);
    lv_anim_start(&anim2);
  }
  
  // Now create the center line so it is drawn on top
  if (!center_line)
  {
    int cont_w = SCREEN_DIAMETER;
    int cont_h = SCREEN_DIAMETER;
    int x_center = cont_w / 2;
    center_line_points[0].x = x_center;
    center_line_points[0].y = 60;
    center_line_points[1].x = x_center;
    center_line_points[1].y = cont_h - 60;
    center_line = lv_line_create(life_counter_container_2p);
    lv_line_set_points(center_line, center_line_points, 2);
    lv_obj_set_style_line_color(center_line, WHITE_COLOR, 0);
    lv_obj_set_style_line_width(center_line, 1, 0);
    lv_obj_set_style_line_opa(center_line, LV_OPA_COVER, 0);
    lv_obj_set_style_line_rounded(center_line, 1, 0);
  }
  
  // Fade in the life labels
  lv_obj_clear_flag(life_label_p1, LV_OBJ_FLAG_HIDDEN);
  fade_in_obj(life_label_p1, 1000, 0, NULL);
  lv_obj_clear_flag(life_label_p2, LV_OBJ_FLAG_HIDDEN);
  fade_in_obj(life_label_p2, 1000, 0, NULL);

  uint64_t show_timer = player_store.getInt(KEY_SHOW_TIMER, 0);
  if (!timer_container && show_timer)
  {
    render_timer(life_counter_container_2p);
    lv_obj_set_grid_cell(timer_container, LV_GRID_ALIGN_CENTER, 0, 5, LV_GRID_ALIGN_START, 2, 1);
  }
}

// Increment life total and update label
void increment_life(int player, step_size_t step_size)
{
  int value = (step_size == STEP_SIZE_SMALL) ? player_store.getInt(KEY_LIFE_STEP_SMALL, DEFAULT_LIFE_INCREMENT_SMALL)
                                             : player_store.getInt(KEY_LIFE_STEP_LARGE, DEFAULT_LIFE_INCREMENT_LARGE);
  queue_life_change_2p(player, value);
}

// Decrement life total and update label
void decrement_life(int player, step_size_t step_size)
{
  int value = (step_size == STEP_SIZE_SMALL) ? player_store.getInt(KEY_LIFE_STEP_SMALL, DEFAULT_LIFE_INCREMENT_SMALL)
                                             : player_store.getInt(KEY_LIFE_STEP_LARGE, DEFAULT_LIFE_INCREMENT_LARGE);
  queue_life_change_2p(player, -value);
}

/**
 * @brief Refresh theme background without full screen re-render (Two Player)
 * 
 * Called when returning to Life Counter from Settings menu.
 * Checks if theme changed and updates background accordingly.
 * Does NOT reset life points or other UI elements.
 */
void refresh_life_counter_2p_theme()
{
  // Only refresh if Life Counter 2P is active
  if (life_counter_container_2p == nullptr)
  {
    printf("[LifeCounter2P] Refresh skipped - screen not active\n");
    return;
  }
  
  // Get current theme from settings
  const Theme* current_theme = getCurrentTheme();
  
  // Check if theme changed since last load
  bool theme_changed = (current_theme != last_loaded_theme_2p);
  
  if (theme_changed)
  {
    printf("[LifeCounter2P] === THEME REFRESH TRIGGERED ===\n");
    printf("[LifeCounter2P] Old theme: %p, New theme: %p\n", last_loaded_theme_2p, current_theme);
    
    // Clean up old background if exists
    if (theme_background_2p)
    {
      lv_obj_del(theme_background_2p);
      theme_background_2p = nullptr;
      printf("[LifeCounter2P] Old background removed\n");
    }
    
    // Create new background if theme is active
    if (current_theme != nullptr && current_theme->background_image != nullptr)
    {
      // Create background image
      theme_background_2p = lv_image_create(life_counter_container_2p);
      lv_image_set_src(theme_background_2p, current_theme->background_image);
      
      // Scale to fill screen (360x360)
      lv_obj_set_size(theme_background_2p, SCREEN_WIDTH, SCREEN_HEIGHT);
      lv_obj_align(theme_background_2p, LV_ALIGN_CENTER, 0, 0);
      
      // Set opacity from theme configuration
      lv_obj_set_style_opa(theme_background_2p, current_theme->background_opacity, 0);
      
      // Ensure background is not clickable and doesn't interfere with UI
      lv_obj_clear_flag(theme_background_2p, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_flag(theme_background_2p, LV_OBJ_FLAG_IGNORE_LAYOUT);
      
      // Move to background (lowest z-order) - ensures all UI elements are on top
      lv_obj_move_background(theme_background_2p);
      
      printf("[LifeCounter2P] New background loaded (opacity: %d)\n", current_theme->background_opacity);
      
      // Recreate counters after theme loading to ensure they're on top
      SimpleCounters::create_all_enabled_ui();
    }
    else
    {
      printf("[LifeCounter2P] Theme OFF - no background\n");
    }
    
    // Update last loaded theme
    last_loaded_theme_2p = current_theme;
    
    // Force LVGL to redraw the screen
    lv_obj_invalidate(life_counter_container_2p);
    
    printf("[LifeCounter2P] === THEME REFRESH COMPLETE ===\n");
  }
  else
  {
    printf("[LifeCounter2P] Theme unchanged - no refresh needed\n");
  }
}

void teardown_life_counter_2P()
{
  // *** SHUTDOWN COUNTERS for 2P mode ***
  SimpleCounters::shutdown();
  
  // *** KEEP CURRENT LIFE: Don't reset history to max life during teardown ***
  // The event_groupers maintain their current state for seamless UI transitions
  teardown_timer();
  clear_gesture_callbacks();
  
  if (life_counter_container_2p)
  {
    lv_obj_del(life_counter_container_2p);
    life_counter_container_2p = nullptr;
  }
  
  life_arc_p1 = nullptr;
  life_arc_p2 = nullptr;
  life_label_p1 = nullptr;
  life_label_p2 = nullptr;
  grouped_change_label_p1 = nullptr;
  grouped_change_label_p2 = nullptr;
  center_line = nullptr;
  theme_background_2p = nullptr;  // Theme background cleanup
  last_loaded_theme_2p = nullptr;  // Reset theme tracking
}

/**
 * @brief Reset theme tracker to force refresh on next render
 * 
 * Used when preset changes in Automatic mode to ensure
 * Life Counter theme updates immediately.
 */
void resetLastLoadedTheme2P() {
  last_loaded_theme_2p = nullptr;  // Force refresh on next render
}

// Reset life total for Player 1
void reset_life_2p()
{
  // Use start life (KEY_LIFE_MAX) for reset - this is the correct value
  int start_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  
  // *** MARK CURRENT LIFE AS INVALID FIRST: Prevents auto-save during reset ***
  clearSavedLife();
  
  event_grouper_p1.resetHistory(start_life);
  update_life_label(1, start_life);
  event_grouper_p2.resetHistory(start_life);
  update_life_label(2, start_life);
  
  // *** SAVE NEW LIFE: Immediately save the reset values and mark as valid ***
  // This ensures persistence works correctly after reset
  saveLifeToNVS(start_life, 1);  // Player 1
  saveLifeToNVS(start_life, 2);  // Player 2
}

// *** SMOOTH ARC ANIMATION CALLBACK (Player 1) ***
// Uses high-resolution interpolation for smooth movement even with low life values
static void arc_sweep_anim_cb_p1(void *var, int32_t v)
{
  // v goes from 0 to SMOOTH_ARC_STEPS (1000) for smooth interpolation
  int interpolated_life = (v * target_life_p1) / SMOOTH_ARC_STEPS;
  if (interpolated_life > target_life_p1)
    interpolated_life = target_life_p1;
  
  int max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  int arc_start = 90 + ARC_GAP_DEGREES / 2;
  int arc_end = 270;
  int arc_span = arc_end - arc_start;
  int sweep = (int)(arc_span * ((float)interpolated_life / (float)max_life) + 0.5f);
  int end_angle = arc_start + sweep;
  if (end_angle > arc_end)
    end_angle = arc_end;
  lv_arc_set_angles(life_arc_p1, arc_start, end_angle);
  update_life_label(1, interpolated_life);
}

// *** SMOOTH ARC ANIMATION CALLBACK (Player 2) ***
// Uses high-resolution interpolation for smooth movement even with low life values
static void arc_sweep_anim_cb_p2(void *var, int32_t v)
{
  // v goes from 0 to SMOOTH_ARC_STEPS (1000) for smooth interpolation
  int interpolated_life = (v * target_life_p2) / SMOOTH_ARC_STEPS;
  if (interpolated_life > target_life_p2)
    interpolated_life = target_life_p2;
  
  int max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  int arc_start = 270;
  int arc_end = 90 - ARC_GAP_DEGREES / 2;
  int arc_span = (arc_end - arc_start + 360) % 360;
  int sweep = (int)(arc_span * ((float)interpolated_life / (float)max_life) + 0.5f);
  if (arc_span == 0)
    arc_span = 360;
  if (sweep > arc_span)
    sweep = arc_span;
  int anim_end = (arc_end - sweep + 360) % 360;
  lv_arc_set_angles(life_arc_p2, anim_end, arc_end);
  update_life_label(2, interpolated_life);
}

// Animation ready callback
static void arc_sweep_anim_ready_cb(lv_anim_t *a)
{
  // Values already loaded in init, just finish initialization
  is_initializing_2p = false;
  
  register_gesture_callback(GestureType::TapTopLeft, []()
                            { increment_life(PLAYER_ONE, step_size_t::STEP_SIZE_SMALL); });
  register_gesture_callback(GestureType::TapBottomLeft, []()
                            { decrement_life(PLAYER_ONE, step_size_t::STEP_SIZE_SMALL); });
  register_gesture_callback(GestureType::LongPressTopLeft, []()
                            { increment_life(PLAYER_ONE, step_size_t::STEP_SIZE_LARGE); });
  register_gesture_callback(GestureType::LongPressBottomLeft, []()
                            { decrement_life(PLAYER_ONE, step_size_t::STEP_SIZE_LARGE); });
  register_gesture_callback(GestureType::TapTopRight, []()
                            { increment_life(PLAYER_TWO, step_size_t::STEP_SIZE_SMALL); });
  register_gesture_callback(GestureType::TapBottomRight, []()
                            { decrement_life(PLAYER_TWO, step_size_t::STEP_SIZE_SMALL); });
  register_gesture_callback(GestureType::LongPressTopRight, []()
                            { increment_life(PLAYER_TWO, step_size_t::STEP_SIZE_LARGE); });
  register_gesture_callback(GestureType::LongPressBottomRight, []()
                            { decrement_life(PLAYER_TWO, step_size_t::STEP_SIZE_LARGE); });

  // Get gesture mode from NVS
  int gesture_mode = player_store.getInt(KEY_GESTURE_MODE, GESTURE_CONTROL_MODE);

  if (gesture_mode == 0) {
    // Classic Mode: Long press center opens menu
    register_gesture_callback(GestureType::LongPressCenter, []() {
        if (getCurrentMenu() == MENU_NONE) {
            renderMenu(MENU_CONTEXTUAL);
        }
    });
  } else {
    // Long Press Mode: Swipe down opens menu
    register_gesture_callback(GestureType::SwipeDown, []()
                              { if(getCurrentMenu() == MENU_NONE)
                                  renderMenu(MENU_CONTEXTUAL); });
  }
}

// Player 1: arc grows clockwise from 90° to 270°
static arc_segment_t life_to_arc_p1(int life_total)
{
  int max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  arc_segment_t seg = {0};
  int arc_life = life_total;
  if (arc_life < 0)
    arc_life = 0;
  if (arc_life > max_life)
    arc_life = max_life;
  lv_color_t arc_color;
  if (arc_life >= (int)(0.875 * max_life))
    arc_color = GREEN_COLOR;
  else if (arc_life >= (int)(0.55 * max_life))
    arc_color = interpolate_color(YELLOW_COLOR, GREEN_COLOR, (uint8_t)(((arc_life - (int)(0.55 * max_life)) * 255) / ((int)(0.875 * max_life) - (int)(0.55 * max_life))));
  else if (arc_life >= (int)(0.25 * max_life))
    arc_color = interpolate_color(RED_COLOR, YELLOW_COLOR, (uint8_t)(((arc_life - (int)(0.25 * max_life)) * 255) / ((int)(0.55 * max_life) - (int)(0.25 * max_life))));
  else
    arc_color = RED_COLOR;
  if (arc_life >= (int)(0.875 * max_life))
    arc_color = GREEN_COLOR;
  int arc_start = 90 + ARC_GAP_DEGREES / 2;
  int arc_end = 270;
  int arc_span = arc_end - arc_start;
  int sweep = (int)(arc_span * ((float)arc_life / (float)max_life) + 0.5f);
  int seg_end = arc_start + sweep;
  if (seg_end > arc_end)
    seg_end = arc_end;
  seg.start_angle = arc_start;
  seg.end_angle = seg_end;
  seg.color = arc_color;
  return seg;
}

// Player 2: arc grows counterclockwise from 270° to 90°
static arc_segment_t life_to_arc_p2(int life_total)
{
  int max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  arc_segment_t seg = {0};
  int arc_life = life_total;
  if (arc_life < 0)
    arc_life = 0;
  if (arc_life > max_life)
    arc_life = max_life;
  lv_color_t arc_color;
  if (arc_life >= (int)(0.875 * max_life))
    arc_color = GREEN_COLOR;
  else if (arc_life >= (int)(0.55 * max_life))
    arc_color = interpolate_color(YELLOW_COLOR, GREEN_COLOR, (uint8_t)(((arc_life - (int)(0.55 * max_life)) * 255) / ((int)(0.875 * max_life) - (int)(0.55 * max_life))));
  else if (arc_life >= (int)(0.25 * max_life))
    arc_color = interpolate_color(RED_COLOR, YELLOW_COLOR, (uint8_t)(((arc_life - (int)(0.25 * max_life)) * 255) / ((int)(0.55 * max_life) - (int)(0.25 * max_life))));
  else
    arc_color = RED_COLOR;
  if (arc_life >= (int)(0.875 * max_life))
    arc_color = GREEN_COLOR;

  int arc_start = 270;
  int arc_end = 90 - ARC_GAP_DEGREES / 2;
  int arc_span = (arc_end - arc_start + 360) % 360;
  float percent = (float)arc_life / (float)max_life;
  int sweep = (int)(arc_span * percent + 0.5f);
  if (arc_span == 0)
    arc_span = 360;
  if (sweep > arc_span)
    sweep = arc_span;
  int seg_start = (arc_end - sweep + 360) % 360;
  seg.start_angle = seg_start;
  seg.end_angle = arc_end;
  seg.color = arc_color;
  return seg;
}

// Update the life label and arc for Player 1 or 2
void update_life_label(int player, int new_life_total)
{
  lv_obj_t *life_label = (player == 1) ? life_label_p1 : life_label_p2;
  lv_obj_t *life_arc = (player == 1) ? life_arc_p1 : life_arc_p2;

  if (life_label != nullptr)
  {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", new_life_total);
    lv_label_set_text(life_label, buf);
    
    // Dynamische Schriftgrößen-Anpassung basierend auf Zahlenlänge
    if (new_life_total > 999)
      lv_obj_set_style_text_font(life_label, &lv_font_montserrat_48, 0);
    else
      lv_obj_set_style_text_font(life_label, &lv_font_montserrat_72, 0);
  }

  if (life_arc != nullptr)
  {
    arc_segment_t seg = (player == 1) ? life_to_arc_p1(new_life_total) : life_to_arc_p2(new_life_total);
    lv_arc_set_angles(life_arc, seg.start_angle, seg.end_angle);
    lv_obj_set_style_arc_color(life_arc, seg.color, LV_PART_INDICATOR);
  }
}

// Helper for color interpolation
static lv_color_t interpolate_color(lv_color_t c1, lv_color_t c2, uint8_t t)
{
  uint16_t c1_16 = lv_color_to_u16(c1);
  uint16_t c2_16 = lv_color_to_u16(c2);
  uint8_t r1 = (c1_16 >> 11) & 0x1F;
  uint8_t g1 = (c1_16 >> 5) & 0x3F;
  uint8_t b1 = c1_16 & 0x1F;
  uint8_t r2 = (c2_16 >> 11) & 0x1F;
  uint8_t g2 = (c2_16 >> 5) & 0x3F;
  uint8_t b2 = c2_16 & 0x1F;
  r1 = (r1 << 3) | (r1 >> 2);
  g1 = (g1 << 2) | (g1 >> 4);
  b1 = (b1 << 3) | (b1 >> 2);
  r2 = (r2 << 3) | (r2 >> 2);
  g2 = (g2 << 2) | (g2 >> 4);
  b2 = (b2 << 3) | (b2 >> 2);
  uint8_t r = (uint8_t)(r1 + ((int)r2 - (int)r1) * t / 255);
  uint8_t g = (uint8_t)(g1 + ((int)g2 - (int)g1) * t / 255);
  uint8_t b = (uint8_t)(b1 + ((int)b2 - (int)b1) * t / 255);
  return lv_color_make(r, g, b);
}

void life_counter2p_loop()
{
  if (event_grouper_p1.isCommitPending())
  {
    event_grouper_p1.loop();
  }
  if (event_grouper_p2.isCommitPending())
  {
    event_grouper_p2.loop();
  }
}

void queue_life_change_2p(int player, int value)
{
  EventGrouper *grouper = (player == 1) ? &event_grouper_p1 : &event_grouper_p2;
  lv_obj_t *grouped_change_label = (player == 1) ? grouped_change_label_p1 : grouped_change_label_p2;
  if (grouped_change_label != nullptr && !is_initializing_2p)
  {
    int pending_change = grouper->getPendingChange() + value;
    int current_life = grouper->getLifeTotal();
    if (player == 1)
      update_life_label(1, (current_life + pending_change));
    else
      update_life_label(2, (current_life + pending_change));
    char buf[8];
    if (pending_change > 0)
    {
      snprintf(buf, sizeof(buf), "+%d", pending_change);
    }
    else
    {
      snprintf(buf, sizeof(buf), "%d", pending_change);
    }
    lv_obj_set_style_text_color(grouped_change_label, pending_change >= 0 ? GREEN_COLOR : RED_COLOR, 0);
    lv_label_set_text(grouped_change_label, buf);
    lv_obj_clear_flag(grouped_change_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_opa(grouped_change_label, LV_OPA_COVER, 0);
    fade_out_obj(grouped_change_label, 100, GROUPER_WINDOW, [](lv_anim_t *fade_out_anim)
                 {
      if (fade_out_anim && fade_out_anim->var) {
        lv_obj_add_flag((lv_obj_t *)fade_out_anim->var, LV_OBJ_FLAG_HIDDEN);
      } });
  }
  grouper->handleChange(player, value, get_elapsed_seconds(), [](const LifeHistoryEvent &evt) {
    // *** AUTO-SAVE: Save life to NVS whenever a change is committed (Two-Player) ***
    saveLifeToNVS(evt.life_total, evt.player_id);
  });
}