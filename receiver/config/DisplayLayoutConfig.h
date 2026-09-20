#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "DisplayConfig.h"

// =====================================================
//               DISPLAY LAYOUT CONFIGURATION
// =====================================================

struct PageLayoutEntry {
  PageId pageId;
  bool   enabled;
};

struct SectionLayoutEntry {
  SectionId       sectionId;
  bool            enabled;
  PageLayoutEntry pages[4]; // Maximum 4 pages per section
  uint8_t         pageCount;
};

struct DisplayLayoutSettings {
  uint16_t           schemaVersion;
  SectionLayoutEntry sections[SECTION_COUNT];
};

class DisplayLayoutManager {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;

  DisplayLayoutManager();

  void begin();
  void loadDefaults();
  bool load();
  bool save();
  bool validate(const DisplayLayoutSettings& settings, String& err);

  const DisplayLayoutSettings& get() const { return _settings; }
  void set(const DisplayLayoutSettings& settings) { _settings = settings; }

  const SectionLayoutEntry* getSection(SectionId sid) const;
  bool isSectionEnabled(SectionId sid) const;
  bool isPageEnabled(PageId pid) const;
  uint8_t getSectionPageCount(SectionId sid) const;
  PageId getSectionPageId(SectionId sid, uint8_t index) const;

private:
  DisplayLayoutSettings _settings;
  Preferences           _prefs;
};

extern DisplayLayoutManager displayLayoutConfig;
