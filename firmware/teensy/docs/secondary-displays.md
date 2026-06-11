# Teensy Status Displays

The two planned `ZJY_M208_25664-4P` displays run continuously with the normal
Teensy SPI and SD logger firmware. There is no separate display test mode.

The S32K OLED is unchanged and remains controlled by the S32K firmware.

## Pages

Display 1 shows S32K-Teensy communication:

- link `OK` or `WAITING`;
- received-frame and S32K sequence counters;
- protocol errors and timeouts;
- S32K app, speed, servo, and ultrasonic values.

Display 2 shows Teensy and SD logging:

- SD `READY`, `NO CARD`, or `ERROR`;
- current `LOGnnn.CSV` filename;
- written KiB and dropped rows;
- live potentiometer and button states;
- Teensy TX and sensor sequence counters;
- honest notes that the current IMU/camera values are mock or missing.

## Current Hardware Assumption

The driver currently assumes:

- SSD1362-compatible 256x64 controller;
- I2C on Teensy `Wire1`;
- display 1 address `0x3C`;
- display 2 address `0x3D`;
- 400 kHz I2C.

These settings are in `include/teensy_config.h`. If Serial reports `DETECTED`
but the screen stays blank, the I2C wiring/address works but the controller
assumption is probably wrong.

Two displays with the same fixed I2C address receive the same commands and
cannot show different pages. Configure unique addresses or add an I2C
multiplexer such as a TCA9548A.

## Wiring

| Teensy signal | Teensy 4.1 pin | Display signal |
|---|---:|---|
| SDA1 | 17 | SDA |
| SCL1 | 16 | SCL |
| 3.3 V | 3.3 V | VCC |
| Ground | GND | GND |

Both Teensy displays share SDA/SCL. They need unique addresses to show the two
different dashboards.

Teensy PCB inputs are read continuously:

| Input | Teensy 4.1 pin | Display/Serial value |
|---|---:|---|
| Potentiometer | 27 / A13 | `POT` / `pot`, raw 12-bit value |
| Button 1 | 28 | `B1` / `b1`, `1` while pressed |
| Button 2 | 29 | `B2` / `b2`, `1` while pressed |

## Timing

The displays remain powered and visible continuously. Only one display is
refreshed every 250 ms, so each display receives fresh values every 500 ms.

The Teensy lowers the READY output before a blocking I2C refresh. The S32K
therefore waits instead of starting a 128-byte SPI transfer during the display
write.

## Files

- `include/teensy_config.h`: pins, addresses, I2C speed, and update period.
- `include/drivers/secondary_displays.h`: dashboard data and status interface.
- `src/drivers/secondary_displays.cpp`: SSD1362/U8x8 display implementation.
- `src/main.cpp`: continuously supplies live SPI and SD status.
- `platformio.ini`: installs U8g2 `2.36.18`.
