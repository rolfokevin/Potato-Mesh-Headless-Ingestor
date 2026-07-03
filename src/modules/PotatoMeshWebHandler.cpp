#if !MESHTASTIC_EXCLUDE_WEBSERVER
#include "modules/PotatoMeshSettings.h"

#undef str
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPURLEncodedBodyParser.hpp>

using namespace httpsserver;

static std::string htmlEscape(const std::string &in)
{
    std::string out;
    out.reserve(in.size());
    for (char c : in) {
        switch (c) {
        case '&':
            out += "&amp;";
            break;
        case '<':
            out += "&lt;";
            break;
        case '>':
            out += "&gt;";
            break;
        case '"':
            out += "&quot;";
            break;
        default:
            out += c;
        }
    }
    return out;
}

static void renderForm(HTTPResponse *res, const char *message = nullptr)
{
    res->setHeader("Content-Type", "text/html");

    std::string html = "<html><head><title>potato-mesh ingestor</title></head><body>";
    html += "<h1>potato-mesh ingestor settings</h1>";
    if (message) {
        html += "<p><b>";
        html += message;
        html += "</b></p>";
    }
    html += "<form method=post action=/potatomesh>";
    html += "<table>";
    html += "<tr><td>Instance domain</td><td><input name=domain size=40 value=\"" + htmlEscape(potatoMeshSettings.domain) +
            "\"></td></tr>";
    html += "<tr><td>API token</td><td><input name=api_token size=40 value=\"" + htmlEscape(potatoMeshSettings.apiToken) +
            "\"></td></tr>";
    html += "<tr><td>Allowed channels (comma-separated, blank = all)</td><td><input name=allowed_channels size=40 value=\"" +
            htmlEscape(potatoMeshSettings.allowedChannels) + "\"></td></tr>";
    html += "<tr><td>Minimum SNR</td><td><input name=min_snr size=10 value=\"" + std::to_string(potatoMeshSettings.minSnr) +
            "\"></td></tr>";
    html += "</table>";
    html += "<input type=submit value=Save>";
    html += "</form>";
    html += "<p>Leave domain or API token blank to disable ingestion.</p>";
    html += "<p><a href=/admin>Back to admin</a></p>";
    html += "</body></html>";

    res->print(html.c_str());
}

void handlePotatoMeshSettings(HTTPRequest *req, HTTPResponse *res)
{
    renderForm(res);
}

void handlePotatoMeshSettingsApply(HTTPRequest *req, HTTPResponse *res)
{
    std::string domain = potatoMeshSettings.domain;
    std::string apiToken = potatoMeshSettings.apiToken;
    std::string allowedChannels = potatoMeshSettings.allowedChannels;
    float minSnr = potatoMeshSettings.minSnr;

    HTTPBodyParser *parser = new HTTPURLEncodedBodyParser(req);
    while (parser->nextField()) {
        std::string name = parser->getFieldName();
        std::string value;
        while (!parser->endOfField()) {
            byte buf[256];
            size_t readLength = parser->read(buf, sizeof(buf));
            if (readLength == 0)
                break;
            value.append(reinterpret_cast<const char *>(buf), readLength);
            if (value.size() > 512)
                break; // guard against absurdly long input
        }

        if (name == "domain")
            domain = value;
        else if (name == "api_token")
            apiToken = value;
        else if (name == "allowed_channels")
            allowedChannels = value;
        else if (name == "min_snr")
            minSnr = strtof(value.c_str(), nullptr);
    }
    delete parser;

    potatoMeshSettings.domain = domain;
    potatoMeshSettings.apiToken = apiToken;
    potatoMeshSettings.allowedChannels = allowedChannels;
    potatoMeshSettings.minSnr = minSnr;
    potatoMeshSettingsSave();

    renderForm(res, "Settings saved.");
}
#endif
