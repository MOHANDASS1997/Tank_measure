#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "BaseConfigManager.h"

// =====================================================
//              TRANSMITTER CONFIGURATION
// =====================================================

#define MAX_TRANSMITTERS 4

struct TransmitterConfig {
  int transmitterAddress;
  char tankId[16];

  // Sensor characteristics
  float sensorMinDistanceCm;
  float sensorMaxDistanceCm;

  // Battery characteristics
  float batteryFullVoltage;
  float batteryEmptyVoltage;
};

struct TransmitterSettings {
  uint16_t schemaVersion;
  uint8_t count;
  TransmitterConfig transmitters[MAX_TRANSMITTERS];
};

class TransmitterConfigManager : public BaseConfigManager<TransmitterConfigManager, TransmitterSettings> {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;
  static const char* getNvsNamespace() { return "cfg_tx"; }
  static const char* getTag() { return "TransmitterConfig"; }

  TransmitterConfigManager();

  void loadDefaults();
  bool validate(const TransmitterSettings& settings, String& err);

  bool findTransmitter(int transmitterAddress, TransmitterConfig& result) const;
  uint8_t getCount() const { return _settings.count; }
  const TransmitterConfig* getTransmitters() const { return _settings.transmitters; }

  bool addTransmitter(const TransmitterConfig& tx);
  bool updateTransmitter(uint8_t index, const TransmitterConfig& tx);
  bool removeTransmitter(uint8_t index);
};

extern TransmitterConfigManager transmitterConfig;
