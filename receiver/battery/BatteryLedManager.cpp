#include "BatteryLedManager.h"

// Instantiate global BatteryLedManager
BatteryLedManager batteryLedManager;

BatteryLedManager::BatteryLedManager()
  : _ina219(INA219_I2C_ADDRESS),
    _voltage(0.0f),
    _shunt_mV(0.0f),
    _load_voltage(0.0f),
    _current_mA(0.0f),
    _power_mW(0.0f),
    _batteryPercent(0.0f),
    _charging(false),
    _ledsEnabled(true),
    _activeLedCount(1),
    _isLowBattery(false),
    _isChargingFull(false),
    _firstSensorRead(true),
    _lastSensorRead(0) {
  for (int i = 0; i < 5; i++) {
    _ledStates[i] = false;
  }
}

// =====================================================
//                 INITIALIZATION
// =====================================================
void BatteryLedManager::begin() {
  // Configure LED pins as OUTPUT and turn OFF (LOW)
  pinMode(BATTERY_LED_1_PIN, OUTPUT);
  pinMode(BATTERY_LED_2_PIN, OUTPUT);
  pinMode(BATTERY_LED_3_PIN, OUTPUT);
  pinMode(BATTERY_LED_4_PIN, OUTPUT);
  pinMode(BATTERY_LED_5_PIN, OUTPUT);

  digitalWrite(BATTERY_LED_1_PIN, LOW);
  digitalWrite(BATTERY_LED_2_PIN, LOW);
  digitalWrite(BATTERY_LED_3_PIN, LOW);
  digitalWrite(BATTERY_LED_4_PIN, LOW);
  digitalWrite(BATTERY_LED_5_PIN, LOW);

  // Initialize INA219 sensor on I2C (GPIO 21 SDA, GPIO 22 SCL)
  bool success = _ina219.begin(INA219_I2C_SDA, INA219_I2C_SCL);
  if (success) {
    Serial.println("INA219 Battery Monitor initialized successfully.");
  } else {
    Serial.println("INA219 Battery Monitor initialization warning: Device not responding at 0x40.");
  }

  // Initial sensor read
  updateSensors();
}

// =====================================================
//                 MAIN UPDATE LOOP
// =====================================================
void BatteryLedManager::update() {
  unsigned long now = millis();

  // Periodically read INA219 sensor data
  if (now - _lastSensorRead >= batteryConfig.get().batteryPollIntervalMs || _lastSensorRead == 0) {
    _lastSensorRead = now;
    updateSensors();
  }

  // Update LED states on each tick using non-blocking timing
  updateLeds();
}

// =====================================================
//                 GETTERS
// =====================================================
float BatteryLedManager::getVoltage() const {
  return _voltage;
}

float BatteryLedManager::getShuntVoltage_mV() const {
  return _shunt_mV;
}

float BatteryLedManager::getLoadVoltage_V() const {
  return _load_voltage;
}

float BatteryLedManager::getCurrent_mA() const {
  return _current_mA;
}

float BatteryLedManager::getPower_mW() const {
  return _power_mW;
}

float BatteryLedManager::getBatteryPercent() const {
  return _batteryPercent;
}

bool BatteryLedManager::isCharging() const {
  return _charging;
}

const char* BatteryLedManager::getChargingStateStr() const {
  if (_current_mA < batteryConfig.get().currentChargingThresholdMa) {
    return "CHARGING";
  } else if (_current_mA > batteryConfig.get().currentDischargingThresholdMa) {
    return "DISCHARGING";
  } else {
    return "NOT CHARGING";
  }
}

bool BatteryLedManager::isSensorConnected() const {
  return _ina219.isConnected();
}

bool BatteryLedManager::getLedPinState(uint8_t index) const {
  if (index < 5) {
    return _ledStates[index];
  }
  return false;
}

void BatteryLedManager::setLedsEnabled(bool enabled) {
  _ledsEnabled = enabled;
  if (!_ledsEnabled && !_charging) {
    for (int i = 0; i < 5; i++) {
      _ledStates[i] = false;
    }
    digitalWrite(BATTERY_LED_1_PIN, LOW);
    digitalWrite(BATTERY_LED_2_PIN, LOW);
    digitalWrite(BATTERY_LED_3_PIN, LOW);
    digitalWrite(BATTERY_LED_4_PIN, LOW);
    digitalWrite(BATTERY_LED_5_PIN, LOW);
  }
}

bool BatteryLedManager::areLedsEnabled() const {
  return _ledsEnabled;
}

