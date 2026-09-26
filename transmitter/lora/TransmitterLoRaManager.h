#pragma once

#include <Arduino.h>
#include "../config/BoardConfig.h"
#include "../config/TransmitterConfig.h"
#include "../config/TxOpConfig.h"

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

  // Listen up to timeoutMs for a SET_CONFIG or ACK reply from the receiver.
  // Exits early as soon as any +RCV= envelope arrives (saves idle radio time).
  // Returns true if any response was received.
  bool listenForConfigResponse(unsigned long timeoutMs = 2000);

  bool sendAT(const String& cmd, unsigned long timeoutMs = 1000);

private:
  HardwareSerial _loraSerial;
};

extern TransmitterLoRaManager transmitterLoRaManager;
