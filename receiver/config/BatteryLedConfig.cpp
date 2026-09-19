#include "BatteryLedConfig.h"

BatteryConfigManager batteryConfig;

BatteryConfigManager::BatteryConfigManager() {
  loadDefaults();
}

void BatteryConfigManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
  _settings.currentChargingThresholdMa = -50.0f;
  _settings.currentDischargingThresholdMa = 50.0f;
  _settings.batteryLedLowThreshold = 10.0f;
  _settings.batteryChargingFullThreshold = 90.0f;
  _settings.batteryLedHysteresisPercent = 2.0f;
  _settings.batteryVoltageEmaAlpha = 0.25f;
  _settings.batteryPollIntervalMs = 200;
  _settings.batteryLedBlinkIntervalMs = 500;

  // LED Thresholds
  _settings.ledThresholdCount = 4;
  _settings.ledThresholds[0] = {85.0f, 4};
  _settings.ledThresholds[1] = {60.0f, 3};
  _settings.ledThresholds[2] = {25.0f, 2};
  _settings.ledThresholds[3] = {0.0f, 1};

  // Voltage-to-Percentage Table
  _settings.voltageTableCount = 12;
  const VoltagePercentPoint defaultTable[12] = {
    {4.20f, 100.0f}, {4.10f, 90.0f}, {4.00f, 80.0f}, {3.90f, 70.0f},
    {3.80f, 60.0f},  {3.70f, 50.0f}, {3.60f, 40.0f}, {3.50f, 30.0f},
    {3.40f, 20.0f},  {3.30f, 10.0f}, {3.20f, 5.0f},  {3.00f, 0.0f}
  };
  for (int i = 0; i < 12; i++) {
    _settings.voltageTable[i] = defaultTable[i];
  }
  for (int i = 12; i < MAX_VOLTAGE_TABLE_POINTS; i++) {
    memset(&_settings.voltageTable[i], 0, sizeof(VoltagePercentPoint));
  }
}

void BatteryConfigManager::begin() {
  if (!load()) {
    Serial.println("[BatteryConfig] No valid stored configuration found. Writing defaults.");
    loadDefaults();
    save();
  } else {
    Serial.println("[BatteryConfig] Loaded persistent configuration successfully.");
  }
}

bool BatteryConfigManager::load() {
  _prefs.begin("cfg_battery", true);
  size_t len = _prefs.getBytesLength("settings");
  if (len != sizeof(BatterySettings)) {
    _prefs.end();
    return false;
  }

  BatterySettings temp;
  _prefs.getBytes("settings", &temp, sizeof(BatterySettings));
  _prefs.end();

  if (temp.schemaVersion != CURRENT_SCHEMA_VERSION) {
    Serial.println("[BatteryConfig] Schema version mismatch; loading defaults.");
    return false;
  }

  String err;
  if (!validate(temp, err)) {
    Serial.print("[BatteryConfig] Validation failed: ");
    Serial.println(err);
    return false;
  }

  _settings = temp;
  return true;
}

bool BatteryConfigManager::save() {
  String err;
  if (!validate(_settings, err)) {
    Serial.print("[BatteryConfig] Cannot save invalid settings: ");
    Serial.println(err);
    return false;
  }

  _prefs.begin("cfg_battery", false);
  size_t written = _prefs.putBytes("settings", &_settings, sizeof(BatterySettings));
  _prefs.end();

  return (written == sizeof(BatterySettings));
}

bool BatteryConfigManager::validate(const BatterySettings& s, String& err) {
  if (s.currentChargingThresholdMa >= s.currentDischargingThresholdMa) {
    err = "Charging current threshold must be less than discharging current threshold";
    return false;
  }
  if (s.batteryLedLowThreshold < 0.0f || s.batteryLedLowThreshold > 50.0f) {
    err = "Low battery threshold must be between 0% and 50%";
    return false;
  }
  if (s.batteryChargingFullThreshold < 50.0f || s.batteryChargingFullThreshold > 100.0f) {
    err = "Charging full threshold must be between 50% and 100%";
    return false;
  }
  if (s.batteryLedHysteresisPercent < 0.1f || s.batteryLedHysteresisPercent > 10.0f) {
    err = "Hysteresis must be between 0.1% and 10.0%";
    return false;
  }
  if (s.batteryVoltageEmaAlpha <= 0.0f || s.batteryVoltageEmaAlpha > 1.0f) {
    err = "Voltage filter EMA alpha must be between 0.01 and 1.0";
    return false;
  }
  if (s.batteryPollIntervalMs < 50 || s.batteryPollIntervalMs > 5000) {
    err = "Battery poll interval must be between 50ms and 5000ms";
    return false;
  }
  if (s.batteryLedBlinkIntervalMs < 100 || s.batteryLedBlinkIntervalMs > 2000) {
    err = "Blink interval must be between 100ms and 2000ms";
    return false;
  }
  if (s.voltageTableCount < 2 || s.voltageTableCount > MAX_VOLTAGE_TABLE_POINTS) {
    err = "Voltage table must have between 2 and " + String(MAX_VOLTAGE_TABLE_POINTS) + " points";
    return false;
  }
  // Check descending voltages
  for (uint8_t i = 0; i < s.voltageTableCount - 1; i++) {
    if (s.voltageTable[i].voltage <= s.voltageTable[i + 1].voltage) {
      err = "Voltage table must be strictly descending in voltage (point " + String(i) + " vs " + String(i + 1) + ")";
      return false;
    }
    if (s.voltageTable[i].percent < s.voltageTable[i + 1].percent) {
      err = "Voltage table percentage must be non-increasing (point " + String(i) + " vs " + String(i + 1) + ")";
      return false;
    }
  }

  return true;
}
