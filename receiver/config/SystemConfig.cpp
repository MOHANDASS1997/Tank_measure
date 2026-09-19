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

void SystemConfigManager::begin() {
  if (!load()) {
    Serial.println("[SystemConfig] No valid stored configuration found. Writing defaults.");
    loadDefaults();
    save();
  } else {
    Serial.println("[SystemConfig] Loaded persistent configuration successfully.");
  }
}

bool SystemConfigManager::load() {
  _prefs.begin("cfg_system", true);
  size_t len = _prefs.getBytesLength("settings");
  if (len != sizeof(SystemSettings)) {
    _prefs.end();
    return false;
  }

  SystemSettings temp;
  _prefs.getBytes("settings", &temp, sizeof(SystemSettings));
  _prefs.end();

  if (temp.schemaVersion != CURRENT_SCHEMA_VERSION) {
    Serial.println("[SystemConfig] Schema version mismatch; migrating to defaults.");
    return false;
  }

  String err;
  if (!validate(temp, err)) {
    Serial.print("[SystemConfig] Validation failed: ");
    Serial.println(err);
    return false;
  }

  _settings = temp;
  return true;
}

bool SystemConfigManager::save() {
  String err;
  if (!validate(_settings, err)) {
    Serial.print("[SystemConfig] Cannot save invalid settings: ");
    Serial.println(err);
    return false;
  }

  _prefs.begin("cfg_system", false);
  size_t written = _prefs.putBytes("settings", &_settings, sizeof(SystemSettings));
  _prefs.end();

  return (written == sizeof(SystemSettings));
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
