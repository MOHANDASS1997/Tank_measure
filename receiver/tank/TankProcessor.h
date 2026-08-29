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
  bool findTransmitter(int transmitterId, TransmitterConfig &result);
  bool findTank(int tankId, TankConfig &result);

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
