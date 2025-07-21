

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

void StateStore::putInt(String key, u_int64_t value)
{
  nvs.setInt(key, value);
}

u_int64_t StateStore::getInt(String key, u_int64_t defaultValue)
{
  return nvs.getInt(key, defaultValue);
}

void StateStore::putString(String key, String value)
{
  nvs.setString(key, value.c_str());
}

String StateStore::getString(String key, String defaultValue)
{
  return nvs.getString(key, defaultValue.c_str());
}

void StateStore::setLife(u_int64_t value)
{
  putInt("life", value);
}

u_int64_t StateStore::getLife(u_int64_t defaultValue)
{
  printf("[StateStore::getLife] Retrieving life with default %llu\n", defaultValue);
  // Use the NVS instance to get the life value
  // This will return the default value if not set
  // or if an error occurs
  u_int64_t life = nvs.getInt("life", defaultValue);
  printf("[StateStore::getLife] Retrieved life: %llu\n", life);

  return life;
}