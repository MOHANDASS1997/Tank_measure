#include "TankProcessor.h"

// Instantiate global TankProcessor
TankProcessor tankProcessor;

// =====================================================
// =====================================================
//                CONFIGURATION LOOKUPS
// =====================================================
// =====================================================

bool TankProcessor::findTransmitter(
  int transmitterId,
  TransmitterConfig &result
) {

  for (
    int i = 0;
    i < TRANSMITTER_COUNT;
    i++
  ) {

    if (
      transmitters[i].transmitterId ==
      transmitterId
    ) {

      result =
        transmitters[i];

      return true;
    }
  }

  return false;
}

// =====================================================

bool TankProcessor::findTank(
  int tankId,
  TankConfig &result
) {

  for (
    int i = 0;
    i < TANK_COUNT;
    i++
  ) {

    if (
      tanks[i].tankId ==
      tankId
    ) {

      result =
        tanks[i];

      return true;
    }
  }

  return false;
}

// =====================================================
// =====================================================
//                  CALCULATIONS
// =====================================================
// =====================================================

// -----------------------------------------------------
// Tank percentage
//
// Uses the actual calibrated FULL and EMPTY distances.
//
// Example:
//
// full  = 10 cm
// empty = 95 cm
//
// 10 cm -> 100%
// 95 cm -> 0%
// -----------------------------------------------------

float TankProcessor::calculateTankPercent(
  float distanceCm,
  const TankConfig &tank,
  const TransmitterConfig &transmitter
) {

  // ---------------------------------------------------
  // First protect against sensor's physical range.
  // ---------------------------------------------------

  float safeDistance =
    constrain(
      distanceCm,
      transmitter.sensorMinDistanceCm,
      transmitter.sensorMaxDistanceCm
    );

  // ---------------------------------------------------
  // Clamp to actual tank measurement region.
  // ---------------------------------------------------

  safeDistance =
    constrain(
      safeDistance,
      tank.fullDistanceCm,
      tank.emptyDistanceCm
    );

  float measurementRange =
    tank.emptyDistanceCm -
    tank.fullDistanceCm;

  if (
    measurementRange <= 0.0
  ) {

    return 0.0;
  }

  float percentage =
    (
      tank.emptyDistanceCm -
      safeDistance
    ) /
    measurementRange *
    100.0;

  return constrain(
    percentage,
    0.0,
    100.0
  );
}

// =====================================================

float TankProcessor::calculateLitres(
  float percentage,
  const TankConfig &tank
) {

  return
    tank.totalCapacityLitres *
    percentage /
    100.0;
}

// =====================================================
// Battery percentage
//
// Initial linear model.
//
// Later this can be replaced by a lookup table without
// changing the transmitter protocol.
// =====================================================

float TankProcessor::calculateBatteryPercent(
  float voltage,
  const TransmitterConfig &transmitter
) {

  float voltageRange =
    transmitter.batteryFullVoltage -
    transmitter.batteryEmptyVoltage;

  if (
    voltageRange <= 0.0
  ) {

    return 0.0;
  }

  float percentage =
    (
      voltage -
      transmitter.batteryEmptyVoltage
    ) /
    voltageRange *
    100.0;

  return constrain(
    percentage,
    0.0,
    100.0
  );
}

// =====================================================
// =====================================================
//                  PROCESS TELEMETRY
// =====================================================
// =====================================================

bool TankProcessor::process(
  const RawTelemetry &raw,
  int rssi,
  int snr,
  DisplayData &data
) {

  // ---------------------------------------------------
  // Find transmitter
  // ---------------------------------------------------

  TransmitterConfig transmitter;

  if (
    !findTransmitter(
      raw.transmitterId,
      transmitter
    )
  ) {

    Serial.print(
      "Unknown transmitter: "
    );

    Serial.println(
      raw.transmitterId
    );

    return false;
  }

  // ---------------------------------------------------
  // Find tank
  // ---------------------------------------------------

  TankConfig tank;

  if (
    !findTank(
      transmitter.tankId,
      tank
    )
  ) {

    Serial.print(
      "Unknown tank: "
    );

    Serial.println(
      transmitter.tankId
    );

    return false;
  }

  // ---------------------------------------------------
  // Validate sensor range
  // ---------------------------------------------------

  if (
    raw.distanceCm <
    transmitter.sensorMinDistanceCm ||
    raw.distanceCm >
    transmitter.sensorMaxDistanceCm
  ) {

    Serial.print(
      "Warning: distance outside sensor range: "
    );

    Serial.print(
      raw.distanceCm,
      1
    );

    Serial.println(
      " cm"
    );

    // We don't reject here.
    //
    // calculateTankPercent() will safely clamp
    // the measurement.
  }

  // ---------------------------------------------------
  // Calculate tank
  // ---------------------------------------------------

  float tankPercent =
    calculateTankPercent(
      raw.distanceCm,
      tank,
      transmitter
    );

  float litres =
    calculateLitres(
      tankPercent,
      tank
    );

  // ---------------------------------------------------
  // Calculate battery
  // ---------------------------------------------------

  float batteryPercent = 0.0;

  if (raw.hasBattery) {
    batteryPercent =
      calculateBatteryPercent(
        raw.batteryVoltage,
        transmitter
      );
  }

  // ---------------------------------------------------
  // Update display model
  // ---------------------------------------------------

  data.valid =
    true;

  data.hasBattery =
    raw.hasBattery;

  data.transmitterId =
    raw.transmitterId;

  data.tankId =
    tank.tankId;

  data.distanceCm =
    raw.distanceCm;

  data.tankPercent =
    tankPercent;

  data.currentLitres =
    litres;

  data.capacityLitres =
    tank.totalCapacityLitres;

  data.batteryPercent =
    batteryPercent;

  data.batteryVoltage =
    raw.hasBattery ? raw.batteryVoltage : 0.0f;

  data.charging =
    raw.hasBattery ? raw.charging : false;

  data.sequence =
    raw.sequence;

  data.lastReceived =
    millis();

  data.rssi =
    rssi;

  data.snr =
    snr;

  // ---------------------------------------------------
  // Debug
  // ---------------------------------------------------

  Serial.println();
  Serial.println(
    "========== DASS HOME =========="
  );

  Serial.print(
    "Transmitter: "
  );

  Serial.println(
    data.transmitterId
  );

  Serial.print(
    "Tank: "
  );

  Serial.println(
    data.tankId
  );

  Serial.print(
    "Distance: "
  );

  Serial.print(
    data.distanceCm,
    1
  );

  Serial.println(
    " cm"
  );

  Serial.print(
    "Tank: "
  );

  Serial.print(
    data.tankPercent,
    1
  );

  Serial.println(
    "%"
  );

  Serial.print(
    "Volume: "
  );

  Serial.print(
    data.currentLitres,
    1
  );

  Serial.print(
    " / "
  );

  Serial.print(
    data.capacityLitres,
    1
  );

  Serial.println(
    " L"
  );

  Serial.print(
    "Battery: "
  );

  Serial.print(
    data.batteryVoltage,
    2
  );

  Serial.print(
    " V / "
  );

  Serial.print(
    data.batteryPercent,
    0
  );

  Serial.println(
    "%"
  );

  Serial.print(
    "Charging: "
  );

  Serial.println(
    data.charging
      ? "YES"
      : "NO"
  );

  Serial.print(
    "RSSI: "
  );

  Serial.println(
    data.rssi
  );

  Serial.print(
    "SNR: "
  );

  Serial.println(
    data.snr
  );

  Serial.println(
    "================================"
  );

  return true;
}
