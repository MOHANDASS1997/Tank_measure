#include <Arduino.h>

#include "config/BoardConfig.h"
#include "config/LoRaConfig.h"
#include "config/PacketConfig.h"
#include "config/TankConfig.h"
#include "config/WiFiConfig.h"
#include "config/TimeConfig.h"
#include "config/BatteryLedConfig.h"

#include "models/Telemetry.h"
#include "models/DisplayData.h"

#include "protocol/PacketParser.h"
#include "lora/LoRaManager.h"
#include "tank/TankProcessor.h"
#include "display/DisplayManager.h"
#include "input/ButtonManager.h"
#include "wifi/WiFiManager.h"
#include "time/TimeManager.h"
#include "storage/StorageManager.h"
#include "mock/MockDataManager.h"
#include "battery/BatteryLedManager.h"

// Arduino IDE ignores .cpp files located in subdirectories unless included:
#include "protocol/PacketParser.cpp"
#include "lora/LoRaManager.cpp"
#include "tank/TankProcessor.cpp"
#include "display/DisplayManager.cpp"
#include "input/ButtonManager.cpp"
#include "wifi/WiFiManager.cpp"
#include "time/TimeManager.cpp"
#include "storage/StorageManager.cpp"
#include "mock/MockDataManager.cpp"
#include "battery/BatteryLedManager.cpp"

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
    "DASS HOME Receiver"
  );

  Serial.println(
    "LoRa Receiver"
  );

  Serial.println(
    "================================"
  );

  // Initialize display
  displayManager.begin();

  // Initialize battery LED indicator & INA219 monitor
  batteryLedManager.begin();

  // Initialize button input (for normal & test mode navigation)
  buttonManager.begin();

#if TEST_MODE
  displayManager.setTestMode(true);
  Serial.println();
  Serial.println("================================");
  Serial.println("TEST MODE: ACTIVE");
  Serial.println("Displaying diagnostic test screens only.");
  Serial.println("Press button on GPIO 27 to cycle test screens.");
  Serial.println("All normal screens disabled.");
  Serial.println("================================");
  return;
#endif

  // =====================================================
  // STEP 1: WI-FI STATUS CHECK & SETUP SCREEN
  // (Executes first before LoRa and any data checks)
  // =====================================================
  wifiManager.begin();
  wifiManager.runStartupFlow(displayManager, buttonManager);

  // Initialize NTP time synchronization
  timeManager.begin();

  // =====================================================
  // STEP 2: RESTORE TELEMETRY FROM PERSISTENT STORAGE
  // =====================================================
  DisplayData cachedData;
  uint32_t savedTimestamp = 0;
  bool hasSavedData = storageManager.load(cachedData, savedTimestamp, USE_MOCK_DATA);

  if (hasSavedData) {
    Serial.println("Restored telemetry data from storage. Displaying immediately.");
    displayManager.updateData(cachedData);
  } else {
    Serial.println("No stored matching telemetry found. Showing waiting screen.");
    displayManager.showNotConnected();
  }

  // =====================================================
  // STEP 3: INITIALIZE DATA SOURCE (LORA / MOCK)
  // =====================================================
  if (USE_MOCK_DATA) {
    mockDataManager.begin();
    Serial.println("Mock Data Mode: ACTIVE (Sending simulated telemetry every 1 min).");
  } else {
    loraManager.begin();
  }

  Serial.println();
  Serial.println(
    "Receiver ready."
  );

  Serial.println(
    "Waiting for DASS HOME packets..."
  );
}

// =====================================================
// =====================================================
//                          LOOP
// =====================================================
// =====================================================

void loop() {

  // Update battery LED indicator & INA219 monitoring
  batteryLedManager.update();

#if TEST_MODE
  // Handle button input for test screen navigation
  buttonManager.update();

  // Render current active test screen with live data
  displayManager.updateTestScreen(batteryLedManager);
  delay(30);
  return;
#endif

  // Handle button input & page navigation
  buttonManager.update();

  // Check for incoming packet (LoRa or Mock Data Layer)
  RawTelemetry raw;
  int rssi = 0;
  int snr = 0;
  bool packetReceived = false;

  if (USE_MOCK_DATA) {
    packetReceived = mockDataManager.poll(raw, rssi, snr);
  } else {
    packetReceived = loraManager.receive(raw, rssi, snr);
  }

  if (packetReceived) {

    DisplayData data;

    if (
      tankProcessor.process(
        raw,
        rssi,
        snr,
        data
      )
    ) {

      // Attach current NTP epoch timestamp
      data.timestamp = timeManager.getEpoch();

      // Persist latest telemetry to NVS flash across power cycles with data mode tag
      storageManager.save(data, data.timestamp, USE_MOCK_DATA);

      // Update in-memory cache and refresh display
      displayManager.updateData(
        data
      );
    }
  }

  // Update animations and dynamic footer elapsed time
  displayManager.update();

  delay(5);
}