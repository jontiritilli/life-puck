#pragma once
#include <vector>
#include <stdint.h>
#include <functional>
#include <Arduino.h>
#include <timer/timer.h>

struct LifeHistoryEvent
{
  int net_life_change;
  int life_total;
  int player_id;        // 0 for single, 1/2 for 2P
  uint32_t timestamp;   // Time since boot
  int change_timestamp; // seconds since game started
};

class EventGrouper
{
public:
  EventGrouper(uint32_t window_ms = 1000, int initial_life = 0, int player_id = 0)
      : grouping_window(window_ms),
        active(false),
        net_change(0),
        player_id(player_id),
        last_event_time(0),
        life_total(initial_life),
        group_start_time(0),
        commit_callback(nullptr),
        change_timestamp(0)
  {
  }

  // Returns true if a group is active and waiting for commit
  bool isCommitPending() const
  {
    return active;
  }

  // Returns the current pending net change (0 if inactive)
  int getPendingChange() const
  {
    printf("[EventGrouper] getPendingChange: active=%d, net_change=%d\n, player_id=%d", active, net_change, player_id);
    return net_change;
  }

  int getLifeTotal() const
  {
    printf("[EventGrouper] getLifeTotal: life_total=%d\n, player_id=%d", life_total, player_id);
    return life_total;
  }

  // Call this for each life change (tap/swipe)
  void handleChange(int player, int change, uint64_t game_timestamp, std::function<void(const LifeHistoryEvent &)> onCommit)
  {
    if (!get_is_timer_running())
    {
      toggle_timer_running(); // Ensure timer is running
    }
    commit_callback = onCommit;
    uint32_t now = millis();
    last_event_time = now;
    net_change += change;
    change_timestamp = game_timestamp;
    if (!active)
    {
      // Start new group
      active = true;
      group_start_time = now;
    }
  }

  // Call this periodically (e.g., in loop) to check for timeout
  void loop()
  {
    uint32_t now = millis(); // MS since boot
    bool isWindowExpired = (now - last_event_time) > grouping_window;
    if (active && isWindowExpired && net_change != 0)
    {
      // printf("[EventGrouper] update: committing due to rolling window timeout\n");
      int new_life_total = life_total + net_change;
      LifeHistoryEvent evt{net_change, new_life_total, player_id, last_event_time, change_timestamp};
      history.push_back(evt);
      life_total = new_life_total; // Update state to latest committed value
      if (commit_callback)
        commit_callback(evt);
      // Clear commit pending state immediately after callback
      active = false;
      net_change = 0;
      commit_callback = nullptr; // Clear callback to avoid dangling reference
      printf("[EventGrouper] commit() exit: life_total=%d, active=%d, net_change=%d\n", life_total, active, net_change);
    }
  }

  // Access history
  std::vector<LifeHistoryEvent> getHistory()
  {
    printf("[EventGrouper] getHistory: size=%zu\n", history.size());

    return history;
  }

  // Helper: Reset history
  void resetHistory(int base_life)
  {
    // printf("[EventGrouper] Before reset: history size=%zu, active=%d, net_change=%d, player_id=%d, group_start_time=%u, last_event_time=%u, life_total=%d\n", history.size(), active, net_change, player_id, group_start_time, last_event_time, life_total);
    history.clear();
    active = false;
    net_change = 0;
    group_start_time = 0;
    last_event_time = 0;
    change_timestamp = 0;
    life_total = base_life;
    // printf("[EventGrouper] After reset: history size=%zu, active=%d, net_change=%d, player_id=%d, group_start_time=%u, last_event_time=%u, life_total=%d\n", history.size(), active, net_change, player_id, group_start_time, last_event_time, life_total);
  }

private:
  uint32_t grouping_window;
  bool active;
  int net_change;
  int player_id;
  int life_total;
  uint32_t group_start_time;
  uint32_t last_event_time;
  int change_timestamp;
  std::vector<LifeHistoryEvent> history;
  std::function<void(const LifeHistoryEvent &)> commit_callback;
};
