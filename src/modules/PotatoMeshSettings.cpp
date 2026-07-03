#include "modules/PotatoMeshSettings.h"
#include "FSCommon.h"
#include "PotatoMeshConfig.h"
#include "SPILock.h"
#include "configuration.h"

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

#define POTATOMESH_SETTINGS_PATH "/potatomesh.json"

PotatoMeshSettings potatoMeshSettings;

static void applyCompileTimeDefaults()
{
    potatoMeshSettings.domain = POTATOMESH_INSTANCE_DOMAIN;
    potatoMeshSettings.apiToken = POTATOMESH_API_TOKEN;
    potatoMeshSettings.allowedChannels = POTATOMESH_ALLOWED_CHANNELS;
    potatoMeshSettings.minSnr = POTATOMESH_MIN_SNR;
}

void potatoMeshSettingsLoad()
{
    applyCompileTimeDefaults();

    concurrency::LockGuard g(spiLock);
    if (!FSCom.exists(POTATOMESH_SETTINGS_PATH))
        return;

    File f = FSCom.open(POTATOMESH_SETTINGS_PATH, FILE_O_READ);
    if (!f)
        return;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    if (err) {
        LOG_WARN("PotatoMesh: failed to parse %s: %s", POTATOMESH_SETTINGS_PATH, err.c_str());
        return;
    }

    if (doc["domain"].is<const char *>())
        potatoMeshSettings.domain = doc["domain"].as<std::string>();
    if (doc["api_token"].is<const char *>())
        potatoMeshSettings.apiToken = doc["api_token"].as<std::string>();
    if (doc["allowed_channels"].is<const char *>())
        potatoMeshSettings.allowedChannels = doc["allowed_channels"].as<std::string>();
    if (doc["min_snr"].is<float>())
        potatoMeshSettings.minSnr = doc["min_snr"].as<float>();

    LOG_INFO("PotatoMesh: loaded settings from %s", POTATOMESH_SETTINGS_PATH);
}

void potatoMeshSettingsSave()
{
    JsonDocument doc;
    doc["domain"] = potatoMeshSettings.domain;
    doc["api_token"] = potatoMeshSettings.apiToken;
    doc["allowed_channels"] = potatoMeshSettings.allowedChannels;
    doc["min_snr"] = potatoMeshSettings.minSnr;

    concurrency::LockGuard g(spiLock);
    File f = FSCom.open(POTATOMESH_SETTINGS_PATH, FILE_O_WRITE);
    if (!f) {
        LOG_WARN("PotatoMesh: failed to open %s for writing", POTATOMESH_SETTINGS_PATH);
        return;
    }
    serializeJson(doc, f);
    f.close();
    LOG_INFO("PotatoMesh: saved settings to %s", POTATOMESH_SETTINGS_PATH);
}
