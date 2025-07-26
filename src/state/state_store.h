#pragma once
#include <Arduino.h>
#include <ArduinoNvs.h>
#include <cstdint>

class StateStore
{
public:
  StateStore(const char *ns = "config");
  ~StateStore();

  void putInt(const char *key, uint64_t value);
  uint64_t getInt(const char *key, uint64_t defaultValue = 0);

  void putString(const char *key, const char *value);
  String getString(const char *key, const char *defaultValue = "");

private:
  const char *nsName;
  ArduinoNvs nvs;
};

extern StateStore player_store;