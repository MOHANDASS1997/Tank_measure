#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "BaseConfigManager.h"

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

class SystemConfigManager : public BaseConfigManager<SystemConfigManager, SystemSettings> {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;
  static const char* getNvsNamespace() { return "cfg_system"; }
  static const char* getTag() { return "SystemConfig"; }

  SystemConfigManager();

  void loadDefaults();
  bool validate(const SystemSettings& settings, String& err);
};

extern SystemConfigManager systemConfig;
