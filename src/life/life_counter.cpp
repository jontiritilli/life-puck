#include "Arduino.h"
#include <lvgl.h>
#include <math.h>
#include <stdio.h>
#include <state/state_store.h>
#include <life/life_counter.h>
#include <helpers/animation_helpers.h>
#include <gestures/gestures.h>
#include <helpers/event_grouper.h>
#include <menu/menu.h>
#include "constants/constants.h"

// --- Life Counter GUI State ---
lv_obj_t *amp_button = nullptr;
lv_obj_t *life_arc = nullptr; // Global for menu access
static lv_obj_t *life_label = nullptr;
static lv_obj_t *grouped_change_label = nullptr;
static lv_obj_t *lbl_amp_label = nullptr;
static int max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
static int amp_value = 0;
static int peak_amp = 8;

EventGrouper event_grouper(GROUPER_WINDOW, max_life, PLAYER_SINGLE); // Single player mode

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
void increment_amp();
void clear_amp();

// Static flag to track initialization state
static bool is_initializing = false;

// Call this after boot animation to show the life counter
void init_life_counter()
{
  is_initializing = true;  // Set flag to indicate initialization is active
  teardown_life_counter(); // Clean up any previous state
  int amp_mode = player_store.getInt(KEY_AMP_MODE, PLAYER_SINGLE);

  // Only create arc and label if they do not exist
  if (!life_arc)
  {
    printf("[init_life_counter] Creating life_arc...\n");
    life_arc = lv_arc_create(lv_scr_act());
    lv_obj_add_flag(life_arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(life_arc, SCREEN_DIAMETER, SCREEN_DIAMETER);
    lv_obj_align(life_arc, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(life_arc, 0, 360);
    lv_arc_set_angles(life_arc, 270, 270);
    lv_obj_set_style_arc_color(life_arc, GREEN_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(life_arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc, ARC_WIDTH, LV_PART_INDICATOR);
    // lv_obj_set_style_arc_rounded(life_arc, 0, LV_PART_INDICATOR); // Square ends
    lv_obj_remove_style(life_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(life_arc, LV_OBJ_FLAG_CLICKABLE);
    printf("[show_life_counter] life_arc created.\n");
  }
  if (!life_label)
  {
    life_label = lv_label_create(lv_scr_act());
    lv_obj_add_flag(life_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(life_label, "0");                                // Always set text immediately
    lv_obj_set_style_text_font(life_label, &lv_font_montserrat_72, 0); // Large font
    lv_obj_set_style_text_color(life_label, lv_color_white(), 0);
    lv_obj_align(life_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_opa(life_label, LV_OPA_TRANSP, 0); // Start transparent

    // Create grouped change label above life_label
    grouped_change_label = lv_label_create(lv_scr_act());
    lv_obj_add_flag(grouped_change_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(grouped_change_label, "0");
    lv_obj_set_style_text_font(grouped_change_label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(grouped_change_label, lv_color_white(), 0);
    lv_obj_align_to(grouped_change_label, life_label, LV_ALIGN_OUT_TOP_MID, -10, -10);
  }
  // Create amp button if not already created
  if (!amp_button)
  {
    amp_button = lv_btn_create(lv_scr_act());
    lv_obj_set_size(amp_button, 110, 110);
    lv_obj_set_style_radius(amp_button, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(amp_button, LIGHTNING_BLUE_COLOR, 0);
    static bool amp_long_press = false;
    lv_obj_add_event_cb(amp_button, [](lv_event_t *e)
                        { amp_long_press = false; }, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(amp_button, [](lv_event_t *e)
                        {
        amp_long_press = true;
        clear_amp(); }, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(amp_button, [](lv_event_t *e)
                        {
        if (!amp_long_press) increment_amp(); }, LV_EVENT_CLICKED, NULL);
    lbl_amp_label = lv_label_create(amp_button);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", amp_value);
    lv_label_set_text(lbl_amp_label, buf);
    lv_obj_set_style_text_color(lbl_amp_label, WHITE_COLOR, 0);
    lv_obj_set_style_text_font(lbl_amp_label, &lv_font_montserrat_36, 0);
    lv_obj_center(lbl_amp_label);
    lv_obj_align_to(amp_button, life_label, LV_ALIGN_RIGHT_MID, 141, 0);
    if (amp_mode)
    {
      // Ensure the button is fully transparent and hidden before fade-in
      lv_obj_set_style_opa(amp_button, LV_OPA_TRANSP, 0); // Fully transparent
      lv_obj_clear_flag(amp_button, LV_OBJ_FLAG_HIDDEN);  // Unhide while still transparent
      fade_in_obj(amp_button, 1500, 500, NULL);           // Animate to visible
    }
    else
    {
      lv_obj_add_flag(amp_button, LV_OBJ_FLAG_HIDDEN);
    }
  }
  // Show arc and animate sweep while fading in the life label in parallel
  if (life_arc)
  {
    printf("[show_life_counter] Showing arc\n");
    lv_obj_clear_flag(life_arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_arc_opa(life_arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, NULL);
    lv_anim_set_exec_cb(&anim, arc_sweep_anim_cb);
    lv_anim_set_values(&anim, 0, max_life);
    lv_anim_set_time(&anim, 1500);
    lv_anim_set_delay(&anim, 0);
    lv_anim_set_ready_cb(&anim, arc_sweep_anim_ready_cb);
    printf("[show_life_counter] Starting arc sweep animation\n");
    lv_anim_start(&anim);
  }
  // Fade in the life label at the same time as the arc sweep
  lv_obj_clear_flag(life_label, LV_OBJ_FLAG_HIDDEN);
  fade_in_obj(life_label, 1000, 0, NULL); // Register gesture callbacks for tap and swipe
}

void increment_amp()
{
  amp_value += 1;
  char buf[8];
  snprintf(buf, sizeof(buf), "+%d", amp_value);
  if (amp_button && lbl_amp_label)
  {
    lv_label_set_text(lbl_amp_label, buf);
    // the closer amp gets to peak_amp, the more red it becomes
    uint8_t t = (uint8_t)(((amp_value > peak_amp ? peak_amp : amp_value) * 255) / peak_amp); // Scale t from 0 to 255
    if (t > 255)
      t = 255; // Clamp to max 255
    lv_color_t amp_color = interpolate_color(LIGHTNING_BLUE_COLOR, RED_COLOR, t);
    lv_obj_set_style_bg_color(amp_button, amp_color, 0);
  }
}
void clear_amp()
{
  amp_value = 0; // Reset amp value
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", amp_value);
  if (amp_button && lbl_amp_label)
  {
    lv_label_set_text(lbl_amp_label, buf);
    lv_obj_set_style_bg_color(amp_button, LIGHTNING_BLUE_COLOR, 0);
  }
}

// Increment life total and update label
void increment_life(int value)
{
  queue_life_change(PLAYER_SINGLE, value);
}

// Decrement life total and update label
void decrement_life(int value)
{
  queue_life_change(PLAYER_SINGLE, -value);
}

// Reset life total to 0 and update label
void reset_life()
{
  event_grouper.resetHistory(max_life);
  update_life_label(max_life);
}

void teardown_life_counter()
{
  printf("[tearDownLifeCounter] Clearing life counter objects\n");
  // Reset the event grouper to clear any pending state when showing the life counter
  event_grouper.resetHistory(max_life);
  clear_gesture_callbacks(); // Clear any previous gesture callbacks
  clear_amp();
  // Clean up previous objects if switching modes
  if (life_arc)
  {
    lv_obj_del(life_arc);
    life_arc = nullptr;
  }
  if (life_label)
  {
    lv_obj_del(life_label);
    life_label = nullptr;
  }
  if (grouped_change_label)
  {
    lv_obj_del(grouped_change_label);
    grouped_change_label = nullptr;
  }
  if (amp_button)
  {
    lv_obj_del(amp_button);
    amp_button = nullptr;
  }
}
// Animation callback for arc
static void arc_anim_cb(void *arc_obj, int32_t v)
{
  lv_arc_set_angles((lv_obj_t *)arc_obj, 0, v);
}

static void life_fadein_ready_cb(lv_anim_t *a)
{
  // Start fade-out animation for the label after fade-in
  if (a && a->var)
  {
    fade_out_obj((lv_obj_t *)a->var, 1500, 0, [](lv_anim_t *anim)
                 {
      // Hide the label after fade-out
      if (anim && anim->var) {
        lv_obj_add_flag((lv_obj_t *)anim->var, LV_OBJ_FLAG_HIDDEN);
      }
      init_life_counter(); });
  }
}

// LVGL animation callback to animate the arc sweep from 0 to 40
static void arc_sweep_anim_cb(void *var, int32_t v)
{
  // Always use the persisted max value for arc calculations
  if (v > max_life)
    v = max_life;       // Ensure v does not exceed max_life
  update_life_label(v); // Use the animation value to update the label
}

// Animation ready callback (optional, can be NULL)
static void arc_sweep_anim_ready_cb(lv_anim_t *a)
{
  int step_small = player_store.getInt(KEY_LIFE_STEP_SMALL, 1); // Default to 1
  int step_large = player_store.getInt(KEY_LIFE_STEP_LARGE, 5); // Default to 5

  is_initializing = false;
  register_gesture_callback(GestureType::TapTop, [step_small]()
                            { increment_life(step_small); });
  register_gesture_callback(GestureType::TapBottom, [step_small]()
                            { decrement_life(step_small); });
  register_gesture_callback(GestureType::LongPressTop, [step_large]()
                            { increment_life(step_large); });
  register_gesture_callback(GestureType::LongPressBottom, [step_large]()
                            { decrement_life(step_large); });
  register_gesture_callback(GestureType::SwipeDown, []()
                            { renderMenu(MENU_CONTEXTUAL); });
}

// Function to convert life total to arc segment
static arc_segment_t life_to_arc(int life_total)
{
  arc_segment_t seg = {0};
  int arc_life = life_total;
  if (arc_life < 0)
    arc_life = 0;
  if (max_life <= 0)
    max_life = 40; // fallback
  float circumference = M_PI * SCREEN_DIAMETER;
  float gap_px = 200.0f;
  float gap_deg = (gap_px / circumference) * 360.0f;
  float arc_span = 360.0f - gap_deg;
  float arc_half = arc_span / 2.0f;
  int base_start = (int)(270.0f - arc_half + 0.5f);
  int base_end = (int)(270.0f + arc_half + 0.5f);

  // Color selection (0-10: red, 10-22: red→yellow, 22-35: yellow→green, 35+: green)
  lv_color_t arc_color;
  if (arc_life >= (int)(0.875 * max_life)) // 35/40 = 0.875
  {
    arc_color = GREEN_COLOR;
  }
  else if (arc_life >= (int)(0.55 * max_life)) // 22/40 = 0.55
  {
    // yellow (at 0.55*max) to green (at 0.875*max)
    uint8_t t = (uint8_t)(((arc_life - (int)(0.55 * max_life)) * 255) / ((int)(0.875 * max_life) - (int)(0.55 * max_life)));
    arc_color = interpolate_color(YELLOW_COLOR, GREEN_COLOR, t);
  }
  else if (arc_life >= (int)(0.25 * max_life)) // 10/40 = 0.25
  {
    // red (at 0.25*max) to yellow (at 0.55*max)
    uint8_t t = (uint8_t)(((arc_life - (int)(0.25 * max_life)) * 255) / ((int)(0.55 * max_life) - (int)(0.25 * max_life)));
    arc_color = interpolate_color(RED_COLOR, YELLOW_COLOR, t);
  }
  else
  {
    // 0–0.25*max: solid red
    arc_color = RED_COLOR;
  }

  // Clamp to pure green for life >= 0.875*max
  if (arc_life >= (int)(0.875 * max_life))
  {
    arc_color = GREEN_COLOR;
  }

  if (arc_life > max_life)
  {
    seg.start_angle = base_start;
    seg.end_angle = base_start + (int)arc_span; // Full arc span, not modulo 360
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

  // Normal arc sweep
  int sweep = (int)(arc_span * ((float)arc_life / (float)max_life) + 0.5f);
  int end_angle = (base_start + sweep) % 360;
  seg.start_angle = base_start;
  seg.end_angle = end_angle;
  seg.color = arc_color;
  return seg;
}

// Update the life label and arc based on the current life total
void update_life_label(int new_life_total)
{
  if (life_label != nullptr)
  {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", new_life_total);
    lv_label_set_text(life_label, buf);
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
  // Scale to 8-bit for interpolation
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

// Wrap life change for 2P
void queue_life_change(int player, int value)
{
  if (grouped_change_label != nullptr && !is_initializing)
  {
    int pending_change = event_grouper.getPendingChange() + value;
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

    // Ensure the label is visible immediately
    lv_obj_clear_flag(grouped_change_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_opa(grouped_change_label, LV_OPA_COVER, 0);
    fade_out_obj(grouped_change_label, 100, GROUPER_WINDOW, [](lv_anim_t *fade_out_anim)
                 {
      // Hide the label after fade-out
      if (fade_out_anim && fade_out_anim->var) {
        lv_obj_add_flag((lv_obj_t *)fade_out_anim->var, LV_OBJ_FLAG_HIDDEN);
      } });
    event_grouper.handleChange(player, value, [](const LifeHistoryEvent &evt)
                               {
      printf("[queue_life_change] Player %d life change committed: %d\n", evt.player_id, evt.life_total);
      // Double-check label and arc are valid before updating
      update_life_label(evt.life_total); });
  }
  else if (grouped_change_label == nullptr && !is_initializing)
  {
    printf("[queue_life_change] grouped_change_label is NULL!\n");
  }
}
