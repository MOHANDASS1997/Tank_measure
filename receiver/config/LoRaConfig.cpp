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
