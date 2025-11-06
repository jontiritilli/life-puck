#pragma once
#include <lvgl.h>
#include "ui_constants.h"

#ifdef __cplusplus
extern "C"
{
#endif

  // ============================================================================
  // UI HELPER FUNCTIONS
  // ============================================================================
  // Standard functions for creating consistent UI elements across all menus
  // ============================================================================

  /**
   * @brief Create a standardized back button
   * @param parent Parent container
   * @param row Grid row position (usually UI_BACK_BTN_ROW)
   * @param col Grid column position (usually UI_BACK_BTN_COL)
   * @param callback Event callback function
   * @return Pointer to created button object
   */
  lv_obj_t *ui_create_back_button(lv_obj_t *parent, int row, int col, lv_event_cb_t callback);

  /**
   * @brief Create a standardized title label
   * @param parent Parent container
   * @param text Title text
   * @param row Grid row position (usually UI_TITLE_ROW)
   * @param col Grid column position (default 0)
   * @return Pointer to created label object
   */
  lv_obj_t *ui_create_title(lv_obj_t *parent, const char *text, int row, int col_span = 1);

  /**
   * @brief Create a standardized button with theme colors
   * @param parent Parent container
   * @param text Button label text
   * @param row Grid row position
   * @param col Grid column position
   * @param callback Event callback function
   * @return Pointer to created button object
   */
  lv_obj_t *ui_create_button(lv_obj_t *parent, const char *text, int row, int col, lv_event_cb_t callback);

  /**
   * @brief Create a standardized toggle button (ON/OFF state)
   * @param parent Parent container
   * @param text_on Text when toggle is ON
   * @param text_off Text when toggle is OFF
   * @param is_on Initial state (true = ON, false = OFF)
   * @param row Grid row position
   * @param col Grid column position
   * @param callback Event callback function
   * @return Pointer to created button object
   */
  lv_obj_t *ui_create_toggle_button(lv_obj_t *parent, const char *text_on, const char *text_off,
                                    bool is_on, int row, int col, lv_event_cb_t callback);

  /**
   * @brief Create a standardized wide button (spans 2 columns)
   * @param parent Parent container
   * @param text Button label text
   * @param row Grid row position
   * @param callback Event callback function
   * @return Pointer to created button object
   */
  lv_obj_t *ui_create_wide_button(lv_obj_t *parent, const char *text, int row, lv_event_cb_t callback);

  /**
   * @brief Create a standardized wide toggle button (spans 2 columns, ON/OFF state)
   * @param parent Parent container
   * @param text_on Text when toggle is ON
   * @param text_off Text when toggle is OFF
   * @param is_on Initial state (true = ON, false = OFF)
   * @param row Grid row position
   * @param callback Event callback function
   * @return Pointer to created button object
   */
  lv_obj_t *ui_create_wide_toggle_button(lv_obj_t *parent, const char *text_on, const char *text_off,
                                         bool is_on, int row, lv_event_cb_t callback);

  /**
   * @brief Setup standard 2-column grid layout for menu
   * @param parent Parent container
   * @param row_count Number of rows in the grid
   */
  void ui_setup_grid_layout(lv_obj_t *parent, int row_count);

  /**
   * @brief Create a standardized menu container
   * @param parent Parent screen
   * @return Pointer to created menu container
   */
  lv_obj_t *ui_create_menu_container(lv_obj_t *parent);

#ifdef __cplusplus
}
#endif
