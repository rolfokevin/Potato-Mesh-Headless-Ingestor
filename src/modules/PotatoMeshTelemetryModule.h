#pragma once

#include "mesh/ProtobufModule.h"
#include "mesh/generated/meshtastic/telemetry.pb.h"

// Posts Telemetry packets (device/environment/power/air_quality variants) to
// potato-mesh's POST /api/telemetry.
class PotatoMeshTelemetryModule : public ProtobufModule<meshtastic_Telemetry>
{
  public:
    PotatoMeshTelemetryModule();

  protected:
    virtual bool handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_Telemetry *t) override;
};

extern PotatoMeshTelemetryModule *potatoMeshTelemetryModule;
