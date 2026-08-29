#include "StorageManager.h"

// Instantiate global StorageManager
StorageManager storageManager;

StorageManager::StorageManager() {
}

void StorageManager::save(const DisplayData& data, uint32_t timestamp, bool isMockData) {

  if (!data.valid) {
    return;
  }

  _preferences.begin("dass_telemetry", false);

  _preferences.putBool("valid", true);
  _preferences.putBool("isMock", isMockData);
  _preferences.putInt("txId", data.transmitterId);
  _preferences.putInt("tankId", data.tankId);
  _preferences.putFloat("dist", data.distanceCm);
  _preferences.putFloat("tankPct", data.tankPercent);
  _preferences.putFloat("litres", data.currentLitres);
  _preferences.putFloat("cap", data.capacityLitres);
  _preferences.putFloat("batPct", data.batteryPercent);
  _preferences.putFloat("batV", data.batteryVoltage);
  _preferences.putBool("chg", data.charging);
  _preferences.putBool("hasBat", data.hasBattery);
  _preferences.putUInt("seq", (uint32_t)data.sequence);
  _preferences.putUInt("time", timestamp);
  _preferences.putInt("rssi", data.rssi);
  _preferences.putInt("snr", data.snr);

  _preferences.end();

  Serial.print("Telemetry persisted to NVS (Type: ");
  Serial.print(isMockData ? "MOCK" : "REAL LORA");
  Serial.println(").");
}

bool StorageManager::load(DisplayData& data, uint32_t& timestamp, bool currentIsMock) {

  _preferences.begin("dass_telemetry", true);

  bool valid = _preferences.getBool("valid", false);

  if (!valid) {
    _preferences.end();
    return false;
  }

  bool storedIsMock = _preferences.getBool("isMock", false);

  _preferences.end();

  // If there is a mismatch between the stored data type and the current operating mode:
  if (storedIsMock != currentIsMock) {
    Serial.println("==================================================");
    Serial.print("Storage type mismatch detected! Stored was: ");
    Serial.print(storedIsMock ? "MOCK" : "REAL LORA");
    Serial.print(", Current mode is: ");
    Serial.println(currentIsMock ? "MOCK" : "REAL LORA");
    Serial.println("Clearing mismatched storage and waiting for new data...");
    Serial.println("==================================================");

    clear();
    return false;
  }

  _preferences.begin("dass_telemetry", true);

  data.valid = true;
  data.transmitterId = _preferences.getInt("txId", 1);
  data.tankId = _preferences.getInt("tankId", 1);
  data.distanceCm = _preferences.getFloat("dist", 0.0f);
  data.tankPercent = _preferences.getFloat("tankPct", 0.0f);
  data.currentLitres = _preferences.getFloat("litres", 0.0f);
  data.capacityLitres = _preferences.getFloat("cap", 0.0f);
  data.batteryPercent = _preferences.getFloat("batPct", 0.0f);
  data.batteryVoltage = _preferences.getFloat("batV", 0.0f);
  data.charging = _preferences.getBool("chg", false);
  data.hasBattery = _preferences.getBool("hasBat", true);
  data.sequence = _preferences.getUInt("seq", 0);
  timestamp = _preferences.getUInt("time", 0);
  data.timestamp = timestamp;
  data.lastReceived = millis();
  data.rssi = _preferences.getInt("rssi", 0);
  data.snr = _preferences.getInt("snr", 0);

  _preferences.end();

  Serial.println("Restored telemetry data from persistent storage.");
  Serial.print("Restored Tank %: ");
  Serial.print(data.tankPercent, 1);
  Serial.print("%, Battery %: ");
  Serial.print(data.batteryPercent, 1);
  Serial.print("%, Type: ");
  Serial.println(storedIsMock ? "MOCK" : "REAL LORA");

  return true;
}

void StorageManager::clear() {

  _preferences.begin("dass_telemetry", false);
  _preferences.clear();
  _preferences.end();
  Serial.println("Cleared persisted telemetry storage.");
}
