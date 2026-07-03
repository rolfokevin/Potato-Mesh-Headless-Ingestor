#include "modules/PotatoMeshTelemetryModule.h"
#include "PotatoMeshConfig.h"
#include "modules/PotatoMeshFilter.h"
#include "modules/PotatoMeshUploader.h"
#include "modules/PotatoMeshUtil.h"

#include "gps/RTC.h"
#include "mesh/NodeDB.h"

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

PotatoMeshTelemetryModule *potatoMeshTelemetryModule;

PotatoMeshTelemetryModule::PotatoMeshTelemetryModule()
    : ProtobufModule("potatoMeshTelemetry", meshtastic_PortNum_TELEMETRY_APP, &meshtastic_Telemetry_msg)
{
    isPromiscuous = true;
    loopbackOk = true;
}

bool PotatoMeshTelemetryModule::handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_Telemetry *t)
{
#if POTATOMESH_ENABLE_TELEMETRY
    if (!t || !PotatoMesh::passesFilter(mp))
        return false;

    const char *type = nullptr;
    if (t->which_variant == meshtastic_Telemetry_device_metrics_tag)
        type = "device";
    else if (t->which_variant == meshtastic_Telemetry_environment_metrics_tag)
        type = "environment";
    else if (t->which_variant == meshtastic_Telemetry_power_metrics_tag)
        type = "power";
    else if (t->which_variant == meshtastic_Telemetry_air_quality_metrics_tag)
        type = "air_quality";
    else
        return false; // no potato-mesh telemetry_type mapping for this variant

    NodeNum num = getFrom(&mp);
    uint32_t rxTime = mp.rx_time ? mp.rx_time : getValidTime(RTCQualityDevice);

    std::string nodeIdStr = PotatoMesh::hexNodeNum(num);
    std::string rxIsoStr = PotatoMesh::isoTime(rxTime);
    std::string ingestorStr = PotatoMesh::hexNodeNum(nodeDB->getNodeNum());

    JsonDocument doc;
    doc["id"] = mp.id;
    doc["rx_time"] = rxTime;
    doc["rx_iso"] = rxIsoStr;
    doc["node_id"] = nodeIdStr;
    doc["node_num"] = num;
    doc["telemetry_time"] = t->time;
    doc["channel"] = mp.channel;
    doc["portnum"] = "TELEMETRY_APP";
    if (mp.rx_snr != 0)
        doc["snr"] = mp.rx_snr;
    if (mp.rx_rssi != 0)
        doc["rssi"] = mp.rx_rssi;
    doc["hop_limit"] = mp.hop_limit;
    doc["payload_b64"] = "";
    doc["telemetry_type"] = type;
    doc["ingestor"] = ingestorStr;
    doc["protocol"] = "meshtastic";

    if (t->which_variant == meshtastic_Telemetry_device_metrics_tag) {
        auto &dm = t->variant.device_metrics;
        if (dm.has_battery_level)
            doc["battery_level"] = dm.battery_level;
        if (dm.has_voltage)
            doc["voltage"] = dm.voltage;
        if (dm.has_channel_utilization)
            doc["channel_utilization"] = dm.channel_utilization;
        if (dm.has_air_util_tx)
            doc["air_util_tx"] = dm.air_util_tx;
        if (dm.has_uptime_seconds)
            doc["uptime_seconds"] = dm.uptime_seconds;
    } else if (t->which_variant == meshtastic_Telemetry_environment_metrics_tag) {
        auto &em = t->variant.environment_metrics;
        if (em.has_temperature)
            doc["temperature"] = em.temperature;
        if (em.has_relative_humidity)
            doc["relative_humidity"] = em.relative_humidity;
        if (em.has_barometric_pressure)
            doc["barometric_pressure"] = em.barometric_pressure;
        if (em.has_gas_resistance)
            doc["gas_resistance"] = em.gas_resistance;
        if (em.has_current)
            doc["current"] = em.current;
        if (em.has_voltage)
            doc["voltage"] = em.voltage;
        if (em.has_iaq)
            doc["iaq"] = em.iaq;
        if (em.has_distance)
            doc["distance"] = em.distance;
        if (em.has_lux)
            doc["lux"] = em.lux;
        if (em.has_white_lux)
            doc["white_lux"] = em.white_lux;
        if (em.has_ir_lux)
            doc["ir_lux"] = em.ir_lux;
        if (em.has_uv_lux)
            doc["uv_lux"] = em.uv_lux;
        if (em.has_wind_direction)
            doc["wind_direction"] = em.wind_direction;
        if (em.has_wind_speed)
            doc["wind_speed"] = em.wind_speed;
        if (em.has_wind_gust)
            doc["wind_gust"] = em.wind_gust;
        if (em.has_wind_lull)
            doc["wind_lull"] = em.wind_lull;
        if (em.has_weight)
            doc["weight"] = em.weight;
        if (em.has_radiation)
            doc["radiation"] = em.radiation;
        if (em.has_rainfall_1h)
            doc["rainfall_1h"] = em.rainfall_1h;
        if (em.has_rainfall_24h)
            doc["rainfall_24h"] = em.rainfall_24h;
        if (em.has_soil_moisture)
            doc["soil_moisture"] = em.soil_moisture;
        if (em.has_soil_temperature)
            doc["soil_temperature"] = em.soil_temperature;
    } else if (t->which_variant == meshtastic_Telemetry_power_metrics_tag) {
        // potato-mesh's telemetry schema only exposes generic voltage/current
        // columns, so map the first power channel onto those.
        auto &pm = t->variant.power_metrics;
        if (pm.has_ch1_voltage)
            doc["voltage"] = pm.ch1_voltage;
        if (pm.has_ch1_current)
            doc["current"] = pm.ch1_current;
    }
    // air_quality: potato-mesh has no confirmed PM/CO2 field names yet, so only
    // the envelope fields above are sent for this variant.

    std::string json;
    serializeJson(doc, json);
    potatoMeshUploader->enqueue("/api/telemetry", json);
#endif
    return false;
}
