#pragma once

#include "mesh/MeshTypes.h"
#include <string>

namespace PotatoMesh
{
// Formats a NodeNum as the "!xxxxxxxx" canonical id convention.
std::string hexNodeNum(NodeNum n);

// Formats a unix epoch as "YYYY-MM-DDTHH:MM:SSZ".
std::string isoTime(uint32_t epochSecs);

// Standard base64 (with padding), used for public_key bytes.
std::string base64Encode(const uint8_t *data, size_t len);
} // namespace PotatoMesh
