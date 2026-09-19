#include "DevConfig.h"

DevConfigManager devConfig;

DevConfigManager::DevConfigManager() {
  loadDefaults();
}

void DevConfigManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
  _settings.devModeEnabled = false; // False by default
}

void DevConfigManager::begin() {
  if (!load()) {
    Serial.println("[DevConfig] No valid stored configuration found. Writing defaults.");
    loadDefaults();
    save();
  } else {
    Serial.println("[DevConfig] Loaded persistent configuration successfully.");
  }
}

bool DevConfigManager::load() {
  _prefs.begin("cfg_dev", true);
  size_t len = _prefs.getBytesLength("settings");
  if (len != sizeof(DevSettings)) {
    _prefs.end();
    return false;
  }

  DevSettings temp;
  _prefs.getBytes("settings", &temp, sizeof(DevSettings));
  _prefs.end();

  if (temp.schemaVersion != CURRENT_SCHEMA_VERSION) {
    Serial.println("[DevConfig] Schema version mismatch; migrating to defaults.");
    return false;
  }

  String err;
  if (!validate(temp, err)) {
    Serial.print("[DevConfig] Validation failed: ");
    Serial.println(err);
    return false;
  }

  _settings = temp;
  return true;
}

bool DevConfigManager::save() {
  String err;
  if (!validate(_settings, err)) {
    Serial.print("[DevConfig] Cannot save invalid settings: ");
    Serial.println(err);
    return false;
  }

  _prefs.begin("cfg_dev", false);
  size_t written = _prefs.putBytes("settings", &_settings, sizeof(DevSettings));
  _prefs.end();

  return (written == sizeof(DevSettings));
}

bool DevConfigManager::validate(const DevSettings& s, String& err) {
  // Boolean values are always valid; schema version check is sufficient
  return true;
}
