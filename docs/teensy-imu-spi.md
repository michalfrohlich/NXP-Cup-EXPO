# Teensy IMU SPI Packet And Two-Board Test Guide

This file documents the S32K side of the Teensy IMU test.

The word "bridge" means the Teensy is between the IMU and the S32K:

```text
MPU6050/GY-521 -> Teensy 4.1 -> SPI packet -> S32K -> display/control
```

The S32K does not read the MPU6050 directly. The Teensy reads and filters the IMU, then sends a fixed packet to the S32K.

## Where The Test Is In Code

There are two ways to run the same S32K display test.

### Option 1: From The NXP Cup Test Menu

Keep this in `src/app/car_config.h`:

```c
#define APP_TEST_NXP_CUP_TESTS            1
#define APP_TEST_TEENSY_IMU_TEST          0
```

Then select `Teensy IMU` from the display test menu.

The menu link is here:

- `src/app/app_common.c`: adds `"Teensy IMU"` to `g_testsMenuItems`.
- `src/app/bench_menu.c`: calls `teensy_imu_test_enter/update/exit`.

### Option 2: Direct Build Mode

Set this in `src/app/car_config.h`:

```c
#define APP_TEST_NXP_CUP_TESTS            0
#define APP_TEST_TEENSY_IMU_TEST          1
```

Then `src/app/app_modes.c` directly runs:

```c
mode_teensy_imu_test();
```

This is useful when you only want to test the IMU display path and do not want to use the runtime test menu.

## Files That Matter

| File | What it does |
|---|---|
| `include/drivers/teensy_imu.h` | Defines the SPI packet, status flags, camera status values, and public functions. |
| `src/drivers/teensy_imu.c` | Validates/decode bytes from SPI and stores the latest good packet. |
| `include/services/imu_motion.h` | Defines converted IMU values in normal units. |
| `src/services/imu_motion.c` | Converts fixed-point packet values to `g`, `deg/s`, degrees, temperature, and slip estimate. |
| `src/app/bench_teensy_imu.c` | S32K display test screen and direct test mode loop. |
| `src/app/app_modes.c` | Build-mode dispatcher. Shows the direct IMU test mode. |
| `src/app/bench_menu.c` | Runtime test dispatcher. Shows the IMU test when using NXP Cup tests. |
| `src/app/car_config.h` | Build flag and display timing config. |

## Teensy To S32K Packet

The Teensy sends exactly 45 bytes.

SPI settings expected by this S32K decoder:

| Setting | Value |
|---|---:|
| Packet bytes | 45 |
| Sync bytes | `0xA5 0x5A` |
| Version | `1` |
| Packet type | `0x49` |
| Endianness | Little endian for multi-byte fields |
| Checksum | XOR of bytes 2 through 43 |

The S32K receive callback must pass the raw received bytes here:

```c
TeensyImu_SubmitRxBytes(rxBytes, TEENSY_IMU_PACKET_BYTES, Timebase_GetMs());
```

This branch does not configure LPSPI hardware yet. That still needs to be generated in S32 Design Studio. This branch provides the parser, data storage, display test, and service conversion.

## Packet Fields

| Byte offset | Field | Type | Unit / meaning |
|---:|---|---|---|
| 0 | `sync0` | `uint8` | `0xA5` |
| 1 | `sync1` | `uint8` | `0x5A` |
| 2 | `version` | `uint8` | `1` |
| 3 | `type` | `uint8` | `0x49` |
| 4 | `packetBytes` | `uint16` | `45` |
| 6 | `sequence` | `uint16` | Increments each Teensy packet |
| 8 | `timeMs` | `uint32` | Teensy time in ms |
| 12 | `statusFlags` | `uint16` | IMU/SD/data health flags |
| 14 | `componentMask` | `uint16` | Which parts exist on Teensy |
| 16 | `camera1Status` | `uint8` | Camera 1 status |
| 17 | `camera2Status` | `uint8` | Camera 2 status |
| 18 | `sampleDtUs` | `uint16` | Teensy IMU filter dt |
| 20 | `axMg` | `sint16` | X accel in milli-g |
| 22 | `ayMg` | `sint16` | Y accel in milli-g |
| 24 | `azMg` | `sint16` | Z accel in milli-g |
| 26 | `gxDps10` | `sint16` | X gyro in deg/s x10 |
| 28 | `gyDps10` | `sint16` | Y gyro in deg/s x10 |
| 30 | `gzDps10` | `sint16` | Z gyro in deg/s x10 |
| 32 | `rollCdeg` | `sint16` | Roll in degrees x100 |
| 34 | `pitchCdeg` | `sint16` | Pitch in degrees x100 |
| 36 | `yawCdeg` | `sint16` | Relative yaw in degrees x100 |
| 38 | `accelNormMg` | `sint16` | Total accel magnitude in milli-g |
| 40 | `lateralMg` | `sint16` | Lateral acceleration estimate in milli-g |
| 42 | `tempC10` | `sint16` | IMU temperature in deg C x10 |
| 44 | `checksum` | `uint8` | XOR checksum |

