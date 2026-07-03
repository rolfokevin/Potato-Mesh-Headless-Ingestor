#include "modules/PotatoMeshNodeModule.h"
#include "PotatoMeshConfig.h"
#include "modules/PotatoMeshFilter.h"
#include "modules/PotatoMeshUploader.h"
#include "modules/PotatoMeshUtil.h"

#include "mesh/NodeDB.h"

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

PotatoMeshNodeModule *potatoMeshNodeModule;

PotatoMeshNodeModule::PotatoMeshNodeModule()
    : ProtobufModule("potatoMeshNode", meshtastic_PortNum_NODEINFO_APP, &meshtastic_User_msg)
{
    isPromiscuous = true;
    loopbackOk = true;
}

bool PotatoMeshNodeModule::handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_User *user)
{
#if POTATOMESH_ENABLE_NODES
    if (!user || !PotatoMesh::passesFilter(mp))
        return false;

    NodeNum num = getFrom(&mp);
    meshtastic_NodeInfoLite *node = nodeDB->getMeshNode(num);
    if (!node)
        return false;

    std::string idStr = PotatoMesh::hexNodeNum(num);
    std::string ingestorStr = PotatoMesh::hexNodeNum(nodeDB->getNodeNum());

    JsonDocument doc;
    JsonObject entry = doc[idStr].to<JsonObject>();

    entry["num"] = num;
    entry["last_heard"] = node->last_heard;
    if (node->has_hops_away)
        entry["hops_away"] = node->hops_away;

    JsonObject userObj = entry["user"].to<JsonObject>();
    userObj["short_name"] = user->short_name;
    userObj["long_name"] = user->long_name;
    userObj["hw_model"] = (int)user->hw_model;
    userObj["role"] = (int)user->role;
    if (user->public_key.size > 0) {
        std::string pubKeyB64 = PotatoMesh::base64Encode(user->public_key.bytes, user->public_key.size);
        userObj["public_key"] = pubKeyB64;
    }

    meshtastic_DeviceMetrics dm = meshtastic_DeviceMetrics_init_zero;
    if (nodeDB->copyNodeTelemetry(num, dm)) {
        JsonObject dmObj = entry["device_metrics"].to<JsonObject>();
        if (dm.has_battery_level)
            dmObj["battery_level"] = dm.battery_level;
        if (dm.has_voltage)
            dmObj["voltage"] = dm.voltage;
        if (dm.has_channel_utilization)
            dmObj["channel_utilization"] = dm.channel_utilization;
        if (dm.has_air_util_tx)
            dmObj["air_util_tx"] = dm.air_util_tx;
        if (dm.has_uptime_seconds)
            dmObj["uptime_seconds"] = dm.uptime_seconds;
    }

    meshtastic_PositionLite pos = meshtastic_PositionLite_init_zero;
    if (nodeDB->copyNodePosition(num, pos) && !(pos.latitude_i == 0 && pos.longitude_i == 0)) {
        JsonObject posObj = entry["position"].to<JsonObject>();
        posObj["latitude"] = pos.latitude_i * 1e-7;
        posObj["longitude"] = pos.longitude_i * 1e-7;
        posObj["altitude"] = pos.altitude;
        if (pos.time > 0)
            posObj["time"] = pos.time;
        posObj["location_source"] = (int)pos.location_source;
        posObj["precision_bits"] = pos.precision_bits;
    }

    doc["ingestor"] = ingestorStr;
    doc["protocol"] = "meshtastic";

    std::string json;
    serializeJson(doc, json);
    potatoMeshUploader->enqueue("/api/nodes", json);
#endif
    return false; // never claim the packet - let NodeInfoModule keep handling it too
}
