#pragma once

#include "mesh/generated/meshtastic/mesh.pb.h"

namespace PotatoMesh
{
// Returns true if this packet should be ingested, per POTATOMESH_ALLOWED_CHANNELS
// and POTATOMESH_MIN_SNR in PotatoMeshConfig.h. Packets with no SNR info (e.g.
// our own originated data, rx_snr == 0) always pass the SNR check.
bool passesFilter(const meshtastic_MeshPacket &mp);
} // namespace PotatoMesh
