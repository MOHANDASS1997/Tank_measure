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

        displayManager.switchPage();
      }
    }
  }

  _lastButtonReading =
    reading;
}
