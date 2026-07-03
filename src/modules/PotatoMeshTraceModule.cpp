#include "modules/PotatoMeshTraceModule.h"
#include "PotatoMeshConfig.h"
#include "modules/PotatoMeshFilter.h"
#include "modules/PotatoMeshUploader.h"
#include "modules/PotatoMeshUtil.h"

#include "gps/RTC.h"
#include "mesh/NodeDB.h"

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

PotatoMeshTraceModule *potatoMeshTraceModule;

PotatoMeshTraceModule::PotatoMeshTraceModule()
    : ProtobufModule("potatoMeshTrace", meshtastic_PortNum_TRACEROUTE_APP, &meshtastic_RouteDiscovery_msg)
{
    isPromiscuous = true; // We want to catch traceroutes even when we're just relaying them
    loopbackOk = true;
}

bool PotatoMeshTraceModule::handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_RouteDiscovery *r)
{
#if POTATOMESH_ENABLE_TRACES
    if (!r || !PotatoMesh::passesFilter(mp))
        return false;

    uint32_t rxTime = mp.rx_time ? mp.rx_time : getValidTime(RTCQualityDevice);
    // A traceroute reply carries the original request's id in decoded.request_id;
    // for the initial request packet that's 0, so fall back to this packet's own id.
    uint32_t requestId = mp.decoded.request_id != 0 ? mp.decoded.request_id : mp.id;

    std::string srcStr = PotatoMesh::hexNodeNum(mp.from);
    std::string destStr = PotatoMesh::hexNodeNum(mp.to);
    std::string rxIsoStr = PotatoMesh::isoTime(rxTime);
    std::string ingestorStr = PotatoMesh::hexNodeNum(nodeDB->getNodeNum());

    JsonDocument doc;
    doc["id"] = mp.id;
    doc["request_id"] = requestId;
    doc["src"] = srcStr;
    doc["dest"] = destStr;
    doc["rx_time"] = rxTime;
    doc["rx_iso"] = rxIsoStr;
    if (mp.rx_snr != 0)
        doc["snr"] = mp.rx_snr;
    if (mp.rx_rssi != 0)
        doc["rssi"] = mp.rx_rssi;
    doc["ingestor"] = ingestorStr;
    doc["protocol"] = "meshtastic";

    JsonArray hops = doc["hops"].to<JsonArray>();
    for (pb_size_t i = 0; i < r->route_count; i++)
        hops.add(r->route[i]);

    std::string json;
    serializeJson(doc, json);
    potatoMeshUploader->enqueue("/api/traces", json);
#endif
    return false;
}
