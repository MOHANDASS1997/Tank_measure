#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include "../config/BoardConfig.h"
#include "../config/LoRaConfig.h"
#include "../models/Telemetry.h"
#include "../protocol/PacketParser.h"
#include "../display/DisplayManager.h"

// =====================================================
//                     LORA MANAGER
// =====================================================

class LoRaManager {
public:
  LoRaManager();

  void begin();
  String command(const char* cmd, unsigned long waitTime = 500);
  bool receive(RawTelemetry& raw, int& rssi, int& snr);

private:
  HardwareSerial _loraSerial;
  bool parseLoRaReceive(const String& line, RawTelemetry& raw, int& rssi, int& snr);
};

extern LoRaManager loraManager;
