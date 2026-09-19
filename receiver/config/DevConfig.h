#pragma once

#include <Arduino.h>
#include <Preferences.h>

// =====================================================
//                     DEV CONFIG
// =====================================================

struct DevSettings {
  uint16_t schemaVersion;
  bool devModeEnabled; // "Enable Dev Mode Entry Point" (default: false)
};

class DevConfigManager {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;

  DevConfigManager();

  void begin();
  void loadDefaults();
  bool load();
  bool save();
  bool validate(const DevSettings& settings, String& err);

  const DevSettings& get() const { return _settings; }
  void set(const DevSettings& settings) { _settings = settings; }

private:
  DevSettings _settings;
  Preferences _prefs;
};

extern DevConfigManager devConfig;
