// ============================================
// Own Header (first!)
// ============================================
#include "history.h"

// ============================================
// System & Framework Headers
// ============================================
#include <lvgl.h>

// ============================================
// Core System
// ============================================
#include "core/state_manager.h"

// ============================================
// UI Screens
// ============================================
#include "ui/screens/life/life_counter.h"
#include "ui/screens/life/life_counter_two_player.h"
#include "ui/screens/menu/menu.h"

// ============================================
// Data Layer
// ============================================
#include "data/constants.h"

extern lv_obj_t *history_menu;

// ============================================
// HELPER FUNCTION: Format Relative Timestamp
// ============================================
// Converts millis() timestamp to relative time string
// Examples: "now", "2m", "15m", "1h", "3d"
static void formatTimestamp(char* buf, size_t bufSize, uint32_t timestamp_millis) {
  uint32_t now = millis();
  
  // Handle millis() overflow (after ~49 days uptime)
  uint32_t elapsed_ms;
  if (now >= timestamp_millis) {
    elapsed_ms = now - timestamp_millis;
  } else {
    // Overflow happened, calculate correct difference
    elapsed_ms = (0xFFFFFFFF - timestamp_millis) + now;
  }
  
  uint32_t elapsed_seconds = elapsed_ms / 1000;
  
  // Format based on age
  if (elapsed_seconds < 60) {
    // Less than 1 minute
    snprintf(buf, bufSize, "now");
  } else if (elapsed_seconds < 3600) {
    // Less than 1 hour: show minutes
    uint32_t mins = elapsed_seconds / 60;
    snprintf(buf, bufSize, "%dm", mins);
  } else if (elapsed_seconds < 86400) {
    // Less than 1 day: show hours
    uint32_t hours = elapsed_seconds / 3600;
    snprintf(buf, bufSize, "%dh", hours);
  } else {
    // More than 1 day: show days
    uint32_t days = elapsed_seconds / 86400;
    snprintf(buf, bufSize, "%dd", days);
  }
}

