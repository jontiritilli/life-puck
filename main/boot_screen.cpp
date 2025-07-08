#include "boot_screen.h"
#include <lvgl.h>
#include "esp_timer.h"

// Internal state
static lv_obj_t* arc = nullptr;
static lv_obj_t* label = nullptr;

static uint32_t anim_start = 0;
static bool active = false;
// Boot screen duration in milliseconds (easy to adjust)
static const uint32_t BOOT_SCREEN_DURATION_MS = 5000;

static void arc_anim_cb(void* obj, int32_t v) {
    lv_arc_set_end_angle((lv_obj_t*)obj, v);
}

static void label_fade_cb(void* obj, int32_t opa) {
    lv_obj_set_style_opa((lv_obj_t*)obj, opa, 0);
}

void boot_screen_show() {
    active = true;  
    // Clear the screen before showing the boot screen
    lv_obj_clean(lv_scr_act());
    printf("[BOOT] boot_screen_show() called\n");
    anim_start = esp_timer_get_time() / 1000;

    // Create red arc (tracer)
    arc = lv_arc_create(lv_scr_act());
    lv_obj_set_size(arc, 340, 340);
    lv_obj_center(arc);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_angles(arc, 0, 0);
    lv_obj_set_style_arc_color(arc, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_arc_width(arc, 12, 0);
    lv_arc_set_rotation(arc, 0);

    // Create "Life Puck" label, initially transparent
    label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Life Puck");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_opa(label, 0, 0);
    lv_obj_center(label);

    // Animate arc: 0 → 360 deg in 1s
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, arc_anim_cb);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_values(&a, 0, 360);
    lv_anim_start(&a);

    // Animate label fade-in after arc completes
    lv_anim_t b;
    lv_anim_init(&b);
    lv_anim_set_var(&b, label);
    lv_anim_set_exec_cb(&b, label_fade_cb);
    lv_anim_set_time(&b, 600);
    lv_anim_set_delay(&b, 1000);
    lv_anim_set_values(&b, 0, LV_OPA_COVER);
    lv_anim_start(&b);
}


bool boot_screen_active() {
    // Boot screen is active for BOOT_SCREEN_DURATION_MS
    return active && ((esp_timer_get_time() / 1000) - anim_start < BOOT_SCREEN_DURATION_MS);
}

void boot_screen_update() {
    if (active && ((esp_timer_get_time() / 1000) - anim_start > BOOT_SCREEN_DURATION_MS)) {
        // Clean up boot screen objects
        if (arc) lv_obj_del(arc);
        if (label) lv_obj_del(label);
        arc = nullptr;
        label = nullptr;
        // Clear the screen after boot screen is done
        lv_obj_clean(lv_scr_act());
        active = false;
    }
}