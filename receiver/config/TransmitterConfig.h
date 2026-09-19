#pragma once

#include <Arduino.h>
#include <Preferences.h>

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

class TransmitterConfigManager {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;

  TransmitterConfigManager();

  void begin();
  void loadDefaults();
  bool load();
  bool save();
  bool validate(const TransmitterSettings& settings, String& err);

  const TransmitterSettings& get() const { return _settings; }
  void set(const TransmitterSettings& settings) { _settings = settings; }

  bool findTransmitter(int transmitterAddress, TransmitterConfig& result) const;
  uint8_t getCount() const { return _settings.count; }
  const TransmitterConfig* getTransmitters() const { return _settings.transmitters; }

  bool addTransmitter(const TransmitterConfig& tx);
  bool updateTransmitter(uint8_t index, const TransmitterConfig& tx);
  bool removeTransmitter(uint8_t index);

private:
  TransmitterSettings _settings;
  Preferences _prefs;
};

extern TransmitterConfigManager transmitterConfig;
