#include "Arduino.h"
#include <lvgl.h>
#include <stdio.h>
#include "gui_components.h"
#include "math.h"
#define SCREEN_WIDTH 360
#define SCREEN_HEIGHT 360

#define ARC_OUTER_DIAMETER 360 // 1px margin all around for 360x360 screen
#define ARC_INNER_DIAMETER 340
#define ARC_WIDTH (ARC_OUTER_DIAMETER - ARC_INNER_DIAMETER)
#define LIFE_STD_START 40

#define GREEN_COLOR lv_color_hex(0x00FF00)  // RGB for green
#define YELLOW_COLOR lv_color_hex(0xFFFF00) // RGB for yellow
#define RED_COLOR lv_color_hex(0xFF0000)    // RGB for red

typedef struct
{
  int start_angle;
  int end_angle;
  lv_color_t color;
} arc_segment_t;

static lv_obj_t *active_screen;
static lv_obj_t *life_label = NULL;
static lv_obj_t *life_arc = NULL;
static lv_obj_t *tap_layer = NULL;
int life_total = 0;

// Animation callback for label fade-in
static void text_fade_anim_cb(void *label_obj, int32_t opa)
{
  lv_obj_set_style_text_opa((lv_obj_t *)label_obj, opa, 0);
}

// Animation callback for arc
static void arc_anim_cb(void *arc_obj, int32_t v)
{
  lv_arc_set_angles((lv_obj_t *)arc_obj, 0, v);
}

// Called when the fade-in animation finishes
// Helper: fade in a label or arc
static void fade_in_obj(lv_obj_t *obj, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb = NULL)
{
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, obj);
  printf("[fade_in_obj] Fading in obj=%p, duration=%u, delay=%u\n", obj, duration, delay);
  if (lv_obj_check_type(obj, &lv_label_class))
  {
    lv_anim_set_exec_cb(&anim, text_fade_anim_cb);
    lv_obj_set_style_text_opa(obj, LV_OPA_TRANSP, 0);
  }
  else
  {
    lv_anim_set_exec_cb(&anim, [](void *arc_obj, int32_t opa)
                        { lv_obj_set_style_arc_opa((lv_obj_t *)arc_obj, opa, LV_PART_INDICATOR); });
    lv_obj_set_style_arc_opa(obj, LV_OPA_TRANSP, LV_PART_INDICATOR);
  }
  lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
  lv_anim_set_time(&anim, duration);
  lv_anim_set_delay(&anim, delay);
  if (ready_cb)
    lv_anim_set_ready_cb(&anim, ready_cb);
  lv_anim_start(&anim);
}

// Helper: fade out a label or arc
static void fade_out_obj(lv_obj_t *obj, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb = NULL)
{
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, obj);
  printf("[fade_out_obj] Fading out obj=%p, duration=%u, delay=%u\n", obj, duration, delay);
  if (lv_obj_check_type(obj, &lv_label_class))
  {
    lv_anim_set_exec_cb(&anim, text_fade_anim_cb);
    lv_obj_set_style_text_opa(obj, LV_OPA_COVER, 0);
  }
  else
  {
    lv_anim_set_exec_cb(&anim, [](void *arc_obj, int32_t opa)
                        { lv_obj_set_style_arc_opa((lv_obj_t *)arc_obj, opa, LV_PART_INDICATOR); });
    lv_obj_set_style_arc_opa(obj, LV_OPA_COVER, LV_PART_INDICATOR);
  }
  lv_anim_set_values(&anim, LV_OPA_COVER, LV_OPA_TRANSP);
  lv_anim_set_time(&anim, duration);
  lv_anim_set_delay(&anim, delay);
  if (ready_cb)
    lv_anim_set_ready_cb(&anim, ready_cb);
  lv_anim_start(&anim);
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
      show_life_counter(); });
  }
}

// Event callback for tap gestures (right = +1, left = -1)
void update_life_label(int value);

