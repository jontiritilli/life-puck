#include "menu.h"
#include <lvgl.h>

// Forward declaration for event handler
static void menu_quadrant_event_cb(lv_event_t *e);

// Helper: Get quadrant from coordinates (relative to menu center)
static int menu_get_quadrant(lv_coord_t x, lv_coord_t y, lv_coord_t cx, lv_coord_t cy) {
    int dx = x - cx;
    int dy = y - cy;
    if (dx >= 0 && dy < 0) return MENU_ACTION_SETTINGS; // Top-right
    if (dx < 0 && dy < 0)  return MENU_ACTION_1P2P;     // Top-left
    if (dx < 0 && dy >= 0) return MENU_ACTION_RESET;     // Bottom-left
    return MENU_ACTION_HISTORY;                          // Bottom-right
}

// Main menu create function
lv_obj_t* menu_create(lv_obj_t* parent, void (*on_action)(int)) {
    // Create a semi-transparent full-screen overlay
    lv_obj_t *overlay = lv_obj_create(parent);
    lv_obj_remove_style_all(overlay);
    lv_obj_set_size(overlay, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

    // Center point
    lv_coord_t cx = lv_obj_get_width(overlay) / 2;
    lv_coord_t cy = lv_obj_get_height(overlay) / 2;

    // Draw 4 quadrant buttons (invisible, catch touch)
    for (int i = 0; i < 4; ++i) {
        lv_obj_t *btn = lv_btn_create(overlay);
        lv_obj_remove_style_all(btn);
        lv_obj_set_size(btn, cx, cy);
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, 0);
        lv_obj_set_style_border_opa(btn, LV_OPA_0, 0);
        lv_obj_set_style_radius(btn, 0, 0);
        // Position in quadrant
        lv_coord_t x = (i == 0 || i == 3) ? cx : 0;
        lv_coord_t y = (i < 2) ? 0 : cy;
        lv_obj_set_pos(btn, x, y);
        lv_obj_set_user_data(btn, (void*)on_action); // Store callback in button
        lv_obj_add_event_cb(btn, menu_quadrant_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i); // Pass quadrant as event user data
    }

    // Draw quadrant icons/text
    static const char *labels[4] = {"\uF013", "1P/2P", "\u21BA", "\uF017"};
    static const char *desc[4]   = {"Settings", "1P/2P", "Reset", "History"};
    for (int i = 0; i < 4; ++i) {
        lv_obj_t *lbl = lv_label_create(overlay);
        lv_label_set_text(lbl, labels[i]);
        lv_obj_set_style_text_font(lbl, LV_FONT_DEFAULT, 0);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_set_style_text_opa(lbl, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_opa(lbl, LV_OPA_0, 0);
        lv_obj_set_style_border_opa(lbl, LV_OPA_0, 0);
        lv_coord_t lx = cx + (i == 0 || i == 3 ? cx/2 : -cx/2);
        lv_coord_t ly = cy + (i < 2 ? -cy/2 : cy/2);
        lv_obj_set_pos(lbl, lx - 16, ly - 16);
        lv_obj_t *desc_lbl = lv_label_create(overlay);
        lv_label_set_text(desc_lbl, desc[i]);
        lv_obj_set_style_text_font(desc_lbl, LV_FONT_DEFAULT, 0);
        lv_obj_set_style_text_color(desc_lbl, lv_color_white(), 0);
        lv_obj_set_style_text_opa(desc_lbl, LV_OPA_80, 0);
        lv_obj_set_style_bg_opa(desc_lbl, LV_OPA_0, 0);
        lv_obj_set_style_border_opa(desc_lbl, LV_OPA_0, 0);
        lv_obj_set_pos(desc_lbl, lx - 24, ly + 8);
    }

    return overlay;
}

// Event handler for quadrant buttons
static void menu_quadrant_event_cb(lv_event_t *e) {
    lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
    int quadrant = (int)(intptr_t)lv_event_get_user_data(e);
    void (*on_action)(int) = (void (*)(int))lv_obj_get_user_data(btn);
    if (on_action) on_action(quadrant);
    // Remove menu overlay
    lv_obj_del(lv_obj_get_parent(btn));
}
