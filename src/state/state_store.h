#pragma once
#include <Arduino.h>
#include <Preferences.h>

class StateStore
{
public:
  StateStore(const char *ns = "config");
  ~StateStore();

  void begin(bool readOnly = false);
  void end();

  // Generic put/get for uint8_t, int, bool, String
  void putU8(const char *key, uint8_t value);
  uint8_t getU8(const char *key, uint8_t def = 0);

  void putInt(const char *key, int value);
  int getInt(const char *key, int def = 0);

  void putBool(const char *key, bool value);
  bool getBool(const char *key, bool def = false);

  void putString(const char *key, const String &value);
  String getString(const char *key, const String &def = "");

  void remove(const char *key);
  void clear();

private:
  Preferences prefs;
  String nsName;
  bool isOpen = false;
};
