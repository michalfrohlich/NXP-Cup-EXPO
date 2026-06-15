# Teensy S32K 128-Byte SPI Test

## Goal

This document explains how to run the two-board SPI bench test after the
packet update to 128 bytes.

The S32K is the main controller and SPI master. The Teensy 4.1 is the sensor
board and SPI slave. Each transfer exchanges one fixed 128-byte command frame
and one fixed 128-byte Teensy telemetry frame at the same time. The S32K
services the link every 5 ms but schedules the actual transfers itself: every
10 ms while the Teensy READY pin is high (see SPI Settings below).

The S32K services this SPI link only while the Teensy link test is active. Use
either the compile-time direct mode or the runtime OLED bench test. There is no
Python display path for this test.

## What To Open

Open this in S32 Design Studio:

```text
C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\s32k144
```

This folder contains the S32K project files, generated RTD files, `Nxp_Cup.mex`,
and the S32K application code.

Open this in VS Code:

```text
C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
```

This folder contains `platformio.ini`, the Teensy sketch, and the Teensy-side
SPI/telemetry code.

The shared packet definition is here:

```text
C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\shared\protocol\teensy_link_protocol.h
```

The shield pinout from the schematic is here:

```text
C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\hardware\shield-v2-pinout.md
```

## Code Locations

| Area | File |
|---|---|
| Shared packet layout | `shared/protocol/teensy_link_protocol.h` |
| Shared CRC | `shared/protocol/teensy_link_crc.h` |
| S32K driver API | `firmware/s32k144/include/drivers/teensy_link.h` |
| S32K driver implementation | `firmware/s32k144/src/drivers/teensy_link.c` |
| S32K test service/viewer | `firmware/s32k144/src/app/modes/bench_teensy_link.c` |
| S32K direct compile-time mode | `firmware/s32k144/src/app/modes/mode_teensy_link.c` |
| S32K app mode selection | `firmware/s32k144/src/app/app_config.h` |
| S32K runtime menu | `firmware/s32k144/src/app/modes/bench_menu.c` |
| Teensy pins and timing | `firmware/teensy/include/teensy_config.h` |
| Teensy SPI slave transport | `firmware/teensy/src/comms/teensy_link_spi_slave.cpp` |
| Teensy packet packing/decoding | `firmware/teensy/src/telemetry/teensy_link_telemetry.cpp` |
| Teensy sample app | `firmware/teensy/src/main.cpp` |

## Wiring

Use a common ground first. Without common ground the data lines can look random.

| S32K signal | S32K pin | Teensy 4.1 pin | Direction |
|---|---:|---:|---|
| SCK | PTB14 | 13 | S32K to Teensy |
| MOSI / S32K SOUT | PTB16 | 11 | S32K to Teensy |
| MISO / S32K SIN | PTB15 | 12 | Teensy to S32K |
| CS / PCS3 | PTB17 | 10 | S32K to Teensy |
| READY | PTD14 | 31 | Teensy to S32K |
| GND | GND | GND | common reference |

Both boards use 3.3 V logic. Do not connect a 5 V signal to the SPI pins.

Keep the first wiring short. If the packet fails CRC with long wires, reduce
the S32K SPI baud rate temporarily and keep the packet at 128 bytes.

## SPI Settings

| Setting | Value |
|---|---|
| Master | S32K144 |
| Slave | Teensy 4.1 |
| Mode | SPI mode 0, CPOL=0, CPHA=0 |
| Bit order | MSB first |
| Word size | 8 bits |
| Frame size | 128 bytes |
| S32K service period | 5 ms |
| Transfer period (READY high) | 10 ms (matches the 100 Hz Teensy sensor rate) |
| Probe period (READY stuck low) | 500 ms heartbeat |
| Target SCK | 2 MHz |
| Wire time at 2 MHz | about 512 us |

The S32K master gates transfers on the Teensy READY pin. While READY is high
it transfers every 10 ms. The Teensy lowers READY during its blocking SD and
display writes (tens of ms); the S32K skips those windows (counted on the
OLED as `SK:`) without touching the bus. Only when READY stays low for
500 ms (broken wire, unpowered/crashed Teensy) does the S32K send probe
transfers, so the link can never die permanently and the boards may be
powered in any order — the link comes back by itself on the first answered
probe. CRC and stale handling are unchanged.

