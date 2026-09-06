// =====================================================
//                 DASS HOME - TRANSMITTER
//                  (ESP32-C3 Supermini)
// =====================================================

#include "config/BoardConfig.h"
#include "config/SensorConfig.h"
#include "config/TransmitterConfig.h"

#include "battery/BatteryManager.h"
#include "lora/TransmitterLoRaManager.h"
#include "sensor/SensorManager.h"
#include "sleep/SleepManager.h"

// Explicit .cpp includes for Arduino IDE multi-directory support
#include "battery/BatteryManager.cpp"
#include "lora/TransmitterLoRaManager.cpp"
#include "sensor/SensorManager.cpp"
#include "sleep/SleepManager.cpp"

// =====================================================
//             PROCESS & TRANSMIT TELEMETRY
// =====================================================

void processAndTransmit() {
  // 1. Measure Ultrasonic Distance (with multi-sample filtering)
  Serial.println("\n--- Starting Measurement Cycle ---");
  Serial.println("Reading ultrasonic distance...");
  float distanceCm = sensorManager.measureFilteredDistanceCm();

  if (distanceCm < 0.0f) {
    Serial.println("Warning: Ultrasonic echo timeout / no echo detected.");
  } else {
    Serial.print("Measured Distance: ");
    Serial.print(distanceCm, 1);
    Serial.println(" cm");
  }

  // 2. Read / Generate Battery Telemetry
  float batteryVoltage = 0.0f;
  bool isCharging = false;
  batteryManager.readBattery(batteryVoltage, isCharging);

  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage, 2);
  Serial.print(" V, Charging: ");
  Serial.println(isCharging ? "YES" : "NO");

  // 3. Get Next Sequence Number (with automatic rollover protection)
  unsigned long sequence = sleepManager.getNextSequenceNumber();
  Serial.print("Packet Sequence: ");
  Serial.println(sequence);

  // 4. Stagger delay before LoRa transmission to allow power rail / capacitors to stabilize
  if (TX_POST_SENSOR_DELAY_MS > 0) {
    delay(TX_POST_SENSOR_DELAY_MS);
  }

  // 5. Transmit Telemetry Packet via LoRa to Receiver (Address 3001)
  if (distanceCm > 0.0f) {
    transmitterLoRaManager.sendTelemetry(sequence, distanceCm, batteryVoltage,
                                         isCharging);
  } else {
    Serial.println("Skipping transmission due to invalid sensor reading.");
  }
}

// =====================================================
//                       SETUP
// =====================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("==================================================");
  Serial.println("         DASS HOME - TRANSMITTER (ESP32-C3)       ");
  Serial.println("==================================================");

  // 1. Initialize Sleep / RTC State
  sleepManager.begin();

  // 2. Initialize Hardware Subsystems
  sensorManager.begin();
  batteryManager.begin();
  transmitterLoRaManager.begin();

  // 3. Configure LoRa Module on first boot
  if (sleepManager.getBootCount() == 1 || !ENABLE_DEEP_SLEEP) {
    transmitterLoRaManager.configure();
  }

  if (ENABLE_DEEP_SLEEP) {
    Serial.println("Mode: DEEP SLEEP (Production)");
    processAndTransmit();

    // Enter Deep Sleep for configured duration
    sleepManager.goToDeepSleep(DEEP_SLEEP_SECONDS);
  } else {
    Serial.println("Mode: DELAY LOOP (Testing / Continuous Serial Debugging)");
    // Run first transmission cycle immediately
    processAndTransmit();
  }
}

// =====================================================
//                        LOOP
// =====================================================

void loop() {
  if (!ENABLE_DEEP_SLEEP) {
    Serial.print("Waiting ");
    Serial.print(TRANSMIT_INTERVAL_SECONDS);
    Serial.println(" seconds before next transmission...");

    delay(TRANSMIT_INTERVAL_SECONDS * 1000UL);

    processAndTransmit();
  }
}
