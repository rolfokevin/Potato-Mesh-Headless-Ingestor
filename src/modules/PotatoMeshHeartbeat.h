#pragma once

#include "concurrency/OSThread.h"

// Periodically POSTs to potato-mesh's POST /api/ingestors so this device shows
// up on the instance's active-ingestors list (separate from the six data
// endpoints, which work independently of this heartbeat).
class PotatoMeshHeartbeat : private concurrency::OSThread
{
  public:
    PotatoMeshHeartbeat();

  protected:
    int32_t runOnce() override;

  private:
    uint32_t startTime = 0;
};

extern PotatoMeshHeartbeat *potatoMeshHeartbeat;
