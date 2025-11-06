/**
 * @file state_manager.cpp
 * @brief Implementation of persistent state storage using ESP32 NVS
 */

#include "core/state_manager.h"
#include "data/constants.h"
#include <ArduinoNvs.h>

/// Global instance for player/game state storage
StateStore player_store(PLAYER_STORE);

// StateStore method implementations
StateStore::StateStore(const char *ns) : nsName(ns)
{
  static bool nvs_global_initialized = false;
  if (!nvs_global_initialized)
  {
    NVS.begin(); // Initialize global NVS partition once
    nvs_global_initialized = true;
  }
  nvs.begin(nsName); // Initialize the namespace for this instance
}

StateStore::~StateStore() {}

void StateStore::putInt(const char *key, u_int64_t value)
{
  bool success = nvs.setInt(key, value);
  if (!success) {
    printf("[NVS ERROR] Failed to write key '%s' with value %llu\n", key, value);
  }
  // ArduinoNvs automatically commits after setInt()
}

u_int64_t StateStore::getInt(const char *key, u_int64_t defaultValue)
{
  return nvs.getInt(key, defaultValue);
}

void StateStore::putString(const char *key, const char *value)
{
  bool success = nvs.setString(key, value);
  if (!success) {
    printf("[NVS ERROR] Failed to write string key '%s'\n", key);
  }
  // ArduinoNvs automatically commits after setString()
}

String StateStore::getString(const char *key, const char *defaultValue)
{
  return nvs.getString(key, defaultValue);
}

// Timer settings functions
bool getTimerEnabled() {
    return player_store.getInt("timer_enabled", 1) != 0; // Default: enabled
}

void toggleTimerEnabled() {
    bool current = getTimerEnabled();
    player_store.putInt("timer_enabled", current ? 0 : 1);
}

int getTimerDuration() {
    return player_store.getInt("timer_duration", 10); // Default: 10 minutes
}

void setTimerDuration(int duration) {
    player_store.putInt("timer_duration", duration);
}

bool getTimerWarningEnabled() {
    return player_store.getInt("timer_warning_enabled", 1) != 0; // Default: enabled
}

void toggleTimerWarningEnabled() {
    bool current = getTimerWarningEnabled();
    player_store.putInt("timer_warning_enabled", current ? 0 : 1);
}