## Values Shown On The S32K Display

The current display test shows:

```text
IMU OK  SQ:1234
R10:  12 P10: -21
Y10: 450 GZ:   7
C1:S32 C2:NC  S:Y
```

Meaning:

| Display | Meaning |
|---|---|
| `SQ` | Packet sequence number |
| `R10` | Roll in degrees x10 |
| `P10` | Pitch in degrees x10 |
| `Y10` | Relative yaw in degrees x10 |
| `GZ` | Yaw rate in deg/s |
| `C1` | Camera 1 status |
| `C2` | Camera 2 status |
| `S` | Teensy SD logging status |

Why `R10/P10/Y10` instead of float values:

- The display helper prints integers.
- Degrees x10 gives one decimal digit without float formatting.

## Status Flags

`statusFlags` from the Teensy:

| Bit | Name | Meaning |
|---:|---|---|
| 0 | `TEENSY_IMU_STATUS_PRESENT` | IMU exists and answered on Teensy |
| 1 | `TEENSY_IMU_STATUS_CALIBRATED` | Teensy gyro calibration finished |
| 2 | `TEENSY_IMU_STATUS_DATA_VALID` | Current IMU data is valid |
| 3 | `TEENSY_IMU_STATUS_ACCEL_TRUSTED` | Accel magnitude is near 1 g |
| 4 | `TEENSY_IMU_STATUS_SD_READY` | Teensy SD logging is active |
| 5 | `TEENSY_IMU_STATUS_YAW_RELATIVE` | Yaw is relative, not compass yaw |

If the IMU is missing, the Teensy should clear `PRESENT` and `DATA_VALID`.

If the SD card is missing, the Teensy should clear `SD_READY`, but still send packets.

## Camera Status

Camera status values:

| Value | Display | Meaning |
|---:|---|---|
| 0 | `NC` | Not connected |
| 1 | `S32` | Camera is handled by S32K |
| 2 | `TEN` | Camera is handled by Teensy |
| 3 | `ERR` | Camera exists but has a fault |

Current intended setup:

```text
camera1Status = S32
camera2Status = NC
```

That means:

- Camera 1 still runs on the S32K.
- Camera 2 is not connected yet.
- The Teensy packet prepares for a future camera split, but this packet version only sends camera status, not camera pixels.

If camera 2 later moves to the Teensy, use `camera2Status = TEN` and add camera data in a future packet version.

## What S32K Receives

The S32K receives from Teensy:

- IMU acceleration
- IMU gyro
- roll/pitch/yaw from Teensy filtering
- yaw rate
- lateral acceleration estimate
- IMU temperature
- packet sequence
- Teensy timestamp
- camera 1 status
- camera 2 status
- SD logging status
- IMU/data/failure flags

## What S32K Sends Back

This packet version is one-way:

```text
Teensy -> S32K
```

The S32K does not send anything back to the Teensy yet.

If we need two-way communication later, possible S32K -> Teensy values are:

- start/stop logging
- reset yaw
- request calibration
- current left motor command
- current right motor command
- current servo command
- S32K camera status/fault

Those should be a new command packet, not added silently to this packet.

## Failure Handling

The S32K rejects a packet when:

- packet length is not 45 bytes,
- sync bytes are not `0xA5 0x5A`,
- checksum is wrong,
- version is not `1`,
- type is not `0x49`,
- packetBytes field is not `45`.

The S32K display shows wait/stale state when no valid packet arrives for:

```c
TEENSY_IMU_PACKET_STALE_MS
```

Current value:

```c
200 ms
```

The display wait screen:

```text
TEENSY IMU
SPI: WAIT
ERR:0000
SW2 DEMO:OFF
```

`ERR` increments when invalid bytes are submitted to the decoder.

## Test 1: S32K Display Test Without Teensy

Purpose:

Proves the S32K screen and parser display path compile and run before SPI hardware is connected.

Steps:

1. In `src/app/car_config.h`, set:

```c
#define APP_TEST_NXP_CUP_TESTS            0
#define APP_TEST_TEENSY_IMU_TEST          1
```

2. Build and flash from S32 Design Studio.
3. The display should show:

```text
TEENSY IMU
SPI: WAIT
ERR:0000
SW2 DEMO:OFF
```

4. Press SW2.
5. Demo mode turns on.
6. The display should show changing sequence/yaw values.

Expected pass:

