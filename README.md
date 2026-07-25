# AirGradient Omniscope Firmware

Privacy-first firmware for the **AirGradient ONE I-9PSL** that sends current
air-quality measurements directly to the Omniscope Workflow API at
[public.omniscope.me](https://public.omniscope.me/).

[Install firmware in Chrome](https://toniopoggi.github.io/airgradient-omniscope/)
·
[Download the Android controller](https://toniopoggi.github.io/airgradient-omniscope-android/)
·
[Technical documentation](OMNISCOPE_WORKFLOW.md)

## Current release

**`3.3.9-omniscope.2`**

- Based on the official AirGradient firmware `3.3.9`.
- Tested on an AirGradient ONE `I-9PSL`.
- Sends one configurable HTTPS request every 60 seconds.
- Includes the networking-stack fix required for HTTPS/TLS on the ESP32-C3.

## Why this fork exists

The standard AirGradient cloud was intentionally excluded from this design.
The monitor sends directly from the local network to your own public Omniscope
instance:

```text
AirGradient ONE
       |
       | HTTPS + Basic authentication
       | one request per minute
       v
Omniscope Workflow API
       |
       v
Parse JSON → add server UTC timestamp → append history → dashboard
```

There is no MQTT broker, local collector, always-on computer, inbound router
port, third-party analytics service, or device-side retry queue.

## Features

- Configurable complete Omniscope `/w/execute` endpoint.
- Configurable workflow block and parameter name.
- Configurable Basic-auth username and password.
- Enable or disable uploads without reflashing.
- Sends the complete AirGradient measurement JSON as a string parameter.
- Supports `refreshFromSource: true`.
- AirGradient cloud connections can remain disabled.
- Local `/config`, `/measures/current`, and `/metrics` APIs remain available.
- Chrome/Edge USB installer with a recoverable stock-firmware path.
- Companion Android app for local configuration and live measurements.

## Install the firmware

Use desktop Chrome, Edge, or another Chromium browser:

1. Open the
   [AirGradient Omniscope installer](https://toniopoggi.github.io/airgradient-omniscope/).
2. Unplug the monitor.
3. Hold the recessed **BOOT** button.
4. Connect USB while holding BOOT, then release it.
5. Click **Connect and install firmware**.
6. Select **USB JTAG / Serial Debug**.
7. Leave **Erase device** unchecked to preserve Wi-Fi and configuration.
8. Install, unplug USB for three seconds, then reconnect normally.

The installer writes the ESP32-C3 bootloader, partition table, boot-app stub,
and application image at their verified flash offsets.

## Configure it

Install the
[AirGradient Omniscope Android app](https://github.com/toniopoggi/airgradient-omniscope-android),
connect to the monitor using its `.local` hostname or private LAN IP, and
complete the **Omniscope Workflow API** section.

| Setting | Purpose |
|---|---|
| `omniscopeWorkflowEnabled` | Enables the one-minute upload |
| `omniscopeWorkflowEndpoint` | Complete HTTPS `/w/execute` URL |
| `omniscopeWorkflowBlock` | Workflow block to execute |
| `omniscopeWorkflowParameter` | Parameter receiving the measurement JSON |
| `omniscopeWorkflowUsername` | Basic-auth username |
| `omniscopeWorkflowPassword` | Basic-auth password |

For private operation:

```json
{
  "offlineMode": false,
  "postDataToAirGradient": false,
  "disableCloudConnection": true,
  "omniscopeWorkflowEnabled": true
}
```

`offlineMode` must remain `false`; otherwise the firmware does not start its
networking task.

## Workflow request

Every 60 seconds the firmware sends:

```json
{
  "blocks": ["CONFIGURED BLOCK"],
  "refreshFromSource": true,
  "cancelExisting": false,
  "waitForIdle": true,
  "params": {
    "updates": [
      {
        "name": "CONFIGURED PARAMETER",
        "value": "{\"wifi\":-55,\"rco2\":620,\"pm02\":4}"
      }
    ],
    "waitForIdle": true
  }
}
```

The value is the complete measurement object serialized as a JSON string. The
Omniscope workflow can parse it, add a server-side UTC timestamp, append the
record, and refresh the dashboard data.

## Security and privacy

- Only HTTPS endpoints are accepted.
- Requests use Basic authentication over TLS.
- The firmware trusts the public Sectigo root used by `*.omniscope.me`.
- No private certificate or Omniscope server key is embedded.
- Credentials are stored locally on the monitor.
- `GET /config` returns `********`, never the stored password.
- Omniscope fields can be changed only through the monitor's local API.
- The Android WebView exposes its native bridge only to bundled trusted HTML.
- No account, advertising, analytics, or third-party cloud is required.

## Debugging

Connect the monitor normally over USB and open **Logs & Console** from the
installer, or use a serial monitor at `115200` baud.

A successful request reports:

```text
Omniscope: submitting workflow (...-byte request)
Omniscope: workflow submitted (HTTP 2xx)
```

Version `.2` reserves a 12288-byte networking-task stack. Version `.1` used
4096 bytes and overflowed during the first TLS request on the ESP32-C3.

## Build from source

The supported build environment is PlatformIO:

```sh
platformio run -e esp32-c3
```

The application image is written to:

```text
.pio/build/esp32-c3/firmware.bin
```

The committed browser-installer binaries and their SHA-256 checksums are under
[`web-flasher/firmware`](web-flasher/firmware).

## Upstream and license

This project is based on the official
[AirGradient Arduino firmware](https://github.com/airgradienthq/arduino) tag
`3.3.9`, commit `d8eb6b3c1a699606c59dc908656d23dc64b04caf`.

The upstream project and this fork retain their existing open-source licenses.
See [LICENSE](LICENSE) and the source-file headers for details. AirGradient is a
trademark of its respective owner; this is an independently modified firmware
fork.
