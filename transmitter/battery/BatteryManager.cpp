#include "BatteryManager.h"

// Instantiate global BatteryManager
BatteryManager batteryManager;

BatteryManager::BatteryManager() :
  _useHardwareADC(false) {
}

void BatteryManager::begin() {
  // If hardware ADC pin is connected in the future (e.g. GPIO 0 or 1 on ESP32-C3):
  // analogReadResolution(12);
}

void BatteryManager::setHardwareMode(bool enabled) {
  _useHardwareADC = enabled;
}

void BatteryManager::readBattery(float &voltage, bool &isCharging) {
  if (_useHardwareADC) {
    // Placeholder for real hardware ADC reading
    // e.g. int raw = analogRead(BATTERY_ADC_PIN);
    // voltage = (raw / 4095.0f) * 3.3f * 2.0f; // with 1:1 voltage divider
    voltage = 3.90f;
    isCharging = false;
    return;
  }

  // Placeholder / Simulated Mode:
  // Generate pseudo-random realistic voltage between 3.30V and 4.20V
  // using esp_random()
  uint32_t randVal = esp_random();
  
  // Map 0..90 to 3.30V..4.20V in 0.01V increments
  int step = (randVal % 91); // 0 to 90
  voltage = 3.30f + (step / 100.0f);

  // If voltage is higher (> 4.00V), simulate 35% chance of charging
  if (voltage >= 4.00f) {
    isCharging = ((randVal >> 8) % 3) == 0; // true ~33%
  } else {
    isCharging = false;
  }
}
