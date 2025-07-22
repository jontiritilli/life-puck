#include <state/state_store.h>
#include "Arduino.h"
#include <helpers/animation_helpers.h>
#include <lvgl.h>
#include <math.h>
#include <stdio.h>
#include <gestures/gestures.h>
#include "life_counter2P.h"
#include <helpers/event_grouper.h>
// --- Arc Segment Definition ---
typedef struct
{
  int start_angle;
  int end_angle;
  lv_color_t color;
} arc_segment_t;

// --- Two Player Life Counter GUI State ---
static lv_obj_t *life_arc_p1 = nullptr;
static lv_obj_t *life_arc_p2 = nullptr;
static lv_obj_t *life_label_p1 = nullptr;
static lv_obj_t *life_label_p2 = nullptr;
int life_total_p1 = 0;
int life_total_p2 = 0;
static int max_life = LIFE_STD_START;

// --- Forward Declarations ---
void update_life_label_p1(int value);
void update_life_label_p2(int value);
static void arc_sweep_anim_cb_p1(void *var, int32_t value);
static void arc_sweep_anim_cb_p2(void *var, int32_t value);
static void arc_sweep_anim_ready_cb(lv_anim_t *a);
static void life_counter_gesture_event_handler(lv_event_t *e);
static lv_color_t interpolate_color(lv_color_t c1, lv_color_t c2, uint8_t t);
void increment_life_p1(int value);
void decrement_life_p1(int value);
void reset_life_p1();
void increment_life_p2(int value);
void decrement_life_p2(int value);
void reset_life_p2();
static bool is_left_half(int x);
void queue_life_change_2p(int player, int value);

// Suppress tap after gesture
static bool gesture_active = false;

// Event grouping for 2P mode
EventGrouper event_grouper_2p(1000); // 1s window for 2P

// Define grouped_change_label and is_initializing for 2P context
static lv_obj_t *grouped_change_label_p1 = nullptr;
static lv_obj_t *grouped_change_label_p2 = nullptr;
static bool is_initializing_2p = false;

