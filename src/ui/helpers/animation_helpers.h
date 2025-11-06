#pragma once
#include <stdio.h>
#include <lvgl.h>

/**
 * @brief Animation callback for text fade effects
 * @param label_obj LVGL label object to animate
 * @param opa Opacity value (0-255)
 */
void text_fade_anim_cb(void *label_obj, int32_t opa);

/**
 * @brief Fade in an LVGL object smoothly
 * @param obj LVGL object to animate
 * @param duration Animation duration in milliseconds
 * @param delay Start delay in milliseconds
 * @param ready_cb Optional callback when animation completes
 */
void fade_in_obj(lv_obj_t *obj, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb = NULL);

/**
 * @brief Fade out an LVGL object smoothly
 * @param obj LVGL object to animate
 * @param duration Animation duration in milliseconds
 * @param delay Start delay in milliseconds
 * @param ready_cb Optional callback when animation completes
 */
void fade_out_obj(lv_obj_t *obj, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb = NULL);

/**
 * @brief Slide object horizontally with smooth animation
 * @param obj LVGL object to animate
 * @param start_x Starting X position
 * @param end_x Target X position
 * @param duration Animation duration in milliseconds
 * @param delay Start delay in milliseconds
 * @param ready_cb Optional callback when animation completes
 */
void slide_in_obj_horizontal(lv_obj_t *obj, lv_coord_t start_x, lv_coord_t end_x, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb = NULL);

/**
 * @brief Slide object vertically with smooth animation
 * @param obj LVGL object to animate
 * @param start_y Starting Y position
 * @param end_y Target Y position
 * @param duration Animation duration in milliseconds
 * @param delay Start delay in milliseconds
 * @param ready_cb Optional callback when animation completes
 */
void slide_in_obj_vertical(lv_obj_t *obj, lv_coord_t start_y, lv_coord_t end_y, uint32_t duration, uint32_t delay, lv_anim_ready_cb_t ready_cb = NULL);
