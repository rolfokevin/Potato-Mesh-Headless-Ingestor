#pragma once

#include "concurrency/OSThread.h"
#include <deque>
#include <string>

// Drains a small queue of {endpoint, json} pairs to a potato-mesh instance over
// HTTPS whenever WiFi is up. Fed by the six PotatoMesh*Module ingestion modules.
class PotatoMeshUploader : private concurrency::OSThread
{
  public:
    PotatoMeshUploader();

    void enqueue(const char *endpoint, const std::string &jsonBody);

  protected:
    int32_t runOnce() override;

  private:
    struct QueueEntry {
        std::string endpoint;
        std::string jsonBody;
    };

    static const size_t maxQueue = 32;
    std::deque<QueueEntry> queue;

    bool postOne(const QueueEntry &entry);
};

extern PotatoMeshUploader *potatoMeshUploader;
