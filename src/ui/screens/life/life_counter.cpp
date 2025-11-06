// ============================================
// Own Header (first!)
// ============================================
#include "life_counter.h"
#include "simple_counters.h"

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


// --- Life Counter GUI State ---
lv_obj_t *life_counter_container = nullptr;
static lv_obj_t *theme_background = nullptr;  // Theme background image
static const Theme* last_loaded_theme = nullptr;  // Track last loaded theme for change detection
static lv_obj_t *life_arc = nullptr;
static lv_obj_t *life_label = nullptr;
static lv_obj_t *grouped_change_label = nullptr;

EventGrouper event_grouper(GROUPER_WINDOW, player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX), PLAYER_SINGLE);

// --- Forward Declarations ---
void update_life_label(int value);
static void arc_sweep_anim_cb(void *var, int32_t value);
static void arc_sweep_anim_ready_cb(lv_anim_t *anim);
void lvgl_gesture_event_handler(lv_event_t *e);
static lv_color_t interpolate_color(lv_color_t c1, lv_color_t c2, uint8_t t);
void increment_life(int value);
void decrement_life(int value);
void reset_life();
void queue_life_change(int player, int value);


// *** PERSISTENT LIFE STORAGE ***
void saveLifeToNVS(int life_value, int player);
int loadLifeFromNVS(int player);
void clearSavedLife();

static bool is_initializing = false;

