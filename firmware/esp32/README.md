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

- Initializes ESP UART1 on GPIO17/GPIO16 at 115200 8N1.
- Initializes the shield GPIO pins and ESP-side I2C bus.
- Continuously receives S32K button frames on UART and ACKs them.
- Optionally joins Wi-Fi and exposes `POST /pid`.
- Converts `Kp=10&Ki=5&Kd=1` HTTP form bodies into the temporary UART frame
  `#P10I05D01_`.
- Converts S32K button frames like `#B100Q04_` into ACK frames like
  `#A100Q04_` and drives the ESP32 status LED when any received S32K button is
  active.

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
| GND | connector pin 2 |
| VIN | connector pin 1 |
| Screen power | 3V3 |
| Screen ground | GND |

The matching constants are in `include/esp32_pin_config.h`.

Use UART for S32K/ESP debug or Bluetooth bridge messages. Use GPIO21/GPIO22 for
the ESP32-side screen I2C connection.
