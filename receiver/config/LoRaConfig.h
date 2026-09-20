#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "BaseConfigManager.h"

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

class LoRaConfigManager : public BaseConfigManager<LoRaConfigManager, LoRaSettings> {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;
  static const char* getNvsNamespace() { return "cfg_lora"; }
  static const char* getTag() { return "LoRaConfig"; }

  LoRaConfigManager();

  void loadDefaults();
  bool validate(const LoRaSettings& settings, String& err);
};

extern LoRaConfigManager loraConfigManager;
#define loraConfig (loraConfigManager.get())

const unsigned long LORA_TIMEOUT_MS = 15000;