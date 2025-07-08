#include "arc_animation.h"
#include <lvgl.h>

// Internal state
static lv_obj_t* arc = nullptr;
static int arc_min = 0;
static int arc_max = 40;
static int arc_value = 20;

// Enhanced: allow dynamic color/thickness, parent, and smooth animation
static int arc_thickness = 18;
static lv_color_t arc_color = lv_palette_main(LV_PALETTE_BLUE);
static lv_obj_t* arc_parent = nullptr;

void arc_animation_set_color(lv_color_t color) {
    arc_color = color;
    if (arc) lv_obj_set_style_arc_color(arc, arc_color, 0);
}

void arc_animation_set_thickness(int thickness) {
    arc_thickness = thickness;
    if (arc) lv_obj_set_style_arc_width(arc, arc_thickness, 0);
}

void arc_animation_init(int min_value, int max_value, int start_value, lv_obj_t* parent) {
    arc_min = min_value;
    arc_max = max_value;
    arc_value = start_value;
    arc_parent = parent ? parent : lv_scr_act();

    // Always delete previous arc to avoid overlap
    if (arc) {
        lv_obj_del(arc);
        arc = nullptr;
    }
    arc = lv_arc_create(arc_parent);
    lv_obj_set_size(arc, 320, 320); // Fit typical round display
    lv_obj_center(arc);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_obj_set_style_arc_width(arc, arc_thickness, 0); // Thicker arc
    lv_obj_set_style_arc_color(arc, lv_color_hex(0xFF0000), 0); // Set arc to red for debug
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE); // Arc is not interactive
    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, 0);
    arc_animation_set(start_value);
}

void arc_animation_set(int value) {
    arc_value = value;
    if (!arc || arc_max == arc_min) return;
    int angle = 360 * (arc_value - arc_min) / (arc_max - arc_min);
    // Animate arc to new angle
    int current = lv_arc_get_angle_end(arc);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, [](void* obj, int32_t v) {
        lv_arc_set_angles((lv_obj_t*)obj, 0, v);
    });
    lv_anim_set_time(&a, 300);
    lv_anim_set_values(&a, current, angle);
    lv_anim_start(&a);
}

void arc_animation_update() {
    // For future smooth animation or effects
}
