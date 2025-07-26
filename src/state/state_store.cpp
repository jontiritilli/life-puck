

#include "state_store.h"
#include <ArduinoNvs.h>

// Global instance definition
StateStore player_store("player");

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
  nvs.setInt(key, value);
}

u_int64_t StateStore::getInt(const char *key, u_int64_t defaultValue)
{
  return nvs.getInt(key, defaultValue);
}

void StateStore::putString(const char *key, const char *value)
{
  nvs.setString(key, value);
}

String StateStore::getString(const char *key, const char *defaultValue)
{
  return nvs.getString(key, defaultValue);
}