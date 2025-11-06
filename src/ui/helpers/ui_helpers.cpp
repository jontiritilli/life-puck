// ============================================
// Own Header (first!)
// ============================================
#include "ui_helpers.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"
#include "data/themes.h"

lv_obj_t *ui_create_back_button(lv_obj_t *parent, int row, int col, lv_event_cb_t callback)
{
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_set_size(btn, UI_BTN_WIDTH_STANDARD, UI_BTN_HEIGHT_STANDARD);
  lv_obj_set_style_bg_color(btn, UI_COLOR_BACK_BUTTON, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, col, 1, LV_GRID_ALIGN_CENTER, row, 1);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, UI_TEXT_BACK);
  lv_obj_set_style_text_font(label, UI_FONT_BUTTON, 0);
  lv_obj_set_style_text_color(label, UI_COLOR_BACK_TEXT, 0);
  lv_obj_center(label);

  if (callback)
  {
    lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, NULL);
  }

  return btn;
}

lv_obj_t *ui_create_title(lv_obj_t *parent, const char *text, int row, int col_span)
{
  lv_obj_t *title = lv_label_create(parent);
  lv_label_set_text(title, text);
  lv_obj_set_style_text_color(title, UI_COLOR_TITLE, 0);
  lv_obj_set_style_text_font(title, UI_FONT_TITLE, 0);
  lv_obj_set_grid_cell(title, LV_GRID_ALIGN_CENTER, 0, col_span, LV_GRID_ALIGN_CENTER, row, 1);

  return title;
}

lv_obj_t *ui_create_button(lv_obj_t *parent, const char *text, int row, int col, lv_event_cb_t callback)
{
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_set_size(btn, UI_BTN_WIDTH_STANDARD, UI_BTN_HEIGHT_STANDARD);
  lv_obj_set_style_bg_color(btn, getThemeButtonColor(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, col, 1, LV_GRID_ALIGN_CENTER, row, 1);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, UI_FONT_BUTTON, 0);
  lv_obj_set_style_text_color(label, getThemeTextColor(), 0);
  lv_obj_center(label);

  if (callback)
  {
    lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, NULL);
  }

  return btn;
}

lv_obj_t *ui_create_toggle_button(lv_obj_t *parent, const char *text_on, const char *text_off,
                                  bool is_on, int row, int col, lv_event_cb_t callback)
{
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_set_size(btn, UI_BTN_WIDTH_STANDARD, UI_BTN_HEIGHT_STANDARD);
  lv_obj_set_style_bg_color(btn, (is_on ? getThemeButtonColor() : UI_COLOR_DISABLED), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, col, 1, LV_GRID_ALIGN_CENTER, row, 1);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, is_on ? text_on : text_off);
  lv_obj_set_style_text_font(label, UI_FONT_TOGGLE, 0);
  lv_obj_set_style_text_color(label, (is_on ? getThemeTextColor() : UI_COLOR_TEXT_PRIMARY), 0);
  lv_obj_center(label);

  if (callback)
  {
    lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, NULL);
  }

  return btn;
}

lv_obj_t *ui_create_wide_button(lv_obj_t *parent, const char *text, int row, lv_event_cb_t callback)
{
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_set_size(btn, UI_BTN_WIDTH_WIDE, UI_BTN_HEIGHT_STANDARD);
  lv_obj_set_style_bg_color(btn, getThemeButtonColor(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, row, 1);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, UI_FONT_BUTTON, 0);
  lv_obj_set_style_text_color(label, getThemeTextColor(), 0);
  lv_obj_center(label);

  if (callback)
  {
    lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, NULL);
  }

  return btn;
}

lv_obj_t *ui_create_wide_toggle_button(lv_obj_t *parent, const char *text_on, const char *text_off,
                                       bool is_on, int row, lv_event_cb_t callback)
{
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_set_size(btn, UI_BTN_WIDTH_WIDE, UI_BTN_HEIGHT_STANDARD);
  lv_obj_set_style_bg_color(btn, (is_on ? getThemeButtonColor() : UI_COLOR_DISABLED), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, row, 1);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, is_on ? text_on : text_off);
  lv_obj_set_style_text_font(label, UI_FONT_TOGGLE, 0);
  lv_obj_set_style_text_color(label, (is_on ? getThemeTextColor() : UI_COLOR_TEXT_PRIMARY), 0);
  lv_obj_center(label);

  if (callback)
  {
    lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, NULL);
  }

  return btn;
}

void ui_setup_grid_layout(lv_obj_t *parent, int row_count)
{
  // Create column descriptors (2 equal columns)
  static lv_coord_t col_dsc[] = {UI_BTN_WIDTH_STANDARD, UI_BTN_WIDTH_STANDARD, LV_GRID_TEMPLATE_LAST};

  // Create row descriptors (dynamic based on row_count)
  static lv_coord_t row_dsc[20]; // Max 20 rows
  for (int i = 0; i < row_count && i < 19; i++)
  {
    row_dsc[i] = UI_GRID_ROW_HEIGHT;
  }
  row_dsc[row_count < 19 ? row_count : 19] = LV_GRID_TEMPLATE_LAST;

  lv_obj_set_style_grid_column_dsc_array(parent, col_dsc, 0);
  lv_obj_set_style_grid_row_dsc_array(parent, row_dsc, 0);
  lv_obj_set_layout(parent, LV_LAYOUT_GRID);
  lv_obj_set_style_pad_column(parent, UI_GRID_GAP, 0);
  lv_obj_set_style_pad_row(parent, UI_GRID_GAP, 0);
}

lv_obj_t *ui_create_menu_container(lv_obj_t *parent)
{
  lv_obj_t *menu = lv_obj_create(parent);
  lv_obj_set_size(menu, UI_MENU_WIDTH, UI_MENU_HEIGHT);
  lv_obj_center(menu);
  lv_obj_set_style_bg_color(menu, UI_MENU_BG_COLOR, 0);
  lv_obj_set_style_radius(menu, UI_MENU_RADIUS, 0);
  lv_obj_set_style_border_width(menu, 0, 0);
  lv_obj_set_scrollbar_mode(menu, LV_SCROLLBAR_MODE_AUTO);

  return menu;
}