By default the S32K moves the 128-byte frame with eDMA instead of busy-wait
polling (`TEENSY_LINK_USE_DMA` in `firmware/s32k144/include/drivers/teensy_link.h`),
so the CPU is free during the ~512 us wire time and the finished frame is
decoded on the next 5 ms service tick. Details and the new `TLINK DMA` OLED
page: `firmware/s32k144/docs/teensy-link-spi.md`.

The S32K generated SPI configuration is in `Nxp_Cup.mex` and generated RTD
files. The important generated names are:

- `TeensySpiFrameChannel`
- `TeensySpiJob`
- `TeensySpiSequence`
- `TeensySpiDevice`
- `TeensyReady`

## Packet Format

Every packet is exactly 128 bytes:

| Byte range | Meaning |
|---:|---|
| 0..15 | common header |
| 16..125 | payload |
| 126..127 | CRC-16/CCITT-FALSE over bytes 0..125 |

Common header:

| Offset | Field |
|---:|---|
| 0 | sync byte 0, `0xA5` |
| 1 | sync byte 1, `0x5A` |
| 2 | protocol version |
| 3 | frame type: S32K=`0x53`, Teensy=`0x54` |
| 4..5 | frame length, always 128 |
| 6..7 | frame sequence |
| 8..11 | sender time in ms |
| 12..13 | frame flags |
| 14..15 | payload length, always 110 |

The CRC is the final check. If header and CRC are valid, the receiver decodes
the payload. If not, it increments error counters and keeps the last good data.

## S32K To Teensy Values

The S32K test sends control and vehicle-state data to the Teensy:

| Value | Purpose |
|---|---|
| ACK of last Teensy sequence | Proves the S32K received a Teensy frame |
| control loop sequence | Lets Teensy detect dropped S32K frames |
| control dt in us | Tells Teensy the S32K control timing |
| app mode and app state | Tells Teensy which S32K mode is active |
| safety flags | Lets Teensy know if S32K is in a safe/fault state |
| camera 0 and camera 1 slots | Lets either board publish camera status |
| steer raw, filtered, output | Lets Teensy log steering behavior |
| target and current speed | Logs target and rate-limited speed commands; neither is measured vehicle speed |
| ESC primary and secondary logical command | Supports separate motor commands |
| servo command | Lets Teensy log steering actuator command |
| actuator flags | Reserved status for actuator state |
| ultrasonic distance, age, flags | Lets Teensy log obstacle detection |
| diag flags | Link and diagnostic flags from the S32K side |

In the current test-owned implementation, the S32K sends the active app/test id,
sequence, ACK, stale S32K camera slots, and zero actuator values. The reserved
fields stay available for future race-mode integration.

Camera slots use this compact wire format:

```text
error_pct_s8, status_u8, feature_u8, confidence_u8,
left_idx_u8, right_idx_u8, age_ms_u8, flags_u8
```

If a camera is missing or handled by the other board, send track lost,
confidence 0, indexes 255, old age, and stale/invalid flags.

## Teensy To S32K Values

The Teensy sends sensor, camera, logger, and link diagnostics back to the S32K:

| Value | Purpose |
|---|---|
| ACK of last S32K sequence | Proves the Teensy received an S32K frame |
| sensor sequence | Lets S32K detect fresh sensor updates |
| sensor dt in us | Tells S32K the Teensy sensor sample period |
| sensor age in ms | Lets S32K reject stale IMU/camera data |
| status flags | IMU present, calibrated, valid, yaw-relative, etc. |
| component mask | Tells S32K which components are physically present |
| camera 0 and camera 1 slots | Supports one or two cameras |
| IMU accel in mg | `ax`, `ay`, `az` |
| IMU gyro in 0.1 dps | `gx`, `gy`, `gz` |
| IMU angle in centidegrees | roll, pitch, yaw |
| accel norm and lateral acceleration | Used later for slip estimation |
| temperature in 0.1 C | IMU temperature |
| logger flags and drop count | SD logging status |
| Teensy RX frame/error counters | Shows if MOSI from S32K is valid |

The current Teensy bench sketch sends:

- Physical MPU6050 data when detected and calibrated.
- Camera 0 and camera 1 stale/missing because no Teensy camera drivers exist.
- Real SD logger status, drop count, and write state.
- RX counters from the S32K-to-Teensy direction.

## Missing Components

The link should not fail just because one component is missing.

Use these rules:

| Missing item | What to send |
|---|---|
| IMU missing | clear IMU bit in component mask, clear IMU valid flag |
| IMU uncalibrated | keep present bit, clear calibrated/valid flags |
| Camera 0 missing | camera 0 status lost, confidence 0, stale flag |
| Camera 1 missing | camera 1 status lost, confidence 0, stale flag |
| Camera on S32K instead of Teensy | set source flag to S32K in S32K payload |
| SD missing | logger ready flag cleared |
| Teensy disconnected | S32K OLED stays WAIT or STALE |
| SPI corrupt | CRC/protocol counters increase |

This is important for the plan where one camera may stay on the S32K and another
camera may move to the Teensy later.

## Build And Upload Teensy

In VS Code, open `firmware/teensy`.

Use the PlatformIO terminal:

```bat
cd /d C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
pio run
pio run -t upload
pio device monitor -b 115200
```

Expected serial output:

```text
Teensy S32K SPI slave v1 link
Packet: 128-byte full-duplex teensy_link frame, CRC-16/CCITT-FALSE
t=1234 txSeq=120 sensorSeq=120 s32k=118 rx=118 err=0 timeout=0 app=5 speed=0/0 servo=0 ultra=0
```

What the serial values mean:

| Field | Meaning |
|---|---|
| `txSeq` | Teensy frame sequence being sent to S32K |
| `sensorSeq` | physical MPU6050 sample sequence |
| `imu` | IMU state: present, calibrated, valid, acceleration trusted |
| `imuErr` | failed MPU6050 I2C burst-read count |
| `ax/ay/az/gz/yaw` | physical IMU measurements and relative yaw |
| `s32k` | last valid S32K frame sequence decoded by Teensy |
| `rx` | S32K frames fully received by Teensy |
| `err` | S32K frames rejected by Teensy protocol/CRC validation |
| `timeout` | CS went active but the full 128-byte clock did not complete |
| `app` | S32K app mode from the received frame |
| `speed` | S32K target/current command values; not measured vehicle speed |
| `servo` | S32K servo command |
| `ultra` | S32K ultrasonic distance in 0.1 cm units |

## Build And Upload S32K

In S32 Design Studio, open/import:

```text
C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\s32k144
```

For the normal bench menu, keep:

```c
#define APP_TEST_NXP_CUP_TESTS            1
```

For direct boot into only the SPI test, use:

```c
#define APP_TEST_TEENSY_LINK_TEST         1
```

Only one `APP_TEST_*` flag may be `1`.

Build and flash the S32K as usual from S32 Design Studio.

## Run The Two-Board Test

1. Wire S32K and Teensy SPI plus common ground.
2. Upload the Teensy firmware.
3. Open the Teensy serial monitor.
4. Upload the S32K firmware.
5. If `APP_TEST_TEENSY_LINK_TEST` is enabled, press Resume / run the S32K
   firmware and it will boot directly into the link test.
6. If `APP_TEST_NXP_CUP_TESTS` is enabled, select `Teensy Link` on the S32K OLED
   menu and turn the mode switch on.
7. Confirm Teensy serial `s32k` becomes nonzero and `rx` increases.
8. Press SW2 to cycle the display pages.

The important check is step 7. If `s32k=0 rx=0` while the link test is running,
the S32K is not clocking SPI or the SCK/CS/MOSI wiring is wrong.

The S32K OLED pages are:

| Page | OLED title | What to check |
|---:|---|---|
| 0 | `TLINK` | status becomes OK, sequence values change, `SK:` stays near 0 with a connected Teensy |
| 5 | `TLINK DMA` | transfer engine (DMA/BLOCKING), starts climbing, timeouts stay 0 |
| 1 | `TLINK CAM0` | stale/missing camera 0 appears without link failure |
| 2 | `TLINK CAM1` | stale/missing camera 1 appears without link failure |
| 3 | `TLINK IMU/LOG` | yaw, gyro Z, lateral acceleration, logger state |
| 4 | `TLINK RX 128B` | good frame count increases, CRC/SPI errors stay low |