// =====================================================
//              PERCENTAGE CALCULATION
// =====================================================
float BatteryLedManager::calculatePercentage(float voltage) {
  const auto& cfg = batteryConfig.get();
  if (cfg.voltageTableCount == 0) {
    return 0.0f;
  }

  // Voltage above or equal to highest point
  if (voltage >= cfg.voltageTable[0].voltage) {
    return 100.0f;
  }

  // Voltage below or equal to lowest point
  if (voltage <= cfg.voltageTable[cfg.voltageTableCount - 1].voltage) {
    return 0.0f;
  }

  // Piecewise linear interpolation between points
  for (size_t i = 0; i < cfg.voltageTableCount - 1; i++) {
    float vHigh = cfg.voltageTable[i].voltage;
    float vLow  = cfg.voltageTable[i + 1].voltage;
    float pHigh = cfg.voltageTable[i].percent;
    float pLow  = cfg.voltageTable[i + 1].percent;

    if (voltage <= vHigh && voltage >= vLow) {
      float fraction = (vHigh == vLow) ? 0.0f : (voltage - vLow) / (vHigh - vLow);
      float pct = pLow + fraction * (pHigh - pLow);
      return constrain(pct, 0.0f, 100.0f);
    }
  }

  return 0.0f;
}

// =====================================================
//      CONFIGURABLE BATTERY PERCENTAGE TO LED COUNT
// =====================================================
uint8_t BatteryLedManager::calculateLedCount(float batteryPercent) {
  const auto& cfg = batteryConfig.get();
  for (size_t i = 0; i < cfg.ledThresholdCount; i++) {
    if (batteryPercent > cfg.ledThresholds[i].minPercent) {
      return cfg.ledThresholds[i].ledCount;
    }
  }
  return 1;
}

uint8_t BatteryLedManager::calculateLedCountWithHysteresis(uint8_t currentCount, float batteryPercent, float hysteresis) {
  const auto& cfg = batteryConfig.get();
  for (size_t i = 0; i < cfg.ledThresholdCount; i++) {
    float threshold = cfg.ledThresholds[i].minPercent;
    uint8_t count = cfg.ledThresholds[i].ledCount;

    if (threshold <= 0.0f) {
      return count;
    }

    float effectiveThreshold = threshold;
    if (currentCount >= count) {
      // Already at or above this count: keep it unless it drops below (threshold - hysteresis)
      effectiveThreshold = threshold - hysteresis;
    } else {
      // Currently below this count: requires exceeding (threshold + hysteresis) to step up
      effectiveThreshold = threshold + hysteresis;
    }

    if (batteryPercent > effectiveThreshold) {
      return count;
    }
  }
  return 1;
}

bool BatteryLedManager::updateLowBatteryWithHysteresis(bool currentLowBattery, float batteryPercent, float lowThreshold, float hysteresis) {
  if (currentLowBattery) {
    if (batteryPercent > (lowThreshold + hysteresis)) {
      return false;
    }
    return true;
  } else {
    if (batteryPercent < (lowThreshold - hysteresis)) {
      return true;
    }
    return false;
  }
}

bool BatteryLedManager::updateChargingFullWithHysteresis(bool currentFull, float batteryPercent, float fullThreshold, float hysteresis) {
  if (currentFull) {
    if (batteryPercent < (fullThreshold - hysteresis)) {
      return false;
    }
    return true;
  } else {
    if (batteryPercent >= fullThreshold) {
      return true;
    }
    return false;
  }
}

// =====================================================
//              CHARGING DETECTION
// =====================================================
bool BatteryLedManager::isChargingCurrent(float current_mA) {
  return (current_mA < batteryConfig.get().currentChargingThresholdMa);
}

// =====================================================
//                 SENSOR READING
// =====================================================
void BatteryLedManager::updateSensors() {
  float rawVoltage = _ina219.getBusVoltage_V();
  _shunt_mV = _ina219.getShuntVoltage_mV();
  _load_voltage = _ina219.getLoadVoltage_V();
  _current_mA = _ina219.getCurrent_mA();
  _power_mW = _ina219.getPower_mW();

  const auto& cfg = batteryConfig.get();

  if (_firstSensorRead) {
    _voltage = rawVoltage;
    _firstSensorRead = false;
  } else {
    _voltage = _voltage + cfg.batteryVoltageEmaAlpha * (rawVoltage - _voltage);
  }

  _charging = isChargingCurrent(_current_mA);
  _batteryPercent = calculatePercentage(_voltage);

  _activeLedCount = calculateLedCountWithHysteresis(_activeLedCount, _batteryPercent, cfg.batteryLedHysteresisPercent);
  _isLowBattery = updateLowBatteryWithHysteresis(_isLowBattery, _batteryPercent, cfg.batteryLedLowThreshold, cfg.batteryLedHysteresisPercent);
  _isChargingFull = updateChargingFullWithHysteresis(_isChargingFull, _batteryPercent, cfg.batteryChargingFullThreshold, cfg.batteryLedHysteresisPercent);
}

