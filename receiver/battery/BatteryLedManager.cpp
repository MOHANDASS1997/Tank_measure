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
  if (now - _lastSensorRead >= BATTERY_POLL_INTERVAL_MS || _lastSensorRead == 0) {
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
  if (_current_mA < CURRENT_CHARGING_THRESHOLD_MA) {
    return "CHARGING";
  } else if (_current_mA > CURRENT_DISCHARGING_THRESHOLD_MA) {
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

// =====================================================
//              PERCENTAGE CALCULATION
// =====================================================
float BatteryLedManager::calculatePercentage(float voltage) {
  // Voltage above or equal to highest point (4.20V)
  if (voltage >= VOLTAGE_PERCENT_TABLE[0].voltage) {
    return 100.0f;
  }

  // Voltage below or equal to lowest point (3.50V)
  if (voltage <= VOLTAGE_PERCENT_TABLE[VOLTAGE_PERCENT_TABLE_SIZE - 1].voltage) {
    return 0.0f;
  }

  // Piecewise linear interpolation between points
  for (size_t i = 0; i < VOLTAGE_PERCENT_TABLE_SIZE - 1; i++) {
    float vHigh = VOLTAGE_PERCENT_TABLE[i].voltage;
    float vLow  = VOLTAGE_PERCENT_TABLE[i + 1].voltage;
    float pHigh = VOLTAGE_PERCENT_TABLE[i].percent;
    float pLow  = VOLTAGE_PERCENT_TABLE[i + 1].percent;

    if (voltage <= vHigh && voltage >= vLow) {
      float fraction = (voltage - vLow) / (vHigh - vLow);
      float pct = pLow + fraction * (pHigh - pLow);
      return constrain(pct, 0.0f, 100.0f);
    }
  }

  return 0.0f;
}

// =====================================================
//              CHARGING DETECTION
// =====================================================
bool BatteryLedManager::isChargingCurrent(float current_mA) {
  // Current < -50 mA  -> CHARGING
  // Current > +50 mA  -> DISCHARGING
  // -50 mA to +50 mA  -> NOT CHARGING
  return (current_mA < CURRENT_CHARGING_THRESHOLD_MA);
}

// =====================================================
//                 SENSOR READING
// =====================================================
void BatteryLedManager::updateSensors() {
  _voltage = _ina219.getBusVoltage_V();
  _shunt_mV = _ina219.getShuntVoltage_mV();
  _load_voltage = _ina219.getLoadVoltage_V();
  _current_mA = _ina219.getCurrent_mA();
  _power_mW = _ina219.getPower_mW();

  _charging = isChargingCurrent(_current_mA);
  _batteryPercent = calculatePercentage(_voltage);
}

// =====================================================
//                 LED CONTROLLER
// =====================================================
void BatteryLedManager::updateLeds() {
  unsigned long now = millis();

  bool normalBlinkOn = ((now / NORMAL_BLINK_INTERVAL_MS) % 2) == 0;
  bool fastBlinkOn   = ((now / FAST_BLINK_INTERVAL_MS) % 2) == 0;

  BatteryLedMode led1Mode = LED_OFF;
  BatteryLedMode led2Mode = LED_OFF;
  BatteryLedMode led3Mode = LED_OFF;
  BatteryLedMode led4Mode = LED_OFF;
  BatteryLedMode led5Mode = LED_OFF;

  if (_charging) {
    // ===================================================
    // CHARGING BEHAVIOR (Normal blink indicates charging)
    // ===================================================
    if (_batteryPercent >= 90.0f) {
      // >=90%: All 5 LEDs NORMAL BLINK simultaneously & synchronously
      led1Mode = LED_NORMAL_BLINK;
      led2Mode = LED_NORMAL_BLINK;
      led3Mode = LED_NORMAL_BLINK;
      led4Mode = LED_NORMAL_BLINK;
      led5Mode = LED_NORMAL_BLINK;
    } else if (_batteryPercent >= 80.0f) {
      // 80 - <90%: LEDs 1-4 SOLID, LED 5 NORMAL BLINK
      led1Mode = LED_SOLID;
      led2Mode = LED_SOLID;
      led3Mode = LED_SOLID;
      led4Mode = LED_SOLID;
      led5Mode = LED_NORMAL_BLINK;
    } else if (_batteryPercent >= 60.0f) {
      // 60 - <80%: LEDs 1-3 SOLID, LED 4 NORMAL BLINK, LED 5 OFF
      led1Mode = LED_SOLID;
      led2Mode = LED_SOLID;
      led3Mode = LED_SOLID;
      led4Mode = LED_NORMAL_BLINK;
      led5Mode = LED_OFF;
    } else if (_batteryPercent >= 40.0f) {
      // 40 - <60%: LEDs 1-2 SOLID, LED 3 NORMAL BLINK, LEDs 4-5 OFF
      led1Mode = LED_SOLID;
      led2Mode = LED_SOLID;
      led3Mode = LED_NORMAL_BLINK;
      led4Mode = LED_OFF;
      led5Mode = LED_OFF;
    } else if (_batteryPercent >= 20.0f) {
      // 20 - <40%: LED 1 SOLID, LED 2 NORMAL BLINK, LEDs 3-5 OFF
      led1Mode = LED_SOLID;
      led2Mode = LED_NORMAL_BLINK;
      led3Mode = LED_OFF;
      led4Mode = LED_OFF;
      led5Mode = LED_OFF;
    } else {
      // 0 - <20%: LED 1 NORMAL BLINK, LEDs 2-5 OFF
      led1Mode = LED_NORMAL_BLINK;
      led2Mode = LED_OFF;
      led3Mode = LED_OFF;
      led4Mode = LED_OFF;
      led5Mode = LED_OFF;
    }
  } else {
    // ===================================================
    // NOT CHARGING / DISCHARGING BEHAVIOR
    // ===================================================
    if (_batteryPercent >= 80.0f) {
      // 80 - 100%: LEDs 1-5 SOLID
      led1Mode = LED_SOLID;
      led2Mode = LED_SOLID;
      led3Mode = LED_SOLID;
      led4Mode = LED_SOLID;
      led5Mode = LED_SOLID;
    } else if (_batteryPercent >= 60.0f) {
      // 60 - <80%: LEDs 1-4 SOLID, LED 5 OFF
      led1Mode = LED_SOLID;
      led2Mode = LED_SOLID;
      led3Mode = LED_SOLID;
      led4Mode = LED_SOLID;
      led5Mode = LED_OFF;
    } else if (_batteryPercent >= 40.0f) {
      // 40 - <60%: LEDs 1-3 SOLID, LEDs 4-5 OFF
      led1Mode = LED_SOLID;
      led2Mode = LED_SOLID;
      led3Mode = LED_SOLID;
      led4Mode = LED_OFF;
      led5Mode = LED_OFF;
    } else if (_batteryPercent >= 20.0f) {
      // 20 - <40%: LEDs 1-2 SOLID, LEDs 3-5 OFF
      led1Mode = LED_SOLID;
      led2Mode = LED_SOLID;
      led3Mode = LED_OFF;
      led4Mode = LED_OFF;
      led5Mode = LED_OFF;
    } else if (_batteryPercent >= 10.0f) {
      // 10 - <20%: LED 1 SOLID, LEDs 2-5 OFF
      led1Mode = LED_SOLID;
      led2Mode = LED_OFF;
      led3Mode = LED_OFF;
      led4Mode = LED_OFF;
      led5Mode = LED_OFF;
    } else {
      // <10%: LED 1 FAST BLINK, LEDs 2-5 OFF
      led1Mode = LED_FAST_BLINK;
      led2Mode = LED_OFF;
      led3Mode = LED_OFF;
      led4Mode = LED_OFF;
      led5Mode = LED_OFF;
    }
  }

  // Drive LED output pins and record their live active states
  _ledStates[0] = driveLed(BATTERY_LED_1_PIN, led1Mode, normalBlinkOn, fastBlinkOn);
  _ledStates[1] = driveLed(BATTERY_LED_2_PIN, led2Mode, normalBlinkOn, fastBlinkOn);
  _ledStates[2] = driveLed(BATTERY_LED_3_PIN, led3Mode, normalBlinkOn, fastBlinkOn);
  _ledStates[3] = driveLed(BATTERY_LED_4_PIN, led4Mode, normalBlinkOn, fastBlinkOn);
  _ledStates[4] = driveLed(BATTERY_LED_5_PIN, led5Mode, normalBlinkOn, fastBlinkOn);
}

// =====================================================
//                 PIN DRIVER
// =====================================================
bool BatteryLedManager::driveLed(uint8_t pin, BatteryLedMode mode, bool normalBlinkOn, bool fastBlinkOn) {
  bool pinState = false;

  switch (mode) {
    case LED_SOLID:
      pinState = true;
      break;

    case LED_NORMAL_BLINK:
      pinState = normalBlinkOn;
      break;

    case LED_FAST_BLINK:
      pinState = fastBlinkOn;
      break;

    case LED_OFF:
    default:
      pinState = false;
      break;
  }

  digitalWrite(pin, pinState ? HIGH : LOW);
  return pinState;
}
