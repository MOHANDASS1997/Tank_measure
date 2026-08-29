#pragma once

// =====================================================
//                  TANK CONFIGURATION
// =====================================================

struct TankConfig {
  const char* tankId;
  float totalLengthCm;
  float totalCapacityLitres;
  float fullDistanceCm;
  float emptyDistanceCm;
};

const TankConfig tanks[] = {
  {
    "tank_1", // Tank ID
    180.0,    // Total tank length (180 cm)
    750.0,    // Total capacity (750 L)
    15.0,     // Full distance (15 cm)
    175.0     // Empty distance (175 cm)
  }
};

const int TANK_COUNT = sizeof(tanks) / sizeof(tanks[0]);

// =====================================================
//              TRANSMITTER CONFIGURATION
// =====================================================

struct TransmitterConfig {
  int transmitterAddress;
  const char* tankId;

  // Sensor characteristics
  float sensorMinDistanceCm;
  float sensorMaxDistanceCm;

  // Battery characteristics
  float batteryFullVoltage;
  float batteryEmptyVoltage;
};

const TransmitterConfig transmitters[] = {
  {
    3201,     // Transmitter Address (Starts with 3201)
    "tank_1", // Tank ID

    25.0,     // Sensor minimum distance (25 cm)
    400.0,    // Sensor maximum distance (400 cm)

    4.20,     // Battery full voltage (4.20 V)
    3.20      // Battery empty voltage (3.20 V)
  }
};

const int TRANSMITTER_COUNT = sizeof(transmitters) / sizeof(transmitters[0]);