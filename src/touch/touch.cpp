#include <lvgl.h>
#include "gui_main.h"
#include "gestures/gestures.h"
#include "esp_display_panel.hpp"

extern esp_panel::board::Board *board;

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
  if (!board->getTouch())
  {
    data->state = LV_INDEV_STATE_REL;
    return;
  }
  std::vector<esp_panel::drivers::TouchPoint> points;
  board->getTouch()->readRawData(1, 0, 1); // Reduced timeout from 10ms to 1ms
  bool isSuccess = board->getTouch()->getPoints(points);
  if (isSuccess && points.size() > 0)
  {
    // printf("[touch_read_cb] point=(%d, %d)\n", points[0].x, points[0].y);
    data->point.x = points[0].x;
    data->point.y = points[0].y;
    data->state = LV_INDEV_STATE_PR;
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

void init_touch()
{
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touch_read_cb);
}