void init_life_counter()
{
  // Hide logo when life counter starts
  printf("[LifeCounter] Starting at %lu ms\n", millis());
  lv_obj_t *screen = lv_screen_active();
  printf("[LifeCounter] Screen has %d children at %lu ms\n", lv_obj_get_child_cnt(screen), millis());
  
  lv_obj_t *logo_img = lv_obj_get_child(screen, -1); // Get last child (should be logo)
  printf("[LifeCounter] Last child: %p at %lu ms\n", logo_img, millis());
  
  if (logo_img) {
    printf("[LifeCounter] Child class: %p at %lu ms\n", lv_obj_get_class(logo_img), millis());
    printf("[LifeCounter] Is image class: %s at %lu ms\n", lv_obj_has_class(logo_img, &lv_image_class) ? "YES" : "NO", millis());
    
    if (lv_obj_has_class(logo_img, &lv_image_class)) {
      lv_obj_add_flag(logo_img, LV_OBJ_FLAG_HIDDEN);
      lv_obj_del(logo_img); // Actually delete the logo object
      printf("[LifeCounter] Logo hidden and deleted at %lu ms\n", millis());
    } else {
      printf("[LifeCounter] Logo not found or not an image at %lu ms\n", millis());
    }
  } else {
    printf("[LifeCounter] No last child found at %lu ms\n", millis());
  }
  
  is_initializing = true;
  teardown_life_counter();
  
  // *** LOAD SAVED LIFE VALUES FIRST ***
  int saved_life = loadLifeFromNVS(1);  // Load before any UI setup
  event_grouper.resetHistory(saved_life);
  
  if (!life_counter_container)
  {
    life_counter_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(life_counter_container, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(life_counter_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(life_counter_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(life_counter_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(life_counter_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(life_counter_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(life_counter_container, LV_OPA_TRANSP, 0);
    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {130, 150, 60, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(life_counter_container, col_dsc, row_dsc);
    lv_obj_set_layout(life_counter_container, LV_LAYOUT_GRID);
  }
  
  // *** THEME BACKGROUND INTEGRATION WITH LIVE UPDATE ***
  // Get current theme and check if it changed
  const Theme* current_theme = getCurrentTheme();
  
  // Detect theme change: either pointer changed or theme properties changed
  bool theme_changed = (current_theme != last_loaded_theme);
  
  if (theme_changed)
  {
    printf("[LifeCounter] Theme change detected - reloading background\n");
    
    // Clean up old background if exists
    if (theme_background)
    {
      lv_obj_del(theme_background);
      theme_background = nullptr;
      printf("[LifeCounter] Old theme background removed\n");
    }
    
    // Create new background if theme is active
    if (current_theme != nullptr && current_theme->background_image != nullptr)
    {
      // Create background image as FIRST child (lowest z-order)
      theme_background = lv_image_create(life_counter_container);
      lv_image_set_src(theme_background, current_theme->background_image);
      
      // Scale to fill screen (360x360)
      lv_obj_set_size(theme_background, SCREEN_WIDTH, SCREEN_HEIGHT);
      lv_obj_align(theme_background, LV_ALIGN_CENTER, 0, 0);
      
      // Set opacity from theme configuration
      lv_obj_set_style_opa(theme_background, current_theme->background_opacity, 0);
      
      // Ensure background is not clickable and doesn't interfere with UI
      lv_obj_clear_flag(theme_background, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_flag(theme_background, LV_OBJ_FLAG_IGNORE_LAYOUT);
      
      // Move to background (lowest z-order) - ensures all UI elements are on top
      lv_obj_move_background(theme_background);
      
      printf("[LifeCounter] New theme background loaded with opacity %d\n", current_theme->background_opacity);
    }
    else
    {
      printf("[LifeCounter] Theme OFF - no background displayed\n");
    }
    
    // Update last loaded theme
    last_loaded_theme = current_theme;
  }
  
  if (!life_arc)
  {
    life_arc = lv_arc_create(life_counter_container);
    lv_obj_add_flag(life_arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(life_arc, SCREEN_DIAMETER, SCREEN_DIAMETER);
    lv_obj_align(life_arc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(life_arc, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_arc_set_bg_angles(life_arc, 0, 360);
    lv_arc_set_angles(life_arc, 270, 270);
    lv_obj_set_style_arc_color(life_arc, GREEN_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(life_arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_remove_style(life_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(life_arc, LV_OBJ_FLAG_CLICKABLE);
  }
  
  if (!life_label)
  {
    life_label = lv_label_create(life_counter_container);
    lv_obj_add_flag(life_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(life_label, "0");
    if (event_grouper.getLifeTotal() > 999)
      lv_obj_set_style_text_font(life_label, &lv_font_montserrat_48, 0);
    else
      lv_obj_set_style_text_font(life_label, &lv_font_montserrat_72, 0);
    lv_obj_set_style_text_color(life_label, lv_color_white(), 0);
    lv_obj_align(life_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_opa(life_label, LV_OPA_TRANSP, 0);
    lv_obj_set_grid_cell(life_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 1, 1);
  }
  
  if (!grouped_change_label)
  {
    grouped_change_label = lv_label_create(life_counter_container);
    lv_obj_add_flag(grouped_change_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(grouped_change_label, "0");
    lv_obj_set_style_text_font(grouped_change_label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(grouped_change_label, lv_color_white(), 0);
    lv_obj_set_grid_cell(grouped_change_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_END, 0, 1);
  }
  
  
  if (life_arc)
  {
    lv_obj_clear_flag(life_arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_arc_opa(life_arc, LV_OPA_COVER, LV_PART_INDICATOR);
    
    // *** Life already loaded at init_life_counter() start ***
    
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, NULL);
    lv_anim_set_exec_cb(&anim, arc_sweep_anim_cb);
    lv_anim_set_values(&anim, 0, SMOOTH_ARC_STEPS);  // Smooth animation with 1000 steps
    lv_anim_set_time(&anim, ARC_ANIMATION_DURATION);  // Faster, responsive animation
    lv_anim_set_delay(&anim, 0);
    lv_anim_set_ready_cb(&anim, arc_sweep_anim_ready_cb);
    lv_anim_start(&anim);
  }

  if (life_label)
  {
    lv_obj_clear_flag(life_label, LV_OBJ_FLAG_HIDDEN);
    fade_in_obj(life_label, 1000, 0, NULL);
  }

  uint64_t show_timer = player_store.getInt(KEY_SHOW_TIMER, 0);
  if (!timer_container && show_timer)
  {
    render_timer(life_counter_container);
    lv_obj_set_grid_cell(timer_container, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  }
  
  SimpleCounters::init();
}

void increment_life(step_size_t step_size)
{
  int value = (step_size == STEP_SIZE_SMALL) ? player_store.getInt(KEY_LIFE_STEP_SMALL, DEFAULT_LIFE_INCREMENT_SMALL)
                                             : player_store.getInt(KEY_LIFE_STEP_LARGE, DEFAULT_LIFE_INCREMENT_LARGE);
  queue_life_change(PLAYER_SINGLE, value);
}

void decrement_life(step_size_t step_size)
{
  int value = (step_size == STEP_SIZE_SMALL) ? player_store.getInt(KEY_LIFE_STEP_SMALL, DEFAULT_LIFE_INCREMENT_SMALL)
                                             : player_store.getInt(KEY_LIFE_STEP_LARGE, DEFAULT_LIFE_INCREMENT_LARGE);
  queue_life_change(PLAYER_SINGLE, -value);
}

void reset_life()
{
  // Use start life (KEY_LIFE_MAX) for reset - this is the correct value
  int start_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  
  // *** MARK CURRENT LIFE AS INVALID FIRST: Prevents auto-save during reset ***
  clearSavedLife();
  
  event_grouper.resetHistory(start_life);
  update_life_label(start_life);
  
  // *** SAVE NEW LIFE: Immediately save the reset value and mark as valid ***
  // This ensures persistence works correctly after reset
  saveLifeToNVS(start_life, PLAYER_SINGLE);
  
  // *** RESET COUNTERS: Reset all enabled counters to 0 ***
  SimpleCounters::reset_all_counters();
}

/**
 * @brief Refresh theme background without full screen re-render
 * 
 * Called when returning to Life Counter from Settings menu.
 * Checks if theme changed and updates background accordingly.
 * Does NOT reset life points or other UI elements.
 */
void refresh_life_counter_theme()
{
  // Only refresh if Life Counter is active
  if (life_counter_container == nullptr)
  {
    printf("[LifeCounter] Refresh skipped - screen not active\n");
    return;
  }
  
  // Get current theme from settings
  const Theme* current_theme = getCurrentTheme();
  
  // Check if theme changed since last load
  bool theme_changed = (current_theme != last_loaded_theme);
  
  if (theme_changed)
  {
    printf("[LifeCounter] === THEME REFRESH TRIGGERED ===\n");
    printf("[LifeCounter] Old theme: %p, New theme: %p\n", last_loaded_theme, current_theme);
    
    // Clean up old background if exists
    if (theme_background)
    {
      lv_obj_del(theme_background);
      theme_background = nullptr;
      printf("[LifeCounter] Old background removed\n");
    }
    
    // Create new background if theme is active
    if (current_theme != nullptr && current_theme->background_image != nullptr)
    {
      // Create background image
      theme_background = lv_image_create(life_counter_container);
      lv_image_set_src(theme_background, current_theme->background_image);
      
      // Scale to fill screen (360x360)
      lv_obj_set_size(theme_background, SCREEN_WIDTH, SCREEN_HEIGHT);
      lv_obj_align(theme_background, LV_ALIGN_CENTER, 0, 0);
      
      // Set opacity from theme configuration
      lv_obj_set_style_opa(theme_background, current_theme->background_opacity, 0);
      
      // Ensure background is not clickable and doesn't interfere with UI
      lv_obj_clear_flag(theme_background, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_flag(theme_background, LV_OBJ_FLAG_IGNORE_LAYOUT);
      
      // Move to background (lowest z-order) - ensures all UI elements are on top
      lv_obj_move_background(theme_background);
      
      printf("[LifeCounter] New background loaded (opacity: %d)\n", current_theme->background_opacity);
    }
    else
    {
      printf("[LifeCounter] Theme OFF - no background\n");
    }
    
    // Update last loaded theme
    last_loaded_theme = current_theme;
    
    // Force LVGL to redraw the screen
    lv_obj_invalidate(life_counter_container);
    
    printf("[LifeCounter] === THEME REFRESH COMPLETE ===\n");
  }
  else
  {
    printf("[LifeCounter] Theme unchanged - no refresh needed\n");
  }
}

void teardown_life_counter()
{
  SimpleCounters::shutdown();
  // *** KEEP CURRENT LIFE: Don't reset history to max life during teardown ***
  // The event_grouper maintains its current state for seamless UI transitions
  clear_gesture_callbacks();
  teardown_timer();
  
  if (life_counter_container)
  {
    lv_obj_del(life_counter_container);
    life_counter_container = nullptr;
  }
  
  life_arc = nullptr;
  life_label = nullptr;
  grouped_change_label = nullptr;
  theme_background = nullptr;  // Theme background cleanup
  last_loaded_theme = nullptr;  // Reset theme tracking
}

/**
 * @brief Reset theme tracker to force refresh on next render
 * 
 * Used when preset changes in Automatic mode to ensure
 * Life Counter theme updates immediately.
 */
void resetLastLoadedTheme() {
  last_loaded_theme = nullptr;  // Force refresh on next render
}

static void arc_anim_cb(void *arc_obj, int32_t v)
{
  lv_arc_set_angles((lv_obj_t *)arc_obj, 0, v);
}

static void life_fadein_ready_cb(lv_anim_t *a)
{
  if (a && a->var)
  {
    fade_out_obj((lv_obj_t *)a->var, 1500, 0, [](lv_anim_t *anim)
                 {
      if (anim && anim->var) {
        lv_obj_add_flag((lv_obj_t *)anim->var, LV_OBJ_FLAG_HIDDEN);
      }
      init_life_counter(); });
  }
}

// *** SMOOTH ARC ANIMATION CALLBACK ***
// Uses high-resolution interpolation for smooth movement even with low life values
static void arc_sweep_anim_cb(void *var, int32_t v)
{
  int target_life = event_grouper.getLifeTotal(); // Use loaded life value instead of max
  // v goes from 0 to SMOOTH_ARC_STEPS (1000) for smooth interpolation
  int interpolated_life = (v * target_life) / SMOOTH_ARC_STEPS;
  if (interpolated_life > target_life)
    interpolated_life = target_life;
  update_life_label(interpolated_life);
}

static void arc_sweep_anim_ready_cb(lv_anim_t *a)
{
  is_initializing = false;
  
  register_gesture_callback(GestureType::TapTop, []()
                            { increment_life(step_size_t::STEP_SIZE_SMALL); });
  register_gesture_callback(GestureType::TapBottom, []()
                            { decrement_life(step_size_t::STEP_SIZE_SMALL); });
  register_gesture_callback(GestureType::LongPressTop, []()
                            { increment_life(step_size_t::STEP_SIZE_LARGE); });
  register_gesture_callback(GestureType::LongPressBottom, []()
                            { decrement_life(step_size_t::STEP_SIZE_LARGE); });

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
                              {
                                if(getCurrentMenu() == MENU_NONE)
                                  renderMenu(MENU_CONTEXTUAL); });
  }
}

static arc_segment_t life_to_arc(int life_total)
{
  int max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  arc_segment_t seg = {0};
  int arc_life = life_total;
  if (arc_life < 0)
    arc_life = 0;
  if (max_life <= 0)
    max_life = 40;
  float circumference = M_PI * SCREEN_DIAMETER;
  float gap_px = 200.0f;
  float gap_deg = (gap_px / circumference) * 360.0f;
  float arc_span = 360.0f - gap_deg;
  float arc_half = arc_span / 2.0f;
  int base_start = (int)(270.0f - arc_half + 0.5f);
  int base_end = (int)(270.0f + arc_half + 0.5f);

  lv_color_t arc_color;
  if (arc_life >= (int)(0.875 * max_life))
  {
    arc_color = GREEN_COLOR;
  }
  else if (arc_life >= (int)(0.55 * max_life))
  {
    uint8_t t = (uint8_t)(((arc_life - (int)(0.55 * max_life)) * 255) / ((int)(0.875 * max_life) - (int)(0.55 * max_life)));
    arc_color = interpolate_color(YELLOW_COLOR, GREEN_COLOR, t);
  }
  else if (arc_life >= (int)(0.25 * max_life))
  {
    uint8_t t = (uint8_t)(((arc_life - (int)(0.25 * max_life)) * 255) / ((int)(0.55 * max_life) - (int)(0.25 * max_life)));
    arc_color = interpolate_color(RED_COLOR, YELLOW_COLOR, t);
  }
  else
  {
    arc_color = RED_COLOR;
  }

  if (arc_life >= (int)(0.875 * max_life))
  {
    arc_color = GREEN_COLOR;
  }

  if (arc_life > max_life)
  {
    seg.start_angle = base_start;
    seg.end_angle = base_start + (int)arc_span;
    seg.color = arc_color;
    return seg;
  }
  if (arc_life == max_life)
  {
    seg.start_angle = base_start;
    seg.end_angle = base_end % 360;
    seg.color = arc_color;
    return seg;
  }
  if (arc_life <= 0)
  {
    seg.start_angle = base_start;
    seg.end_angle = base_start;
    seg.color = arc_color;
    return seg;
  }

  int sweep = (int)(arc_span * ((float)arc_life / (float)max_life) + 0.5f);
  int end_angle = (base_start + sweep) % 360;
  seg.start_angle = base_start;
  seg.end_angle = end_angle;
  seg.color = arc_color;
  return seg;
}

void update_life_label(int new_life_total)
{
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
    arc_segment_t seg = life_to_arc(new_life_total);
    uint16_t c16 = lv_color_to_u16(seg.color);
    uint8_t r = (c16 >> 11) & 0x1F;
    uint8_t g = (c16 >> 5) & 0x3F;
    uint8_t b = c16 & 0x1F;
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    lv_arc_set_angles(life_arc, seg.start_angle, seg.end_angle);
    lv_obj_set_style_arc_color(life_arc, seg.color, LV_PART_INDICATOR);
  }
}

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

void life_counter_loop()
{
  if (event_grouper.isCommitPending())
  {
    event_grouper.loop();
  }
}

void queue_life_change(int player, int value)
{
  if (grouped_change_label != nullptr && !is_initializing)
  {
    int pending_change = event_grouper.getPendingChange() + value;
    int current_life = event_grouper.getLifeTotal();
    update_life_label((current_life + pending_change));
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
    event_grouper.handleChange(player, value, get_elapsed_seconds(), [](const LifeHistoryEvent &evt) {
      // *** AUTO-SAVE: Save life to NVS whenever a change is committed ***
      saveLifeToNVS(evt.life_total, evt.player_id);
    });
  }
  else if (grouped_change_label == nullptr && !is_initializing)
  {
  }
}

// ============================================
// PERSISTENT LIFE STORAGE FUNCTIONS
// ============================================

/**
 * @brief Save current life value to NVS for persistence across reboots
 * @param life_value Current life value to save
 * @param player Player ID (1 for single/player1, 2 for player2)
 */
void saveLifeToNVS(int life_value, int player) {
    const char* key = (player == 2) ? KEY_CURRENT_LIFE_P2 : KEY_CURRENT_LIFE_P1;
    
    player_store.putInt(key, life_value);
    player_store.putInt(KEY_CURRENT_LIFE_VALID, 1);  // Mark current life as valid
    
    printf("[LifePersist] Saved P%d current life: %d\n", player, life_value);
}

/**
 * @brief Load saved life value from NVS with intelligent fallback and corruption detection
 * @param player Player ID (1 for single/player1, 2 for player2) 
 * @return Saved life value, or start life if no valid save exists
 * 
 * Persistence behavior:
 * - First boot: valid=0 (default), loads start_life and marks as valid for future
 * - Normal operation: valid=1, loads current_life (persists across reboots)
 * - After reset/preset/mode change: valid=0 (explicit), loads start_life
 * 
 * Fallback triggers ONLY when:
 * - KEY_CURRENT_LIFE_VALID is 0 (first boot, reset, preset change, mode change)
 * - Stored value is unreasonably large (>999999 = likely corruption)
 * - NVS corruption detected (getInt returns default despite valid flag)
 */
int loadLifeFromNVS(int player) {
    // Check if current life is valid
    int is_valid = player_store.getInt(KEY_CURRENT_LIFE_VALID, -1);  // Use -1 to detect first boot
    
    // *** FIRST BOOT DETECTION: If never set before (truly first boot) ***
    // Note: is_valid == -1 means KEY_CURRENT_LIFE_VALID was never written to NVS
    // This is DIFFERENT from is_valid == 0 which means explicitly invalidated
    if (is_valid == -1) {
        printf("[LifePersist] First boot detected - initializing persistence system\n");
        // On first boot, always use start_life (don't try to recover old values)
        int start_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
        if (start_life >= 999999) {
            start_life = DEFAULT_LIFE_MAX;
        }
        printf("[LifePersist] First boot: Using start_life %d for P%d, initializing persistence\n", start_life, player);
        saveLifeToNVS(start_life, player);  // Save and mark as valid
        return start_life;
    }
    
    if (is_valid == 1) {
        // Load current life - use INT32_MAX as impossible default to detect corruption
        const char* key = (player == 2) ? KEY_CURRENT_LIFE_P2 : KEY_CURRENT_LIFE_P1;
        int current_life = player_store.getInt(key, 999999);  // Use high value to detect NVS read failure
        
        // *** CORRUPTION DETECTION: Reject unreasonable values ***
        // Accept 0 and negative values (user might decrement below 0 or set 0 as start life)
        // Only reject if value is suspiciously high (likely NVS corruption/read failure)
        if (current_life < 999999) {
            printf("[LifePersist] Loaded P%d current life: %d\n", player, current_life);
            return current_life;
        }
        
        // NVS corruption detected: valid flag is 1, but stored value is corrupted
        printf("[LifePersist] CRITICAL: P%d current life corrupted (got %d), invalidating and using fallback\n", player, current_life);
        player_store.putInt(KEY_CURRENT_LIFE_VALID, 0);  // Invalidate to prevent repeated warnings
    }
    
    // Fallback: Use start life (KEY_LIFE_MAX) when:
    // - KEY_CURRENT_LIFE_VALID is explicitly 0 (user reset/preset/mode change)
    // - NVS corruption detected and auto-invalidated
    int start_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
    
    // Additional safety: validate start_life is reasonable
    if (start_life >= 999999) {
        printf("[LifePersist] CRITICAL: KEY_LIFE_MAX corrupted (%d), using DEFAULT_LIFE_MAX\n", start_life);
        start_life = DEFAULT_LIFE_MAX;
    }
    
    printf("[LifePersist] Using fallback start_life for P%d: %d (valid=%d)\n", player, start_life, is_valid);
    return start_life;
}

/**
 * @brief Clear saved life data (e.g., when user resets game)
 * IMPORTANT: KEY_LIFE_MAX is NEVER changed - it remains as permanent fallback
 */
void clearSavedLife() {
    // Mark current life as invalid (don't delete the values)
    player_store.putInt(KEY_CURRENT_LIFE_VALID, 0);
    
    // Legacy support: also clear old keys
    player_store.putInt(KEY_LIFE_SAVE_VALID, 0);
    player_store.putInt(KEY_SAVED_LIFE_P1, 0);  
    player_store.putInt(KEY_SAVED_LIFE_P2, 0);
    
    printf("[LifePersist] Marked current life as invalid, KEY_LIFE_MAX preserved\n");
}

