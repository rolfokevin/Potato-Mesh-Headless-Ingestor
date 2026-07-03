#pragma once

#include "mesh/ProtobufModule.h"
#include "mesh/generated/meshtastic/mesh.pb.h"

// Upserts node identity/telemetry/position snapshots to potato-mesh's
// POST /api/nodes whenever a NodeInfo (User) packet is seen on the mesh.
class PotatoMeshNodeModule : public ProtobufModule<meshtastic_User>
{
  public:
    PotatoMeshNodeModule();

  protected:
    virtual bool handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_User *user) override;
};

extern PotatoMeshNodeModule *potatoMeshNodeModule;
