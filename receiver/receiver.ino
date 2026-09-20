#include <Arduino.h>

#include "config/BoardConfig.h"
#include "config/BaseConfigManager.h"
#include "config/SystemConfig.h"
#include "config/LoRaConfig.h"
#include "config/PacketConfig.h"
#include "config/TankConfig.h"
#include "config/TransmitterConfig.h"
#include "config/WiFiConfig.h"
#include "config/TimeConfig.h"
#include "config/DevConfig.h"
#include "config/BatteryLedConfig.h"
#include "config/ConfigJsonHelper.h"
#include "config/DisplayConfig.h"
#include "config/DisplayLayoutConfig.h"

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
#include "config/SystemConfig.cpp"
#include "config/WiFiConfig.cpp"
#include "config/TankConfig.cpp"
#include "config/TransmitterConfig.cpp"
#include "config/BatteryLedConfig.cpp"
#include "config/LoRaConfig.cpp"
#include "config/TimeConfig.cpp"
#include "config/DevConfig.cpp"
#include "config/DisplayLayoutConfig.cpp"
#include "config/ConfigJsonHelper.cpp"

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

  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("================================");
  Serial.println("DASS HOME Receiver");
  Serial.println("TankSync ESP32 Receiver");
  Serial.println("================================");

  // Initialize 8 distinct persistent configuration objects
  systemConfig.begin();
  wifiConfig.begin();
  tankConfig.begin();
  transmitterConfig.begin();
  batteryConfig.begin();
  loraConfigManager.begin();
  timeConfig.begin();
  devConfig.begin();
  displayLayoutConfig.begin();

  // Initialize display
  displayManager.begin();

  // Initialize battery LED indicator & INA219 monitor
  batteryLedManager.begin();

  // Initialize button input (for normal navigation & long-press selection menu)
  buttonManager.begin();

  // Initialize Wi-Fi lifecycle (OFF by default, ready for config mode & packet sync)
  wifiManager.begin();

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
  // STEP 3: INITIAL WI-FI SETUP CHECK
  // =====================================================
  if (!wifiManager.hasConfiguredSSID()) {
    Serial.println("[Boot] Fresh boot: No Wi-Fi configured yet. Entering Initial Setup Config Mode.");
    displayManager.setSection(SECTION_CONFIG, 0);
  }

  // =====================================================
  // STEP 4: INITIALIZE DATA SOURCE (LORA / MOCK)
  // =====================================================
  if (USE_MOCK_DATA) {
    mockDataManager.begin();
    Serial.println("Mock Data Mode: ACTIVE (Sending simulated telemetry every 1 min).");
  } else {
    loraManager.begin();
  }

  Serial.println();
  Serial.println("Receiver ready.");
  Serial.println("Wi-Fi is OFF by default. Listening for packets...");
}

// =====================================================
// =====================================================
//                          LOOP
// =====================================================
// =====================================================

void loop() {

  // Update battery LED indicator & INA219 monitoring
  batteryLedManager.update();

  // Detect transition into charging and trigger charging animation
  displayManager.checkChargingTransition(batteryLedManager.isCharging());

  // Handle button input (short-press page navigation & long-press config mode)
  buttonManager.update();

  // Update Wi-Fi lifecycle manager (handles web server clients, captive portal & transient timestamp sync)
  wifiManager.update();

  // Unified display update (handles config mode screen, selection menu, dev screens, normal screens, animations & timeouts)
  displayManager.update();

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
    // Only request Wi-Fi timestamp synchronization if Wi-Fi is configured
    if (wifiManager.hasConfiguredSSID()) {
      wifiManager.requestTimestampSync();
    }

    DisplayData data;
    if (tankProcessor.process(raw, rssi, snr, data)) {
      // Attach current NTP epoch timestamp (or 0 fallback handled by TimeManager)
      data.timestamp = timeManager.getEpoch();

      // Persist latest telemetry to NVS flash across power cycles with data mode tag
      storageManager.save(data, data.timestamp, USE_MOCK_DATA);

      // Update in-memory cache and refresh display
      displayManager.updateData(data);
    }
  }

  delay(2);
}