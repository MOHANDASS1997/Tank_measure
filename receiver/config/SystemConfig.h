#pragma once

#include <Arduino.h>
#include <Preferences.h>

// =====================================================
//                   SYSTEM CONFIG
// =====================================================

struct SystemSettings {
  uint16_t schemaVersion;
  uint32_t uiTimeoutMs;                 // Inactivity timeout before OLED turns OFF (ms)
  uint32_t configTimeoutMs;             // Auto-exit timeout for configuration mode (ms)
  uint32_t longPressDurationMs;         // Button hold duration to trigger config mode (ms)
  uint32_t chargingAnimationDurationMs; // Charging splash animation duration (ms)
  bool autoSleepEnabled;                // true = OLED sleeps after uiTimeoutMs, false = Always ON
};

class SystemConfigManager {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;

  SystemConfigManager();

  void begin();
  void loadDefaults();
  bool load();
  bool save();
  bool validate(const SystemSettings& settings, String& err);

  const SystemSettings& get() const { return _settings; }
  void set(const SystemSettings& settings) { _settings = settings; }

private:
  SystemSettings _settings;
  Preferences _prefs;
};

extern SystemConfigManager systemConfig;
