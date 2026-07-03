#include "modules/PotatoMeshFilter.h"
#include "mesh/Channels.h"
#include "modules/PotatoMeshSettings.h"

#include <cstring>

namespace PotatoMesh
{

static bool channelAllowed(uint8_t chIndex)
{
    if (potatoMeshSettings.allowedChannels.empty())
        return true; // empty list = no channel filtering

    const char *name = channels.getName(chIndex);
    if (!name)
        return false;

    size_t nameLen = strlen(name);
    const char *p = potatoMeshSettings.allowedChannels.c_str();
    while (*p) {
        const char *comma = strchr(p, ',');
        size_t entryLen = comma ? (size_t)(comma - p) : strlen(p);
        if (entryLen == nameLen && strncmp(p, name, entryLen) == 0)
            return true;
        if (!comma)
            break;
        p = comma + 1;
    }
    return false;
}

bool passesFilter(const meshtastic_MeshPacket &mp)
{
    if (!channelAllowed(mp.channel))
        return false;

    if (mp.rx_snr != 0 && mp.rx_snr < potatoMeshSettings.minSnr)
        return false;

    return true;
}

} // namespace PotatoMesh
