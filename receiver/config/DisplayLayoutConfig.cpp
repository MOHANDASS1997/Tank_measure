#include "DisplayLayoutConfig.h"

DisplayLayoutManager displayLayoutConfig;

DisplayLayoutManager::DisplayLayoutManager() {
  loadDefaults();
}

void DisplayLayoutManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;

  // 1. Tank Data Section
  _settings.sections[0].sectionId = SECTION_TANK;
  _settings.sections[0].enabled = true;
  _settings.sections[0].pageCount = 2;
  _settings.sections[0].pages[0].pageId = PAGE_TANK_LEVEL;
  _settings.sections[0].pages[0].enabled = true;
  _settings.sections[0].pages[1].pageId = PAGE_TRANSMITTER_BATTERY;
  _settings.sections[0].pages[1].enabled = true;

  // 2. Config Mode Section
  _settings.sections[1].sectionId = SECTION_CONFIG;
  _settings.sections[1].enabled = true;
  _settings.sections[1].pageCount = 1;
  _settings.sections[1].pages[0].pageId = PAGE_CONFIG_PORTAL;
  _settings.sections[1].pages[0].enabled = true;

  // 3. Dev Mode Section
  _settings.sections[2].sectionId = SECTION_DEV;
  _settings.sections[2].enabled = false; // Dev mode disabled by default
  _settings.sections[2].pageCount = 1;
  _settings.sections[2].pages[0].pageId = PAGE_DEV_INA219;
  _settings.sections[2].pages[0].enabled = true;
}

void DisplayLayoutManager::begin() {
  if (!load()) {
    Serial.println("[DisplayLayout] No valid stored configuration found. Writing defaults.");
    loadDefaults();
    save();
  } else {
    Serial.println("[DisplayLayout] Loaded persistent configuration successfully.");
  }
}

bool DisplayLayoutManager::load() {
  _prefs.begin("cfg_dlayout", true);
  size_t len = _prefs.getBytesLength("settings");
  if (len != sizeof(DisplayLayoutSettings)) {
    _prefs.end();
    return false;
  }

  DisplayLayoutSettings temp;
  _prefs.getBytes("settings", &temp, sizeof(DisplayLayoutSettings));
  _prefs.end();

  if (temp.schemaVersion != CURRENT_SCHEMA_VERSION) {
    Serial.println("[DisplayLayout] Schema version mismatch; migrating to defaults.");
    return false;
  }

  String err;
  if (!validate(temp, err)) {
    Serial.print("[DisplayLayout] Validation failed: ");
    Serial.println(err);
    return false;
  }

  _settings = temp;
  return true;
}

bool DisplayLayoutManager::save() {
  String err;
  if (!validate(_settings, err)) {
    Serial.print("[DisplayLayout] Cannot save invalid settings: ");
    Serial.println(err);
    return false;
  }

  _prefs.begin("cfg_dlayout", false);
  size_t written = _prefs.putBytes("settings", &_settings, sizeof(DisplayLayoutSettings));
  _prefs.end();

  return (written == sizeof(DisplayLayoutSettings));
}

bool DisplayLayoutManager::validate(const DisplayLayoutSettings& s, String& err) {
  if (s.schemaVersion != CURRENT_SCHEMA_VERSION) {
    err = "Invalid schema version";
    return false;
  }

  bool hasTank = false;
  bool hasConfig = false;

  for (int i = 0; i < SECTION_COUNT; i++) {
    const SectionLayoutEntry& sec = s.sections[i];
    if (sec.sectionId == SECTION_TANK) {
      hasTank = true;
      if (!sec.enabled) {
        err = "Tank section cannot be disabled";
        return false;
      }
    } else if (sec.sectionId == SECTION_CONFIG) {
      hasConfig = true;
      if (!sec.enabled) {
        err = "Config section cannot be disabled";
        return false;
      }
    }

    if (sec.pageCount == 0 || sec.pageCount > 4) {
      err = "Invalid page count in section " + String(sec.sectionId);
      return false;
    }

    // Ensure at least one page is enabled in each section
    bool anyPageEnabled = false;
    for (uint8_t p = 0; p < sec.pageCount; p++) {
      if (sec.pages[p].enabled) {
        anyPageEnabled = true;
      }
      if (sec.pages[p].pageId >= PAGE_ID_COUNT) {
        err = "Invalid page ID in section " + String(sec.sectionId);
        return false;
      }
    }

    if (!anyPageEnabled) {
      err = "At least one page must be enabled in section " + String(sec.sectionId);
      return false;
    }
  }

  if (!hasTank || !hasConfig) {
    err = "Missing mandatory sections";
    return false;
  }

  return true;
}

const SectionLayoutEntry* DisplayLayoutManager::getSection(SectionId sid) const {
  for (int i = 0; i < SECTION_COUNT; i++) {
    if (_settings.sections[i].sectionId == sid) {
      return &_settings.sections[i];
    }
  }
  return nullptr;
}

bool DisplayLayoutManager::isSectionEnabled(SectionId sid) const {
  if (sid == SECTION_TANK || sid == SECTION_CONFIG) {
    return true; // Always enabled
  }
  const SectionLayoutEntry* sec = getSection(sid);
  return (sec != nullptr && sec->enabled);
}

bool DisplayLayoutManager::isPageEnabled(PageId pid) const {
  for (int i = 0; i < SECTION_COUNT; i++) {
    const SectionLayoutEntry& sec = _settings.sections[i];
    for (uint8_t p = 0; p < sec.pageCount; p++) {
      if (sec.pages[p].pageId == pid) {
        // Page must be enabled AND its owning section must be enabled
        return (sec.enabled && sec.pages[p].enabled);
      }
    }
  }
  return false;
}

uint8_t DisplayLayoutManager::getSectionPageCount(SectionId sid) const {
  const SectionLayoutEntry* sec = getSection(sid);
  return (sec != nullptr) ? sec->pageCount : 0;
}

PageId DisplayLayoutManager::getSectionPageId(SectionId sid, uint8_t index) const {
  const SectionLayoutEntry* sec = getSection(sid);
  if (sec != nullptr && index < sec->pageCount) {
    return sec->pages[index].pageId;
  }
  return PAGE_TANK_LEVEL;
}
