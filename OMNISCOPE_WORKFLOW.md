# Omniscope Workflow API extension

This branch is an adaptation of the official
[AirGradient Arduino firmware](https://github.com/airgradienthq/arduino) tag
`3.3.9`, commit `d8eb6b3c1a699606c59dc908656d23dc64b04caf`.

It adds a direct, optional HTTPS request from an AirGradient ONE I-9PSL to an
Omniscope Workflow API once per minute. It does not use AirGradient Cloud,
MQTT, a local collector, an inbound router port, or a device-side retry queue.

The adaptation remains under the repository's CC BY-SA 4.0 license.

## Behaviour

When enabled and connected over Wi-Fi, the monitor takes the same current
measurement JSON used by the existing firmware and sends one `POST` request to
the configured `/w/execute` endpoint every 60 seconds.

The request is:

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

The parameter value is the complete AirGradient measurement object serialized
as a JSON string.

The call uses HTTP Basic authentication and accepts any HTTP `2xx` response as
success. Connect and response timeouts are 15 seconds. A failure is logged and
that minute is skipped; there is no retry or flash buffer.

## Configuration fields

These fields are accepted only by a local `/config` update:

| Field | Type | Default |
|---|---|---|
| `omniscopeWorkflowEnabled` | boolean | `false` |
| `omniscopeWorkflowEndpoint` | string | empty |
| `omniscopeWorkflowBlock` | string | empty |
| `omniscopeWorkflowParameter` | string | `measurementJson` |
| `omniscopeWorkflowUsername` | string | empty |
| `omniscopeWorkflowPassword` | string | empty |

The endpoint must begin with `https://`. Uploads require all fields, including
the username and password, to be non-empty.

The password must be stored on the device for unattended Basic authentication.
It is never returned by `GET /config`: a configured password is represented by
`********` and `omniscopeWorkflowPasswordSet: true`. Sending `********` back
preserves the stored password; sending an empty string clears it. Configuration
logs redact payloads that contain the password property.

No endpoint or credentials are compiled into this repository.

## TLS

The firmware contains the public Sectigo Public Server Authentication Root E46
trust anchor currently used by the `*.omniscope.me` certificate chain. This is
a public CA certificate, not an Omniscope server certificate and not a private
key.

Moving the workflow within `public.omniscope.me` requires only a configuration
change. Moving to a host whose certificate chains to a different root requires
a firmware trust-anchor update.

## Privacy settings

To use the Omniscope upload without AirGradient Cloud:

```json
{
  "disableCloudConnection": true,
  "offlineMode": false,
  "postDataToAirGradient": false
}
```

`offlineMode` must remain `false` because it disables the networking task
entirely. The Omniscope schedule runs before the existing
`disableCloudConnection` early exit, so it remains active while AirGradient
Cloud and automatic AirGradient OTA checks are disabled.

## Build

Using PlatformIO:

```powershell
platformio run -e esp32-c3
```

The application image is produced at:

```text
.pio/build/esp32-c3/firmware.bin
```

The build identifies itself as `3.3.9-omniscope.2`.

Version `.2` increases the networking task stack from 4096 to 12288 bytes.
The original stack size overflowed during the ESP32-C3 TLS handshake on the
first scheduled Omniscope request.

## Chrome installer

The static installer is in [`web-flasher`](web-flasher). It uses
[ESP Web Tools](https://esphome.github.io/esp-web-tools/) and Chrome or Edge
Web Serial to write:

| Image | Offset |
|---|---:|
| `bootloader.bin` | `0x0000` |
| `partitions.bin` | `0x8000` |
| `boot_app0.bin` | `0xE000` |
| `firmware.bin` | `0x10000` |

The page and binaries are public static files. Flashing occurs locally between
the browser and the ESP32-C3 over USB.

The installer offers the choice to erase the device. Erasing removes Wi-Fi and
monitor configuration. Preserving data keeps the existing NVS and SPIFFS
areas; the new Omniscope fields start disabled when absent.

## Recovery

The official firmware can be restored with AirGradient's
[browser flashing page](https://www.airgradient.com/documentation/kb/kb-firmwares-airgradient-one-i-9psl-open-air-o-1pst-and-open-air-max-o-m-1ppst-firmware-versions/).
