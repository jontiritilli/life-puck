#pragma once
#include <Arduino.h>
#include <ArduinoNvs.h>
#include <cstdint>

class StateStore
{
public:
  StateStore(const char *ns = "config");
  ~StateStore();

  void putInt(String key, uint64_t value);
  uint64_t getInt(String key, uint64_t defaultValue = 0);

  void putString(String key, String value);
  String getString(String key, String defaultValue = "");

  void setLife(uint64_t value);
  uint64_t getLife(uint64_t defaultValue = 40);

private:
  String nsName;
  ArduinoNvs nvs;
};

extern StateStore player_store;