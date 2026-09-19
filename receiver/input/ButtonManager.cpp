#include "ButtonManager.h"

ButtonManager buttonManager;

ButtonManager::ButtonManager()
  : _lastReading(HIGH),
    _stableState(HIGH),
    _lastDebounceTime(0),
    _pressStartTime(0),
    _longPressHandled(false) {
}

void ButtonManager::begin() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void ButtonManager::update() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != _lastReading) {
    _lastDebounceTime = millis();
  }

  if (millis() - _lastDebounceTime > DEBOUNCE_TIME) {
    // State transition detected
    if (reading != _stableState) {
      _stableState = reading;

      if (_stableState == LOW) {
        // Button Pressed DOWN
        _pressStartTime = millis();
        _longPressHandled = false;
      } else {
        // Button Released UP
        if (!_longPressHandled) {
          // It was a SHORT PRESS
          if (displayManager.isChargingAnimationActive()) {
            _lastReading = reading;
            return;
          }

          if (wifiManager.isConfigModeActive()) {
            // In config mode: short press only wakes display if asleep
            if (!displayManager.isAwake()) {
              displayManager.wakeDisplay();
            }
          } else {
            // Normal mode
            if (!displayManager.isAwake()) {
              displayManager.wakeDisplay();
            } else {
              displayManager.resetTimeout();
              displayManager.switchPage();
            }
          }
        }
      }
    }

    // Check for LONG PRESS while button is being held down
    if (_stableState == LOW && !_longPressHandled) {
      unsigned long longPressMs = systemConfig.get().longPressDurationMs;
      if (millis() - _pressStartTime >= longPressMs) {
        _longPressHandled = true;

        if (displayManager.isChargingAnimationActive()) {
          _lastReading = reading;
          return;
        }

        Serial.println("[Button] Long press triggered (>= 2s). Toggling configuration mode.");

        // Always wake display on long press
        if (!displayManager.isAwake()) {
          displayManager.wakeDisplay();
        }

        // Toggle Configuration Mode
        wifiManager.toggleConfigMode();
      }
    }
  }

  _lastReading = reading;
}
