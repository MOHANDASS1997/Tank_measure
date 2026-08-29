#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "../models/DisplayData.h"

// =====================================================
//                   STORAGE MANAGER
// =====================================================

class StorageManager {
public:
  StorageManager();

  void save(const DisplayData& data, uint32_t timestamp, bool isMockData);
  bool load(DisplayData& data, uint32_t& timestamp, bool currentIsMock);
  void clear();

private:
  Preferences _preferences;
};

extern StorageManager storageManager;
