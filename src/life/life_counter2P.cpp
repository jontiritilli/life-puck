#include "Arduino.h"
#include <lvgl.h>
#include <math.h>
#include <stdio.h>
#include "life_counter2P.h"
#include <gestures/gestures.h>
#include <helpers/animation_helpers.h>
#include <state/state_store.h>
#include <helpers/event_grouper.h>
#include <menu/menu.h>
#include "constants/constants.h"

// --- Two Player Life Counter GUI State ---
lv_obj_t *life_arc_p1 = nullptr; // Now global for menu access
lv_obj_t *life_arc_p2 = nullptr; // Now global for menu access
static lv_obj_t *life_label_p1 = nullptr;
static lv_obj_t *life_label_p2 = nullptr;
static lv_obj_t *center_line = nullptr;

static int max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);

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

// Suppress tap after gesture
static bool gesture_active = false;

// Event grouping for 2P mode
EventGrouper event_grouper_p1(GROUPER_WINDOW, max_life, PLAYER_ONE);
EventGrouper event_grouper_p2(GROUPER_WINDOW, max_life, PLAYER_TWO);

// Define grouped_change_label and is_initializing for 2P context
static lv_obj_t *grouped_change_label_p1 = nullptr;
static lv_obj_t *grouped_change_label_p2 = nullptr;
static bool is_initializing_2p = false;

