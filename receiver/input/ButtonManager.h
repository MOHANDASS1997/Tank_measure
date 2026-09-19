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

  static const unsigned long DEBOUNCE_TIME = 35;
};

extern ButtonManager buttonManager;
