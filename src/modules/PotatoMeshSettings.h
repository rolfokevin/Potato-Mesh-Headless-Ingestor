#pragma once

#include <string>

// Runtime-configurable potato-mesh ingestor settings. Seeded from the
// compile-time POTATOMESH_* defines in PotatoMeshConfig.h, then overridden by
// whatever's saved on flash at /potatomesh.json (edited via the device's own
// web UI at http://<device-ip>/potatomesh - no reflash needed to change them).
struct PotatoMeshSettings {
    std::string domain;
    std::string apiToken;
    std::string allowedChannels; // comma-separated channel names, "" = all
    float minSnr;
};

extern PotatoMeshSettings potatoMeshSettings;

// Loads from /potatomesh.json if present, else falls back to PotatoMeshConfig.h
// compile-time defaults. Call once at startup before any module reads settings.
void potatoMeshSettingsLoad();

// Persists the current in-memory settings to /potatomesh.json.
void potatoMeshSettingsSave();
