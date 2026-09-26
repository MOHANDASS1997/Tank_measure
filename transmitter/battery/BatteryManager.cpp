#include "BatteryManager.h"

// Instantiate global BatteryManager
BatteryManager batteryManager;

BatteryManager::BatteryManager() :
  _useHardwareADC(false),
  _ina219(INA219_I2C_ADDRESS),
  _ina219Ok(false) {
}

void BatteryManager::begin() {
  // Initialize INA219 over I2C using GPIO0 (SDA) and GPIO1 (SCL)
  _ina219Ok = _ina219.begin(INA219_SDA_PIN, INA219_SCL_PIN);

  if (_ina219Ok) {
    Serial.println("INA219 Battery Monitor initialized successfully.");
  } else {
    Serial.println("WARNING: INA219 Battery Monitor not responding. "
                   "Battery readings will be unavailable.");
  }
}

void BatteryManager::setHardwareMode(bool enabled) {
  _useHardwareADC = enabled;
}

void BatteryManager::readBattery(float &voltage, bool &isCharging) {
  // --------------------------------------------------------
  // INA219 path (hardware)
  // --------------------------------------------------------
  if (_ina219Ok) {
    // Battery voltage: INA219 bus-voltage measurement.
    // The INA219 is wired: CN3065 BATT+ -> VIN+ -> VIN- -> Battery+
    // Bus voltage is measured at VIN- (battery side), which is the actual
    // battery terminal voltage (~3.0V–4.2V).
    voltage = _ina219.getBusVoltage_V();

    // Battery current from the INA219 shunt register.
    // The INA219Driver sign convention (as documented in INA219Driver.h):
    //   Positive current = current flowing out of battery (discharging)
    //   Negative current = current flowing into battery (charging)
    //
    // Wiring is the same as the receiver:
    //   CN3065 BATT+ -> INA219 VIN+ -> INA219 VIN- -> Battery+
    // No sign correction needed — negative current means charging.
    float batteryCurrent_mA = _ina219.getCurrent_mA();

    // Charging is true only when current is below the (negative) threshold
    isCharging = (batteryCurrent_mA < CHARGING_CURRENT_THRESHOLD_MA);

    // Log battery data using the project's existing Serial logging
    Serial.print("Battery Voltage: ");
    Serial.print(voltage, 2);
    Serial.println(" V");
    Serial.print("Battery Current: ");
    Serial.print(batteryCurrent_mA, 2);
    Serial.println(" mA");
    Serial.print("Charging: ");
    Serial.println(isCharging ? "true" : "false");

    return;
  }

  // --------------------------------------------------------
  // Fallback: INA219 not available
  // Return safe zero/false values — same representation the
  // project uses when data is unavailable. Do NOT crash or
  // use dummy/simulated values.
  // --------------------------------------------------------
  Serial.println("WARNING: INA219 unavailable — battery data not reported.");
  voltage = 0.0f;
  isCharging = false;
}
