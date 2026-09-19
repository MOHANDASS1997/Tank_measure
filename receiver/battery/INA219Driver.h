#pragma once

#include <Arduino.h>
#include <Wire.h>

// =====================================================
//                   INA219 REGISTERS
// =====================================================
#define INA219_REG_CONFIG          0x00
#define INA219_REG_SHUNTVOLTAGE    0x01
#define INA219_REG_BUSVOLTAGE      0x02
#define INA219_REG_POWER           0x03
#define INA219_REG_CURRENT         0x04
#define INA219_REG_CALIBRATION     0x05

// Default Config: 32V range, +/-320mV shunt, 12-bit ADC, continuous mode
#define INA219_CONFIG_DEFAULT      0x399F

// Default Calibration for 0.1 ohm shunt and 0.1mA LSB
#define INA219_CALIBRATION_DEFAULT 4096

class INA219Driver {
public:
  INA219Driver(uint8_t address = 0x40)
    : _address(address), _initialized(false) {}

  bool begin(uint8_t sdaPin = 21, uint8_t sclPin = 22) {
    Wire.begin(sdaPin, sclPin);

    // Test communication and configure sensor
    if (!writeRegister16(INA219_REG_CALIBRATION, INA219_CALIBRATION_DEFAULT)) {
      _initialized = false;
      return false;
    }

    if (!writeRegister16(INA219_REG_CONFIG, INA219_CONFIG_DEFAULT)) {
      _initialized = false;
      return false;
    }

    _initialized = true;
    return true;
  }

  bool isConnected() const {
    return _initialized;
  }

  // Returns Bus Voltage in Volts (V)
  float getBusVoltage_V() {
    uint16_t raw = readRegister16(INA219_REG_BUSVOLTAGE);
    // Shift right 3 bits, multiply by 4mV LSB
    int16_t bus_mV = (int16_t)((raw >> 3) * 4);
    return bus_mV * 0.001f;
  }

  // Returns Shunt Voltage in millivolts (mV)
  float getShuntVoltage_mV() {
    int16_t raw = (int16_t)readRegister16(INA219_REG_SHUNTVOLTAGE);
    // 10 uV per LSB = 0.01 mV
    return raw * 0.01f;
  }

  // Returns Current in milliamps (mA)
  // Positive = Discharging (Current flowing out of battery)
  // Negative = Charging (Current flowing into battery)
  float getCurrent_mA() {
    // For standard 0.1 ohm shunt: I = Vshunt / 0.1 ohm = Vshunt_mV / 0.1 = Vshunt_mV * 10
    // Raw shunt register LSB = 10uV (0.01 mV) -> Current LSB = 0.01 mV / 0.1 ohm = 0.1 mA
    int16_t rawShunt = (int16_t)readRegister16(INA219_REG_SHUNTVOLTAGE);
    return rawShunt * 0.1f;
  }

private:
  uint8_t _address;
  bool _initialized;

  bool writeRegister16(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.write((uint8_t)((value >> 8) & 0xFF));
    Wire.write((uint8_t)(value & 0xFF));
    return (Wire.endTransmission() == 0);
  }

  uint16_t readRegister16(uint8_t reg) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
      return 0;
    }

    if (Wire.requestFrom((uint8_t)_address, (uint8_t)2) != 2) {
      return 0;
    }

    uint16_t value = ((uint16_t)Wire.read() << 8) | Wire.read();
    return value;
  }
};
