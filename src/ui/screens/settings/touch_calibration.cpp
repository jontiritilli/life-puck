// =========================================================================
// TOUCH_CALIBRATION.CPP - CLEAN 3-ROUND CALIBRATION
// Round 1: Full calibration (Coarse + Fine)
// Round 2: Refinement of all parameters
// Round 3: Final offset adjustment
// =========================================================================

#include "touch_calibration.h"

#include <lvgl.h>
#include <ArduinoNvs.h>
#include <math.h>

#include "config.h"
#include "core/state_manager.h"
#include "hardware/display/LVGL_Driver.h"
#include "hardware/touch/Touch_CST816.h"
#include "hardware/system/power_management.h"
#include "ui/screens/menu/menu.h"
#include "data/constants.h"

extern struct CST816_Touch touch_data;

#define KEY_TOUCH_CAL_VALID "touch_valid"
#define KEY_TOUCH_PENDING_CONFIRM "touch_pending"

#define TOLERANCE_COARSE 5.0f
#define TOLERANCE_FINE 2.0f
#define CORRECTION_SPEED_COARSE 0.3f
#define CORRECTION_SPEED_FINE 0.1f

typedef enum
{
  // Round 1: Full calibration (Coarse + Fine)
  CAL_STEP_OFFSET_X,
  CAL_STEP_OFFSET_Y,
  CAL_STEP_ROTATION,
  CAL_STEP_SCALE_X,
  CAL_STEP_SCALE_Y,

  // Round 2: Refinement
  CAL_STEP_ROUND2_OFFSET_X,
  CAL_STEP_ROUND2_OFFSET_Y,
  CAL_STEP_ROUND2_ROTATION,
  CAL_STEP_ROUND2_SCALE_X,
  CAL_STEP_ROUND2_SCALE_Y,

  // Round 3: Final offset
  CAL_STEP_ROUND3_OFFSET_X,
  CAL_STEP_ROUND3_OFFSET_Y,

  CAL_STEP_SUMMARY,
  CAL_STEP_FINISHED
} calibration_step_t;

typedef enum
{
  CAL_PASS_COARSE,
  CAL_PASS_FINE
} calibration_pass_t;

static lv_obj_t *cal_screen = nullptr;
static lv_timer_t *cal_timer = nullptr;
static calibration_step_t current_step;
static calibration_pass_t current_pass;
static float temp_cal_matrix[7];
static lv_obj_t *info_label = nullptr;
static lv_obj_t *target_visual = nullptr;
static bool step_is_locked = false;
static int lock_frame_counter = 0;

static float current_theta = 0.0f;
static float current_scale_x = 1.0f;
static float current_scale_y = 1.0f;

static lv_obj_t *confirmation_screen = nullptr;
static lv_obj_t *countdown_label = nullptr;
static uint32_t confirmation_start_time = 0;
static lv_timer_t *confirmation_timer = nullptr;

static const char *TXT_OFFSET_X_COARSE = "Hold red dot\n(Coarse)";
static const char *TXT_OFFSET_X_FINE = "Hold red dot\n(Fine)";
static const char *TXT_OFFSET_Y_COARSE = "Keep holding\n(Coarse)";
static const char *TXT_OFFSET_Y_FINE = "Keep holding\n(Fine)";
static const char *TXT_ROTATION_COARSE = "Hold red dot\non red line (Coarse)";
static const char *TXT_ROTATION_FINE = "Hold red dot\non red line (Fine)";
static const char *TXT_SCALE_X_COARSE = "Hold red dot\n(Coarse)";
static const char *TXT_SCALE_X_FINE = "Hold red dot\n(Fine)";
static const char *TXT_SCALE_Y_COARSE = "Hold red dot\n(Coarse)";
static const char *TXT_SCALE_Y_FINE = "Hold red dot\n(Fine)";

