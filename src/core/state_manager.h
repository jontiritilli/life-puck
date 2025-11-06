#pragma once
#include <Arduino.h>
#include <ArduinoNvs.h>
#include <cstdint>

/**
 * @brief Persistent state storage manager using ESP32 NVS
 * 
 * Provides a convenient wrapper around Arduino NVS for storing
 * configuration values and game state persistently in flash memory.
 * All data survives power cycles and firmware updates.
 */
class StateStore
{
public:
  /**
   * @brief Constructor for StateStore
   * @param ns Namespace for NVS storage (default: "config")
   */
  StateStore(const char *ns = "config");
  
  /**
   * @brief Destructor - closes NVS handle
   */
  ~StateStore();

  /**
   * @brief Store an integer value
   * @param key Storage key name
   * @param value Integer value to store
   */
  void putInt(const char *key, uint64_t value);
  
  /**
   * @brief Retrieve an integer value
   * @param key Storage key name
   * @param defaultValue Default value if key doesn't exist
   * @return Stored integer value or default
   */
  uint64_t getInt(const char *key, uint64_t defaultValue = 0);

  /**
   * @brief Store a string value
   * @param key Storage key name
   * @param value String value to store
   */
  void putString(const char *key, const char *value);
  
  /**
   * @brief Retrieve a string value
   * @param key Storage key name
   * @param defaultValue Default value if key doesn't exist
   * @return Stored string value or default
   */
  String getString(const char *key, const char *defaultValue = "");

private:
  const char *nsName;  ///< NVS namespace name
  ArduinoNvs nvs;     ///< Arduino NVS instance
};

/// Global instance for player/game state storage
extern StateStore player_store;

// Timer settings functions
bool getTimerEnabled();
void toggleTimerEnabled();
int getTimerDuration();
void setTimerDuration(int duration);
bool getTimerWarningEnabled();
void toggleTimerWarningEnabled();