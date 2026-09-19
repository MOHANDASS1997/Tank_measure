#include "ButtonManager.h"

// Instantiate global ButtonManager
ButtonManager buttonManager;

ButtonManager::ButtonManager()
  : _lastButtonReading(HIGH),
    _stableButtonState(HIGH),
    _lastDebounceTime(0) {
}

void ButtonManager::begin() {

  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );
}

void ButtonManager::update() {

  bool reading =
    digitalRead(
      BUTTON_PIN
    );

  if (
    reading !=
    _lastButtonReading
  ) {

    _lastDebounceTime =
      millis();
  }

  if (
    millis() -
    _lastDebounceTime >
    DEBOUNCE_TIME
  ) {

    if (
      reading !=
      _stableButtonState
    ) {

      _stableButtonState =
        reading;

      if (
        _stableButtonState ==
        LOW
      ) {

        // Edge Case: If charging animation is running, DO NOTHING
        if (displayManager.isChargingAnimationActive()) {
          return;
        }

        // Inactivity Wake Action: First press only wakes display and keeps current page
        if (!displayManager.isAwake()) {
          displayManager.wakeDisplay();
          return;
        }

        // UI is awake: restart 15s timeout and navigate pages
        displayManager.resetTimeout();
        displayManager.switchPage();
      }
    }
  }

  _lastButtonReading =
    reading;
}
