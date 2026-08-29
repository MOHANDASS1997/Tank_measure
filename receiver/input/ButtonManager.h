#pragma once

#include <Arduino.h>
#include "../config/BoardConfig.h"
#include "../display/DisplayManager.h"

// =====================================================
//                    BUTTON MANAGER
// =====================================================

class ButtonManager {
public:
  ButtonManager();

  void begin();
  void update();

private:
  bool _lastButtonReading;
  bool _stableButtonState;
  unsigned long _lastDebounceTime;
  static const unsigned long DEBOUNCE_TIME = 35;
};

extern ButtonManager buttonManager;
