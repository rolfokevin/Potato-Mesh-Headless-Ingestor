#include "modules/PotatoMeshHeartbeat.h"
#include "DisplayFormatters.h"
#include "gps/RTC.h"
#include "mesh/NodeDB.h"
#include "mesh/RadioLibInterface.h"
#include "modules/PotatoMeshUploader.h"
#include "modules/PotatoMeshUtil.h"

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

#define POTATOMESH_INGESTOR_VERSION "1.0.0"

PotatoMeshHeartbeat *potatoMeshHeartbeat;

PotatoMeshHeartbeat::PotatoMeshHeartbeat() : concurrency::OSThread("PotatoMeshHeartbeat") {}

int32_t PotatoMeshHeartbeat::runOnce()
{
    uint32_t now = getValidTime(RTCQualityDevice);
    if (now == 0)
        return 5000; // wait until we have a valid clock

    if (startTime == 0)
        startTime = now;

    std::string nodeIdStr = PotatoMesh::hexNodeNum(nodeDB->getNodeNum());
    const char *presetName =
        DisplayFormatters::getModemPresetDisplayName(config.lora.modem_preset, false, config.lora.use_preset);
    int freqMhz = RadioLibInterface::instance ? (int)RadioLibInterface::instance->getFreq() : 0;

    JsonDocument doc;
    doc["node_id"] = nodeIdStr;
    doc["start_time"] = startTime;
    doc["last_seen_time"] = now;
    doc["version"] = POTATOMESH_INGESTOR_VERSION;
    doc["protocol"] = "meshtastic";
    doc["lora_freq"] = freqMhz;
    doc["modem_preset"] = presetName;

    std::string json;
    serializeJson(doc, json);
    potatoMeshUploader->enqueue("/api/ingestors", json);

    return 5 * 60 * 1000; // heartbeat every 5 minutes
}
