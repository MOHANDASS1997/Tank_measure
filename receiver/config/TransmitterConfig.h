#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "BaseConfigManager.h"

// =====================================================
//              TRANSMITTER CONFIGURATION
// =====================================================

#define MAX_TRANSMITTERS  4
#define MAX_WAKE_SLOTS    6   // Maximum time-based schedule slots per transmitter

// Single time-of-day interval slot.
// startHour and endHour are in 24-h format (0-23).
// When startHour > endHour the slot spans midnight (e.g. 18 → 6).
struct WakeScheduleSlot {
  uint8_t  startHour;  // Inclusive start hour (0-23)
  uint8_t  endHour;    // Exclusive end hour   (0-23, wraps if < startHour)
  uint16_t wakeSec;    // Transmit interval for this window (seconds)
};

struct TransmitterConfig {
  int   transmitterAddress;
  char  tankId[16];

  // Sensor calibration
  float sensorMinDistanceCm;
  float sensorMaxDistanceCm;

  // Battery calibration
  float batteryFullVoltage;
  float batteryEmptyVoltage;

  // Fixed sampling parameters (receiver-controlled)
  uint8_t  samplesPerWake;       // Median filter samples (1-20)
  uint16_t samplingIntervalMs;   // Delay between samples (10-5000 ms)

  // Time-based wake schedule (receiver-controlled)
  uint8_t          scheduleSlotCount;
  WakeScheduleSlot scheduleSlots[MAX_WAKE_SLOTS];
};

struct TransmitterSettings {
  uint16_t schemaVersion;
  uint8_t  count;
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

  // Calculate the effective wake interval for a given transmitter at the given local hour.
  // Falls back to the last slot's wakeSec if no slot matches, or 3600 s if schedule is empty.
  uint16_t getEffectiveWakeSec(int transmitterAddress, uint8_t localHour) const;

  // One-time NVS migration: detects old struct layout and upgrades in-place.
  bool migrateFromV1IfNeeded();
};

extern TransmitterConfigManager transmitterConfig;


