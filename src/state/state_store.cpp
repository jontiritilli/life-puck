#include "state_store.h"

StateStore::StateStore(const char *ns) : nsName(ns) {}
StateStore::~StateStore()
{
  if (isOpen)
    prefs.end();
}

void StateStore::begin(bool readOnly)
{
  if (!isOpen)
  {
    prefs.begin(nsName.c_str(), readOnly);
    isOpen = true;
  }
}
void StateStore::end()
{
  if (isOpen)
  {
    prefs.end();
    isOpen = false;
  }
}
void StateStore::putU8(const char *key, uint8_t value)
{
  begin(false);
  prefs.putUChar(key, value);
  end();
}
uint8_t StateStore::getU8(const char *key, uint8_t def)
{
  begin(true);
  uint8_t v = prefs.getUChar(key, def);
  end();
  return v;
}
void StateStore::putInt(const char *key, int value)
{
  begin(false);
  prefs.putInt(key, value);
  end();
}
int StateStore::getInt(const char *key, int def)
{
  begin(true);
  int v = prefs.getInt(key, def);
  end();
  return v;
}
void StateStore::putBool(const char *key, bool value)
{
  putU8(key, value ? 1 : 0);
}
bool StateStore::getBool(const char *key, bool def)
{
  return getU8(key, def ? 1 : 0) != 0;
}
void StateStore::putString(const char *key, const String &value)
{
  begin(false);
  prefs.putString(key, value);
  end();
}
String StateStore::getString(const char *key, const String &def)
{
  begin(true);
  String v = prefs.getString(key, def);
  end();
  return v;
}
void StateStore::remove(const char *key)
{
  begin(false);
  prefs.remove(key);
  end();
}
void StateStore::clear()
{
  begin(false);
  prefs.clear();
  end();
}
