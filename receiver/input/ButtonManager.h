#pragma once

#include <Arduino.h>
#include "../config/BoardConfig.h"
#include "../config/SystemConfig.h"
#include "../display/DisplayManager.h"
#include "../wifi/WiFiManager.h"

// =====================================================
//                    BUTTON MANAGER
// =====================================================

class ButtonManager {
public:
  ButtonManager();

  void begin();
  void update();

private:
  bool _lastReading;
  bool _stableState;
  unsigned long _lastDebounceTime;
  unsigned long _pressStartTime;
  bool _longPressHandled;
  bool _wokeFromSleep;

  // Multi-click detection for Selection Screen
  uint8_t _clickCount;
  unsigned long _lastClickReleaseTime;

  static const unsigned long DEBOUNCE_TIME = 35;
  static const unsigned long DOUBLE_CLICK_TIME_MS = 300;
};

extern ButtonManager buttonManager;
