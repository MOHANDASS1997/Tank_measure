#include "SystemConfig.h"
#include "BoardConfig.h"

SystemConfigManager systemConfig;

SystemConfigManager::SystemConfigManager() {
  loadDefaults();
}

void SystemConfigManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
  _settings.uiTimeoutMs = UI_TIMEOUT_MS;
  _settings.configTimeoutMs = 300000; // 5 minutes (300,000 ms)
  _settings.longPressDurationMs = 2000; // 2 seconds
  _settings.chargingAnimationDurationMs = CHARGING_ANIMATION_DURATION_MS;
  _settings.autoSleepEnabled = true;
}

bool SystemConfigManager::validate(const SystemSettings& s, String& err) {
  if (s.autoSleepEnabled) {
    if (s.uiTimeoutMs < 2000 || s.uiTimeoutMs > 120000) {
      err = "UI timeout must be between 2s and 120s";
      return false;
    }
  }
  if (s.configTimeoutMs < 30000 || s.configTimeoutMs > 1800000) {
    err = "Config timeout must be between 30s and 30m";
    return false;
  }
  if (s.longPressDurationMs < 800 || s.longPressDurationMs > 5000) {
    err = "Long press duration must be between 800ms and 5000ms";
    return false;
  }
  if (s.chargingAnimationDurationMs < 500 || s.chargingAnimationDurationMs > 10000) {
    err = "Charging animation duration must be between 500ms and 10000ms";
    return false;
  }
  return true;
}
