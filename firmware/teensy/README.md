# Teensy Firmware

Teensy-side firmware for the S32K-master SPI link.

Open this folder in VS Code:

```text
C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
```

Current layout:
- `include/teensy_config.h`: Teensy pins and timing constants.
- `include/comms/` and `src/comms/`: Teensy SPI slave transport.
- `include/telemetry/` and `src/telemetry/`: packing and decoding for the shared 128-byte `teensy_link` frame.
- `src/main.cpp`: bring-up loop with mock IMU/camera data.

The shared packet contract lives in `../../shared/protocol/teensy_link_protocol.h`.
Do not reintroduce the old 45-byte IMU packet as active code.

## Build And Upload

Use the PlatformIO terminal in VS Code:

```bat
cd /d C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
pio run
pio run -t upload
pio device monitor -b 115200
```

The serial monitor should print lines like:

```text
t=1234 txSeq=120 sensorSeq=120 s32k=118 rx=118 err=0 timeout=0 app=5 speed=0/0 servo=0 ultra=0
```

`rx` increasing means the Teensy decoded S32K MOSI frames. `err` or `timeout`
increasing means the S32K frame is corrupt, missing, or not clocked completely.

## Bench Payload

The current sketch sends deterministic sample data:

- IMU present, calibrated, valid, and yaw-relative.
- Camera 0 valid from the Teensy side.
- Camera 1 intentionally stale/missing.
- SD/logger not ready yet.
- RX counters from the S32K-to-Teensy direction.

This is deliberate. It lets the S32K OLED show both a good component and a
missing component during the same 128-byte SPI test.

## Wiring Summary

S32K link pins:

| S32K signal | S32K pin | Teensy 4.1 pin |
|---|---:|---:|
| SCK | PTB14 | 13 |
| MOSI / S32K SOUT | PTB16 | 11 |
| MISO / S32K SIN | PTB15 | 12 |
| CS / PCS3 | PTB17 | 10 |
| READY input | PTD14 | 31 |
| GND | GND | GND |

Both boards are 3.3 V logic. Do not level-shift to 5 V.

Other Teensy shield pins from the schematic:

| Function | Teensy 4.1 pin |
|---|---:|
| IMU SDA | 18 |
| IMU SCL | 19 |
| IMU interrupt | 30 |
| Display SDA | 17 |
| Display SCL | 16 |
| Camera 1 SI | 4 |
| Camera 2 SI | 5 |
| Camera 1 CLK | 6 |
| Camera 2 CLK | 7 |
| Camera 1 analog | 15 |
| Camera 2 analog | 14 |

Full run instructions are in `../../docs/protocols/teensy-s32k-128b-spi-test.md`.
The full shield pinout is in `../../hardware/shield-v2-pinout.md`.

The current `TeensyLinkSpiSlave` keeps the polling slave structure from the
bring-up sketch, but now transfers the v1 128-byte frame and decodes MOSI from
the S32K. If the 2 MHz bench link is unstable with the current wiring, first
lower the S32K SPI baud rate for bring-up. For final high-rate use, replace only
this transport with a hardware SPI slave implementation while keeping the same
public API.
