#pragma once

#include "mesh/SinglePortModule.h"

// Posts plaintext channel messages to potato-mesh's POST /api/messages.
class PotatoMeshMessageModule : public SinglePortModule
{
  public:
    PotatoMeshMessageModule();

  protected:
    virtual ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;
};

extern PotatoMeshMessageModule *potatoMeshMessageModule;
