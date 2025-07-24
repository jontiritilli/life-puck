#include "history.h"
#include <lvgl.h>
#include <constants/constants.h>
#include <menu/menu.h>
#include <state/state_store.h>
#include <life/life_counter.h>
#include <life/life_counter2P.h>

void renderHistoryOverlay()
{
  int player_mode = player_store.getInt(KEY_PLAYER_MODE, 0);
  std::vector<LifeHistoryEvent> history;
  if (player_mode == 1)
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

  extern lv_obj_t *history_menu;
  history_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(history_menu, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(history_menu, BLACK_COLOR, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(history_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_opa(history_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(history_menu, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_radius(history_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_scrollbar_mode(history_menu, LV_SCROLLBAR_MODE_OFF);
  lv_obj_remove_flag(history_menu, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling

  // Back button
  lv_obj_t *btn_back = lv_btn_create(history_menu);
  lv_obj_set_size(btn_back, 80, 40);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_width(btn_back, 2, LV_PART_MAIN);
  lv_obj_align(btn_back, LV_ALIGN_TOP_MID, 0, 5);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_16, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e)
                      { renderMenu(MENU_CONTEXTUAL); }, LV_EVENT_CLICKED, NULL);
  // Table
  lv_obj_t *table = lv_table_create(history_menu);
  lv_obj_set_size(table, SCREEN_WIDTH, SCREEN_HEIGHT - 120);
  lv_obj_align(table, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_set_style_radius(table, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_border_opa(table, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(table, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_bg_color(table, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(table, LV_OPA_COVER, LV_PART_MAIN);
  if (player_mode == 1)
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
  lv_obj_set_style_text_font(table, &lv_font_montserrat_20, LV_PART_ITEMS | LV_STATE_DEFAULT); // Header font

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
  int row = 1;
  for (size_t i = 0; i < history.size(); ++i)
  {
    const LifeHistoryEvent &evt = history[i];
    uint32_t minutes_since_boot = evt.timestamp / 60000;
    int life_change = evt.net_life_change;
    int life_total = evt.life_total;
    char buf[64] = "";
    if (player_mode == 1 && evt.player_id == 0)
    {
      // Single player mode, only one column
      if (life_change > 0)
        snprintf(buf, sizeof(buf), "+%d @ %lum [%d]", life_change, (unsigned long)minutes_since_boot, life_total);
      else if (life_change < 0)
        snprintf(buf, sizeof(buf), "%d @ %lum [%d]", life_change, (unsigned long)minutes_since_boot, life_total);
      else
        snprintf(buf, sizeof(buf), "0 @ %lum [%d]", (unsigned long)minutes_since_boot, life_total);
      lv_table_set_row_cnt(table, row + 1);
      lv_table_set_cell_value(table, row, 0, buf);
    }
    else if (player_mode != 1)
    {
      // Two player mode, two columns
      char p1_buf[64] = "";
      char p2_buf[64] = "";
      if (evt.player_id == 1)
      {
        if (life_change > 0)
          snprintf(p1_buf, sizeof(p1_buf), "+%d @ %lum [%d]", life_change, (unsigned long)minutes_since_boot, life_total);
        else if (life_change < 0)
          snprintf(p1_buf, sizeof(p1_buf), "%d @ %lum [%d]", life_change, (unsigned long)minutes_since_boot, life_total);
        else
          snprintf(p1_buf, sizeof(p1_buf), "0 @ %lum [%d]", (unsigned long)minutes_since_boot, life_total);
      }
      if (evt.player_id == 2)
      {
        if (life_change > 0)
          snprintf(p2_buf, sizeof(p2_buf), "+%d @ %lum [%d]", life_change, (unsigned long)minutes_since_boot, life_total);
        else if (life_change < 0)
          snprintf(p2_buf, sizeof(p2_buf), "%d @ %lum [%d]", life_change, (unsigned long)minutes_since_boot, life_total);
        else
          snprintf(p2_buf, sizeof(p2_buf), "0 @ %lum [%d]", (unsigned long)minutes_since_boot, life_total);
      }
      lv_table_set_row_cnt(table, row + 1);
      lv_table_set_cell_value(table, row, 0, p1_buf);
      lv_table_set_cell_value(table, row, 1, p2_buf);
    }
    row++;
  }
}