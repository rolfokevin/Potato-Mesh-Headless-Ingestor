#pragma once

#include "mesh/ProtobufModule.h"
#include "mesh/generated/meshtastic/mesh.pb.h"

// Posts NeighborInfo packets to potato-mesh's POST /api/neighbors.
class PotatoMeshNeighborModule : public ProtobufModule<meshtastic_NeighborInfo>
{
  public:
    PotatoMeshNeighborModule();

  protected:
    virtual bool handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_NeighborInfo *ni) override;
};

extern PotatoMeshNeighborModule *potatoMeshNeighborModule;
