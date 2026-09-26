#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "BoardConfig.h"

// =====================================================
//       TRANSMITTER OPERATIONAL CONFIG (NVS)
//
//  Receiver-controlled; persisted across deep sleep.
//  wakeDurationSec is the resolved value for the current
//  time slot, pushed by the receiver each cycle.
//  samplesPerWake and samplingIntervalMs are fixed values
//  set through the receiver web portal.
// =====================================================

struct TxOpSettings {
  uint16_t wakeDurationSec;     // Resolved sleep duration for current time slot
  uint8_t  samplesPerWake;      // Median filter sample count (1-20)
  uint16_t samplingIntervalMs;  // Delay between samples (ms)
};

class TxOpConfig {
public:
  TxOpConfig();

  // Load from NVS on boot (falls back to defaults if missing / invalid)
  void begin();

  // Persist current settings to NVS
  bool save();

  const TxOpSettings& get() const { return _settings; }
  void set(const TxOpSettings& s) { _settings = s; }

  // Shared validation — used on both load and save paths
  bool validate(const TxOpSettings& s, String& err) const;

private:
  TxOpSettings _settings;
  Preferences  _prefs;

  static const char* NVS_NAMESPACE; // "tx_op_cfg"
  static const char* NVS_KEY;       // "settings"

  void _applyDefaults();
};

extern TxOpConfig txOpConfig;

