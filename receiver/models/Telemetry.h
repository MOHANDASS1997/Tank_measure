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
};