static const char *TXT_ROUND2_OFFSET_X = "Refine X";
static const char *TXT_ROUND2_OFFSET_Y = "Refine Y";
static const char *TXT_ROUND2_ROTATION = "Refine angle";
static const char *TXT_ROUND2_SCALE_X = "Refine scale X";
static const char *TXT_ROUND2_SCALE_Y = "Refine scale Y";

static const char *TXT_ROUND3_OFFSET_X = "Final X";
static const char *TXT_ROUND3_OFFSET_Y = "Final Y";

static const char *TXT_LOCKED = "OK!\nRelease finger";
static const char *TXT_SUMMARY = "Calibration complete!\n\nTouch to exit";

static void setup_ui_for_step(calibration_step_t step);
static void process_calibration_logic(lv_timer_t *timer);
static void save_and_exit_calibration();
static void advance_to_next_step();
static void update_rotation_matrix(float theta);

void resetTouchCalibrationToDefaults()
{
  printf("[TouchCal] Resetting to factory defaults\n");
  const float *defaults = getDefaultCalibrationMatrix();
  NVS.setInt(KEY_TOUCH_CAL_VALID, (int32_t)1);
  NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)0);
  for (int i = 0; i < 7; i++)
  {
    char key[20];
    snprintf(key, sizeof(key), "touch_cal_%d", i);
    NVS.setFloat(key, defaults[i]);
  }
  updateTouchCalibrationMatrix(defaults);
}

void loadTouchCalibrationFromNVS()
{
  if (NVS.getInt(KEY_TOUCH_CAL_VALID, 0) == 1)
  {
    float matrix[7];
    for (int i = 0; i < 7; i++)
    {
      char key[20];
      snprintf(key, sizeof(key), "touch_cal_%d", i);
      matrix[i] = NVS.getFloat(key, 0.0f);
    }
    updateTouchCalibrationMatrix(matrix);
    printf("[TouchCal] Loaded from NVS\n");
  }
  else
  {
    resetTouchCalibrationToDefaults();
  }
}

bool needsTouchCalibrationConfirmation()
{
  return NVS.getInt(KEY_TOUCH_PENDING_CONFIRM, 0) == 1;
}

void confirmTouchCalibration()
{
  NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)0);
  printf("[TouchCal] Confirmed\n");
}

static void update_rotation_matrix(float theta)
{
  temp_cal_matrix[1] = current_scale_x * cosf(theta);
  temp_cal_matrix[2] = current_scale_x * sinf(theta);
  temp_cal_matrix[4] = -current_scale_y * sinf(theta);
  temp_cal_matrix[5] = current_scale_y * cosf(theta);
}

