#ifndef SIMPLE_COUNTERS_H
#define SIMPLE_COUNTERS_H

#include "config.h"
#include <lvgl.h>

struct SimpleCounter {
    int slot;
    int value;
    bool enabled;
    lv_obj_t* container;
    lv_obj_t* circle;
    lv_obj_t* label;
    uint32_t press_start_time;
};

class SimpleCounters {
public:
    static SimpleCounter counters[SIMPLE_COUNTER_COUNT];
    
    static void init();
    static void shutdown();
    static void enable_counter(int slot);
    static void disable_counter(int slot);
    static void increment(int slot);
    static void decrement(int slot);
    static void reset(int slot);
    static int get_value(int slot);
    static bool is_enabled(int slot);
    static void save_to_nvs(int slot);
    static void load_from_nvs(int slot);
    static void save_all();
    static void load_all_from_nvs();
    static void create_counter_ui(int slot);
    static void destroy_counter_ui(int slot);
    static void update_display(int slot);
    static void create_all_enabled_ui();
    static void reset_all_counters();
};

#endif // SIMPLE_COUNTERS_H
