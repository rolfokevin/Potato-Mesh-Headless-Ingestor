# potato-mesh Headless Ingestor

This is a fork of [Meshtastic firmware](https://github.com/meshtastic/firmware)
with an added on-device ingestor for [potato-mesh](https://github.com/l5yth/potato-mesh).

Instead of running a separate Python process that connects to a Meshtastic
node over serial/TCP/BLE and forwards data to a potato-mesh instance, this
firmware POSTs data directly from the node itself over WiFi — no external
host required.

## What it does

Six new modules listen to mesh traffic (promiscuously, so they see traffic
even when it's not addressed to this node) and POST matching JSON to a
potato-mesh instance's API:

| Module | potato-mesh endpoint |
| --- | --- |
| `PotatoMeshNodeModule` | `POST /api/nodes` |
| `PotatoMeshPositionModule` | `POST /api/positions` |
| `PotatoMeshTelemetryModule` | `POST /api/telemetry` |
| `PotatoMeshMessageModule` | `POST /api/messages` |
| `PotatoMeshNeighborModule` | `POST /api/neighbors` |
| `PotatoMeshTraceModule` | `POST /api/traces` |

A seventh module, `PotatoMeshHeartbeat`, periodically POSTs to
`POST /api/ingestors` so the device shows up on the instance's active-ingestor
list. All seven share `PotatoMeshUploader`, a small queued HTTPS client, and
`PotatoMeshFilter`, which applies channel-name and minimum-SNR filtering.

The node keeps working as a normal Meshtastic device the whole time — LoRa
routing, BLE app pairing, and the rest of the stock firmware are untouched.

## Configuring it

Settings (instance domain, API token, allowed channels, minimum SNR) are
**not** compiled into the firmware. They're editable at runtime, with no
reflash required, from the device's own web UI:

```
http://<device-ip>/potatomesh
```

Settings are saved to `/potatomesh.json` on the device's flash filesystem, so
they survive reflashing (the firmware image and the filesystem are separate
flash partitions). Leaving the domain or API token blank disables ingestion.

`src/modules/PotatoMeshConfig.h.example` documents the compile-time seed
values used only the very first time a fresh device boots (before anyone has
saved settings through the web page above). Copy it to
`src/modules/PotatoMeshConfig.h` if you want to change those seed defaults;
that file is gitignored so real credentials never get committed.

## Building and flashing

Same as any Meshtastic build — see the
[official build instructions](https://meshtastic.org/docs/development/firmware/build).
This has been built and tested for the `heltec-v3` environment (Heltec WiFi
LoRa 32 V3, ESP32-S3):

```
pio run -e heltec-v3
pio run -e heltec-v3 -t upload
```

## Known limitations

- Traceroute records don't include `elapsed_ms` (would need request/response
  pairing not currently implemented).
- Power telemetry only maps channel 1's voltage/current to potato-mesh's
  generic `voltage`/`current` fields; air-quality telemetry sends envelope
  fields only, no PM/CO2 readings (potato-mesh has no confirmed schema for
  either yet).
- No `HIDDEN_CHANNELS`-equivalent (the reference Python ingestor supports
  excluding specific channels' content even when otherwise allowed).
- Settings aren't wired into Meshtastic's native moduleConfig system, so they
  won't appear in the official Meshtastic app's Module Configuration screen —
  that would require changes to Meshtastic's protobufs and to the separate
  mobile/web app codebases. The web page above is the tradeoff made instead.
