#include "modules/PotatoMeshUploader.h"
#include "configuration.h"
#include "modules/PotatoMeshSettings.h"

#if HAS_WIFI
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

PotatoMeshUploader *potatoMeshUploader;

PotatoMeshUploader::PotatoMeshUploader() : concurrency::OSThread("PotatoMeshUploader") {}

void PotatoMeshUploader::enqueue(const char *endpoint, const std::string &jsonBody)
{
    if (queue.size() >= maxQueue) {
        queue.pop_front(); // drop oldest, mirrors MQTT's own queue backpressure
    }
    queue.push_back({endpoint, jsonBody});
    setIntervalFromNow(0);
}

bool PotatoMeshUploader::postOne(const QueueEntry &entry)
{
#if HAS_WIFI
    if (potatoMeshSettings.domain.empty() || potatoMeshSettings.apiToken.empty())
        return false; // ingestion disabled until configured via /potatomesh

    WiFiClientSecure client;
    client.setInsecure(); // same TLS trust model Meshtastic's own MQTT module uses
    HTTPClient http;
    std::string url = std::string("https://") + potatoMeshSettings.domain + entry.endpoint;
    if (!http.begin(client, url.c_str())) {
        LOG_WARN("PotatoMesh: failed to begin HTTP request to %s", entry.endpoint.c_str());
        return false;
    }
    http.addHeader("Content-Type", "application/json");
    std::string authHeader = "Bearer " + potatoMeshSettings.apiToken;
    http.addHeader("Authorization", authHeader.c_str());
    int code = http.POST((uint8_t *)entry.jsonBody.data(), entry.jsonBody.size());
    http.end();
    if (code != 201) {
        LOG_WARN("PotatoMesh: POST %s -> %d", entry.endpoint.c_str(), code);
        return false;
    }
    LOG_DEBUG("PotatoMesh: POST %s -> 201", entry.endpoint.c_str());
    return true;
#else
    return false;
#endif
}

int32_t PotatoMeshUploader::runOnce()
{
#if HAS_WIFI
    if (!WiFi.isConnected())
        return 5000;

    if (queue.empty())
        return 30000;

    QueueEntry entry = queue.front();
    queue.pop_front();
    postOne(entry);
    return queue.empty() ? 30000 : 200;
#else
    return 30000;
#endif
}