- Display updates.
- SW2 toggles demo mode.
- Sequence changes.
- Camera line shows `C1:S32 C2:NC`.

## Test 2: S32K Runtime Menu Test Without Teensy

Purpose:

Proves the IMU test is wired into the normal NXP Cup runtime test menu.

Steps:

1. In `src/app/car_config.h`, set:

```c
#define APP_TEST_NXP_CUP_TESTS            1
#define APP_TEST_TEENSY_IMU_TEST          0
```

2. Build and flash.
3. Use the pot to select `Teensy IMU`.
4. Turn the mode switch on.
5. The display should enter the same IMU screen.
6. Press SW2 to enable demo data.

Expected pass:

- `Teensy IMU` appears in the menu.
- Enter/update/exit works through `bench_menu.c`.
- Turning the mode switch off returns to the menu.

## Test 3: Teensy Alone

Purpose:

Proves the Teensy is reading and filtering IMU data before connecting to S32K.

Steps:

1. Connect MPU6050/GY-521 to Teensy I2C.
2. Keep IMU still during startup.
3. Open Teensy serial monitor.
4. Confirm:

```text
MPU6050 detected
Calibration: OK
```

5. Move the IMU.
6. Check serial values:

- roll changes on left/right tilt,
- pitch changes on forward/back tilt,
- `gz` changes when rotating,
- yaw changes while rotating,
- `accel_trusted` may drop during fast shaking.

Expected pass:

- IMU data changes correctly.
- No repeated IMU read errors.
- SD status is correct if SD is installed.

## Test 4: Full SPI Link With Both Boards

Purpose:

Proves Teensy -> S32K packet transfer works.

Wiring:

```text
Teensy 4.1      S32K SPI slave
--------------------------------
CS  pin 10      PCS / CS input
MOSI pin 11     MOSI / SIN input
MISO pin 12     MISO / SOUT output
SCK pin 13      SCK input
GND             GND
```

Steps:

1. Flash Teensy IMU sender firmware.
2. Flash S32K with either direct `APP_TEST_TEENSY_IMU_TEST` or menu `APP_TEST_NXP_CUP_TESTS`.
3. Confirm the S32K SPI receive callback calls:

```c
TeensyImu_SubmitRxBytes(rxBytes, TEENSY_IMU_PACKET_BYTES, Timebase_GetMs());
```

4. Start Teensy.
5. Start S32K.
6. Watch the S32K display.

Expected pass:

- Display changes from `SPI: WAIT` to `IMU OK`.
- Sequence increments.
- Roll/pitch/yaw react when IMU moves.
- Camera status shows `C1:S32 C2:NC`.
- SD status shows `Y` when Teensy SD logging is ready, otherwise `N`.

## Test 5: Failure Tests

Unplug Teensy SPI:

- S32K should show `SPI: WAIT` after about 200 ms.

Power Teensy without IMU:

- S32K should still receive packets if Teensy firmware sends failure packets.
- `PRESENT` and `DATA_VALID` should be clear.

Remove Teensy SD card:

- S32K should show `S:N`.
- IMU data should still work.

Send bad packet/checksum:

- `TeensyImu_SubmitRxBytes()` should return `FALSE`.
- Error count should increase.

Set camera 2 missing:

- Display should show `C2:NC`.

Set camera 2 on Teensy in future:

- Display should show `C2:TEN`.

## Test 6: MATLAB/Control Preparation

Purpose:

Checks whether the values are useful for future speed/curve/slip work.

Useful packet values:

- `gzDps10`: yaw rate
- `lateralMg`: lateral acceleration
- `rollCdeg`, `pitchCdeg`: sanity checks
- `sequence`: dropped packet check
- `statusFlags`: filter invalid data

Future slip estimate:

```text
expected_lateral_accel = speed_mps * yaw_rate_rad_s
slip_indicator = measured_lateral_accel - expected_lateral_accel
```

The helper is already in:

```c
ImuMotion_EstimateSlipG()
```

## Notes For Partner Adding SPI

The missing piece is generated S32K LPSPI slave receive code.

The IMU parser does not care whether bytes come from interrupt, DMA, or polling. It only needs exactly 45 bytes in packet order.

Minimum required callback behavior:

```c
void SomeSpiRxCallback(void)
{
    uint8 rxBytes[TEENSY_IMU_PACKET_BYTES];

    /* Fill rxBytes from LPSPI driver first. */
    (void)TeensyImu_SubmitRxBytes(rxBytes,
                                  TEENSY_IMU_PACKET_BYTES,
                                  Timebase_GetMs());
}
```

Do not decode packet fields inside the SPI callback. Keep the callback short and pass the bytes to `TeensyImu_SubmitRxBytes()`.
