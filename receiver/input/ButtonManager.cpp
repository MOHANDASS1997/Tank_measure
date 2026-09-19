#include "ButtonManager.h"

ButtonManager buttonManager;

ButtonManager::ButtonManager()
  : _lastReading(HIGH),
    _stableState(HIGH),
    _lastDebounceTime(0),
    _pressStartTime(0),
    _longPressHandled(false),
    _wokeFromSleep(false),
    _clickCount(0),
    _lastClickReleaseTime(0) {
}

void ButtonManager::begin() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void ButtonManager::update() {
  unsigned long now = millis();
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != _lastReading) {
    _lastDebounceTime = now;
  }

  if (now - _lastDebounceTime > DEBOUNCE_TIME) {
    // State transition detected
    if (reading != _stableState) {
      _stableState = reading;

      if (_stableState == LOW) {
        // Button Pressed DOWN
        _pressStartTime = now;
        _longPressHandled = false;

        // If display is asleep, wake it up immediately and mark _wokeFromSleep
        if (!displayManager.isAwake()) {
          displayManager.wakeDisplay();
          _wokeFromSleep = true;
        } else {
          _wokeFromSleep = false;
        }
      } else {
        // Button Released UP
        if (!_longPressHandled) {
          if (_wokeFromSleep) {
            // First press was just to wake display; do not execute menu/page action
            _wokeFromSleep = false;
            _clickCount = 0;
          } else if (displayManager.isChargingAnimationActive()) {
            _lastReading = reading;
            return;
          } else if (displayManager.isSelectionScreenActive()) {
            // In selection screen: distinguish single-click vs double-click
            _clickCount++;
            _lastClickReleaseTime = now;

            if (_clickCount >= 2) {
              _clickCount = 0;
              // DOUBLE CLICK -> Confirm highlighted screen
              Serial.println("[Button] Double click confirmed in Selection Screen.");
              displayManager.confirmSelection();
            }
          } else {
            // Outside selection screen: immediate short press
            displayManager.handleShortPress();
          }
        }
      }
    }

    // Check for LONG PRESS while button is being held down
    if (_stableState == LOW && !_longPressHandled) {
      unsigned long longPressMs = systemConfig.get().longPressDurationMs;
      if (now - _pressStartTime >= longPressMs) {
        _longPressHandled = true;
        _clickCount = 0; // Cancel any pending clicks
        _wokeFromSleep = false;

        if (displayManager.isChargingAnimationActive()) {
          _lastReading = reading;
          return;
        }

        Serial.println("[Button] Long press triggered. Opening Selection Screen.");

        // Always wake display on long press
        if (!displayManager.isAwake()) {
          displayManager.wakeDisplay();
        }

        // Trigger Selection Screen
        displayManager.handleLongPress();
      }
    }
  }

  // Handle single click timeout for Selection Screen
  if (_clickCount == 1 && (now - _lastClickReleaseTime > DOUBLE_CLICK_TIME_MS)) {
    _clickCount = 0;
    if (displayManager.isSelectionScreenActive()) {
      // SINGLE CLICK -> Move highlight to next item
      displayManager.nextSelectionItem();
    }
  }

  _lastReading = reading;
}
