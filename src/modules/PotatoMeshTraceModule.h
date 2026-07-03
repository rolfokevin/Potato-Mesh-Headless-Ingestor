#pragma once

#include "mesh/ProtobufModule.h"
#include "mesh/generated/meshtastic/mesh.pb.h"

// Posts overheard traceroute results to potato-mesh's POST /api/traces.
class PotatoMeshTraceModule : public ProtobufModule<meshtastic_RouteDiscovery>
{
  public:
    PotatoMeshTraceModule();

  protected:
    virtual bool handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_RouteDiscovery *r) override;
};

extern PotatoMeshTraceModule *potatoMeshTraceModule;
