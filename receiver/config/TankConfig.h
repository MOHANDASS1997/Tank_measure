#pragma once

// =====================================================
//                  TANK CONFIGURATION
// =====================================================

struct TankConfig {
  int tankId;
  float totalLengthCm;
  float totalCapacityLitres;
  float fullDistanceCm;
  float emptyDistanceCm;
};

const TankConfig tanks[] = {
  {
    1,       // Tank ID
    100.0,   // Total tank length
    750.0,   // Capacity
    10.0,    // Full distance
    95.0     // Empty distance
  }
};

const int TANK_COUNT = sizeof(tanks) / sizeof(tanks[0]);

// =====================================================
//              TRANSMITTER CONFIGURATION
// =====================================================

struct TransmitterConfig {
  int transmitterId;
  int tankId;

  // Sensor characteristics
  float sensorMinDistanceCm;
  float sensorMaxDistanceCm;

  // Battery characteristics
  float batteryFullVoltage;
  float batteryEmptyVoltage;
};

const TransmitterConfig transmitters[] = {
  {
    1,       // Transmitter ID
    1,       // Tank ID

    2.0,     // Sensor minimum distance
    400.0,   // Sensor maximum distance

    4.20,    // Battery full voltage
    3.20     // Battery empty voltage
  }
};

const int TRANSMITTER_COUNT = sizeof(transmitters) / sizeof(transmitters[0]);