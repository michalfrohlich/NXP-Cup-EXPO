# ESP32 Firmware

Placeholder for future ESP32 firmware.

Expected role:
- Wireless communication.
- PC UI bridge.
- Forwarding tuning commands to the S32K.
- Forwarding vehicle status and telemetry to the PC.

No build system is defined here yet.

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