// Overlay fade-out animation callback
static void overlay_fade_cb(void *obj, int32_t opa)
{
  lv_obj_set_style_bg_opa((lv_obj_t *)obj, opa, 0);
}

// LVGL animation callback to animate the arc sweep from 0 to 40
static void arc_sweep_anim_cb(void *var, int32_t v)
{
  printf("[arc_sweep_anim_cb] v=%d\n", v);
  update_life_label(v);
}

// Animation ready callback (optional, can be NULL)
static void arc_sweep_anim_ready_cb(lv_anim_t *a) {}

// Remove overlay after animation
static void overlay_del_cb(lv_anim_t *a)
{
  printf("[overlay_del_cb] Deleting overlay obj=%p\n", a->var);
  lv_obj_del((lv_obj_t *)a->var);
}

static void tap_gesture_cb(lv_event_t *e)
{
  lv_point_t p;
  lv_indev_t *indev = lv_event_get_indev(e);
  lv_indev_get_point(indev, &p);
  bool is_right = (p.x > 160);

  // Find the tap layer (assume it's the last child, so overlay goes just below it)
  uint32_t child_cnt = lv_obj_get_child_cnt(active_screen);
  lv_obj_t *tap_layer = NULL;
  if (child_cnt > 0)
  {
    tap_layer = lv_obj_get_child(active_screen, child_cnt - 1);
  }

  // Create overlay for feedback
  printf("[tap_gesture_cb] Creating overlay for tap feedback\n");
  lv_obj_t *overlay = lv_obj_create(active_screen);
  lv_obj_set_size(overlay, SCREEN_WIDTH / 2, SCREEN_HEIGHT);
  lv_obj_set_style_bg_opa(overlay, LV_OPA_40, LV_PART_MAIN);
  lv_obj_set_style_border_width(overlay, 0, LV_PART_MAIN);
  lv_obj_add_flag(overlay, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(overlay, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(overlay, LV_OBJ_FLAG_GESTURE_BUBBLE); // Ensure overlay is non-interactive
  if (is_right)
  {
    printf("[tap_gesture_cb] Tap on right, incrementing life\n");
    lv_obj_align(overlay, LV_ALIGN_TOP_RIGHT, 0, 0);
    // Always yellow for tap feedback
    lv_obj_set_style_bg_color(overlay, lv_color_make(0xB0, 0xF9, 0xFF), LV_PART_MAIN);
    update_life_label(life_total + 1);
  }
  else
  {
    printf("[tap_gesture_cb] Tap on left, decrementing life\n");
    lv_obj_align(overlay, LV_ALIGN_TOP_LEFT, 0, 0);
    // Always yellow for tap feedback
    lv_obj_set_style_bg_color(overlay, lv_color_make(0xB0, 0xF9, 0xFF), LV_PART_MAIN);
    update_life_label(life_total - 1);
  }

  // Move overlay below tap_layer so it doesn't block further taps
  if (tap_layer)
  {
    lv_obj_move_to_index(overlay, child_cnt - 1); // Just below tap_layer
  }

  // Animate overlay fade out (faster)
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, overlay);
  lv_anim_set_exec_cb(&a, overlay_fade_cb);
  lv_anim_set_values(&a, LV_OPA_40, LV_OPA_TRANSP);
  lv_anim_set_time(&a, 90);  // Faster fade
  lv_anim_set_delay(&a, 10); // Shorter delay
  lv_anim_set_ready_cb(&a, overlay_del_cb);
  lv_anim_start(&a);
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

// Function to convert life total to arc segment
static arc_segment_t life_to_arc(int life_total)
{
  arc_segment_t seg = {0};
  int arc_life = life_total;
  if (arc_life < 0)
    arc_life = 0;
  float circumference = M_PI * ARC_OUTER_DIAMETER;
  float gap_px = 200.0f;
  float gap_deg = (gap_px / circumference) * 360.0f;
  float arc_span = 360.0f - gap_deg;
  float arc_half = arc_span / 2.0f;
  int base_start = (int)(270.0f - arc_half + 0.5f);
  int base_end = (int)(270.0f + arc_half + 0.5f);

  // Color selection (0-10: red, 10-22: red→yellow, 22-35: yellow→green, 35+: green)
  lv_color_t arc_color;
  if (arc_life >= 35)
  {
    arc_color = GREEN_COLOR;
  }
  else if (arc_life >= 22)
  {
    // 22–35: yellow (at 22) to green (at 35)
    uint8_t t = (uint8_t)(((arc_life - 22) * 255) / (35 - 22));
    arc_color = interpolate_color(YELLOW_COLOR, GREEN_COLOR, t);
  }
  else if (arc_life >= 10)
  {
    // 10–22: red (at 10) to yellow (at 22)
    uint8_t t = (uint8_t)(((arc_life - 10) * 255) / (22 - 10));
    arc_color = interpolate_color(RED_COLOR, YELLOW_COLOR, t);
  }
  else
  {
    // 0–10: solid red
    arc_color = RED_COLOR;
  }

  // Clamp to pure green for life >= 35
  if (arc_life >= 35)
  {
    arc_color = GREEN_COLOR;
  }

  if (arc_life > 40)
  {
    seg.start_angle = base_start;
    seg.end_angle = (base_start + 360) % 360;
    seg.color = arc_color;
    return seg;
  }
  if (arc_life == 40)
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
  int sweep = (int)(arc_span * ((float)arc_life / 40.0f) + 0.5f);
  int end_angle = (base_start + sweep) % 360;
  seg.start_angle = base_start;
  seg.end_angle = end_angle;
  seg.color = arc_color;
  return seg;
}

// Update the life label and arc based on the current life total
void update_life_label(int value)
{
  static int last_life_total = 0;
  life_total = value;
  if (life_label)
  {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", life_total);
    // Only update text if it changed
    if (life_total != last_life_total || strcmp(lv_label_get_text(life_label), buf) != 0)
    {
      lv_label_set_text(life_label, buf);
      last_life_total = life_total;
    }
  }

  // Update arc to reflect life total
  if (life_arc)
  {
    arc_segment_t seg = life_to_arc(life_total);
    uint16_t c16 = lv_color_to_u16(seg.color);
    uint8_t r = (c16 >> 11) & 0x1F;
    uint8_t g = (c16 >> 5) & 0x3F;
    uint8_t b = c16 & 0x1F;
    // Scale to 8-bit for debug
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    printf("[update_life_label] life_total=%d, arc: start=%d end=%d color=(%d,%d,%d)\n",
           life_total, seg.start_angle, seg.end_angle, r, g, b);
    lv_arc_set_angles(life_arc, seg.start_angle, seg.end_angle);
    lv_obj_set_style_arc_color(life_arc, seg.color, LV_PART_INDICATOR);
  }
}

// Call this after boot animation to show the life counter
void show_life_counter()
{
  // Only create arc and label if they do not exist
  if (!life_arc)
  {
    printf("[show_life_counter] Creating life_arc...\n");
    life_arc = lv_arc_create(active_screen);
    lv_obj_add_flag(life_arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(life_arc, ARC_OUTER_DIAMETER, ARC_OUTER_DIAMETER);
    lv_obj_align(life_arc, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(life_arc, 0, 360);
    lv_arc_set_angles(life_arc, 270, 270);
    lv_obj_set_style_arc_color(life_arc, GREEN_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(life_arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_width(life_arc, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(life_arc, 0, LV_PART_INDICATOR); // Square ends
    lv_obj_remove_style(life_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(life_arc, LV_OBJ_FLAG_CLICKABLE);
    printf("[show_life_counter] life_arc created.\n");
  }
  if (!life_label)
  {
    life_label = lv_label_create(active_screen);
    lv_obj_add_flag(life_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(life_label, "0");                                // Always set text immediately
    lv_obj_set_style_text_font(life_label, &lv_font_montserrat_48, 0); // Large font
    lv_obj_set_style_text_color(life_label, lv_color_white(), 0);
    lv_obj_align(life_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_opa(life_label, LV_OPA_TRANSP, 0); // Start transparent
  }

  // If the screen is cleared or reloaded, reset tap_layer so it is recreated
  if (tap_layer && lv_obj_get_parent(tap_layer) != active_screen)
  {
    tap_layer = NULL;
  }

  // Show arc and animate sweep while fading in the life label in parallel
  if (life_arc)
  {
    printf("[show_life_counter] Showing arc\n");
    lv_obj_clear_flag(life_arc, LV_OBJ_FLAG_HIDDEN);
    update_life_label(0); // Ensure arc is at 0
    lv_obj_set_style_arc_opa(life_arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, NULL);
    lv_anim_set_exec_cb(&anim, arc_sweep_anim_cb);
    lv_anim_set_values(&anim, 0, LIFE_STD_START);
    lv_anim_set_time(&anim, 2000);
    lv_anim_set_delay(&anim, 0);
    lv_anim_set_ready_cb(&anim, arc_sweep_anim_ready_cb);
    printf("[show_life_counter] Starting arc sweep animation\n");
    lv_anim_start(&anim);
  }
  // Fade in the life label at the same time as the arc sweep
  lv_obj_clear_flag(life_label, LV_OBJ_FLAG_HIDDEN);
  fade_in_obj(life_label, 1000, 0, NULL);

  // // Add a transparent full-screen object to catch taps (only create once)
  // if (!tap_layer)
  // {
  //   printf("[show_life_counter] Creating tap_layer\n");
  //   tap_layer = lv_obj_create(active_screen);
  //   lv_obj_set_size(tap_layer, SCREEN_WIDTH, SCREEN_HEIGHT);
  //   lv_obj_align(tap_layer, LV_ALIGN_CENTER, 0, 0);
  //   lv_obj_set_style_bg_color(tap_layer, lv_color_black(), LV_PART_MAIN);
  //   lv_obj_set_style_bg_opa(tap_layer, LV_OPA_TRANSP, LV_PART_MAIN);
  //   lv_obj_set_style_border_width(tap_layer, 0, LV_PART_MAIN);
  //   lv_obj_add_event_cb(tap_layer, tap_gesture_cb, LV_EVENT_CLICKED, NULL);
  // }
}

void ui_init(void)
{
  // Create screen and all objects before loading
  active_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(active_screen, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(active_screen, LV_OPA_COVER, LV_PART_MAIN);
  // Create "Life Puck" label (title)
  lv_obj_t *title_label = lv_label_create(active_screen);
  lv_label_set_text(title_label, "Life Puck");
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_36, 0);
  lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
  lv_obj_set_style_text_opa(title_label, LV_OPA_TRANSP, 0);
  lv_obj_align(title_label, LV_ALIGN_CENTER, 0, SCREEN_HEIGHT / 9);
  lv_obj_add_flag(title_label, LV_OBJ_FLAG_HIDDEN);

  // Now load the screen (all objects are hidden/transparent)
  printf("[lv_create_main_gui] Loading screen\n");
  lv_scr_load(active_screen);

  // Start fade-in for title label, then fade out and show life counter via show_life_counter()
  lv_obj_clear_flag(title_label, LV_OBJ_FLAG_HIDDEN);
  fade_in_obj(title_label, 1000, 500, [](lv_anim_t *a)
              {
    // Fade out title label, then show life counter
    if (a && a->var) {
      fade_out_obj((lv_obj_t *)a->var, 1000, 0, [](lv_anim_t *anim) {
        if (anim && anim->var) {
          lv_obj_add_flag((lv_obj_t *)anim->var, LV_OBJ_FLAG_HIDDEN);
        }
        // Now show the life counter (arc, label, animation)
        show_life_counter();
      });
    } });
}