// Call this after boot animation to show the two-player life counter
void init_life_counter_2P()
{
  is_initializing_2p = true;  // Set flag to indicate initialization is active
  teardown_life_counter_2P(); // Clean up any previous state

  if (!center_line)
  {
    printf("[init_life_counter_2P] Creating center_line\n");
    // Add a thin yellow vertical line at the center of the screen
    static lv_point_precise_t line_points[2];
    line_points[0].x = SCREEN_WIDTH / 2;
    line_points[0].y = 0 + 60;
    line_points[1].x = SCREEN_WIDTH / 2;
    line_points[1].y = SCREEN_HEIGHT - 60;
    center_line = lv_line_create(lv_scr_act());
    lv_line_set_points(center_line, line_points, 2);
    lv_obj_set_style_line_color(center_line, WHITE_COLOR, 0);
    lv_obj_set_style_line_width(center_line, 1, 0); // Very Thin line
    lv_obj_set_style_line_opa(center_line, LV_OPA_COVER, 0);
    lv_obj_set_style_line_rounded(center_line, 1, 0);
  }
  if (!life_arc_p1)
  {
    printf("[init_life_counter_2P] Creating life_arc_p1\n");
    // Create arc/label for Player 1 (sweep left: 90° to 0°, centered)
    life_arc_p1 = lv_arc_create(lv_scr_act());
    lv_obj_add_flag(life_arc_p1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(life_arc_p1, SCREEN_DIAMETER, SCREEN_DIAMETER);
    lv_obj_align(life_arc_p1, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(life_arc_p1, 0, 360); // left sweep background
    lv_arc_set_angles(life_arc_p1, 90, 270);   // indicator starts at bottom center
    lv_obj_set_style_arc_color(life_arc_p1, GREEN_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(life_arc_p1, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc_p1, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc_p1, ARC_WIDTH, LV_PART_INDICATOR);
    // lv_obj_set_style_arc_rounded(life_arc_p1, 0, LV_PART_INDICATOR);
    lv_obj_remove_style(life_arc_p1, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(life_arc_p1, LV_OBJ_FLAG_CLICKABLE);
  }
  if (!life_label_p1)
  {
    printf("[init_life_counter_2P] Creating life_label_p1\n");
    // Create label for Player 1
    life_label_p1 = lv_label_create(lv_scr_act());
    lv_obj_add_flag(life_label_p1, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(life_label_p1, "0");
    lv_obj_set_style_text_font(life_label_p1, &lv_font_montserrat_72, 0);
    lv_obj_set_style_text_color(life_label_p1, lv_color_white(), 0);
    lv_obj_align(life_label_p1, LV_ALIGN_CENTER, -SCREEN_DIAMETER / 4, 0);
    lv_obj_set_style_text_opa(life_label_p1, LV_OPA_TRANSP, 0);
  }
  if (!life_arc_p2)
  {
    printf("[init_life_counter_2P] Creating life_arc_p2\n");
    // Create arc/label for Player 2 (sweep right: 90° to 180°, centered)
    life_arc_p2 = lv_arc_create(lv_scr_act());
    lv_obj_add_flag(life_arc_p2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(life_arc_p2, SCREEN_DIAMETER, SCREEN_DIAMETER);
    lv_obj_align(life_arc_p2, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(life_arc_p2, 0, 360);         // right sweep background
    lv_arc_set_angles(life_arc_p2, 270, 90);           // indicator starts at top center, grows counterclockwise
    lv_arc_set_mode(life_arc_p2, LV_ARC_MODE_REVERSE); // Enable reverse mode for counterclockwise sweep
    lv_obj_set_style_arc_color(life_arc_p2, GREEN_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(life_arc_p2, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc_p2, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc_p2, ARC_WIDTH, LV_PART_INDICATOR);
    // lv_obj_set_style_arc_rounded(life_arc_p2, 0, LV_PART_INDICATOR); // Square ends
    lv_obj_remove_style(life_arc_p2, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(life_arc_p2, LV_OBJ_FLAG_CLICKABLE);
  }
  if (!life_label_p2)
  {
    printf("[init_life_counter_2P] Creating life_label_p2\n");
    life_label_p2 = lv_label_create(lv_scr_act());
    lv_obj_add_flag(life_label_p2, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(life_label_p2, "0");
    lv_obj_set_style_text_font(life_label_p2, &lv_font_montserrat_72, 0);
    lv_obj_set_style_text_color(life_label_p2, lv_color_white(), 0);
    lv_obj_align(life_label_p2, LV_ALIGN_CENTER, SCREEN_DIAMETER / 4, 0);
  }
  // Add grouped change labels for Player 1 and Player 2

  if (!grouped_change_label_p1 && life_label_p1)
  {
    printf("[init_life_counter_2P] Creating grouped_change_label_p1\n");
    grouped_change_label_p1 = lv_label_create(lv_scr_act());
    lv_obj_add_flag(grouped_change_label_p1, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(grouped_change_label_p1, "0");
    lv_obj_set_style_text_font(grouped_change_label_p1, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(grouped_change_label_p1, lv_color_white(), 0);
    lv_obj_align_to(grouped_change_label_p1, life_label_p1, LV_ALIGN_OUT_TOP_MID, -10, -10);
  }

  if (!grouped_change_label_p2 && life_label_p2)
  {
    printf("[init_life_counter_2P] Creating grouped_change_label_p2\n");
    grouped_change_label_p2 = lv_label_create(lv_scr_act());
    lv_obj_add_flag(grouped_change_label_p2, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(grouped_change_label_p2, "0");
    lv_obj_set_style_text_font(grouped_change_label_p2, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(grouped_change_label_p2, lv_color_white(), 0);
    lv_obj_align_to(grouped_change_label_p2, life_label_p2, LV_ALIGN_OUT_TOP_MID, -10, -10);
  }

  if (life_arc_p1)
  {
    printf("[init_life_counter_2P] Animating life_arc_p1\n");
    // Show Player 1 arc and animate sweep
    // Show arcs and animate sweep while fading in the life labels in parallel
    lv_obj_clear_flag(life_arc_p1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_arc_opa(life_arc_p1, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_anim_t anim1;
    lv_anim_init(&anim1);
    lv_anim_set_var(&anim1, NULL);
    lv_anim_set_exec_cb(&anim1, arc_sweep_anim_cb_p1);
    lv_anim_set_values(&anim1, 0, max_life);
    lv_anim_set_time(&anim1, 1500);
    lv_anim_set_delay(&anim1, 0);
    lv_anim_set_ready_cb(&anim1, arc_sweep_anim_ready_cb);
    lv_anim_start(&anim1);
  }

  if (life_arc_p2)
  {
    printf("[init_life_counter_2P] Animating life_arc_p2\n");
    // Show Player 2 arc and animate sweep
    lv_obj_clear_flag(life_arc_p2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_arc_opa(life_arc_p2, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_anim_t anim2;
    lv_anim_init(&anim2);
    lv_anim_set_var(&anim2, NULL);
    lv_anim_set_exec_cb(&anim2, arc_sweep_anim_cb_p2);
    lv_anim_set_values(&anim2, 0, max_life);
    lv_anim_set_time(&anim2, 1500);
    lv_anim_set_delay(&anim2, 0);
    lv_anim_set_ready_cb(&anim2, arc_sweep_anim_ready_cb);
    lv_anim_start(&anim2);
  }
  // Fade in the life labels
  lv_obj_clear_flag(life_label_p1, LV_OBJ_FLAG_HIDDEN);
  fade_in_obj(life_label_p1, 1000, 0, NULL);
  lv_obj_clear_flag(life_label_p2, LV_OBJ_FLAG_HIDDEN);
  fade_in_obj(life_label_p2, 1000, 0, NULL);
}

// Increment life total and update label
void increment_life(int player, int value)
{
  queue_life_change_2p(player, value);
}

// Decrement life total and update label
void decrement_life(int player, int value)
{
  queue_life_change_2p(player, -value);
}

void teardown_life_counter_2P()
{
  printf("[tearDownLifeCounter2P] Clearing life counter objects\n");
  event_grouper_p1.resetHistory(max_life);
  event_grouper_p2.resetHistory(max_life);
  clear_gesture_callbacks(); // Clear any previous gesture callbacks
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
  if (grouped_change_label_p1)
  {
    lv_obj_del(grouped_change_label_p1);
    grouped_change_label_p1 = nullptr;
  }
  if (grouped_change_label_p2)
  {
    lv_obj_del(grouped_change_label_p2);
    grouped_change_label_p2 = nullptr;
  }
  if (center_line)
  {
    lv_obj_del(center_line);
    center_line = nullptr;
  }
}

// Reset life total for Player 1
void reset_life_p1()
{
  event_grouper_p1.resetHistory(max_life);
  update_life_label(1, max_life);
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
  event_grouper_p2.resetHistory(max_life);
  update_life_label(2, max_life);
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
  update_life_label(1, v);
}

// Animation callback for arc (Player 2)
static void arc_sweep_anim_cb_p2(void *var, int32_t v)
{
  if (v > max_life)
    v = max_life;
  // For right half: use start_angle=270, end_angle=90 (reverse mode)
  lv_arc_set_angles(life_arc_p2, 270, 90);
  lv_arc_set_value(life_arc_p2, v);
  update_life_label(2, v);
}

// Animation ready callback (optional, can be NULL)
static void arc_sweep_anim_ready_cb(lv_anim_t *a)
{
  is_initializing_2p = false;
  // Register gesture callbacks for tap and swipe, consistent with 1P mode
  register_gesture_callback(GestureType::TapTopLeft, []()
                            { increment_life(PLAYER_ONE, 1); });
  register_gesture_callback(GestureType::TapBottomLeft, []()
                            { decrement_life(PLAYER_ONE, 1); });
  register_gesture_callback(GestureType::SwipeUpLeft, []()
                            { increment_life(PLAYER_ONE, 5); });
  register_gesture_callback(GestureType::SwipeDownLeft, []()
                            { decrement_life(PLAYER_ONE, 5); });
  register_gesture_callback(GestureType::TapTopRight, []()
                            { increment_life(PLAYER_TWO, 1); });
  register_gesture_callback(GestureType::TapBottomRight, []()
                            { decrement_life(PLAYER_TWO, 1); });
  register_gesture_callback(GestureType::SwipeUpRight, []()
                            { increment_life(PLAYER_TWO, 5); });
  register_gesture_callback(GestureType::SwipeDownRight, []()
                            { decrement_life(PLAYER_TWO, 5); });
  register_gesture_callback(GestureType::LongPressMenu, []()
                            { renderMenu(MENU_CONTEXTUAL); });
}

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
  return seg;
}

// Update the life label and arc for Player 1
void update_life_label(int player, int new_life_total)
{
  lv_obj_t *life_label = (player == 1) ? life_label_p1 : life_label_p2;
  lv_obj_t *life_arc = (player == 1) ? life_arc_p1 : life_arc_p2;

  if (life_label != nullptr)
  {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", new_life_total);
    lv_label_set_text(life_label, buf);
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
    // Show the pending change BEFORE the grouper updates its state
    int pending_change = grouper->getPendingChange() + value;
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
  else if (grouped_change_label == nullptr && !is_initializing_2p)
  {
    printf("[queue_life_change_2p] grouped_change_label is NULL for player %d!\n", player);
  }
  grouper->handleChange(player, value, [player](const LifeHistoryEvent &evt)
                        {
    printf("[queue_life_change_2p] Player %d life change committed: %d\n", player, evt.life_total);
    // Hide grouped change label after commit
    lv_obj_t *grouped_change_label = (player == 1) ? grouped_change_label_p1 : grouped_change_label_p2;
    if (is_initializing_2p) {
      printf("[queue_life_change_2p] Skipping update_life_label during initialization.\n");
      return;
    }
    if (player == 1)
      update_life_label(1, evt.life_total);
    else
      update_life_label(2, evt.life_total); });
}