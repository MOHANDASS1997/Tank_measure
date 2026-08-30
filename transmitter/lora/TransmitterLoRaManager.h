#pragma once

#include <Arduino.h>
#include "../config/BoardConfig.h"
#include "../config/TransmitterConfig.h"

// =====================================================
//              TRANSMITTER LORA MANAGER
// =====================================================

class TransmitterLoRaManager {
public:
  TransmitterLoRaManager();

  void begin();
  bool configure();
  bool sendTelemetry(
    unsigned long sequence,
    float distanceCm,
    float batteryVoltage,
    bool charging
  );

  bool sendAT(const String& cmd, unsigned long timeoutMs = 1000);

private:
  HardwareSerial _loraSerial;
};

extern TransmitterLoRaManager transmitterLoRaManager;
