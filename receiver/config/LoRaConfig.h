#pragma once

#include <Arduino.h>
#include <Preferences.h>

// =====================================================
//                     LORA CONFIG
// =====================================================

struct LoRaSettings {
  uint16_t schemaVersion;
  unsigned long band;
  int networkId;
  int address;

  int spreadingFactor;
  int bandwidth;
  int codingRate;
  int preambleLength;

  unsigned long baudRate;
};

class LoRaConfigManager {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;

  LoRaConfigManager();

  void begin();
  void loadDefaults();
  bool load();
  bool save();
  bool validate(const LoRaSettings& settings, String& err);

  const LoRaSettings& get() const { return _settings; }
  void set(const LoRaSettings& settings) { _settings = settings; }

private:
  LoRaSettings _settings;
  Preferences _prefs;
};

extern LoRaConfigManager loraConfigManager;
#define loraConfig (loraConfigManager.get())

const unsigned long LORA_TIMEOUT_MS = 15000;