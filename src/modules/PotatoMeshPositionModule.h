#pragma once

#include "mesh/ProtobufModule.h"
#include "mesh/generated/meshtastic/mesh.pb.h"

// Posts Position packets to potato-mesh's POST /api/positions.
class PotatoMeshPositionModule : public ProtobufModule<meshtastic_Position>
{
  public:
    PotatoMeshPositionModule();

  protected:
    virtual bool handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_Position *pos) override;
};

extern PotatoMeshPositionModule *potatoMeshPositionModule;
