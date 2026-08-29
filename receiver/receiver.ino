#include <Arduino.h>

#include "config/BoardConfig.h"
#include "config/LoRaConfig.h"
#include "config/PacketConfig.h"
#include "config/TankConfig.h"

#include "models/Telemetry.h"
#include "models/DisplayData.h"

#include "protocol/PacketParser.h"
#include "lora/LoRaManager.h"
#include "tank/TankProcessor.h"
#include "display/DisplayManager.h"
#include "input/ButtonManager.h"

// Arduino IDE ignores .cpp files located in subdirectories unless included:
#include "protocol/PacketParser.cpp"
#include "lora/LoRaManager.cpp"
#include "tank/TankProcessor.cpp"
#include "display/DisplayManager.cpp"
#include "input/ButtonManager.cpp"

// =====================================================
// =====================================================
//                         SETUP
// =====================================================
// =====================================================

void setup() {

  Serial.begin(
    115200
  );

  delay(500);

  Serial.println();
  Serial.println(
    "================================"
  );

  Serial.println(
    "Tank_sync Receiver"
  );

  Serial.println(
    "LoRa Receiver"
  );

  Serial.println(
    "================================"
  );

  // Initialize display
  displayManager.begin();

  // Initialize button input
  buttonManager.begin();

  // Initial UI state
  displayManager.showNotConnected();

  // Initialize LoRa radio
  loraManager.begin();

  Serial.println();
  Serial.println(
    "Receiver ready."
  );

  Serial.println(
    "Waiting for Tank_sync packets..."
  );
}

// =====================================================
// =====================================================
//                          LOOP
// =====================================================
// =====================================================

void loop() {

  // Handle button input & page navigation
  buttonManager.update();

  // Check for incoming LoRa packet
  RawTelemetry raw;
  int rssi = 0;
  int snr = 0;

  if (
    loraManager.receive(
      raw,
      rssi,
      snr
    )
  ) {

    DisplayData data;

    if (
      tankProcessor.process(
        raw,
        rssi,
        snr,
        data
      )
    ) {

      displayManager.updateData(
        data
      );
    }
  }

  // Update animations and check connection timeout
  displayManager.update();

  delay(5);
}