# ESP32 Firmware

The ESP32 is the wireless PC/UI bridge for the NXP Cup car. It runs as a
standalone Wi-Fi access point and communicates with the S32K144 over UART.

## Direct Wi-Fi Connection

The access point configuration is intentionally fixed in
`src/pc_web_link.c`:

| Setting | Value |
|---|---|
| SSID | `EXPO_NXP_DATA` |
| Password | `nobodyguessesthispassword1234` |
| Web UI | `http://192.168.4.1/` |
| ESP32 address | `192.168.4.1` |
| Maximum clients | 4 |

To use the current PID page:

1. Power or flash the ESP32.
2. Connect the PC or phone to `EXPO_NXP_DATA`.
3. Enter the password shown above.
4. Open `http://192.168.4.1/`.
5. Set `Kp`, `Ki`, and `Kd`, then press **Send PID**.

The page validates integer values from `0` to `99` and sends the temporary
UART frame:

```text
#P10I05D01_
```

HTTP `OK` currently means that the complete frame was written to the ESP32
UART driver. The S32K firmware does not yet decode or apply this PID command.

## Source Ownership

```text
include/
├── pc_web_link.h
├── s32k_uart_link.h
├── esp32_app_status.h
└── esp32_display.h

src/
├── main.c
├── pc_web_link.c
├── s32k_uart_link.c
└── esp32_display.c
```

- `pc_web_link` owns Wi-Fi AP configuration, connected-client state, the HTTP
  server, webpage, form validation, and PC-facing responses.
- `s32k_uart_link` owns UART1, shared protocol encoding/decoding, its service
  task, button-frame handling, ACK transmission, and UART diagnostics.
- `main.c` initializes both links and forwards accepted web PID commands to
  the UART link through a callback.
- `esp32_display` owns the I2C bus and SH1122 OLED diagnostics.

The UART link remains operational if Wi-Fi or the HTTP server fails to start.

## S32K UART

| Signal | ESP32 GPIO |
|---|---:|
| UART RX | GPIO16 |
| UART TX | GPIO17 |

The link uses UART1 at `460800 8N1`. The matching S32K generated setup is
LPUART2 on PTD6/PTD7.

The existing bring-up protocol also:

- Receives S32K button frames such as `#B100Q04T1234_`.
- Replies with ACK frames such as `#A100Q04T1234_`.
- Tracks UART receive, protocol, and ACK errors.

Shared frame definitions are in
`../../shared/protocol/esp_s32k_uart_protocol.h`.

## OLED

The SH1122 256x64 OLED uses:

| OLED signal | ESP32 GPIO / supply |
|---|---:|
| SDA | GPIO21 |
| SCL | GPIO22 |
| Address | `0x3C` |
| VCC | 3V3 |
| GND | GND |

The display shows UART diagnostics and one of these network states:

- `wifi:init`: AP initialization is in progress.
- `wifi:ap`: AP and HTTP server are ready.
- `wifi:pc`: at least one client is connected.
- `wifi:err`: AP or HTTP startup failed.

## Build

From `firmware/esp32/`:

```sh
pio run --environment esp32dev
```

Upload and monitor using the appropriate local serial port:

```sh
pio run --environment esp32dev --target upload
pio device monitor
```

The first build downloads the pinned U8g2 ESP-IDF component into
`managed_components/`. That directory is generated and ignored.

## Deferred

- S32K decoding and application of PID values.
- S32K applied/rejected acknowledgement.
- Decimal or fixed-point PID values.
- Vision tuning in the web UI.
- Persistent settings.
- Bluetooth.
- Station or AP+station Wi-Fi modes.