RGB LED meaning:

| LED | Meaning |
|---|---|
| Blue | no valid frame yet |
| Green | live valid link |
| Yellow | stale frame exists but not live |
| Red | repeated CRC/protocol/SPI errors |

## Specific Tests To Run

### 1. Teensy-only serial and physical IMU test

Run only the Teensy and open the serial monitor.

Expected:

- `txSeq` and `sensorSeq` increase.
- `imu=PCVA` appears after stationary startup calibration.
- Moving the IMU changes `ax/ay/az/gz/yaw`.
- `rx` stays 0 because the S32K is not clocking SPI.
- `err` stays 0.

### 2. S32K-only disconnected test

Run the S32K `Teensy Link` test without the Teensy connected.

Expected:

- OLED status stays `WAIT`.
- LED stays blue or moves to stale if an old frame existed.
- This proves the missing Teensy path does not crash the S32K.

### 3. Full link test

Run both boards.

Expected:

- Teensy serial `rx` increases.
- Teensy serial `s32k` becomes nonzero.
- S32K `TLINK` changes to `OK`.
- `S32`/`TEN` sequence values change.
- S32K `TLINK RX 128B` good count increases.

### 4. MOSI direction test

Check the Teensy serial monitor while the S32K link test is running.

Expected:

- `rx` increases.
- `err` does not increase quickly.
- `app`, `speed`, `servo`, or `ultra` fields change when the S32K state changes.

If the S32K OLED receives Teensy data but Teensy `rx` stays 0, MISO may be
correct but MOSI/CS/SCK to the Teensy is wrong.

### 5. MISO direction test

Check the S32K OLED.

Expected:

- `TLINK` status is `OK`.
- Teensy sequence changes.
- IMU page shows physical yaw/gyro/lateral values changing with motion.

If Teensy `rx` increases but S32K stays `WAIT`, MOSI into Teensy may be correct
but MISO back to S32K is wrong.

### 6. Missing camera test

Open the `TLINK CAM0` and `TLINK CAM1` pages.

Expected:

- Both Teensy camera slots show lost/stale style data.
- Link status is still OK.

This proves missing cameras are data states, not transport failures.

### 7. Disconnect/stale test

While the link is OK, disconnect CS or SCK.

Expected:

- S32K stops receiving fresh frames.
- OLED changes from OK to stale/wait behavior after the stale timeout.
- Teensy timeout may increase if CS is active without a full frame.

Reconnect and confirm the link recovers.

### 8. CRC/noise test

If errors increase:

- shorten wires,
- check common ground,
- check MISO/MOSI direction,
- check SPI mode 0,
- temporarily lower S32K `TeensySpiDevice` baud rate and regenerate.

Do not change the 128-byte packet to debug wiring. Keep the packet fixed.

## Ultrasonic And Victory Test Context

The ultrasonic and victory-lap bench changes are documented here:

```text
firmware/s32k144/docs/ultrasonic-victory-tests.md
```

Summary:

- `Ultrasonic` tests the distance sensor alone.
- `Ultra+ESC` drives both ESC outputs at 50 percent when clear.
- At 45 cm it locks the servo straight, turns yellow, and drops to the low
  obstacle speed.
- At 8 cm it turns red, locks steering straight, and stops both ESC outputs.
- `Victory Lap` is a separate test that approaches a pole, stops at 8 cm, and
  runs a Mario-style victory note sequence.

These tests are not dependent on each other. They only share common drivers such
as OLED display, ESC, servo, and ultrasonic.

The current ESC API is RC-style throttle PWM, so it cannot directly play real
BLDC phase music notes yet. The note table is prepared for a future motor-tone
driver, while the current test remains safe.

## Notes For The Report

Use this wording if you need a short design explanation:

The S32K is kept as the real-time vehicle controller because it already owns
the motor, servo, display, buttons, and safety modes. The Teensy is used as the
sensor and logging coprocessor because it has more spare CPU for IMU filtering,
camera-side processing, SD logging, and later MATLAB/ESP telemetry support. A
fixed 128-byte SPI packet is used because it gives deterministic timing,
constant buffer sizes, simple CRC validation, and enough reserved space for the
second camera, SD status, and future slip-control values.