// Call this after boot animation to show the two-player life counter
void init_life_counter_2P()
{
  // Clean up previous objects before creating new ones
  if (life_arc_p1)
  {
    lv_obj_del(life_arc_p1);
    life_arc_p1 = nullptr;
  }
  if (life_arc_p2)
  {
    lv_obj_del(life_arc_p2);
    life_arc_p2 = nullptr;
  }
  if (life_label_p1)
  {
    lv_obj_del(life_label_p1);
    life_label_p1 = nullptr;
  }
  if (life_label_p2)
  {
    lv_obj_del(life_label_p2);
    life_label_p2 = nullptr;
  }
  // Optionally clean up center line if needed
  static lv_obj_t *center_line = nullptr;
  if (center_line)
  {
    lv_obj_del(center_line);
    center_line = nullptr;
  }
  // Add a thin yellow vertical line at the center of the screen
  static lv_point_precise_t line_points[2];
  line_points[0].x = SCREEN_WIDTH / 2;
  line_points[0].y = 0 + 20;
  line_points[1].x = SCREEN_WIDTH / 2;
  line_points[1].y = SCREEN_HEIGHT - 20;
  center_line = lv_line_create(lv_scr_act());
  lv_line_set_points(center_line, line_points, 2);
  lv_obj_set_style_line_color(center_line, WHITE_COLOR, 0);
  lv_obj_set_style_line_width(center_line, 1, 0); // Very Thin line
  lv_obj_set_style_line_opa(center_line, LV_OPA_COVER, 0);
  lv_obj_set_style_line_rounded(center_line, 1, 0);
  max_life = player_store.getLife(LIFE_STD_START);

  // Create arc/label for Player 1 (sweep left: 90° to 0°, centered)
  life_arc_p1 = lv_arc_create(lv_scr_act());
  lv_obj_add_flag(life_arc_p1, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(life_arc_p1, ARC_OUTER_DIAMETER, ARC_OUTER_DIAMETER);
  lv_obj_align(life_arc_p1, LV_ALIGN_CENTER, 0, 0);
  lv_arc_set_bg_angles(life_arc_p1, 0, 360); // left sweep background
  lv_arc_set_angles(life_arc_p1, 90, 270);   // indicator starts at bottom center
  lv_obj_set_style_arc_color(life_arc_p1, GREEN_COLOR, LV_PART_INDICATOR);
  lv_obj_set_style_arc_opa(life_arc_p1, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_arc_width(life_arc_p1, 0, LV_PART_MAIN);
  lv_obj_set_style_arc_width(life_arc_p1, ARC_WIDTH_2P, LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(life_arc_p1, 0, LV_PART_INDICATOR);
  lv_obj_remove_style(life_arc_p1, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(life_arc_p1, LV_OBJ_FLAG_CLICKABLE);

  life_label_p1 = lv_label_create(lv_scr_act());
  lv_obj_add_flag(life_label_p1, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(life_label_p1, "0");
  lv_obj_set_style_text_font(life_label_p1, &lv_font_montserrat_64, 0);
  lv_obj_set_style_text_color(life_label_p1, lv_color_white(), 0);
  lv_obj_align(life_label_p1, LV_ALIGN_CENTER, -ARC_OUTER_DIAMETER / 4, 0);
  lv_obj_set_style_text_opa(life_label_p1, LV_OPA_TRANSP, 0);

  // Create arc/label for Player 2 (sweep right: 90° to 180°, centered)
  life_arc_p2 = lv_arc_create(lv_scr_act());
  lv_obj_add_flag(life_arc_p2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(life_arc_p2, ARC_OUTER_DIAMETER, ARC_OUTER_DIAMETER);
  lv_obj_align(life_arc_p2, LV_ALIGN_CENTER, 0, 0);
  lv_arc_set_bg_angles(life_arc_p2, 0, 360);         // right sweep background
  lv_arc_set_angles(life_arc_p2, 270, 90);           // indicator starts at top center, grows counterclockwise
  lv_arc_set_mode(life_arc_p2, LV_ARC_MODE_REVERSE); // Enable reverse mode for counterclockwise sweep
  lv_obj_set_style_arc_color(life_arc_p2, GREEN_COLOR, LV_PART_INDICATOR);
  lv_obj_set_style_arc_opa(life_arc_p2, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_arc_width(life_arc_p2, 0, LV_PART_MAIN);
  lv_obj_set_style_arc_width(life_arc_p2, ARC_WIDTH_2P, LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(life_arc_p2, 0, LV_PART_INDICATOR);
  lv_obj_remove_style(life_arc_p2, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(life_arc_p2, LV_OBJ_FLAG_CLICKABLE);

  life_label_p2 = lv_label_create(lv_scr_act());
  lv_obj_add_flag(life_label_p2, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(life_label_p2, "0");
  lv_obj_set_style_text_font(life_label_p2, &lv_font_montserrat_64, 0);
  lv_obj_set_style_text_color(life_label_p2, lv_color_white(), 0);
  lv_obj_align(life_label_p2, LV_ALIGN_CENTER, ARC_OUTER_DIAMETER / 4, 0);

  // Add grouped change labels for Player 1 and Player 2
  if (!grouped_change_label_p1)
  {
    grouped_change_label_p1 = lv_label_create(lv_scr_act());
    lv_obj_add_flag(grouped_change_label_p1, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(grouped_change_label_p1, "0");
    lv_obj_set_style_text_font(grouped_change_label_p1, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(grouped_change_label_p1, lv_color_white(), 0);
    lv_obj_align_to(grouped_change_label_p1, life_label_p1, LV_ALIGN_OUT_TOP_MID, 0, -10);
  }

  if (!grouped_change_label_p2)
  {
    grouped_change_label_p2 = lv_label_create(lv_scr_act());
    lv_obj_add_flag(grouped_change_label_p2, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(grouped_change_label_p2, "0");
    lv_obj_set_style_text_font(grouped_change_label_p2, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(grouped_change_label_p2, lv_color_white(), 0);
    lv_obj_align_to(grouped_change_label_p2, life_label_p2, LV_ALIGN_OUT_TOP_MID, 0, -10);
  }

  // Show arcs and animate sweep while fading in the life labels in parallel
  lv_obj_clear_flag(life_arc_p1, LV_OBJ_FLAG_HIDDEN);
  update_life_label_p1(max_life);
  lv_obj_set_style_arc_opa(life_arc_p1, LV_OPA_COVER, LV_PART_INDICATOR);
  lv_anim_t anim1;
  lv_anim_init(&anim1);
  lv_anim_set_var(&anim1, NULL);
  lv_anim_set_exec_cb(&anim1, arc_sweep_anim_cb_p1);
  lv_anim_set_values(&anim1, 0, max_life);
  lv_anim_set_time(&anim1, 2000);
  lv_anim_set_delay(&anim1, 0);
  lv_anim_set_ready_cb(&anim1, arc_sweep_anim_ready_cb);
  lv_anim_start(&anim1);

  lv_obj_clear_flag(life_arc_p2, LV_OBJ_FLAG_HIDDEN);
  update_life_label_p2(max_life);
  lv_obj_set_style_arc_opa(life_arc_p2, LV_OPA_COVER, LV_PART_INDICATOR);
  lv_anim_t anim2;
  lv_anim_init(&anim2);
  lv_anim_set_var(&anim2, NULL);
  lv_anim_set_exec_cb(&anim2, arc_sweep_anim_cb_p2);
  lv_anim_set_values(&anim2, 0, max_life);
  lv_anim_set_time(&anim2, 2000);
  lv_anim_set_delay(&anim2, 0);
  lv_anim_set_ready_cb(&anim2, arc_sweep_anim_ready_cb);
  lv_anim_start(&anim2);

  // Fade in the life labels
  lv_obj_clear_flag(life_label_p1, LV_OBJ_FLAG_HIDDEN);
  fade_in_obj(life_label_p1, 1000, 0, NULL);
  lv_obj_clear_flag(life_label_p2, LV_OBJ_FLAG_HIDDEN);
  fade_in_obj(life_label_p2, 1000, 0, NULL);

  // Register gesture event handler for the screen
  lv_obj_add_event_cb(lv_scr_act(), life_counter_gesture_event_handler, LV_EVENT_GESTURE, NULL);
  // Register tap (clicked) event handler for the screen
  lv_obj_add_event_cb(lv_scr_act(), [](lv_event_t *e)
                      {
    if (gesture_active) {
      gesture_active = false; // Reset for next event
      return;
    }
    lv_indev_t *indev = lv_indev_get_act();
    if (!indev)
      return;
    lv_point_t p;
    lv_indev_get_point(indev, &p);
    int x = p.x;
    int y = p.y;
    bool is_left = is_left_half(x);
    bool is_top = y < (SCREEN_HEIGHT / 2);
    if (is_left) {
      if (is_top)
        increment_life_p1(1);
      else
        decrement_life_p1(1);
    } else {
      if (is_top)
        increment_life_p2(1);
      else
        decrement_life_p2(1);
    } }, LV_EVENT_CLICKED, NULL);
}

// Gesture event handler for two-player mode
static void life_counter_gesture_event_handler(lv_event_t *e)
{
  lv_indev_t *indev = lv_indev_get_act();
  if (!indev)
    return;
  lv_point_t p;
  lv_indev_get_point(indev, &p);
  int x = p.x;
  int y = p.y;
  // Detect left or right half (no center exclusion)
  bool is_left = is_left_half(x);
  // Detect gesture type
  lv_dir_t gesture = lv_indev_get_gesture_dir(indev);
  gesture_active = true;
  switch (gesture)
  {
  case LV_DIR_TOP:
    if (is_left)
      increment_life_p1(5);
    else
      increment_life_p2(5);
    break;
  case LV_DIR_BOTTOM:
    if (is_left)
      decrement_life_p1(5);
    else
      decrement_life_p2(5);
    break;
  case LV_DIR_NONE:
  default:
    // Treat as tap
    if (is_left)
      increment_life_p1(1);
    else
      increment_life_p2(1);
    break;
  }
}

// Increment life total and update label for Player 1
void increment_life_p1(int value)
{
  queue_life_change_2p(1, value);
}

// Decrement life total and update label for Player 1
void decrement_life_p1(int value)
{
  queue_life_change_2p(1, -value);
}

// Reset life total for Player 1
void reset_life_p1()
{
  int start_life_conf = player_store.getLife(LIFE_STD_START);
  int life_offset = start_life_conf - life_total_p1;
  update_life_label_p1(life_offset);
}

// Increment life total and update label for Player 2
void increment_life_p2(int value)
{
  queue_life_change_2p(2, value);
}

// Decrement life total and update label for Player 2
void decrement_life_p2(int value)
{
  queue_life_change_2p(2, -value);
}

// Reset life total for Player 2
void reset_life_p2()
{
  int start_life_conf = player_store.getLife(LIFE_STD_START);
  int life_offset = start_life_conf - life_total_p2;
  update_life_label_p2(life_offset);
}

// Animation callback for arc (Player 1)
static void arc_sweep_anim_cb_p1(void *var, int32_t v)
{
  if (v > max_life)
    v = max_life;
  // Sweep left: 90° to 0° (Player 1)
  int sweep = (int)(90.0f * ((float)v / (float)max_life) + 0.5f);
  int end_angle = 90 - sweep;
  if (end_angle < 0)
    end_angle = 0;
  lv_arc_set_angles(life_arc_p1, 90, end_angle);
  update_life_label_p1(v - life_total_p1);
}

// Animation callback for arc (Player 2)
static void arc_sweep_anim_cb_p2(void *var, int32_t v)
{
  if (v > max_life)
    v = max_life;
  // For right half: use start_angle=270, end_angle=90 (reverse mode)
  lv_arc_set_angles(life_arc_p2, 270, 90);
  lv_arc_set_value(life_arc_p2, v);
  update_life_label_p2(v - life_total_p2);
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
      init_life_counter_2P(); });
  }
}

// Animation ready callback (optional, can be NULL)
static void arc_sweep_anim_ready_cb(lv_anim_t *a) {}

// Function to convert life total to arc segment
// Player 1: 210° to 90° (counterclockwise)
// Player 1: arc grows clockwise from 90° to 270°
static arc_segment_t life_to_arc_p1(int life_total)
{
  arc_segment_t seg = {0};
  int arc_life = life_total;
  // use cached max_life
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
  int start_angle = 90;
  int sweep = (int)(180.0f * ((float)arc_life / (float)max_life) + 0.5f);
  int arc_end = start_angle + sweep;
  if (arc_end > 360)
    arc_end -= 360;
  seg.start_angle = start_angle;
  seg.end_angle = arc_end;
  seg.color = arc_color;
  return seg;
}

// Player 2: arc grows counterclockwise from 270° to 90° (reverse mode)
static arc_segment_t life_to_arc_p2(int life_total)
{
  arc_segment_t seg = {0};
  int arc_life = life_total;
  // use cached max_life
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

  seg.end_angle = 90;
  if (arc_life <= 0)
  {
    seg.start_angle = 90;
  }
  else if (arc_life >= max_life)
  {
    seg.start_angle = 270;
  }
  else
  {
    // if the life value is something else, calculate the angle to be between 0-90 or 270-360
    // 0% life = 90° (bottom center), 100% life = 270° (top center)
    // Calculate the angle based on the life percentage
    // 0% life = 90° (bottom center), 100% life = 270° (top center)
    // 50% life = 360° (right center)
    // 25% life = 315° (bottom right), 75% life = 290° (top right)
    float percent = (float)arc_life / (float)max_life; // 0.0 to 1.0
    int arc_span = (int)(percent * 180.0f + 0.5f);     // 0 to 180°

    seg.start_angle = (seg.end_angle - arc_span + 360) % 360; // Always positive
  }
  seg.color = arc_color;
  printf("life_p2: %d / %d => start_angle: %d, end_angle: %d\n", arc_life, max_life, seg.start_angle, seg.end_angle);
  return seg;
}

// Update the life label and arc for Player 1
void update_life_label_p1(int grouped_change)
{
  static int last_life_total_p1 = 0;
  life_total_p1 = life_total_p1 + grouped_change;
  if (life_label_p1)
  {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", life_total_p1);
    // Only update text if it changed
    if (life_total_p1 != last_life_total_p1 || strcmp(lv_label_get_text(life_label_p1), buf) != 0)
    {
      lv_label_set_text(life_label_p1, buf);
      last_life_total_p1 = life_total_p1;
    }
  }

  // Update arc to reflect life total
  if (life_arc_p1)
  {
    arc_segment_t seg = life_to_arc_p1(life_total_p1);
    uint16_t c16 = lv_color_to_u16(seg.color);
    uint8_t r = (c16 >> 11) & 0x1F;
    uint8_t g = (c16 >> 5) & 0x3F;
    uint8_t b = c16 & 0x1F;
    // Scale to 8-bit for debug
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    printf("[update_life_label_p1] life_total_p1=%d, arc: start=%d end=%d color=(%d,%d,%d)\n",
           life_total_p1, seg.start_angle, seg.end_angle, r, g, b);
    lv_arc_set_angles(life_arc_p1, seg.start_angle, seg.end_angle);
    lv_obj_set_style_arc_color(life_arc_p1, seg.color, LV_PART_INDICATOR);
  }
}

// Update the life label and arc for Player 2
void update_life_label_p2(int grouped_change)
{
  static int last_life_total_p2 = 0;
  life_total_p2 = life_total_p2 + grouped_change;
  if (life_label_p2)
  {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", life_total_p2);
    // Only update text if it changed
    if (life_total_p2 != last_life_total_p2 || strcmp(lv_label_get_text(life_label_p2), buf) != 0)
    {
      lv_label_set_text(life_label_p2, buf);
      last_life_total_p2 = life_total_p2;
    }
  }

  // Update arc to reflect life total
  if (life_arc_p2)
  {
    arc_segment_t seg = life_to_arc_p2(life_total_p2);
    uint16_t c16 = lv_color_to_u16(seg.color);
    uint8_t r = (c16 >> 11) & 0x1F;
    uint8_t g = (c16 >> 5) & 0x3F;
    uint8_t b = c16 & 0x1F;
    // Scale to 8-bit for debug
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    printf("[update_life_label_p2] life_total_p2=%d, arc: start=%d end=%d color=(%d,%d,%d)\n",
           life_total_p2, seg.start_angle, seg.end_angle, r, g, b);
    lv_arc_set_angles(life_arc_p2, seg.start_angle, seg.end_angle);
    lv_obj_set_style_arc_color(life_arc_p2, seg.color, LV_PART_INDICATOR);
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

// Helper: returns true if x is in the left half of the screen
static bool is_left_half(int x)
{
  return x < (SCREEN_WIDTH / 2);
}

// --- Event Handling for 2P Mode ---
void life_counter2p_loop()
{
  if (event_grouper_2p.isCommitPending())
  {
    event_grouper_2p.update();
  }
}

// Wrap life change for 2P
void queue_life_change_2p(int player, int value)
{
  lv_obj_t *grouped_change_label = (player == 1) ? grouped_change_label_p1 : grouped_change_label_p2;
  if (grouped_change_label && !is_initializing_2p)
  {
    int pending_change = event_grouper_2p.getPendingChange() + value;
    if (pending_change != 0) // Only proceed if pending_change is not 0
    {
      char buf[8];
      snprintf(buf, sizeof(buf), "%d", pending_change);
      lv_obj_set_style_text_color(grouped_change_label, pending_change >= 0 ? GREEN_COLOR : RED_COLOR, 0);
      lv_label_set_text(grouped_change_label, buf);

      // Ensure the label is visible immediately
      lv_obj_clear_flag(grouped_change_label, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_text_opa(grouped_change_label, LV_OPA_COVER, 0);

      // Trigger fade-in animation immediately
      fade_in_obj(grouped_change_label, 100, 0, [](lv_anim_t *anim)
                  {
        // After fade-in, start fade-out after 500ms
        fade_out_obj((lv_obj_t *)anim->var, 500, 500, [](lv_anim_t *fade_out_anim) {
          // Hide the label after fade-out
          if (fade_out_anim && fade_out_anim->var) {
            lv_obj_add_flag((lv_obj_t *)fade_out_anim->var, LV_OBJ_FLAG_HIDDEN);
          }
        }); });
    }
  }
  event_grouper_2p.handleChange(player, value, [player](const LifeHistoryEvent &evt)
                                {
                                  if (player == 1)
                                    update_life_label_p1(evt.net_life_change);
                                  else
                                    update_life_label_p2(evt.net_life_change); });
}