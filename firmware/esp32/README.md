# ESP32 Firmware

The ESP32 is the wireless PC/UI bridge for the NXP Cup car. It runs its own
Wi-Fi access point and HTTP server, then communicates with the S32K144 over
UART. There is no separate PC-side web server.

## Direct Wi-Fi Connection

| Setting | Value |
|---|---|
| SSID | `EXPO_NXP_DATA` |
| Password | `nobodyguessesthispassword1234` |
| Web UI | `http://192.168.4.1/` |
| ESP32 address | `192.168.4.1` |
| Maximum clients | 4 |

The access-point configuration is intentionally fixed in
`src/pc_web_link.c`.

## Build, Flash, And Monitor

Install PlatformIO Core or use a VS Code PlatformIO terminal. From the
repository root:

```powershell
cd firmware/esp32
pio run --environment esp32dev
pio device list
pio run --environment esp32dev --target upload --upload-port COM5
pio device monitor --port COM5 --baud 115200
```

Replace `COM5` with the ESP32 serial port shown by `pio device list`. The
monitor is optional, but it shows AP startup, connected clients, UART frames,
and S32K tuning responses.

The first build downloads the pinned U8g2 ESP-IDF component into
`managed_components/`. That directory is generated and ignored.

## Wiring And Use

1. Flash the ESP32.
2. Flash the S32K with `APP_TEST_ESP_LINK_TEST` enabled.
3. Wire ESP32 TX GPIO17 to S32K LPUART2 RX PTD6.
4. Wire ESP32 RX GPIO16 to S32K LPUART2 TX PTD7.
5. Connect ESP32 and S32K ground.
6. Power both boards.
7. Connect the PC or phone to `EXPO_NXP_DATA`.
8. Enter the password shown above.
9. Open `http://192.168.4.1/`.
10. Edit values and press **Apply to S32K**.

If the operating system says the Wi-Fi network has no internet, remain
connected; that is expected for this direct local link.

## Web Tuning

The page edits the same eight RAM-backed values exposed by the S32K
`Serial tune` bench item:

| Group | Values | Web range |
|---|---|---:|
| Controller | `KP`, `KI`, `KD` | `0..99.999` |
| Steering output | command clamp | `0..100` |
| Steering output | LPF alpha | `0..1.000` |
| Line detector | minimum contrast | `0..4095` |
| Line detector | high/low edge percentages | `1..100` |

The web limits are intentionally narrower than the permissive text parser in
`bench_serial_tune.c` for fields such as clamp and LPF alpha. They match the
meaningful operating domain of those values.

The ESP32 converts decimal controller values to integer milli-units and sends
one fixed-width tuning snapshot. An example frame is:

```text
#T07P04500I00030D02000C070L0450M0650H040E055X29_
```

`07` is the request sequence and `29` is the CRC-8/ATM value. After
validating and storing the complete snapshot, the S32K replies:

```text
#R07OX49_
```

`O` means accepted. `E` is reserved for a rejected, well-formed request.
The browser reports success only after the matching S32K response arrives.
Values remain RAM-only and return to defaults after reset.

## Source Ownership

```text
include/
|-- pc_web_link.h
|-- s32k_uart_link.h
|-- esp32_app_status.h
`-- esp32_display.h

src/
|-- main.c
|-- pc_web_link.c
|-- s32k_uart_link.c
`-- esp32_display.c
```

- `pc_web_link` owns Wi-Fi AP configuration, connected-client state, the HTTP
  server, webpage, form validation, and PC-facing responses.
- `s32k_uart_link` owns UART1, shared protocol encoding/decoding, its service
  task, button-frame handling, command/result correlation, and diagnostics.
- `main.c` initializes both links and forwards accepted web tuning snapshots
  to the UART link through a callback.
- `esp32_display` owns the I2C bus and SH1122 OLED diagnostics.

The UART link remains operational if Wi-Fi or the HTTP server fails to start.

## UART Protocol

| Signal | ESP32 GPIO | S32K signal |
|---|---:|---|
| ESP32 RX | GPIO16 | LPUART2 TX PTD7 |
| ESP32 TX | GPIO17 | LPUART2 RX PTD6 |

The link uses UART1/LPUART2 at `470588 8N1`, matching the actual
S32DS-generated S32K LPUART2 rate.

The bring-up protocol also:

- Receives CRC-protected S32K button frames such as
  `#B100Q04T1234Xcc_`.
- Replies with CRC-protected button ACK frames such as
  `#A100Q04T1234Xcc_`.
- Sends full tuning snapshots and waits for the matching S32K result, retrying
  once if the first 150 ms attempt times out.
- Tracks UART receive, protocol, ACK, tuning-result, and timeout counters.

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

## Current Timing

- The browser sends one HTTP `POST /tune` only when **Apply to S32K** is
  pressed. Sliders do not continuously transmit.
- The 48-byte tuning frame takes about `1.02 ms` on the UART wire at
  `470588 8N1`.
- The S32K LPUART2 ISR stores received bytes in a 256-byte software ring and
  records hardware errors. The foreground service performs all frame parsing.
- The S32K applies the validated snapshot immediately in the ESP link test,
  then transmits the 9-byte CRC-protected result with a
  four-byte-per-iteration TX budget.
- The ESP32 UART task blocks in the UART driver for up to 20 ms when idle and
  processes received bytes immediately without an additional polling delay.
- The ESP32 waits 150 ms for a matching result and retries the same sequence
  once. After 300 ms without a result, the browser receives a timeout error.
- S32K button state is sampled on its normal button period and queued at least
  every 50 ms. The ESP32 replies to each valid button frame with its existing
  17-byte CRC-protected ACK.
- The S32K OLED is disabled in the dedicated ESP link test while UART
  reliability is validated. The ESP32 OLED refreshes every 50 ms when
  connected.

## Deferred

- Applying ESP32 tuning snapshots safely while a race controller is running.
- Reporting active S32K values back to a newly connected browser.
- Persistent settings.
- Stop command and high-level vehicle telemetry.
- Bluetooth.
- Station or AP+station Wi-Fi modes.
