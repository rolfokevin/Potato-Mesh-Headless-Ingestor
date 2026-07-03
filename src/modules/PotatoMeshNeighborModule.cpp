#include "modules/PotatoMeshNeighborModule.h"
#include "PotatoMeshConfig.h"
#include "modules/PotatoMeshFilter.h"
#include "modules/PotatoMeshUploader.h"
#include "modules/PotatoMeshUtil.h"

#include "gps/RTC.h"
#include "mesh/NodeDB.h"

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

PotatoMeshNeighborModule *potatoMeshNeighborModule;

PotatoMeshNeighborModule::PotatoMeshNeighborModule()
    : ProtobufModule("potatoMeshNeighbor", meshtastic_PortNum_NEIGHBORINFO_APP, &meshtastic_NeighborInfo_msg)
{
    isPromiscuous = true;
    loopbackOk = true;
}

bool PotatoMeshNeighborModule::handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_NeighborInfo *ni)
{
#if POTATOMESH_ENABLE_NEIGHBORS
    if (!ni || !PotatoMesh::passesFilter(mp))
        return false;

    uint32_t rxTime = mp.rx_time ? mp.rx_time : getValidTime(RTCQualityDevice);
    std::string nodeIdStr = PotatoMesh::hexNodeNum(ni->node_id);
    std::string rxIsoStr = PotatoMesh::isoTime(rxTime);
    std::string ingestorStr = PotatoMesh::hexNodeNum(nodeDB->getNodeNum());

    JsonDocument doc;
    doc["node_id"] = nodeIdStr;
    doc["node_num"] = ni->node_id;
    doc["rx_time"] = rxTime;
    doc["rx_iso"] = rxIsoStr;
    if (ni->node_broadcast_interval_secs > 0)
        doc["node_broadcast_interval_secs"] = ni->node_broadcast_interval_secs;
    if (ni->last_sent_by_id > 0) {
        std::string lastSentByStr = PotatoMesh::hexNodeNum(ni->last_sent_by_id);
        doc["last_sent_by_id"] = lastSentByStr;
    }
    doc["ingestor"] = ingestorStr;
    doc["protocol"] = "meshtastic";

    JsonArray neighbors = doc["neighbors"].to<JsonArray>();
    for (pb_size_t i = 0; i < ni->neighbors_count; i++) {
        auto &n = ni->neighbors[i];
        std::string neighborIdStr = PotatoMesh::hexNodeNum(n.node_id);
        uint32_t entryRxTime = n.last_rx_time > 0 ? n.last_rx_time : rxTime;
        std::string entryRxIsoStr = PotatoMesh::isoTime(entryRxTime);

        JsonObject entry = neighbors.add<JsonObject>();
        entry["neighbor_id"] = neighborIdStr;
        entry["neighbor_num"] = n.node_id;
        entry["snr"] = n.snr;
        entry["rx_time"] = entryRxTime;
        entry["rx_iso"] = entryRxIsoStr;
    }

    std::string json;
    serializeJson(doc, json);
    potatoMeshUploader->enqueue("/api/neighbors", json);
#endif
    return false;
}
