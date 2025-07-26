#include "start_life.h"
#include <lvgl.h>
#include <menu/menu.h>
#include "state/state_store.h"

extern lv_obj_t *life_config_menu;
int max_life;
int small_step;
int large_step;

// Shared input state struct
struct SharedInputState
{
  lv_obj_t *ta;
  lv_obj_t *kb;
  int *current_var;
  lv_obj_t *current_label;
};

// Static event callback for value buttons
void open_input_cb(lv_event_t *e)
{
  SharedInputState *state = (SharedInputState *)lv_event_get_user_data(e);
  lv_obj_t *btn = lv_event_get_target_obj(e);
  int *var = (int *)lv_obj_get_user_data(btn);
  lv_obj_t *lbl = lv_obj_get_child(btn, 0); // label
  // Always reset/hide before showing
  lv_obj_add_flag(state->ta, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(state->kb, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_textarea(state->kb, NULL);
  state->current_var = nullptr;
  state->current_label = nullptr;
  // Now set up for new input
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", *var);
  lv_textarea_set_text(state->ta, buf);
  state->current_var = var;
  state->current_label = lbl;
  lv_obj_clear_flag(state->ta, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(state->kb, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_textarea(state->kb, state->ta);
  lv_obj_move_foreground(state->kb);
}

// Static event callback for shared textarea
void shared_ta_event_cb(lv_event_t *e)
{
  SharedInputState *state = (SharedInputState *)lv_event_get_user_data(e);
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED && state->current_var && state->current_label)
  {
    int value = atoi(lv_textarea_get_text(state->ta));
    if (value >= 0 && value < 10000)
    {
      *(state->current_var) = value;
      char buf[16];
      snprintf(buf, sizeof(buf), "%d", value);
      lv_label_set_text(state->current_label, buf);
    }
  }
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
  {
    // Always fully reset state
    lv_obj_add_flag(state->ta, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(state->kb, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(state->kb, NULL);
    state->current_var = nullptr;
    state->current_label = nullptr;
  }
}

struct NumberInputData
{
  int *target_var;
  lv_obj_t *ta;
  lv_obj_t *kb;
};
// Forward declaration
void create_number_input(lv_obj_t *parent, int *target_var, int initial_value, const char *label_text);

// static void slider_handle_change_cb(lv_event_t *e)
// {
//   SaveBtnData *data = (SaveBtnData *)lv_event_get_user_data(e);
//   int value = lv_slider_get_value(data->slider);
//   // Update label with current value
//   char buf[16];
//   snprintf(buf, sizeof(buf), "%d", value);
//   lv_label_set_text(data->lbl_current, buf);
// }

static void ta_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *textarea = lv_event_get_target_obj(e);
  lv_obj_t *keyboard = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED)
  {
    lv_keyboard_set_textarea(keyboard, textarea);
    lv_obj_remove_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  }

  if (code == LV_EVENT_DEFOCUSED)
  {
    lv_keyboard_set_textarea(keyboard, NULL);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  }
}

static void number_input_event_cb(lv_event_t *e)
{
  NumberInputData *data = (NumberInputData *)lv_event_get_user_data(e);
  lv_obj_t *ta = data->ta;
  lv_obj_t *kb = data->kb;
  lv_event_code_t code = lv_event_get_code(e);
  printf("[number_input_event_cb] Event code: %d\n", code);
  if (code == LV_EVENT_FOCUSED)
  {
    printf("[number_input_event_cb] Textarea focused, showing keyboard\n");
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(kb); // Bring keyboard to front
  }
  if (code == LV_EVENT_DEFOCUSED)
  {
    printf("[number_input_event_cb] Textarea defocused, hiding keyboard\n");
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    int value = atoi(lv_textarea_get_text(ta));
    printf("[number_input_event_cb] Value changed: %d\n", value);
    if (value >= 0 && value < 10000)
    {
      *(data->target_var) = value;
      printf("[number_input_event_cb] target_var updated to %d\n", value);
    }
  }
}
static const char *kb_map[] = {
    "7", "8", "9", "\n",
    "4", "5", "6", "\n",
    "1", "2", "3", "\n",
    LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_OK, NULL};
/*Set the relative width of the buttons and other controls*/
static const lv_buttonmatrix_ctrl_t kb_ctrl[] = {
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1,
    LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1, LV_BUTTONMATRIX_CTRL_WIDTH_1};

// Save button event callback
static void save_btn_event_cb(lv_event_t *e)
{
  // add save logic here
  printf("Save button clicked\n");
  player_store.putInt(KEY_LIFE_MAX, max_life);
  player_store.putInt(KEY_LIFE_STEP_SMALL, small_step);
  player_store.putInt(KEY_LIFE_STEP_LARGE, large_step);
  renderMenu(MENU_SETTINGS);
}

void renderLifeConfigScreen()
{
  teardownStartLifeScreen();
  max_life = player_store.getInt(KEY_LIFE_MAX, DEFAULT_LIFE_MAX);
  small_step = player_store.getInt(KEY_LIFE_STEP_SMALL, DEFAULT_LIFE_INCREMENT_SMALL);
  large_step = player_store.getInt(KEY_LIFE_STEP_LARGE, DEFAULT_LIFE_INCREMENT_LARGE);

  // Create the main menu object
  life_config_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(life_config_menu, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(life_config_menu, BLACK_COLOR, LV_PART_MAIN); // dark background
  lv_obj_set_style_bg_opa(life_config_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_opa(life_config_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(life_config_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_radius(life_config_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_pad_all(life_config_menu, 16, LV_PART_MAIN);

  // Define grid: 5 rows, 1 column
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {60, 50, 50, 50, 50, LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(life_config_menu, col_dsc, row_dsc);
  lv_obj_set_layout(life_config_menu, LV_LAYOUT_GRID);

  // Back button
  lv_obj_t *btn_back = lv_btn_create(life_config_menu);
  lv_obj_set_size(btn_back, 100, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { renderMenu(MENU_SETTINGS); }, LV_EVENT_CLICKED, NULL);

  // Life Max label
  lv_obj_t *lbl_life_max = lv_label_create(life_config_menu);
  lv_label_set_text(lbl_life_max, "Life Start");
  lv_obj_set_style_text_font(lbl_life_max, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lbl_life_max, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_life_max, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  // Life Max value button
  lv_obj_t *btn_max_life = lv_btn_create(life_config_menu);
  lv_obj_set_size(btn_max_life, 80, 40);
  lv_obj_set_grid_cell(btn_max_life, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_t *lbl_max_life_val = lv_label_create(btn_max_life);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", max_life);
  lv_label_set_text(lbl_max_life_val, buf);
  lv_obj_center(lbl_max_life_val);

  // Life Small Step label
  lv_obj_t *lbl_small_step = lv_label_create(life_config_menu);
  lv_label_set_text(lbl_small_step, "Small Step");
  lv_obj_set_style_text_font(lbl_small_step, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lbl_small_step, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_small_step, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  // Life Small Step value button
  lv_obj_t *btn_small_step = lv_btn_create(life_config_menu);
  lv_obj_set_size(btn_small_step, 80, 40);
  lv_obj_set_grid_cell(btn_small_step, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_t *lbl_small_step_val = lv_label_create(btn_small_step);
  snprintf(buf, sizeof(buf), "%d", small_step);
  lv_label_set_text(lbl_small_step_val, buf);
  lv_obj_center(lbl_small_step_val);

  // Life Large Step label
  lv_obj_t *lbl_large_step = lv_label_create(life_config_menu);
  lv_label_set_text(lbl_large_step, "Large Step");
  lv_obj_set_style_text_font(lbl_large_step, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lbl_large_step, lv_color_white(), 0);
  lv_obj_set_grid_cell(lbl_large_step, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);

  // Life Large Step value button
  lv_obj_t *btn_large_step = lv_btn_create(life_config_menu);
  lv_obj_set_size(btn_large_step, 80, 40);
  lv_obj_set_grid_cell(btn_large_step, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_t *lbl_large_step_val = lv_label_create(btn_large_step);
  snprintf(buf, sizeof(buf), "%d", large_step);
  lv_label_set_text(lbl_large_step_val, buf);
  lv_obj_center(lbl_large_step_val);
  static SharedInputState shared_input_state = {nullptr, nullptr, nullptr, nullptr};

  // Create shared text area and keyboard
  // Create shared text area above keyboard, not overlapping
  shared_input_state.ta = lv_textarea_create(life_config_menu);
  lv_textarea_set_one_line(shared_input_state.ta, true);
  lv_obj_set_style_text_font(shared_input_state.ta, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(shared_input_state.ta, lv_color_white(), 0);
  lv_obj_set_size(shared_input_state.ta, SCREEN_WIDTH - 80, 50);
  lv_obj_align(shared_input_state.ta, LV_ALIGN_TOP_MID, 0, 40);
  lv_obj_add_flag(shared_input_state.ta, LV_OBJ_FLAG_HIDDEN);

  // Create shared keyboard, docked below textarea
  shared_input_state.kb = lv_keyboard_create(life_config_menu);
  lv_keyboard_set_mode(shared_input_state.kb, LV_KEYBOARD_MODE_NUMBER);
  lv_obj_add_flag(shared_input_state.kb, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_map(shared_input_state.kb, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
  lv_keyboard_set_mode(shared_input_state.kb, LV_KEYBOARD_MODE_USER_1);
  lv_obj_set_size(shared_input_state.kb, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 150);
  lv_obj_align(shared_input_state.kb, LV_ALIGN_TOP_MID, 0, 90);

  // Attach user data to buttons
  lv_obj_set_user_data(btn_max_life, &max_life);
  lv_obj_set_user_data(btn_small_step, &small_step);
  lv_obj_set_user_data(btn_large_step, &large_step);

  lv_obj_add_event_cb(btn_max_life, open_input_cb, LV_EVENT_CLICKED, &shared_input_state);
  lv_obj_add_event_cb(btn_small_step, open_input_cb, LV_EVENT_CLICKED, &shared_input_state);
  lv_obj_add_event_cb(btn_large_step, open_input_cb, LV_EVENT_CLICKED, &shared_input_state);

  lv_obj_add_event_cb(shared_input_state.ta, shared_ta_event_cb, LV_EVENT_ALL, &shared_input_state);

  // Save button
  lv_obj_t *btn_save = lv_btn_create(life_config_menu);
  lv_obj_set_size(btn_save, 100, 60);
  lv_obj_set_grid_cell(btn_save, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 4, 1);
  lv_obj_t *lbl_save = lv_label_create(btn_save);
  lv_label_set_text(lbl_save, "Save");
  lv_obj_set_style_text_font(lbl_save, &lv_font_montserrat_20, 0);
  lv_obj_set_style_bg_color(btn_save, GREEN_COLOR, LV_PART_MAIN); // Green background
  lv_obj_center(lbl_save);
  lv_obj_add_event_cb(btn_save, save_btn_event_cb, LV_EVENT_CLICKED, NULL);
}

void teardownStartLifeScreen()
{
  if (life_config_menu)
  {
    lv_obj_del(life_config_menu);
    life_config_menu = nullptr;
  }
}