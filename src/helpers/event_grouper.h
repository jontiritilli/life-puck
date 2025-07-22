#pragma once
#include <vector>
#include <stdint.h>
#include <functional>
#include <Arduino.h>

struct LifeHistoryEvent
{
  int net_life_change;
  int player_id;      // 0 for single, 1/2 for 2P
  uint32_t timestamp; // ms since boot
};

class EventGrouper
{
public:
  void commit()
  {
    if (active && net_change != 0)
    {
      LifeHistoryEvent evt{net_change, player_id, group_start_time};
      history.push_back(evt);
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
  EventGrouper(uint32_t window_ms = 1000)
      : grouping_window(window_ms), active(false), net_change(0), player_id(0), last_event_time(0) {}

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
  const std::vector<LifeHistoryEvent> &getHistory() const { return history; }

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
