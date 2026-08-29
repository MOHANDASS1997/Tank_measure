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

  unsigned long sequence;
  unsigned long lastReceived;

  int rssi;
  int snr;
};