#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "BaseConfigManager.h"

// =====================================================
//                     DEV CONFIG
// =====================================================

struct DevSettings {
  uint16_t schemaVersion;
  bool devModeEnabled; // "Enable Dev Mode Entry Point" (default: false)
};

class DevConfigManager : public BaseConfigManager<DevConfigManager, DevSettings> {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;
  static const char* getNvsNamespace() { return "cfg_dev"; }
  static const char* getTag() { return "DevConfig"; }

  DevConfigManager();

  void loadDefaults();
  bool validate(const DevSettings& settings, String& err);
};

extern DevConfigManager devConfig;
