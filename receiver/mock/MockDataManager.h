#pragma once

#include <Arduino.h>
#include "../config/BoardConfig.h"
#include "../models/Telemetry.h"

// =====================================================
//                 MOCK DATA MANAGER
// =====================================================

class MockDataManager {
public:
  MockDataManager();

  void begin(unsigned long initialDelayMs = MOCK_INITIAL_DELAY_MS, unsigned long intervalMs = MOCK_DATA_INTERVAL_MS);
  bool poll(RawTelemetry& raw, int& rssi, int& snr);

private:
  unsigned long _startTime;
  unsigned long _lastMockTime;
  unsigned long _initialDelayMs;
  unsigned long _intervalMs;
  unsigned long _mockSequence;
  int _stateIndex;
  bool _firstPoll;
};

extern MockDataManager mockDataManager;
