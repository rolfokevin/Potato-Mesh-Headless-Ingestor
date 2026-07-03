#include "modules/PotatoMeshPositionModule.h"
#include "PotatoMeshConfig.h"
#include "modules/PotatoMeshFilter.h"
#include "modules/PotatoMeshUploader.h"
#include "modules/PotatoMeshUtil.h"

#include "gps/RTC.h"
#include "mesh/NodeDB.h"

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

PotatoMeshPositionModule *potatoMeshPositionModule;

PotatoMeshPositionModule::PotatoMeshPositionModule()
    : ProtobufModule("potatoMeshPosition", meshtastic_PortNum_POSITION_APP, &meshtastic_Position_msg)
{
    isPromiscuous = true;
    loopbackOk = true;
}

bool PotatoMeshPositionModule::handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_Position *pos)
{
#if POTATOMESH_ENABLE_POSITIONS
    if (!pos || !PotatoMesh::passesFilter(mp))
        return false;

    NodeNum num = getFrom(&mp);
    uint32_t rxTime = mp.rx_time ? mp.rx_time : getValidTime(RTCQualityDevice);

    std::string nodeIdStr = PotatoMesh::hexNodeNum(num);
    std::string fromIdStr = nodeIdStr;
    std::string toIdStr = PotatoMesh::hexNodeNum(mp.to);
    std::string rxIsoStr = PotatoMesh::isoTime(rxTime);
    std::string ingestorStr = PotatoMesh::hexNodeNum(nodeDB->getNodeNum());

    JsonDocument doc;
    doc["id"] = mp.id;
    doc["rx_time"] = rxTime;
    doc["rx_iso"] = rxIsoStr;
    doc["node_id"] = nodeIdStr;
    doc["node_num"] = num;
    doc["from_id"] = fromIdStr;
    doc["to_id"] = toIdStr;

    bool hasLatLon = !(pos->latitude_i == 0 && pos->longitude_i == 0);
    if (hasLatLon) {
        doc["latitude"] = pos->latitude_i * 1e-7;
        doc["longitude"] = pos->longitude_i * 1e-7;
        doc["altitude"] = pos->altitude;
        doc["location_source"] = (int)pos->location_source;
    }
    if (pos->time > 0)
        doc["position_time"] = pos->time;
    doc["precision_bits"] = pos->precision_bits;
    if (pos->sats_in_view > 0)
        doc["sats_in_view"] = pos->sats_in_view;
    if (pos->PDOP > 0)
        doc["pdop"] = pos->PDOP;
    if (pos->has_ground_speed)
        doc["ground_speed"] = pos->ground_speed;
    if (pos->has_ground_track)
        doc["ground_track"] = pos->ground_track;
    if (mp.rx_snr != 0)
        doc["snr"] = mp.rx_snr;
    if (mp.rx_rssi != 0)
        doc["rssi"] = mp.rx_rssi;
    doc["hop_limit"] = mp.hop_limit;
    doc["ingestor"] = ingestorStr;
    doc["protocol"] = "meshtastic";

    std::string json;
    serializeJson(doc, json);
    potatoMeshUploader->enqueue("/api/positions", json);
#endif
    return false;
}
