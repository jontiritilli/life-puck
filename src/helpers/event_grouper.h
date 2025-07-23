#pragma once
#include <vector>
#include <stdint.h>
#include <functional>
#include <Arduino.h>
#include <string>

struct LifeHistoryEvent
{
  int net_life_change;
  int life_total;
  int player_id;      // 0 for single, 1/2 for 2P
  uint32_t timestamp; // ms since boot

  // Returns time as "HH:MM" string since boot
  std::string getHourMinute() const
  {
    uint32_t total_seconds = timestamp / 1000;
    uint32_t hours = total_seconds / 3600;
    uint32_t minutes = (total_seconds % 3600) / 60;
    char buf[6];
    snprintf(buf, sizeof(buf), "%02u:%02u", hours, minutes);
    return std::string(buf);
  }
};

class EventGrouper
{
public:
  int life_total = 0; // Always reflects the latest committed state

  void setInitialLifeTotal(int value)
  {
    life_total = value;
  }
  void commit()
  {
    if (active && net_change != 0)
    {
      int new_life_total = life_total + net_change;
      LifeHistoryEvent evt{net_change, new_life_total, player_id, last_event_time};
      history.push_back(evt);
      life_total = new_life_total; // Update state to latest committed value
      printf("[EventGrouper] Committing event: Change=%d, total=%d, Player=%d, Time=%u\n", evt.net_life_change, evt.life_total, evt.player_id, evt.timestamp);
      if (commit_callback)
        commit_callback(evt);
      // Clear commit pending state immediately after callback
      active = false;
      net_change = 0;
    }
    else
    {
      // Always clear state if not committing
      active = false;
      net_change = 0;
    }
  }

  // Returns the current pending net change (0 if inactive)
  int getPendingChange() const { return active ? net_change : 0; }
  int getPendingChangeForPlayer(int player) const
  {
    return (active && player_id == player) ? net_change : 0;
  }
  EventGrouper(uint32_t window_ms = 1000, int initial_life = 0)
      : grouping_window(window_ms), active(false), net_change(0), player_id(0), last_event_time(0), life_total(initial_life) {}

  // Call this for each life change (tap/swipe)
  void handleChange(int player, int change, std::function<void(const LifeHistoryEvent &)> onCommit)
  {
    commit_callback = onCommit;
    uint32_t now = millis();
    // Commit previous group if needed before starting a new one
    if (!active || player != player_id || (now - last_event_time) > grouping_window)
    {
      if (active && net_change != 0)
      {
        commit();
      }
      // Start new group
      active = true;
      net_change = change;
      player_id = player;
      group_start_time = now;
    }
    else
    {
      // Accumulate
      net_change += change;
    }
    last_event_time = now;
    scheduleCommit();
  }

  // Call this periodically (e.g., in loop) to check for timeout
  void update()
  {
    if (active && (millis() - last_event_time) > grouping_window)
    {
      commit();
    }
  }

  // Access history
  const std::vector<LifeHistoryEvent> &getHistory() const
  {
    printf("[EventGrouper] History size: %zu\n", history.size());
    for (const auto &event : history)
    {
      printf("[EventGrouper] History event: Change=%d, Player=%d, Time=%u\n", event.net_life_change, event.player_id, event.timestamp);
    }
    return history;
  }

  // Helper: Reset history
  void resetHistory()
  {
    printf("[EventGrouper] Resetting history\n");
    history.clear();
    active = false;
    net_change = 0;
    player_id = 0;
    group_start_time = 0;
    last_event_time = 0;
    life_total = 0;
    scheduleCommit(); // Reset any pending commit state
  }

  // Helper: returns true if a commit is pending (for polling in main loop)
  bool isCommitPending() const
  {
    return active && net_change != 0;
  }

private:
  uint32_t grouping_window;
  bool active;
  int net_change;
  int player_id;
  uint32_t group_start_time;
  uint32_t last_event_time;
  std::vector<LifeHistoryEvent> history;
  std::function<void(const LifeHistoryEvent &)> commit_callback;

  void scheduleCommit()
  {
    // For embedded, actual timer/callback may be handled in main loop via update()
    // This is a placeholder for future timer integration
  }
};
