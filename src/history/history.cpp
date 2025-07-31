#include "history.h"
#include <lvgl.h>
#include <constants/constants.h>
#include <menu/menu.h>
#include <state/state_store.h>
#include <life/life_counter.h>
#include <life/life_counter2P.h>

extern lv_obj_t *history_menu;

void renderHistoryOverlay()
{
  teardownHistoryOverlay(); // Clean up previous overlay if it exists
  PlayerMode player_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
  std::vector<LifeHistoryEvent> history;
  if (player_mode == PLAYER_MODE_ONE_PLAYER)
  {
    history = event_grouper.getHistory();
  }
  else
  {
    // 2P mode: merge and sort both groupers
    std::vector<LifeHistoryEvent> h1 = event_grouper_p1.getHistory();
    std::vector<LifeHistoryEvent> h2 = event_grouper_p2.getHistory();
    history.reserve(h1.size() + h2.size());
    history.insert(history.end(), h1.begin(), h1.end());
    history.insert(history.end(), h2.begin(), h2.end());
    std::sort(history.begin(), history.end(), [](const LifeHistoryEvent &a, const LifeHistoryEvent &b)
              { return a.timestamp < b.timestamp; });
  }
  history_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(history_menu, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(history_menu, BLACK_COLOR, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(history_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_opa(history_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(history_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_radius(history_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_scrollbar_mode(history_menu, LV_SCROLLBAR_MODE_OFF);
  lv_obj_remove_flag(history_menu, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling

  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {60, SCREEN_HEIGHT - 140, LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(history_menu, col_dsc, row_dsc);

  // Back button (row 0)
  lv_obj_t *btn_back = lv_btn_create(history_menu);
  lv_obj_set_size(btn_back, 100, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_width(btn_back, 2, LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);

  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { renderMenu(MENU_CONTEXTUAL, false); }, LV_EVENT_CLICKED, NULL);
  // Table
  lv_obj_t *table = lv_table_create(history_menu);
  // Place table in col 0, row 1, spanning 1 col and 1 row, stretched to fill cell
  lv_obj_set_grid_cell(table, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
  lv_obj_set_style_radius(table, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_border_opa(table, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(table, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_bg_color(table, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(table, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_scroll_dir(table, LV_DIR_VER);
  if (player_mode == PLAYER_MODE_ONE_PLAYER)
  {
    lv_table_set_col_cnt(table, 1);
    lv_table_set_row_cnt(table, 1); // Will grow as we add rows
    lv_table_set_col_width(table, 0, SCREEN_WIDTH);
    lv_table_set_cell_value(table, 0, 0, "Life Events");
  }
  else
  {
    lv_table_set_col_cnt(table, 2);
    lv_table_set_row_cnt(table, 1); // Will grow as we add rows
    lv_table_set_col_width(table, 0, SCREEN_WIDTH / 2);
    lv_table_set_col_width(table, 1, SCREEN_WIDTH / 2);
    lv_table_set_cell_value(table, 0, 0, "P1");
    lv_table_set_cell_value(table, 0, 1, "P2");
  }
  lv_obj_set_style_text_font(table, &lv_font_montserrat_18, LV_PART_ITEMS | LV_STATE_DEFAULT); // Header font

  // Custom draw event for styling
  auto draw_event_cb = [](lv_event_t *e)
  {
    lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);
    if (base_dsc->part == LV_PART_ITEMS)
    {
      uint32_t row = base_dsc->id1;
      uint32_t col = base_dsc->id2;
      // Center align header
      if (row == 0)
      {
        lv_draw_label_dsc_t *label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
        if (label_draw_dsc)
          label_draw_dsc->align = LV_TEXT_ALIGN_CENTER;
        lv_draw_fill_dsc_t *fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
        if (fill_draw_dsc)
        {
          fill_draw_dsc->color = lv_color_black();
          fill_draw_dsc->opa = LV_OPA_COVER;
        }
      }
      // Center align both columns
      else if (col == 0 || col == 1)
      {
        lv_draw_label_dsc_t *label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
        if (label_draw_dsc)
          label_draw_dsc->align = LV_TEXT_ALIGN_CENTER;
      }
      // Alternate row color
      if ((row != 0 && row % 2) == 0)
      {
        lv_draw_fill_dsc_t *fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
        if (fill_draw_dsc)
        {
          fill_draw_dsc->color = DARK_GRAY_COLOR;
          fill_draw_dsc->opa = LV_OPA_COVER;
        }
      }
    }
  };
  lv_obj_add_event_cb(table, draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
  lv_obj_add_flag(table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

  // Build table rows
  for (size_t i = 0; i < history.size(); ++i)
  {
    const LifeHistoryEvent &evt = history[i];
    // format the time
    char time_buf[32];
    printf("[renderHistoryOverlay] Event %zu: player_id=%d, timestamp=%u\n", i, evt.player_id, evt.timestamp);
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d", evt.change_timestamp / 60, evt.change_timestamp % 60);
    int life_change = evt.net_life_change;
    int life_total = evt.life_total;
    char buf[64] = "";
    size_t row_idx = i + 1; // row 0 is header
    if (player_mode == PLAYER_MODE_ONE_PLAYER && evt.player_id == PLAYER_SINGLE)
    {
      if (life_change > 0)
        snprintf(buf, sizeof(buf), "+%d@%s[%d]", life_change, time_buf, life_total);
      else if (life_change <= 0)
        snprintf(buf, sizeof(buf), "%d@%s[%d]", life_change, time_buf, life_total);
      lv_table_set_row_cnt(table, row_idx + 1);
      lv_table_set_cell_value(table, row_idx, 0, buf);
    }
    else
    {
      // Two player mode, two columns
      char p1_buf[64] = "";
      char p2_buf[64] = "";
      if (life_change > 0)
        snprintf(evt.player_id == PLAYER_ONE ? p1_buf : p2_buf, sizeof(p1_buf), "+%d@%s[%d]", life_change, time_buf, life_total);
      else if (life_change <= 0)
        snprintf(evt.player_id == PLAYER_ONE ? p1_buf : p2_buf, sizeof(p1_buf), "%d@%s[%d]", life_change, time_buf, life_total);
      lv_table_set_row_cnt(table, row_idx + 1);
      lv_table_set_cell_value(table, row_idx, 0, p1_buf);
      lv_table_set_cell_value(table, row_idx, 1, p2_buf);
    }
  }
}

void teardownHistoryOverlay()
{
  if (history_menu)
  {
    lv_obj_del(history_menu);
    history_menu = nullptr;
  }
}