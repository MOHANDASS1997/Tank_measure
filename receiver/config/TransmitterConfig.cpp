#include "TransmitterConfig.h"

TransmitterConfigManager transmitterConfig;

TransmitterConfigManager::TransmitterConfigManager() {
  loadDefaults();
}

void TransmitterConfigManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
  _settings.count = 1;

  // Default transmitter 3201 mapped to tank_1 (from original firmware source of truth)
  _settings.transmitters[0].transmitterAddress = 3201;
  strncpy(_settings.transmitters[0].tankId, "tank_1", sizeof(_settings.transmitters[0].tankId) - 1);
  _settings.transmitters[0].tankId[sizeof(_settings.transmitters[0].tankId) - 1] = '\0';
  _settings.transmitters[0].sensorMinDistanceCm = 25.0f;
  _settings.transmitters[0].sensorMaxDistanceCm = 400.0f;
  _settings.transmitters[0].batteryFullVoltage = 4.20f;
  _settings.transmitters[0].batteryEmptyVoltage = 3.20f;

  for (int i = 1; i < MAX_TRANSMITTERS; i++) {
    memset(&_settings.transmitters[i], 0, sizeof(TransmitterConfig));
  }
}

void TransmitterConfigManager::begin() {
  if (!load()) {
    Serial.println("[TransmitterConfig] No valid stored configuration found. Writing defaults.");
    loadDefaults();
    save();
  } else {
    Serial.println("[TransmitterConfig] Loaded persistent configuration successfully.");
  }
}

bool TransmitterConfigManager::load() {
  _prefs.begin("cfg_tx", true);
  size_t len = _prefs.getBytesLength("settings");
  if (len != sizeof(TransmitterSettings)) {
    _prefs.end();
    return false;
  }

  TransmitterSettings temp;
  _prefs.getBytes("settings", &temp, sizeof(TransmitterSettings));
  _prefs.end();

  if (temp.schemaVersion != CURRENT_SCHEMA_VERSION) {
    Serial.println("[TransmitterConfig] Schema version mismatch; loading defaults.");
    return false;
  }

  String err;
  if (!validate(temp, err)) {
    Serial.print("[TransmitterConfig] Validation failed: ");
    Serial.println(err);
    return false;
  }

  _settings = temp;
  return true;
}

bool TransmitterConfigManager::save() {
  String err;
  if (!validate(_settings, err)) {
    Serial.print("[TransmitterConfig] Cannot save invalid settings: ");
    Serial.println(err);
    return false;
  }

  _prefs.begin("cfg_tx", false);
  size_t written = _prefs.putBytes("settings", &_settings, sizeof(TransmitterSettings));
  _prefs.end();

  return (written == sizeof(TransmitterSettings));
}

bool TransmitterConfigManager::validate(const TransmitterSettings& s, String& err) {
  if (s.count == 0 || s.count > MAX_TRANSMITTERS) {
    err = "Transmitter count must be between 1 and " + String(MAX_TRANSMITTERS);
    return false;
  }

  for (uint8_t i = 0; i < s.count; i++) {
    if (s.transmitters[i].transmitterAddress <= 0) {
      err = "Invalid transmitter address at index " + String(i);
      return false;
    }
    if (strlen(s.transmitters[i].tankId) == 0) {
      err = "Tank ID for transmitter " + String(s.transmitters[i].transmitterAddress) + " cannot be empty";
      return false;
    }
    if (s.transmitters[i].sensorMinDistanceCm >= s.transmitters[i].sensorMaxDistanceCm) {
      err = "Sensor min distance must be less than max distance for transmitter " + String(s.transmitters[i].transmitterAddress);
      return false;
    }
    if (s.transmitters[i].batteryFullVoltage <= s.transmitters[i].batteryEmptyVoltage) {
      err = "Battery full voltage must be greater than empty voltage for transmitter " + String(s.transmitters[i].transmitterAddress);
      return false;
    }
  }

  return true;
}

bool TransmitterConfigManager::findTransmitter(int transmitterAddress, TransmitterConfig& result) const {
  for (uint8_t i = 0; i < _settings.count; i++) {
    if (_settings.transmitters[i].transmitterAddress == transmitterAddress) {
      result = _settings.transmitters[i];
      return true;
    }
  }
  return false;
}

bool TransmitterConfigManager::addTransmitter(const TransmitterConfig& tx) {
  if (_settings.count >= MAX_TRANSMITTERS) return false;
  _settings.transmitters[_settings.count] = tx;
  _settings.count++;
  return true;
}

bool TransmitterConfigManager::updateTransmitter(uint8_t index, const TransmitterConfig& tx) {
  if (index >= _settings.count) return false;
  _settings.transmitters[index] = tx;
  return true;
}

bool TransmitterConfigManager::removeTransmitter(uint8_t index) {
  if (index >= _settings.count || _settings.count <= 1) return false;
  for (uint8_t i = index; i < _settings.count - 1; i++) {
    _settings.transmitters[i] = _settings.transmitters[i + 1];
  }
  _settings.count--;
  memset(&_settings.transmitters[_settings.count], 0, sizeof(TransmitterConfig));
  return true;
}
