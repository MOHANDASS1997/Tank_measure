#pragma once

// =====================================================
//                 DISPLAY DATA MODEL
// =====================================================

struct DisplayData {
  bool valid;

  int transmitterId;
  int tankId;

  float distanceCm;

  float tankPercent;
  float currentLitres;
  float capacityLitres;

  float batteryPercent;
  float batteryVoltage;

  bool charging;
  bool hasBattery;

  unsigned long sequence;
  unsigned long lastReceived;
  uint32_t timestamp;

  int rssi;
  int snr;
};