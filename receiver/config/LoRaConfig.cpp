#include "LoRaConfig.h"

LoRaConfigManager loraConfigManager;

LoRaConfigManager::LoRaConfigManager() {
  loadDefaults();
}

void LoRaConfigManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
  _settings.band = 867000000UL;
  _settings.networkId = 18;
  _settings.address = 3001;
  _settings.spreadingFactor = 9;
  _settings.bandwidth = 7;
  _settings.codingRate = 1;
  _settings.preambleLength = 12;
  _settings.baudRate = 115200;
}

void LoRaConfigManager::begin() {
  if (!load()) {
    Serial.println("[LoRaConfig] No valid stored configuration found. Writing defaults.");
    loadDefaults();
    save();
  } else {
    Serial.println("[LoRaConfig] Loaded persistent configuration successfully.");
  }
}

bool LoRaConfigManager::load() {
  _prefs.begin("cfg_lora", true);
  size_t len = _prefs.getBytesLength("settings");
  if (len != sizeof(LoRaSettings)) {
    _prefs.end();
    return false;
  }

  LoRaSettings temp;
  _prefs.getBytes("settings", &temp, sizeof(LoRaSettings));
  _prefs.end();

  if (temp.schemaVersion != CURRENT_SCHEMA_VERSION) {
    Serial.println("[LoRaConfig] Schema version mismatch; loading defaults.");
    return false;
  }

  String err;
  if (!validate(temp, err)) {
    Serial.print("[LoRaConfig] Validation failed: ");
    Serial.println(err);
    return false;
  }

  _settings = temp;
  return true;
}

bool LoRaConfigManager::save() {
  String err;
  if (!validate(_settings, err)) {
    Serial.print("[LoRaConfig] Cannot save invalid settings: ");
    Serial.println(err);
    return false;
  }

  _prefs.begin("cfg_lora", false);
  size_t written = _prefs.putBytes("settings", &_settings, sizeof(LoRaSettings));
  _prefs.end();

  return (written == sizeof(LoRaSettings));
}

bool LoRaConfigManager::validate(const LoRaSettings& s, String& err) {
  if (s.band < 137000000UL || s.band > 1020000000UL) {
    err = "LoRa frequency band must be valid (137-1020 MHz)";
    return false;
  }
  if (s.networkId < 0 || s.networkId > 255) {
    err = "LoRa Network ID must be between 0 and 255";
    return false;
  }
  if (s.address < 0 || s.address > 65535) {
    err = "LoRa Address must be between 0 and 65535";
    return false;
  }
  if (s.spreadingFactor < 7 || s.spreadingFactor > 12) {
    err = "Spreading Factor must be between 7 and 12";
    return false;
  }
  if (s.bandwidth < 0 || s.bandwidth > 9) {
    err = "Bandwidth index must be between 0 and 9";
    return false;
  }
  if (s.codingRate < 1 || s.codingRate > 4) {
    err = "Coding rate must be between 1 and 4";
    return false;
  }
  if (s.preambleLength < 4 || s.preambleLength > 255) {
    err = "Preamble length must be between 4 and 255";
    return false;
  }
  return true;
}
