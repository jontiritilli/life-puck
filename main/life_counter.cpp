#include "life_counter.h"
#include "arc_animation.h"
#include <lvgl.h>

// Internal state
static int life_value = 20;
static lv_obj_t* label = nullptr;
static lv_obj_t* parent_obj = nullptr;

// Use a larger font for the life counter (ensure it's enabled in lv_conf.h or menuconfig)
#if LV_FONT_MONTSERRAT_48
#define LIFE_COUNTER_FONT &lv_font_montserrat_48
#elif LV_FONT_MONTSERRAT_36
#define LIFE_COUNTER_FONT &lv_font_montserrat_36
#else
#define LIFE_COUNTER_FONT &lv_font_montserrat_14
#endif

// Initialize the life counter with a parent object and starting value
void life_counter_init(int start_value, lv_obj_t* parent) {
    life_value = start_value;
    parent_obj = parent ? parent : lv_scr_act();
    // Create or update the label
    if (!label) {
        label = lv_label_create(parent_obj);
        lv_obj_set_style_text_font(label, LIFE_COUNTER_FONT, 0);
        lv_obj_center(label);
    } else {
        lv_obj_set_parent(label, parent_obj);
        lv_obj_set_style_text_font(label, LIFE_COUNTER_FONT, 0);
        lv_obj_center(label);
    }
    lv_label_set_text_fmt(label, "%d", life_value);
    arc_animation_init(0, 40, life_value, parent_obj);
}

void life_counter_set(int value) {
    life_value = value;
    if (label) lv_label_set_text_fmt(label, "%d", life_value);
    arc_animation_set(life_value);
}

int life_counter_get() {
    return life_value;
}

void life_counter_increment(int delta) {
    life_value += delta;
    if (label) lv_label_set_text_fmt(label, "%d", life_value);
    arc_animation_set(life_value);
}

void life_counter_reset(int default_value) {
    life_value = default_value;
    if (label) lv_label_set_text_fmt(label, "%d", life_value);
    arc_animation_set(life_value);
}

void life_counter_set_color(lv_color_t color) {
    if (label) lv_obj_set_style_text_color(label, color, 0);
}

void life_counter_set_font(const lv_font_t* font) {
    if (label) lv_obj_set_style_text_font(label, font, 0);
}

void life_counter_update() {
    // For future animation or effects; currently not needed
}