// =====================================================
//                 LED CONTROLLER
// =====================================================
void BatteryLedManager::updateLeds() {
  // If not charging and LEDs are disabled by UI timeout sleep, turn all LEDs OFF
  if (!_charging && !_ledsEnabled) {
    for (int i = 0; i < 5; i++) {
      _ledStates[i] = false;
    }
    digitalWrite(BATTERY_LED_1_PIN, LOW);
    digitalWrite(BATTERY_LED_2_PIN, LOW);
    digitalWrite(BATTERY_LED_3_PIN, LOW);
    digitalWrite(BATTERY_LED_4_PIN, LOW);
    digitalWrite(BATTERY_LED_5_PIN, LOW);
    return;
  }

  unsigned long now = millis();
  bool blinkOn = ((now / batteryConfig.get().batteryLedBlinkIntervalMs) % 2) == 0;

  BatteryLedMode led1Mode = LED_OFF;
  BatteryLedMode led2Mode = LED_OFF;
  BatteryLedMode led3Mode = LED_OFF;
  BatteryLedMode led4Mode = LED_OFF;
  BatteryLedMode led5Mode = LED_OFF;

  if (_charging) {
    // ===================================================
    // CHARGING BEHAVIOR
    // LED5: Charging indicator
    //   - Normal charging (< 90%): Blinks
    //   - Full charge (>= 90% with hysteresis): Solid glow
    // LED1-LED4: Battery level (remain SOLID according to SOC with hysteresis)
    // ===================================================

    // Charging Indicator (LED5)
    led5Mode = _isChargingFull ? LED_SOLID : LED_BLINK;

    // Battery LEDs (LED1 - LED4) - remain solid according to SOC using configurable threshold & hysteresis
    led1Mode = (_activeLedCount >= 1) ? LED_SOLID : LED_OFF;
    led2Mode = (_activeLedCount >= 2) ? LED_SOLID : LED_OFF;
    led3Mode = (_activeLedCount >= 3) ? LED_SOLID : LED_OFF;
    led4Mode = (_activeLedCount >= 4) ? LED_SOLID : LED_OFF;

  } else {
    // ===================================================
    // NOT CHARGING / DISCHARGING BEHAVIOR
    // LED5: OFF
    // LED1-LED4: Battery level (<10% blinks, otherwise solid with hysteresis)
    // ===================================================
    led5Mode = LED_OFF;

    if (_isLowBattery) {
      // <10% with hysteresis: LED1 blinking, LED2-4 OFF
      led1Mode = LED_BLINK;
      led2Mode = LED_OFF;
      led3Mode = LED_OFF;
      led4Mode = LED_OFF;
    } else {
      led1Mode = (_activeLedCount >= 1) ? LED_SOLID : LED_OFF;
      led2Mode = (_activeLedCount >= 2) ? LED_SOLID : LED_OFF;
      led3Mode = (_activeLedCount >= 3) ? LED_SOLID : LED_OFF;
      led4Mode = (_activeLedCount >= 4) ? LED_SOLID : LED_OFF;
    }
  }

  // Drive LED output pins and record their live active states
  _ledStates[0] = driveLed(BATTERY_LED_1_PIN, led1Mode, blinkOn);
  _ledStates[1] = driveLed(BATTERY_LED_2_PIN, led2Mode, blinkOn);
  _ledStates[2] = driveLed(BATTERY_LED_3_PIN, led3Mode, blinkOn);
  _ledStates[3] = driveLed(BATTERY_LED_4_PIN, led4Mode, blinkOn);
  _ledStates[4] = driveLed(BATTERY_LED_5_PIN, led5Mode, blinkOn);
}

// =====================================================
//                 PIN DRIVER
// =====================================================
bool BatteryLedManager::driveLed(uint8_t pin, BatteryLedMode mode, bool blinkOn) {
  bool pinState = false;

  switch (mode) {
    case LED_SOLID:
      pinState = true;
      break;

    case LED_BLINK:
      pinState = blinkOn;
      break;

    case LED_OFF:
    default:
      pinState = false;
      break;
  }

  digitalWrite(pin, pinState ? HIGH : LOW);
  return pinState;
}
