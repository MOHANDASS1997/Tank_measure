#pragma once

#include <Arduino.h>
#include <Preferences.h>

template <typename TDerived, typename TSettings>
class BaseConfigManager {
public:
  void begin() {
    if (!load()) {
      Serial.printf("[%s] No valid stored configuration found. Writing defaults.\n", TDerived::getTag());
      static_cast<TDerived*>(this)->loadDefaults();
      save();
    } else {
      Serial.printf("[%s] Loaded persistent configuration successfully.\n", TDerived::getTag());
    }
  }

  bool load() {
    TDerived* self = static_cast<TDerived*>(this);
    const char* nvsNamespace = TDerived::getNvsNamespace();
    const char* tag = TDerived::getTag();
    uint16_t currentSchema = TDerived::CURRENT_SCHEMA_VERSION;

    _prefs.begin(nvsNamespace, true);
    size_t len = _prefs.getBytesLength("settings");
    if (len != sizeof(TSettings)) {
      _prefs.end();
      return false;
    }

    TSettings temp;
    _prefs.getBytes("settings", &temp, sizeof(TSettings));
    _prefs.end();

    if (temp.schemaVersion != currentSchema) {
      Serial.printf("[%s] Schema version mismatch; loading defaults.\n", tag);
      return false;
    }

    String err;
    if (!self->validate(temp, err)) {
      Serial.printf("[%s] Validation failed: %s\n", tag, err.c_str());
      return false;
    }

    _settings = temp;
    return true;
  }

  bool save() {
    TDerived* self = static_cast<TDerived*>(this);
    const char* nvsNamespace = TDerived::getNvsNamespace();
    const char* tag = TDerived::getTag();

    String err;
    if (!self->validate(_settings, err)) {
      Serial.printf("[%s] Cannot save invalid settings: %s\n", tag, err.c_str());
      return false;
    }

    _prefs.begin(nvsNamespace, false);
    size_t written = _prefs.putBytes("settings", &_settings, sizeof(TSettings));
    _prefs.end();

    return (written == sizeof(TSettings));
  }

  const TSettings& get() const { return _settings; }
  void set(const TSettings& settings) { _settings = settings; }

protected:
  TSettings _settings;
  Preferences _prefs;
};
