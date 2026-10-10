# ESP32 Application

ESP32 is the PC/phone-to-car bridge. It runs an ESP-IDF HTTP server and its own
Wi-Fi access point. The webpage is embedded as a C string in
`firmware/esp32/src/pc_web_link.c`; there is no separate HTML server to start
on the PC.

## Connect

1. Build/upload environment `esp32dev`.
2. Connect ESP TX GPIO17 to S32K PTD6 RX, ESP RX GPIO16 to S32K PTD7 TX,
   and connect ground.
3. For motionless verification, run the S32K `ESP Link` bench item.
4. Connect the PC/phone to **EXPO_NXP_DATA**, password
   `nobodyguessesthispassword1234`.
5. Stay connected if the OS reports no internet. Open
   [http://192.168.4.1/](http://192.168.4.1/).

The AP uses WPA2-PSK and allows four clients. Commands are in the
[testing guide](../system/testing.md#build-and-upload).

## Module Ownership And Startup

| Module | Responsibility |
|---|---|
| `main.c` | Startup, callbacks connecting web requests to UART, display snapshots/retries |
| `pc_web_link.c` | AP/DHCP, client state, HTTP endpoints, embedded HTML, validation |
| `s32k_uart_link.c` | UART task, encoding/parsing, matched results, retry/timeout, pending LED |
| `esp32_display.c` | I2C and SH1122 rendering |
| `esp32_app_status.h` | Status snapshot shared by application and display |

Startup initializes the display bus, scans/probes the OLED, and shows a 1.5 s
self-test when found. It then starts UART and its task, followed by Wi-Fi/HTTP.
An OLED probe failure triggers retry; a Wi-Fi/HTTP failure is logged while UART
continues. Low-level bus/UART initialization failures use `ESP_ERROR_CHECK`
and are not silently ignored.

## User Actions And Results

`GET /` serves the page, `POST /tune` submits all eight tune values, and
`POST /drive` submits Start or Stop. Moving a slider alone does not transmit.
The browser's initial/default values are local UI values, not read back from
S32K. Apply sends the complete snapshot and can overwrite previously tuned
fields.

The UART task runs independently of the HTTP handler waiting for a result.
It uses 470588 baud, 8N1, matching generated S32K configuration. The ESP waits
150 ms per attempt and retries the same sequence once. The response waits
total 300 ms; this is not a strict browser round-trip bound because locking,
Wi-Fi, task scheduling, and HTTP add time.

Success means S32K accepted the request in its current mode. In ESP Link tests,
Start/Stop is acknowledged without movement; Stack Drive executes it. Tune
Drive accepts tuning only, and standalone race does not consume ESP requests.
Teensy parameter acknowledgment is separate and asynchronous. See
[tuning and command scope](../protocols/tuning.md).

## Indicators And Limits

The GPIO2 blue LED stays on while awaiting a S32K response, with a minimum
100 ms visible pulse. It turns off after completion or timeout; the light
alone does not identify success. S32K `swPcb` does not control this LED.

The SH1122 **256x64** OLED uses SDA21/SCL22, address `0x3C`, and configured
I2C clock **800 kHz**. The main loop targets a 50 ms refresh whether or not
a browser is associated. It shows button-frame fields, UART/error counters,
request results, and `wifi:init/ap/pc/err`. Those are bridge diagnostics, not
live vehicle speed/phase telemetry. Button fields may remain unchanged in
modes that do not publish buttons.

Settings are RAM-only. There is no continuous car-status web feed, persistent
configuration, or readback of active tuning into a newly opened browser.
Neither losing Wi-Fi nor closing the browser itself sends a Stop command.

[ESP source and wiring reference](https://github.com/michalfrohlich/NXP-Cup-EXPO/blob/main/firmware/esp32/README.md)
