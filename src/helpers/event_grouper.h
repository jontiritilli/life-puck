#pragma once
#include <vector>
#include <stdint.h>
#include <functional>
#include <Arduino.h>

struct LifeHistoryEvent
{
  int net_life_change;
  int life_total;
  int player_id;      // 0 for single, 1/2 for 2P
  uint32_t timestamp; // ms since boot
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
        commit_callback(nullptr)
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
  void handleChange(int player, int change, std::function<void(const LifeHistoryEvent &)> onCommit)
  {
    commit_callback = onCommit;
    uint32_t now = millis();
    last_event_time = now;
    net_change += change;
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
      printf("[EventGrouper] update: active=%d, net_change=%d, now=%u, last_event_time=%u, grouping_window=%u, delta=%u\n", active, net_change, now, last_event_time, grouping_window, now - last_event_time);

      printf("[EventGrouper] update: committing due to rolling window timeout\n");
      int new_life_total = life_total + net_change;
      LifeHistoryEvent evt{net_change, new_life_total, player_id, last_event_time};
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
    printf("[EventGrouper] Before reset: history size=%zu, active=%d, net_change=%d, player_id=%d, group_start_time=%u, last_event_time=%u, life_total=%d\n", history.size(), active, net_change, player_id, group_start_time, last_event_time, life_total);
    history.clear();
    active = false;
    net_change = 0;
    group_start_time = 0;
    last_event_time = 0;
    life_total = base_life;
    printf("[EventGrouper] After reset: history size=%zu, active=%d, net_change=%d, player_id=%d, group_start_time=%u, last_event_time=%u, life_total=%d\n", history.size(), active, net_change, player_id, group_start_time, last_event_time, life_total);
  }

private:
  uint32_t grouping_window;
  bool active;
  int net_change;
  int player_id;
  int life_total;
  uint32_t group_start_time;
  uint32_t last_event_time;
  std::vector<LifeHistoryEvent> history;
  std::function<void(const LifeHistoryEvent &)> commit_callback;
};
