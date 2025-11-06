// ============================================
// Own Header (first!)
// ============================================
#include "shutdown.h"

// ============================================
// System & Framework Headers
// ============================================
#include <stdbool.h>
#include <lvgl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


/**
 * @brief Display shutdown animation before power off
 * 
 * Shows a smooth fade-out animation of the entire screen over 1 second
 * to provide visual feedback before the device shuts down.
 */
void show_shutdown_animation()
{
    // Simple shutdown animation: fade-out of the entire screen
    lv_obj_t *screen = lv_scr_act();
    if (screen) {
        printf("[Shutdown] Showing shutdown animation...\n");
        
        // Fade-out animation for 1 second
        lv_anim_t fade_anim;
        lv_anim_init(&fade_anim);
        lv_anim_set_var(&fade_anim, screen);
        lv_anim_set_values(&fade_anim, LV_OPA_COVER, LV_OPA_TRANSP);
        lv_anim_set_time(&fade_anim, 1000);
        lv_anim_set_exec_cb(&fade_anim, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
        lv_anim_set_ready_cb(&fade_anim, [](lv_anim_t *a){
            printf("[Shutdown] Animation completed\n");
        });
        lv_anim_start(&fade_anim);
        
        // Wait briefly to show animation
        vTaskDelay(pdMS_TO_TICKS(1100));
    }
}