void renderTouchCalibrationConfirmation()
{
  teardownTouchCalibrationConfirmation();

  confirmation_screen = lv_obj_create(lv_scr_act());
  lv_obj_set_size(confirmation_screen, 320, 240);
  lv_obj_align(confirmation_screen, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(confirmation_screen, lv_color_hex(0x2C2C2C), 0);
  lv_obj_set_style_border_width(confirmation_screen, 2, 0);
  lv_obj_set_style_border_color(confirmation_screen, lv_color_hex(0x00FF00), 0);
  lv_obj_move_foreground(confirmation_screen);

  lv_obj_t *title = lv_label_create(confirmation_screen);
  lv_label_set_text(title, "Touch Calibration");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  lv_obj_t *msg = lv_label_create(confirmation_screen);
  lv_label_set_text(msg, "New calibration applied!\nKeep these settings?");
  lv_obj_set_style_text_color(msg, lv_color_white(), 0);
  lv_obj_set_style_text_font(msg, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(msg, 300);
  lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 45);

  countdown_label = lv_label_create(confirmation_screen);
  lv_label_set_text(countdown_label, "Auto-revert in: 10s");
  lv_obj_set_style_text_color(countdown_label, lv_color_hex(0xFFAA00), 0);
  lv_obj_set_style_text_font(countdown_label, &lv_font_montserrat_14, 0);
  lv_obj_set_width(countdown_label, 300);
  lv_obj_set_style_text_align(countdown_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(countdown_label, LV_ALIGN_CENTER, 0, 30);

  lv_obj_t *keep_btn = lv_btn_create(confirmation_screen);
  lv_obj_set_size(keep_btn, 120, 50);
  lv_obj_align(keep_btn, LV_ALIGN_BOTTOM_LEFT, 20, -10);
  lv_obj_set_style_bg_color(keep_btn, lv_color_hex(0x00AA00), 0);
  lv_obj_t *keep_lbl = lv_label_create(keep_btn);
  lv_label_set_text(keep_lbl, "Keep");
  lv_obj_set_style_text_color(keep_lbl, lv_color_white(), 0);
  lv_obj_center(keep_lbl);
  lv_obj_add_event_cb(keep_btn, [](lv_event_t *e)
                      {
        confirmTouchCalibration();
        teardownTouchCalibrationConfirmation();
        renderMenu(MENU_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  lv_obj_t *revert_btn = lv_btn_create(confirmation_screen);
  lv_obj_set_size(revert_btn, 120, 50);
  lv_obj_align(revert_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
  lv_obj_set_style_bg_color(revert_btn, lv_color_hex(0xAA0000), 0);
  lv_obj_t *revert_lbl = lv_label_create(revert_btn);
  lv_label_set_text(revert_lbl, "Revert");
  lv_obj_set_style_text_color(revert_lbl, lv_color_white(), 0);
  lv_obj_center(revert_lbl);
  lv_obj_add_event_cb(revert_btn, [](lv_event_t *e)
                      {
        resetTouchCalibrationToDefaults();
        teardownTouchCalibrationConfirmation();
        renderMenu(MENU_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  confirmation_start_time = millis();
  confirmation_timer = lv_timer_create([](lv_timer_t *timer)
                                       {
        long remaining_ms = (long)CALIBRATION_TIMEOUT_MS - (long)(millis() - confirmation_start_time);
        if (remaining_ms > 0) {
            char txt[50];
            snprintf(txt, sizeof(txt), "Auto-revert in: %lds", (remaining_ms / 1000) + 1);
            if (countdown_label) lv_label_set_text(countdown_label, txt);
        } else {
            resetTouchCalibrationToDefaults();
            teardownTouchCalibrationConfirmation();
            renderMenu(MENU_SETTINGS);
        } }, 500, NULL);
}

void teardownTouchCalibrationConfirmation()
{
  power_suspend(false);
  if (confirmation_timer)
  {
    lv_timer_del(confirmation_timer);
    confirmation_timer = nullptr;
  }
  if (confirmation_screen)
  {
    lv_obj_del(confirmation_screen);
    confirmation_screen = nullptr;
  }
}

void renderTouchCalibrationScreen()
{
  printf("[TouchCal] Starting clean 3-round calibration\n");

  power_suspend(true);
  teardownTouchCalibrationScreen();
  vTaskDelay(pdMS_TO_TICKS(50));

  cal_screen = lv_obj_create(lv_scr_act());
  lv_obj_remove_style_all(cal_screen);
  lv_obj_set_style_bg_color(cal_screen, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(cal_screen, LV_OPA_COVER, 0);
  lv_obj_set_size(cal_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_center(cal_screen);

  const float *defaults = getDefaultCalibrationMatrix();
  for (int i = 0; i < 7; i++)
  {
    temp_cal_matrix[i] = defaults[i];
  }

  current_scale_x = temp_cal_matrix[1];
  current_scale_y = temp_cal_matrix[5];
  current_theta = 0.0f;

  if (fabs(temp_cal_matrix[2]) > 0.01f)
  {
    current_scale_x = sqrtf(temp_cal_matrix[1] * temp_cal_matrix[1] +
                            temp_cal_matrix[2] * temp_cal_matrix[2]);
    current_scale_y = sqrtf(temp_cal_matrix[4] * temp_cal_matrix[4] +
                            temp_cal_matrix[5] * temp_cal_matrix[5]);
    current_theta = atan2f(temp_cal_matrix[2], temp_cal_matrix[1]);
  }

  printf("[TouchCal] Init: offset=(%.1f,%.1f) scale=(%.3f,%.3f) theta=%.3f\n",
         temp_cal_matrix[0], temp_cal_matrix[3], current_scale_x, current_scale_y, current_theta);

  current_step = CAL_STEP_OFFSET_X;
  current_pass = CAL_PASS_COARSE;
  step_is_locked = false;
  lock_frame_counter = 0;

  setup_ui_for_step(CAL_STEP_OFFSET_X);
  cal_timer = lv_timer_create(process_calibration_logic, 50, NULL);
}

void teardownTouchCalibrationScreen()
{
  power_suspend(false);
  if (cal_timer)
  {
    lv_timer_del(cal_timer);
    cal_timer = nullptr;
  }
  if (cal_screen)
  {
    lv_obj_del(cal_screen);
    cal_screen = nullptr;
  }
  info_label = nullptr;
  target_visual = nullptr;
}

static void advance_to_next_step()
{
  printf("[TouchCal] Advancing from step %d (pass %d)\n", current_step, current_pass);

  // Round 1: Coarse -> Fine
  if (current_step <= CAL_STEP_SCALE_Y && current_pass == CAL_PASS_COARSE)
  {
    current_pass = CAL_PASS_FINE;
    step_is_locked = false;
    lock_frame_counter = 0;
    setup_ui_for_step(current_step);
    return;
  }

  // All other steps: advance
  step_is_locked = false;
  lock_frame_counter = 0;
  current_pass = CAL_PASS_COARSE;
  current_step = (calibration_step_t)(current_step + 1);

  if (current_step == CAL_STEP_FINISHED)
  {
    save_and_exit_calibration();
  }
  else
  {
    setup_ui_for_step(current_step);
  }
}

static void setup_ui_for_step(calibration_step_t step)
{
  printf("[TouchCal] UI for step %d (pass %d)\n", step, current_pass);

  lv_obj_clean(cal_screen);
  target_visual = nullptr;
  info_label = nullptr;

  if (step == CAL_STEP_SUMMARY)
  {
    info_label = lv_label_create(cal_screen);
    lv_obj_set_style_text_color(info_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(info_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(info_label, SCREEN_WIDTH - 40);

    char summary[300];
    snprintf(summary, sizeof(summary),
             "%s\n\n"
             "Offset: (%.1f, %.1f)\n"
             "Rotation: %.1f°\n"
             "Scale: (%.3f, %.3f)",
             TXT_SUMMARY,
             temp_cal_matrix[0], temp_cal_matrix[3],
             current_theta * 180.0f / M_PI,
             current_scale_x, current_scale_y);

    lv_label_set_text(info_label, summary);
    lv_obj_center(info_label);
    return;
  }

  info_label = lv_label_create(cal_screen);
  lv_obj_set_style_text_color(info_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(info_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(info_label, SCREEN_WIDTH - 40);
  lv_obj_align(info_label, LV_ALIGN_TOP_MID, 0, 20);

  switch (step)
  {
  case CAL_STEP_OFFSET_X:
  case CAL_STEP_ROUND2_OFFSET_X:
  case CAL_STEP_ROUND3_OFFSET_X:
  {
    if (step == CAL_STEP_ROUND3_OFFSET_X)
    {
      lv_label_set_text_static(info_label, TXT_ROUND3_OFFSET_X);
    }
    else if (step == CAL_STEP_ROUND2_OFFSET_X)
    {
      lv_label_set_text_static(info_label, TXT_ROUND2_OFFSET_X);
    }
    else
    {
      lv_label_set_text_static(info_label,
                               current_pass == CAL_PASS_COARSE ? TXT_OFFSET_X_COARSE : TXT_OFFSET_X_FINE);
    }

    target_visual = lv_obj_create(cal_screen);
    lv_obj_remove_style_all(target_visual);
    lv_obj_set_style_bg_color(target_visual, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(target_visual, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(target_visual, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_size(target_visual, 30, 30);
    lv_obj_clear_flag(target_visual, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(target_visual, LV_ALIGN_CENTER, 0, 0);
    break;
  }

  case CAL_STEP_OFFSET_Y:
  case CAL_STEP_ROUND2_OFFSET_Y:
  case CAL_STEP_ROUND3_OFFSET_Y:
  {
    if (step == CAL_STEP_ROUND3_OFFSET_Y)
    {
      lv_label_set_text_static(info_label, TXT_ROUND3_OFFSET_Y);
    }
    else if (step == CAL_STEP_ROUND2_OFFSET_Y)
    {
      lv_label_set_text_static(info_label, TXT_ROUND2_OFFSET_Y);
    }
    else
    {
      lv_label_set_text_static(info_label,
                               current_pass == CAL_PASS_COARSE ? TXT_OFFSET_Y_COARSE : TXT_OFFSET_Y_FINE);
    }

    target_visual = lv_obj_create(cal_screen);
    lv_obj_remove_style_all(target_visual);
    lv_obj_set_style_bg_color(target_visual, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(target_visual, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(target_visual, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_size(target_visual, 30, 30);
    lv_obj_clear_flag(target_visual, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(target_visual, LV_ALIGN_CENTER, 0, 0);
    break;
  }

  case CAL_STEP_ROTATION:
  case CAL_STEP_ROUND2_ROTATION:
  {
    lv_label_set_text_static(info_label,
                             step == CAL_STEP_ROUND2_ROTATION ? TXT_ROUND2_ROTATION : (current_pass == CAL_PASS_COARSE ? TXT_ROTATION_COARSE : TXT_ROTATION_FINE));

    target_visual = lv_obj_create(cal_screen);
    lv_obj_remove_style_all(target_visual);
    lv_obj_set_style_bg_color(target_visual, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(target_visual, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(target_visual, 0, 0);
    lv_obj_set_size(target_visual, 5, SCREEN_HEIGHT);
    lv_obj_clear_flag(target_visual, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(target_visual, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *guide_dot = lv_obj_create(cal_screen);
    lv_obj_remove_style_all(guide_dot);
    lv_obj_set_style_bg_color(guide_dot, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(guide_dot, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(guide_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_size(guide_dot, 25, 25);
    lv_obj_set_pos(guide_dot, (SCREEN_WIDTH / 2) - 12, (SCREEN_HEIGHT / 4) - 12);
    lv_obj_clear_flag(guide_dot, LV_OBJ_FLAG_CLICKABLE);
    break;
  }

  case CAL_STEP_SCALE_X:
  case CAL_STEP_ROUND2_SCALE_X:
  {
    lv_label_set_text_static(info_label,
                             step == CAL_STEP_ROUND2_SCALE_X ? TXT_ROUND2_SCALE_X : (current_pass == CAL_PASS_COARSE ? TXT_SCALE_X_COARSE : TXT_SCALE_X_FINE));

    target_visual = lv_obj_create(cal_screen);
    lv_obj_remove_style_all(target_visual);
    lv_obj_set_style_bg_color(target_visual, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(target_visual, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(target_visual, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_size(target_visual, 30, 30);
    lv_obj_clear_flag(target_visual, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(target_visual, LV_ALIGN_RIGHT_MID, -20, 0);
    break;
  }

  case CAL_STEP_SCALE_Y:
  case CAL_STEP_ROUND2_SCALE_Y:
  {
    lv_label_set_text_static(info_label,
                             step == CAL_STEP_ROUND2_SCALE_Y ? TXT_ROUND2_SCALE_Y : (current_pass == CAL_PASS_COARSE ? TXT_SCALE_Y_COARSE : TXT_SCALE_Y_FINE));

    target_visual = lv_obj_create(cal_screen);
    lv_obj_remove_style_all(target_visual);
    lv_obj_set_style_bg_color(target_visual, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(target_visual, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(target_visual, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_size(target_visual, 30, 30);
    lv_obj_clear_flag(target_visual, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(target_visual, LV_ALIGN_BOTTOM_MID, 0, -20);
    break;
  }

  default:
    break;
  }

  lv_obj_update_layout(cal_screen);
}

static void process_calibration_logic(lv_timer_t *timer)
{
  static uint32_t last_touch_time = 0;
  static uint32_t summary_touch_time = 0;

  if (current_step == CAL_STEP_SUMMARY)
  {
    if (step_is_locked)
    {
      Touch_Read_Data();
      int pts = touch_data.points;
      touch_data.points = 0;

      if (pts == 0)
      {
        if (summary_touch_time == 0)
        {
          summary_touch_time = millis();
        }
        else if (millis() - summary_touch_time > 300)
        {
          summary_touch_time = 0;
          advance_to_next_step();
        }
      }
      else
      {
        summary_touch_time = 0;
      }
      return;
    }

    Touch_Read_Data();
    if (touch_data.points > 0)
    {
      step_is_locked = true;
    }
    return;
  }

  if (step_is_locked)
  {
    bool is_round2 = (current_step >= CAL_STEP_ROUND2_OFFSET_X && current_step <= CAL_STEP_ROUND2_SCALE_Y);
    bool is_round3 = (current_step >= CAL_STEP_ROUND3_OFFSET_X);
    bool needs_release = (current_pass == CAL_PASS_FINE) || is_round2 || is_round3;

    if (needs_release)
    {
      Touch_Read_Data();
      int pts = touch_data.points;
      touch_data.points = 0;

      if (pts == 0)
      {
        if (last_touch_time == 0)
        {
          last_touch_time = millis();
        }
        else if (millis() - last_touch_time > 300)
        {
          last_touch_time = 0;
          advance_to_next_step();
        }
      }
      else
      {
        last_touch_time = 0;
      }
      return;
    }
    else
    {
      advance_to_next_step();
      return;
    }
  }

  Touch_Read_Data();
  if (touch_data.points == 0)
  {
    lock_frame_counter = 0;
    return;
  }

  int16_t x_raw = touch_data.x;
  int16_t y_raw = touch_data.y;

  float cal_x = (temp_cal_matrix[1] * x_raw) + (temp_cal_matrix[2] * y_raw) + temp_cal_matrix[0];
  float cal_y = (temp_cal_matrix[4] * x_raw) + (temp_cal_matrix[5] * y_raw) + temp_cal_matrix[3];
  cal_x /= temp_cal_matrix[6];
  cal_y /= temp_cal_matrix[6];

  int16_t x_cal = (int16_t)(cal_x + 0.5f);
  int16_t y_cal = (int16_t)(cal_y + 0.5f);

  bool is_round2 = (current_step >= CAL_STEP_ROUND2_OFFSET_X && current_step <= CAL_STEP_ROUND2_SCALE_Y);
  bool is_round3 = (current_step >= CAL_STEP_ROUND3_OFFSET_X);

  float tolerance = (is_round2 || is_round3) ? TOLERANCE_FINE : ((current_pass == CAL_PASS_COARSE) ? TOLERANCE_COARSE : TOLERANCE_FINE);
  float correction_speed = (is_round2 || is_round3) ? CORRECTION_SPEED_FINE * 0.5f : ((current_pass == CAL_PASS_COARSE) ? CORRECTION_SPEED_COARSE : CORRECTION_SPEED_FINE);
  int required_lock_frames = (is_round2 || is_round3) ? (CALIBRATION_LOCK_FRAMES * 2) : CALIBRATION_LOCK_FRAMES;

  static int debug_ctr = 0;
  bool do_debug = (debug_ctr++ % 20 == 0);

  switch (current_step)
  {
  case CAL_STEP_OFFSET_X:
  case CAL_STEP_ROUND2_OFFSET_X:
  case CAL_STEP_ROUND3_OFFSET_X:
  {
    int target_x = SCREEN_WIDTH / 2;
    float error_x = x_cal - target_x;

    if (do_debug)
    {
      const char *round_name = (current_step == CAL_STEP_ROUND3_OFFSET_X) ? "R3" : (current_step == CAL_STEP_ROUND2_OFFSET_X) ? "R2"
                                                                                                                              : "R1";
      printf("[CAL-%s-X] raw=%d cal=%d target=%d error=%.1f\n",
             round_name, x_raw, x_cal, target_x, error_x);
    }

    if (fabs(error_x) > tolerance)
    {
      temp_cal_matrix[0] -= error_x * correction_speed;
      lock_frame_counter = 0;
    }
    else
    {
      lock_frame_counter++;
      if (lock_frame_counter >= required_lock_frames)
      {
        const char *round_name = (current_step == CAL_STEP_ROUND3_OFFSET_X) ? "R3" : (current_step == CAL_STEP_ROUND2_OFFSET_X) ? "R2"
                                                                                                                                : "R1";
        printf("[CAL-%s-X] LOCKED at %.2f\n", round_name, temp_cal_matrix[0]);
        step_is_locked = true;
        if (current_pass == CAL_PASS_FINE || is_round2 || is_round3)
        {
          lv_label_set_text_static(info_label, TXT_LOCKED);
        }
      }
    }
    break;
  }

  case CAL_STEP_OFFSET_Y:
  case CAL_STEP_ROUND2_OFFSET_Y:
  case CAL_STEP_ROUND3_OFFSET_Y:
  {
    int target_y = SCREEN_HEIGHT / 2;
    float error_y = y_cal - target_y;

    if (do_debug)
    {
      const char *round_name = (current_step == CAL_STEP_ROUND3_OFFSET_Y) ? "R3" : (current_step == CAL_STEP_ROUND2_OFFSET_Y) ? "R2"
                                                                                                                              : "R1";
      printf("[CAL-%s-Y] raw=%d cal=%d target=%d error=%.1f\n",
             round_name, y_raw, y_cal, target_y, error_y);
    }

    if (fabs(error_y) > tolerance)
    {
      temp_cal_matrix[3] -= error_y * correction_speed;
      lock_frame_counter = 0;
    }
    else
    {
      lock_frame_counter++;
      if (lock_frame_counter >= required_lock_frames)
      {
        const char *round_name = (current_step == CAL_STEP_ROUND3_OFFSET_Y) ? "R3" : (current_step == CAL_STEP_ROUND2_OFFSET_Y) ? "R2"
                                                                                                                                : "R1";
        printf("[CAL-%s-Y] LOCKED at %.2f\n", round_name, temp_cal_matrix[3]);
        step_is_locked = true;
        if (current_pass == CAL_PASS_FINE || is_round2 || is_round3)
        {
          lv_label_set_text_static(info_label, TXT_LOCKED);
        }
      }
    }
    break;
  }

  case CAL_STEP_ROTATION:
  case CAL_STEP_ROUND2_ROTATION:
  {
    int target_x = SCREEN_WIDTH / 2;
    float error_x = x_cal - target_x;

    if (do_debug)
    {
      printf("[CAL-%s-ROT] raw=(%d,%d) cal=(%d,%d) error_x=%.1f theta=%.3f\n",
             is_round2 ? "R2" : "R1", x_raw, y_raw, x_cal, y_cal, error_x, current_theta);
    }

    if (fabs(error_x) > tolerance)
    {
      current_theta -= error_x * 0.002f * correction_speed;
      update_rotation_matrix(current_theta);
      lock_frame_counter = 0;
    }
    else
    {
      lock_frame_counter++;
      if (lock_frame_counter >= required_lock_frames)
      {
        printf("[CAL-%s-ROT] LOCKED at %.3f (%.1f°)\n",
               is_round2 ? "R2" : "R1", current_theta, current_theta * 180.0f / M_PI);
        step_is_locked = true;
        if (current_pass == CAL_PASS_FINE || is_round2)
        {
          lv_label_set_text_static(info_label, TXT_LOCKED);
        }
      }
    }
    break;
  }

  case CAL_STEP_SCALE_X:
  case CAL_STEP_ROUND2_SCALE_X:
  {
    int target_x = SCREEN_WIDTH - 20;
    float error_x = x_cal - target_x;

    if (do_debug)
    {
      printf("[CAL-%s-SCALE_X] raw=%d cal=%d target=%d error=%.1f scale=%.3f\n",
             is_round2 ? "R2" : "R1", x_raw, x_cal, target_x, error_x, current_scale_x);
    }

    if (fabs(error_x) > tolerance)
    {
      float scale_correction = 1.0f - (error_x / (float)target_x) * correction_speed;
      current_scale_x *= scale_correction;

      if (current_scale_x < 0.5f)
        current_scale_x = 0.5f;
      if (current_scale_x > 2.0f)
        current_scale_x = 2.0f;

      update_rotation_matrix(current_theta);
      lock_frame_counter = 0;
    }
    else
    {
      lock_frame_counter++;
      if (lock_frame_counter >= required_lock_frames)
      {
        printf("[CAL-%s-SCALE_X] LOCKED at %.3f\n", is_round2 ? "R2" : "R1", current_scale_x);
        step_is_locked = true;
        if (current_pass == CAL_PASS_FINE || is_round2)
        {
          lv_label_set_text_static(info_label, TXT_LOCKED);
        }
      }
    }
    break;
  }

  case CAL_STEP_SCALE_Y:
  case CAL_STEP_ROUND2_SCALE_Y:
  {
    int target_y = SCREEN_HEIGHT - 20;
    float error_y = y_cal - target_y;

    if (do_debug)
    {
      printf("[CAL-%s-SCALE_Y] raw=%d cal=%d target=%d error=%.1f scale=%.3f\n",
             is_round2 ? "R2" : "R1", y_raw, y_cal, target_y, error_y, current_scale_y);
    }

    if (fabs(error_y) > tolerance)
    {
      float scale_correction = 1.0f - (error_y / (float)target_y) * correction_speed;
      current_scale_y *= scale_correction;

      if (current_scale_y < 0.5f)
        current_scale_y = 0.5f;
      if (current_scale_y > 2.0f)
        current_scale_y = 2.0f;

      update_rotation_matrix(current_theta);
      lock_frame_counter = 0;
    }
    else
    {
      lock_frame_counter++;
      if (lock_frame_counter >= required_lock_frames)
      {
        printf("[CAL-%s-SCALE_Y] LOCKED at %.3f\n", is_round2 ? "R2" : "R1", current_scale_y);
        step_is_locked = true;
        if (current_pass == CAL_PASS_FINE || is_round2)
        {
          lv_label_set_text_static(info_label, TXT_LOCKED);
        }
      }
    }
    break;
  }

  default:
    break;
  }
}

static void save_and_exit_calibration()
{
  printf("[TouchCal] Saving final matrix...\n");

  NVS.setInt(KEY_TOUCH_PENDING_CONFIRM, (int32_t)1);
  NVS.setInt(KEY_TOUCH_CAL_VALID, (int32_t)1);

  for (int i = 0; i < 7; i++)
  {
    char key[20];
    snprintf(key, sizeof(key), "touch_cal_%d", i);
    NVS.setFloat(key, temp_cal_matrix[i]);
  }

  updateTouchCalibrationMatrix(temp_cal_matrix);

  printf("[TouchCal] DONE! Clean 3-round calibration complete!\n");
  printf("  Matrix: [%.2f, %.3f, %.3f, %.2f, %.3f, %.3f, %.3f]\n",
         temp_cal_matrix[0], temp_cal_matrix[1], temp_cal_matrix[2],
         temp_cal_matrix[3], temp_cal_matrix[4], temp_cal_matrix[5], temp_cal_matrix[6]);

  renderMenu(MENU_SETTINGS);
}
