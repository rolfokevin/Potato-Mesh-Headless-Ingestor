#include "modules/PotatoMeshUtil.h"
#include <cstdio>
#include <ctime>

namespace PotatoMesh
{

std::string hexNodeNum(NodeNum n)
{
    char buf[10];
    snprintf(buf, sizeof(buf), "!%08x", (unsigned int)n);
    return std::string(buf);
}

std::string isoTime(uint32_t epochSecs)
{
    time_t t = (time_t)epochSecs;
    struct tm tmVal;
    gmtime_r(&t, &tmVal);
    char buf[21];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tmVal);
    return std::string(buf);
}

std::string base64Encode(const uint8_t *data, size_t len)
{
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);

    size_t i = 0;
    while (i + 3 <= len) {
        uint32_t n = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8) | data[i + 2];
        out += table[(n >> 18) & 0x3F];
        out += table[(n >> 12) & 0x3F];
        out += table[(n >> 6) & 0x3F];
        out += table[n & 0x3F];
        i += 3;
    }

    size_t rem = len - i;
    if (rem == 1) {
        uint32_t n = (uint32_t)data[i] << 16;
        out += table[(n >> 18) & 0x3F];
        out += table[(n >> 12) & 0x3F];
        out += "==";
    } else if (rem == 2) {
        uint32_t n = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8);
        out += table[(n >> 18) & 0x3F];
        out += table[(n >> 12) & 0x3F];
        out += table[(n >> 6) & 0x3F];
        out += "=";
    }
    return out;
}

} // namespace PotatoMesh
