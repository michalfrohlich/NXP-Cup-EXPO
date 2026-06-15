# Teensy Firmware

Teensy-side firmware for the S32K-master SPI link.

Open this folder in VS Code:

```text
C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
```

Current layout:
- `include/teensy_config.h`: Teensy pins and timing constants.
- `include/comms/` and `src/comms/`: Teensy SPI slave transport.
- `include/drivers/` and `src/drivers/`: physical MPU6050 acquisition and always-on status displays.
- `include/telemetry/` and `src/telemetry/`: packing and decoding for the shared 128-byte `teensy_link` frame.
- `include/logging/` and `src/logging/`: SdFat CSV logger for the built-in SD slot.
- `src/main.cpp`: bring-up loop with physical IMU acquisition and honest missing-camera states.
- `docs/imu-mpu6050.md`: IMU wiring, filtering, status, and hardware tests.
- `docs/secondary-displays.md`: two-display hardware assumptions and pages.
- `docs/integrated-bring-up.md`: combined displays, SPI, and SD test.

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
t=1234 txSeq=120 sensorSeq=120 s32k=118 rx=118 err=0 timeout=0 app=5 speed=0/0 servo=0 ultra=0 sd=R drop=0 sdkB=12
```

`rx` increasing means the Teensy decoded S32K MOSI frames. `err` or `timeout`
increasing means the S32K frame is corrupt, missing, or not clocked completely.

Both Teensy displays start automatically. Display 1 shows SPI health and
display 2 shows SD health. No separate display mode is required.

## SD Logging

With an SD card in the built-in slot, each boot creates the next free
`LOGnnn.CSV` and streams telemetry rows at the sensor rate. Without a card
the firmware runs normally and serial shows `sd=-`. Status fields:
`sd=R/-/E` (ready / no card / error), `drop=` dropped rows, `sdkB=` KiB
written. The same status reaches the S32K OLED IMU/LOG page over SPI.

Details, column list, and MATLAB import: `../../docs/protocols/teensy-sd-logging.md`.

## Bench Payload

The current sketch sends:

- Physical MPU6050 acceleration, gyro, temperature, filtered roll/pitch, and
  relative yaw when the sensor is detected and calibrated.
- Camera 0 and camera 1 explicitly stale/missing.
- Real SD/logger ready, error, drop, and written-byte status.
- RX counters from the S32K-to-Teensy direction.

No Teensy camera data is fabricated. Both camera slots remain lost/stale until
their physical acquisition drivers are integrated.

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
Physical IMU details are in `docs/imu-mpu6050.md`.
The full shield pinout is in `../../hardware/shield-v2-pinout.md`.

The current `TeensyLinkSpiSlave` keeps the polling slave structure from the
bring-up sketch, but now transfers the v1 128-byte frame and decodes MOSI from
the S32K. If the 2 MHz bench link is unstable with the current wiring, first
lower the S32K SPI baud rate for bring-up. For final high-rate use, replace only
this transport with a hardware SPI slave implementation while keeping the same
public API.
