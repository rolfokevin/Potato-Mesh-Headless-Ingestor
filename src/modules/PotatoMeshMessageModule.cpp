#include "modules/PotatoMeshMessageModule.h"
#include "PotatoMeshConfig.h"
#include "modules/PotatoMeshFilter.h"
#include "modules/PotatoMeshUploader.h"
#include "modules/PotatoMeshUtil.h"

#include "gps/RTC.h"
#include "mesh/NodeDB.h"

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

PotatoMeshMessageModule *potatoMeshMessageModule;

PotatoMeshMessageModule::PotatoMeshMessageModule()
    : SinglePortModule("potatoMeshMessage", meshtastic_PortNum_TEXT_MESSAGE_APP)
{
    isPromiscuous = true;
    loopbackOk = true;
}

ProcessMessage PotatoMeshMessageModule::handleReceived(const meshtastic_MeshPacket &mp)
{
#if POTATOMESH_ENABLE_MESSAGES
    if (mp.which_payload_variant != meshtastic_MeshPacket_decoded_tag || mp.decoded.portnum != ourPortNum ||
        !PotatoMesh::passesFilter(mp))
        return ProcessMessage::CONTINUE;

    NodeNum num = getFrom(&mp);
    uint32_t rxTime = mp.rx_time ? mp.rx_time : getValidTime(RTCQualityDevice);
    std::string text(reinterpret_cast<const char *>(mp.decoded.payload.bytes), mp.decoded.payload.size);

    std::string fromIdStr = PotatoMesh::hexNodeNum(num);
    std::string toIdStr = PotatoMesh::hexNodeNum(mp.to);
    std::string rxIsoStr = PotatoMesh::isoTime(rxTime);
    std::string ingestorStr = PotatoMesh::hexNodeNum(nodeDB->getNodeNum());

    JsonDocument doc;
    doc["id"] = mp.id;
    doc["rx_time"] = rxTime;
    doc["rx_iso"] = rxIsoStr;
    doc["from_id"] = fromIdStr;
    doc["to_id"] = toIdStr;
    doc["channel"] = mp.channel;
    doc["portnum"] = "TEXT_MESSAGE_APP";
    doc["text"] = text;
    doc["encrypted"] = false;
    if (mp.rx_snr != 0)
        doc["snr"] = mp.rx_snr;
    if (mp.rx_rssi != 0)
        doc["rssi"] = mp.rx_rssi;
    doc["hop_limit"] = mp.hop_limit;
    doc["ingestor"] = ingestorStr;
    doc["protocol"] = "meshtastic";

    std::string json;
    serializeJson(doc, json);
    potatoMeshUploader->enqueue("/api/messages", json);
#endif
    return ProcessMessage::CONTINUE;
}
