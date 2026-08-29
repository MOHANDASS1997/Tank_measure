#include "MockDataManager.h"

// Instantiate global MockDataManager
MockDataManager mockDataManager;

MockDataManager::MockDataManager()
  : _startTime(0),
    _lastMockTime(0),
    _initialDelayMs(MOCK_INITIAL_DELAY_MS),
    _intervalMs(MOCK_DATA_INTERVAL_MS),
    _mockSequence(100),
    _stateIndex(0),
    _firstPoll(true) {
}

void MockDataManager::begin(unsigned long initialDelayMs, unsigned long intervalMs) {

  _initialDelayMs = initialDelayMs;
  _intervalMs = intervalMs;
  _startTime = millis();
  _lastMockTime = millis();
  _firstPoll = true;
  _stateIndex = 0;

  Serial.println("================================");
  Serial.print("Mock Data Layer initialized. First packet in ");
  Serial.print(_initialDelayMs / 1000);
  Serial.print("s, then every ");
  Serial.print(_intervalMs / 1000);
  Serial.println("s.");
  Serial.println("================================");
}

bool MockDataManager::poll(RawTelemetry& raw, int& rssi, int& snr) {

  // Wait for initial delay before emitting the very first mock packet
  if (_firstPoll) {
    if (millis() - _startTime < _initialDelayMs) {
      return false;
    }
    _firstPoll = false;
    _lastMockTime = millis();
  } else {
    if (millis() - _lastMockTime < _intervalMs) {
      return false;
    }
    _lastMockTime = millis();
  }

  _mockSequence++;
  raw.transmitterId = 1;
  raw.sequence = _mockSequence;
  rssi = -68 - (_stateIndex * 3);
  snr = 9 + (_stateIndex % 4);

  // Rotate through diverse states
  switch (_stateIndex) {
    case 0:
      // State 0: Tank ~75%, Battery 80% (3.95V), Not Charging
      raw.distanceCm = 30.0f;
      raw.batteryVoltage = 3.95f;
      raw.charging = false;
      raw.hasBattery = true;
      Serial.println("[MOCK] Generated packet: Tank 75%, Battery 3.95V (Not Charging)");
      break;

    case 1:
      // State 1: Tank ~45%, Battery 95% (4.10V), Charging
      raw.distanceCm = 54.0f;
      raw.batteryVoltage = 4.10f;
      raw.charging = true;
      raw.hasBattery = true;
      Serial.println("[MOCK] Generated packet: Tank 45%, Battery 4.10V (CHARGING)");
      break;

    case 2:
      // State 2: Tank ~90%, NO BATTERY DATA
      raw.distanceCm = 18.0f;
      raw.batteryVoltage = 0.0f;
      raw.charging = false;
      raw.hasBattery = false;
      Serial.println("[MOCK] Generated packet: Tank 90%, NO BATTERY DATA");
      break;

    case 3:
      // State 3: Tank ~15%, Low Battery 20% (3.45V), Not Charging
      raw.distanceCm = 78.0f;
      raw.batteryVoltage = 3.45f;
      raw.charging = false;
      raw.hasBattery = true;
      Serial.println("[MOCK] Generated packet: Tank 15%, Battery 3.45V (Low Battery)");
      break;

    case 4:
    default:
      // State 4: Tank ~60%, Battery 60% (3.80V), Charging
      raw.distanceCm = 42.0f;
      raw.batteryVoltage = 3.80f;
      raw.charging = true;
      raw.hasBattery = true;
      Serial.println("[MOCK] Generated packet: Tank 60%, Battery 3.80V (CHARGING)");
      break;
  }

  _stateIndex = (_stateIndex + 1) % 5;
  return true;
}
