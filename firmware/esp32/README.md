# ESP32 Firmware

Expected role:
- Wireless communication.
- PC UI bridge.
- Forwarding tuning commands to the S32K.
- Forwarding vehicle status and telemetry to the PC.

This folder now contains the first cleaned ESP32 PlatformIO/ESP-IDF bring-up
project imported from the standalone teammate prototype. It is intentionally a
small bridge, not final vehicle UI firmware.

## Current Prototype Behavior

- Initializes ESP UART1 on GPIO17/GPIO16 at 460800 8N1.
- Initializes the shield GPIO pins, ESP-side I2C bus, and SH1122 OLED.
- Probes the OLED address at startup and shows a short display self-test pattern
  before switching to the normal bring-up status page.
- Continuously receives S32K button frames on UART and ACKs them.
- Refreshes a simple OLED bring-up screen with UART RX/error counters, the last
  decoded S32K button frame, ACK errors, and Wi-Fi state.
- Optionally joins Wi-Fi and exposes `POST /pid`.
- Converts `Kp=10&Ki=5&Kd=1` HTTP form bodies into the temporary UART frame
  `#P10I05D01_`.
- Converts S32K button frames like `#B100Q04T1234_` into ACK frames like
  `#A100Q04T1234_` and drives the ESP32 status LED when any received S32K
  button is active. The `Tmmmm` field is the S32K timestamp modulo 10000 ms,
  echoed by the ESP so the S32K can calculate ACK age.

The matching S32K generated setup is `LPUART2` on `PTD6/PTD7`.

Wi-Fi credentials are not committed. Provide them as PlatformIO build flags or
through your local environment:

```ini
build_flags =
    -I../../shared/protocol
    -DESP32_WIFI_SSID=\"your-ssid\"
    -DESP32_WIFI_PASSWORD=\"your-password\"
```

Build from this folder with:

```sh
pio run
```

The first build downloads the pinned U8g2 ESP-IDF component into
`managed_components/`. That folder is generated and ignored; `dependencies.lock`
is kept so the resolved U8g2 revision stays reproducible.

## OLED Bring-Up Display

The ESP32-side display foundation is intentionally split into layers:

- `include/esp32_app_status.h` defines the app-neutral status snapshot shown on
  the display.
- `include/esp32_display.h` exposes the small OLED API used by `main.c`.
- `src/esp32_display.c` owns the U8g2 SH1122 setup, I2C byte callback, and
  bring-up screen rendering.
- `esp32_s32k_bridge.c` owns UART/protocol state and only publishes a status
  snapshot through `EspBridge_GetStatus()`.

The current OLED target is a 256x64 SH1122 module on the shield I2C bus:

| OLED signal | ESP32 GPIO / supply |
|---|---:|
| SDA | GPIO21 |
| SCL | GPIO22 |
| VCC | 3V3 |
| GND | GND |

The default address is `0x3C` and the bus is configured for `800 kHz` in
`include/esp32_pin_config.h`.

On boot, the firmware first scans the I2C bus and prints every responding 7-bit
address, plus the equivalent 8-bit write address. A PCB marking of `0x78`
should appear in the scan as `7bit=0x3C write8=0x78`. The display init then
probes address `0x3C`. If the OLED does not ACK, the UART bridge continues
running and the serial log reports the display probe error. If the OLED is
found, a short self-test screen appears before the normal UART/Wi-Fi status
screen.

## Shield Pin Plan

Use these pins for the ESP32 side of the shield:

| Function | ESP32 GPIO / connector pin |
|---|---:|
| UART RX | GPIO16 |
| UART TX | GPIO17 |
| BTN1 | GPIO18 |
| BTN2 | GPIO19 |
| I2C data / SDA | GPIO21 |
| I2C clock / SCL | GPIO22 |
| SH1122 OLED address | 0x3C |
| GND | connector pin 2 |
| VIN | connector pin 1 |
| Screen power | 3V3 |
| Screen ground | GND |

The matching constants are in `include/esp32_pin_config.h`.

Use UART for S32K/ESP debug or Bluetooth bridge messages. Use GPIO21/GPIO22 for
the ESP32-side screen I2C connection.
