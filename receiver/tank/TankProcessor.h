#pragma once

#include <Arduino.h>
#include "../config/TankConfig.h"
#include "../models/Telemetry.h"
#include "../models/DisplayData.h"

// =====================================================
//                  TANK PROCESSOR
// =====================================================

class TankProcessor {
public:
  bool findTransmitter(int transmitterAddress, TransmitterConfig &result);
  bool findTank(const char* tankId, TankConfig &result);

  float calculateTankPercent(
    float distanceCm,
    const TankConfig &tank,
    const TransmitterConfig &transmitter
  );

  float calculateLitres(
    float percentage,
    const TankConfig &tank
  );

  float calculateBatteryPercent(
    float voltage,
    const TransmitterConfig &transmitter
  );

  bool process(
    const RawTelemetry &raw,
    int rssi,
    int snr,
    DisplayData &data
  );
};

extern TankProcessor tankProcessor;
