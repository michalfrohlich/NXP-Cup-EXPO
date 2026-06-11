# Teensy Status Displays

The two `ZJY_M208_25664-4P` displays run continuously with the normal Teensy
SPI and SD logger firmware and **both show the same combined dashboard**.
A dedicated bench check exists in the hardware self-test mode
(`TEENSY_APP_HARDWARE_TEST` in `include/teensy_app_config.h`).

The S32K OLED is unchanged and remains controlled by the S32K firmware.
Per the shield schematic, the second display header (J25) belongs to the
ESP32, not the Teensy — it is reserved for later.

## Combined dashboard (identical on both panels)

- link `OK` or `WAITING`, received frames, errors, timeouts, S32K sequence;
- S32K app, speed, servo, and ultrasonic values;
- SD `READY`, `NO CARD`, or `ERROR`, filename, drops, written KiB;
- live potentiometer and button states;
- Teensy TX and sensor sequence counters.

If both modules answer on one I2C address they mirror by hardware (serial
shows `d1=MIRRORED d2=MIRRORED`); with unique addresses the firmware writes
the same dashboard to each panel alternately.

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

Two displays with the same fixed I2C address receive the same commands —
which is fine now, because both panels intentionally show the same
dashboard. Unique addresses also work; the content is identical either way.

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
