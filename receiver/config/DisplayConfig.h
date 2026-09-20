#pragma once

#include <Arduino.h>

// =====================================================
//                 DISPLAY SECTIONS & PAGES
// =====================================================

enum SectionId {
  SECTION_TANK = 0,
  SECTION_CONFIG,
  SECTION_DEV,
  SECTION_COUNT
};

enum PageId {
  PAGE_TANK_LEVEL = 0,
  PAGE_TRANSMITTER_BATTERY,
  PAGE_CONFIG_PORTAL,
  PAGE_DEV_INA219,
  PAGE_ID_COUNT
};

struct SectionDef {
  SectionId     id;
  const char*   name;
  const PageId* pages;
  uint8_t       pageCount;
  bool          requiresDevMode;
  uint16_t      updateIntervalMs;
};

// -----------------------------------------------------
// Page Arrays per Section (Easily reorderable / configurable)
// -----------------------------------------------------

static const PageId TANK_SECTION_PAGES[] = {
  PAGE_TANK_LEVEL,
  PAGE_TRANSMITTER_BATTERY
};

static const PageId CONFIG_SECTION_PAGES[] = {
  PAGE_CONFIG_PORTAL
};

static const PageId DEV_SECTION_PAGES[] = {
  PAGE_DEV_INA219
};

// -----------------------------------------------------
// Section Registry
// -----------------------------------------------------

static const SectionDef DISPLAY_SECTIONS[] = {
  {
    SECTION_TANK,
    "Tank Data",
    TANK_SECTION_PAGES,
    sizeof(TANK_SECTION_PAGES) / sizeof(TANK_SECTION_PAGES[0]),
    false,
    250  // 250ms for smooth animations / hourglass updates
  },
  {
    SECTION_CONFIG,
    "Config Mode",
    CONFIG_SECTION_PAGES,
    sizeof(CONFIG_SECTION_PAGES) / sizeof(CONFIG_SECTION_PAGES[0]),
    false,
    1000 // 1s interval for countdown updates
  },
  {
    SECTION_DEV,
    "Dev Mode",
    DEV_SECTION_PAGES,
    sizeof(DEV_SECTION_PAGES) / sizeof(DEV_SECTION_PAGES[0]),
    true,
    33   // ~30 FPS for live diagnostics
  }
};

static const uint8_t DISPLAY_SECTION_COUNT = sizeof(DISPLAY_SECTIONS) / sizeof(DISPLAY_SECTIONS[0]);

// Default Section loaded on system boot
static const SectionId DEFAULT_SECTION_ID = SECTION_TANK;
