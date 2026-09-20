#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "DisplayConfig.h"
#include "BaseConfigManager.h"

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

class DisplayLayoutManager : public BaseConfigManager<DisplayLayoutManager, DisplayLayoutSettings> {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;
  static const char* getNvsNamespace() { return "cfg_dlayout"; }
  static const char* getTag() { return "DisplayLayout"; }

  DisplayLayoutManager();

  void loadDefaults();
  bool validate(const DisplayLayoutSettings& settings, String& err);

  const SectionLayoutEntry* getSection(SectionId sid) const;
  bool isSectionEnabled(SectionId sid) const;
  bool isPageEnabled(PageId pid) const;
  uint8_t getSectionPageCount(SectionId sid) const;
  PageId getSectionPageId(SectionId sid, uint8_t index) const;
};

extern DisplayLayoutManager displayLayoutConfig;
