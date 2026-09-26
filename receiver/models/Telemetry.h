#pragma once

// =====================================================
//                  RAW TELEMETRY
// =====================================================

struct RawTelemetry {
  int transmitterAddress;
  unsigned long sequence;
  float distanceCm;
  float batteryVoltage;
  bool charging;
  bool hasBattery;

  // Receiver-controlled operational parameters (from wds/sps/sim packet fields)
  bool     hasOpConfig;
  uint16_t wakeDurationSec;
  uint8_t  samplesPerWake;
  uint16_t samplingIntervalMs;
};

// =====================================================
//             TEMPORARY PARSER STATE
// =====================================================

struct ParsedPacket {
  bool versionFound;
  int version;

  bool sequenceFound;
  unsigned long sequence;

  bool distanceFound;
  float distanceCm;

  bool batteryFound;
  float batteryVoltage;

  bool chargingFound;
  bool charging;

  // Receiver-controlled operational parameters (optional fields)
  bool     wdsFound;
  uint16_t wakeDurationSec;

  bool     spsFound;
  uint8_t  samplesPerWake;

  bool     simFound;
  uint16_t samplingIntervalMs;
};