// ============================================
// MAIN RENDER FUNCTION
// ============================================
void renderHistoryOverlay()
{
  teardownHistoryOverlay();
  PlayerMode player_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
  
  // ========== FULLSCREEN OVERLAY ==========
  history_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(history_menu, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(history_menu, BLACK_COLOR, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(history_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_radius(history_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_pad_all(history_menu, 0, LV_PART_MAIN);
  
  // ========== BACK BUTTON ==========
  lv_obj_t *btn_back = lv_btn_create(history_menu);
  lv_obj_set_size(btn_back, 120, 50);
  lv_obj_align(btn_back, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_radius(btn_back, 8, LV_PART_MAIN);
  
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_center(lbl_back);
  
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e) { 
    renderMenu(MENU_CONTEXTUAL, false); 
  }, LV_EVENT_CLICKED, NULL);
  
  // ========== SCROLLABLE CONTAINER ==========
  lv_obj_t *scroll_container = lv_obj_create(history_menu);
  lv_obj_set_size(scroll_container, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 80);
  lv_obj_align(scroll_container, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_bg_color(scroll_container, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_border_width(scroll_container, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(scroll_container, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_row(scroll_container, 6, LV_PART_MAIN);
  
  // Flex Layout: Vertical stacking
  lv_obj_set_layout(scroll_container, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(scroll_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(scroll_container, 
                        LV_FLEX_ALIGN_START,     // Top to bottom
                        LV_FLEX_ALIGN_CENTER,    // Horizontal center
                        LV_FLEX_ALIGN_CENTER);   // Center in track
  
  // Scrolling
  lv_obj_set_scroll_dir(scroll_container, LV_DIR_VER);
  lv_obj_add_flag(scroll_container, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_add_flag(scroll_container, LV_OBJ_FLAG_SCROLL_MOMENTUM);
  lv_obj_set_scrollbar_mode(scroll_container, LV_SCROLLBAR_MODE_AUTO);
  lv_obj_set_style_width(scroll_container, 4, LV_PART_SCROLLBAR); // LVGL 9.3: Style-based width
  
  // ========== TWO PLAYER MODE ==========
  if (player_mode == PLAYER_MODE_TWO_PLAYER) {
    // Header Row (P1 | P2)
    lv_obj_t *header_row = lv_obj_create(scroll_container);
    lv_obj_set_width(header_row, SCREEN_WIDTH - 40);
    lv_obj_set_height(header_row, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(header_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(header_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(header_row, 8, LV_PART_MAIN);
    
    lv_obj_set_layout(header_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(header_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // P1 Header
    lv_obj_t *lbl_p1_header = lv_label_create(header_row);
    lv_label_set_text(lbl_p1_header, "P1");
    lv_obj_set_width(lbl_p1_header, 150);
    lv_obj_set_style_text_color(lbl_p1_header, lv_color_white(), 0);
    lv_obj_set_style_text_font(lbl_p1_header, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_align(lbl_p1_header, LV_TEXT_ALIGN_CENTER, 0);
    
    // P2 Header
    lv_obj_t *lbl_p2_header = lv_label_create(header_row);
    lv_label_set_text(lbl_p2_header, "P2");
    lv_obj_set_width(lbl_p2_header, 150);
    lv_obj_set_style_text_color(lbl_p2_header, lv_color_white(), 0);
    lv_obj_set_style_text_font(lbl_p2_header, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_align(lbl_p2_header, LV_TEXT_ALIGN_CENTER, 0);
    
    // Get histories
    std::vector<LifeHistoryEvent> h1 = event_grouper_p1.getHistory();
    std::vector<LifeHistoryEvent> h2 = event_grouper_p2.getHistory();
    size_t max_rows = (h1.size() > h2.size()) ? h1.size() : h2.size();
    
    // Render rows
    for (size_t i = 0; i < max_rows; ++i) {
      lv_obj_t *row = lv_obj_create(scroll_container);
      lv_obj_set_width(row, SCREEN_WIDTH - 40);
      lv_obj_set_height(row, LV_SIZE_CONTENT);
      lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
      lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
      lv_obj_set_style_pad_all(row, 2, LV_PART_MAIN);
      
      lv_obj_set_layout(row, LV_LAYOUT_FLEX);
      lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
      lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
      
      // ===== P1 LABEL =====
      lv_obj_t *lbl_p1 = lv_label_create(row);
      lv_obj_set_width(lbl_p1, 150);
      lv_obj_set_style_text_align(lbl_p1, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_style_text_font(lbl_p1, &lv_font_montserrat_16, 0);
      lv_label_set_long_mode(lbl_p1, LV_LABEL_LONG_CLIP);
      
      // Background box (same as Single Player)
      lv_obj_set_style_bg_opa(lbl_p1, LV_OPA_20, LV_PART_MAIN);
      lv_obj_set_style_bg_color(lbl_p1, lv_color_white(), LV_PART_MAIN);
      lv_obj_set_style_radius(lbl_p1, 6, LV_PART_MAIN);
      lv_obj_set_style_pad_top(lbl_p1, 6, 0);
      lv_obj_set_style_pad_bottom(lbl_p1, 6, 0);
      lv_obj_set_style_pad_left(lbl_p1, 4, 0);
      lv_obj_set_style_pad_right(lbl_p1, 4, 0);
      
      if (i < h1.size()) {
        const LifeHistoryEvent &evt = h1[i];
        char buf[64];
        char time_buf[16];
        formatTimestamp(time_buf, sizeof(time_buf), evt.timestamp);
        
        if (evt.net_life_change > 0) {
          snprintf(buf, sizeof(buf), LV_SYMBOL_UP " +%d [%d]\n%s", 
                   evt.net_life_change, evt.life_total, time_buf);
          lv_obj_set_style_text_color(lbl_p1, lv_color_hex(0x00e31f), 0);
        } else {
          snprintf(buf, sizeof(buf), LV_SYMBOL_DOWN " %d [%d]\n%s", 
                   evt.net_life_change, evt.life_total, time_buf);
          lv_obj_set_style_text_color(lbl_p1, lv_color_hex(0xe80000), 0);
        }
        lv_label_set_text(lbl_p1, buf);
      } else {
        lv_label_set_text(lbl_p1, "");
      }
      
      // ===== P2 LABEL =====
      lv_obj_t *lbl_p2 = lv_label_create(row);
      lv_obj_set_width(lbl_p2, 150);
      lv_obj_set_style_text_align(lbl_p2, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_style_text_font(lbl_p2, &lv_font_montserrat_16, 0);
      lv_label_set_long_mode(lbl_p2, LV_LABEL_LONG_CLIP);
      
      // Background box (same as Single Player)
      lv_obj_set_style_bg_opa(lbl_p2, LV_OPA_20, LV_PART_MAIN);
      lv_obj_set_style_bg_color(lbl_p2, lv_color_white(), LV_PART_MAIN);
      lv_obj_set_style_radius(lbl_p2, 6, LV_PART_MAIN);
      lv_obj_set_style_pad_top(lbl_p2, 6, 0);
      lv_obj_set_style_pad_bottom(lbl_p2, 6, 0);
      lv_obj_set_style_pad_left(lbl_p2, 4, 0);
      lv_obj_set_style_pad_right(lbl_p2, 4, 0);
      
      if (i < h2.size()) {
        const LifeHistoryEvent &evt = h2[i];
        char buf[64];
        char time_buf[16];
        formatTimestamp(time_buf, sizeof(time_buf), evt.timestamp);
        
        if (evt.net_life_change > 0) {
          snprintf(buf, sizeof(buf), LV_SYMBOL_UP " +%d [%d]\n%s", 
                   evt.net_life_change, evt.life_total, time_buf);
          lv_obj_set_style_text_color(lbl_p2, lv_color_hex(0x00e31f), 0);
        } else {
          snprintf(buf, sizeof(buf), LV_SYMBOL_DOWN " %d [%d]\n%s", 
                   evt.net_life_change, evt.life_total, time_buf);
          lv_obj_set_style_text_color(lbl_p2, lv_color_hex(0xe80000), 0);
        }
        lv_label_set_text(lbl_p2, buf);
      } else {
        lv_label_set_text(lbl_p2, "");
      }
    }
    
  // ========== SINGLE PLAYER MODE ==========
  } else {
    // Header
    lv_obj_t *lbl_header = lv_label_create(scroll_container);
    lv_label_set_text(lbl_header, "History");
    lv_obj_set_width(lbl_header, SCREEN_WIDTH - 40);
    lv_obj_set_style_text_color(lbl_header, lv_color_white(), 0);
    lv_obj_set_style_text_font(lbl_header, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_align(lbl_header, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_bottom(lbl_header, 12, 0);
    
    std::vector<LifeHistoryEvent> history = event_grouper.getHistory();
    
    // Render entries
    for (size_t i = 0; i < history.size(); ++i) {
      const LifeHistoryEvent &evt = history[i];
      
      lv_obj_t *lbl_entry = lv_label_create(scroll_container);
      lv_obj_set_width(lbl_entry, SCREEN_WIDTH - 60);
      lv_obj_set_style_text_align(lbl_entry, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_style_text_font(lbl_entry, &lv_font_montserrat_20, 0);
      lv_label_set_long_mode(lbl_entry, LV_LABEL_LONG_CLIP);
      
      // Semi-transparent background
      lv_obj_set_style_bg_opa(lbl_entry, LV_OPA_20, LV_PART_MAIN);
      lv_obj_set_style_bg_color(lbl_entry, lv_color_white(), LV_PART_MAIN);
      lv_obj_set_style_radius(lbl_entry, 6, LV_PART_MAIN);
      lv_obj_set_style_pad_top(lbl_entry, 8, 0);
      lv_obj_set_style_pad_bottom(lbl_entry, 8, 0);
      
      char buf[64];
      char time_buf[16];
      formatTimestamp(time_buf, sizeof(time_buf), evt.timestamp);
      
      if (evt.net_life_change > 0) {
        snprintf(buf, sizeof(buf), LV_SYMBOL_UP " +%d [%d]  %s", 
                 evt.net_life_change, evt.life_total, time_buf);
        lv_obj_set_style_text_color(lbl_entry, lv_color_hex(0x00e31f), 0);
      } else {
        snprintf(buf, sizeof(buf), LV_SYMBOL_DOWN " %d [%d]  %s", 
                 evt.net_life_change, evt.life_total, time_buf);
        lv_obj_set_style_text_color(lbl_entry, lv_color_hex(0xe80000), 0);
      }
      lv_label_set_text(lbl_entry, buf);
    }
  }
}

void teardownHistoryOverlay() {
  if (history_menu) {
    lv_obj_del(history_menu);
    history_menu = nullptr;
  }
}
