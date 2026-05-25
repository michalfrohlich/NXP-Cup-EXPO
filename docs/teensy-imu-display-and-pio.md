# Teensy IMU Display, PlatformIO, And Python Notes

This file explains which part runs where. The S32K project does not run Python and does not use PlatformIO. PlatformIO is only for the Teensy firmware if we build the Teensy sender separately.

## What Runs On The S32K

The S32K side is built and flashed from S32 Design Studio.

The S32K code added in this repo does three things:

1. Receives a 45-byte packet from the Teensy over SPI.
2. Decodes and validates that packet.
3. Shows the latest IMU values on the display connected to the S32K.

Main S32K files:

- `include/drivers/teensy_imu.h`
- `src/drivers/teensy_imu.c`
- `src/app/bench_teensy_imu.c`
- `docs/teensy-imu-spi.md`

## What Runs On The Teensy

The Teensy reads the IMU. The S32K does not directly read the MPU6050.

Expected Teensy work:

1. Read MPU6050/GY-521 over I2C.
2. Run calibration and filtering.
3. Build the 45-byte packet.
4. Send that packet to the S32K over SPI.
5. Optionally save CSV data to the Teensy SD card.

That Teensy firmware can be built with PlatformIO because Teensy 4.1 support is easiest there.

PlatformIO is not used for the S32K build.

## Is There Any Python?

No Python is needed for the S32K display test.

Python is only optional for PC-side plotting or checking Teensy serial CSV data. It is not part of the S32K firmware.

Use Python only when you want a laptop visualization like:

```text
Teensy USB serial -> Python plotter -> PC window
```

The real car display path is:

```text
Teensy SPI packet -> S32K -> DisplayText/DisplayValue -> S32K display
```

## How The S32K Display Shows The Data

The display screen is drawn in:

```text
src/app/bench_teensy_imu.c
```

It uses the existing display driver:

```c
DisplayTextPadded()
DisplayText()
DisplayValue()
DisplayRefresh()
```

The test does not create graphics or run a fancy UI. It prints four display rows.

With no packet:

```text
TEENSY IMU
SPI: WAIT
ERR:0000
SW2 DEMO:OFF
```

With valid packet or demo packet:

```text
IMU OK  SQ:1234
R10:  12 P10: -21
Y10: 450 GZ:   7
C1:S32 C2:NC  S:Y
```

Meaning:

- `SQ`: packet sequence number.
- `R10`: roll in degrees x10.
- `P10`: pitch in degrees x10.
- `Y10`: yaw in degrees x10.
- `GZ`: yaw rate in degrees/second.
- `C1`: camera 1 status.
- `C2`: camera 2 status.
- `S`: Teensy SD logging ready, `Y` or `N`.

## Why R10/P10/Y10

The display helper prints integers. Instead of using float text, the test prints degrees x10.

Example:

```text
R10: 123
```

Means:

```text
roll = 12.3 degrees
```

The packet itself stores angles as degrees x100, but the display divides by 10 to show degrees x10.

## How To Run S32K Display Test Directly

Use direct mode when you only want this IMU screen.

In `src/app/car_config.h`:

```c
#define APP_TEST_LINEAR_CAMERA_TEST       0
#define APP_TEST_NXP_CUP                  0
#define APP_TEST_RACE_MODE                0
#define APP_TEST_HONOR_LAP                0
#define APP_TEST_SERVO_RATE_TEST          0
#define APP_TEST_TEENSY_IMU_TEST          1
#define APP_TEST_NXP_CUP_TESTS            0
```

Then build and flash in S32 Design Studio.

Press `SW2` to toggle demo packets. Demo mode proves the display and parser work before SPI hardware is connected.

## How To Run From NXP Cup Tests Menu

Use menu mode when you want it with the other tests.

In `src/app/car_config.h`:

```c
#define APP_TEST_TEENSY_IMU_TEST          0
#define APP_TEST_NXP_CUP_TESTS            1
```

Then:

1. Build and flash in S32 Design Studio.
2. Use the pot to select `Teensy IMU`.
3. Turn the mode switch on.
4. Press `SW2` to enable demo data if SPI is not connected yet.

## How Real SPI Connects Later

The S32K LPSPI slave hardware still needs to be configured/generated in S32 Design Studio.

When the S32K receives exactly 45 bytes, the SPI callback should call:

```c
TeensyImu_SubmitRxBytes(rxBytes, TEENSY_IMU_PACKET_BYTES, Timebase_GetMs());
```

Do not decode the packet in the SPI callback. Keep the callback short and pass the bytes to the driver.

The decoder then:

1. Checks packet length.
2. Checks sync bytes.
3. Checks checksum.
4. Checks version/type.
5. Stores the latest good packet.
6. Increments error count for bad packets.

## When PlatformIO Is Needed

Use PlatformIO only for the Teensy sender firmware.

Example Teensy flow:

```powershell
cd "path\to\Teensy+IMU,Test"
pio run -e teensy41
pio run -e teensy41 -t upload
```

That is separate from S32 Design Studio.

## When Python Is Useful

Python is optional and only for laptop-side visualization or quick serial checks.

Example:

```text
Teensy serial CSV -> Python -> live plot
```

Python is not needed for:

- S32K build.
- S32K flashing.
- S32K display test.
- SPI packet decoding on S32K.

## What Needs To Be Tested With Both Boards

1. Teensy alone:
   - IMU detected.
   - Calibration OK.
   - Serial values move when IMU moves.
   - SD status works if card is inserted.

2. S32K alone:
   - Direct `APP_TEST_TEENSY_IMU_TEST` screen boots.
   - SW2 demo mode changes values.
   - Display shows `C1:S32 C2:NC`.

3. Both boards:
   - Shared ground.
   - Teensy SPI pins wired to S32K SPI slave.
   - S32K SPI callback submits 45 bytes.
   - Display changes from `SPI: WAIT` to `IMU OK`.
   - Sequence increments.
   - Moving the IMU changes roll/pitch/yaw/GZ.

## Summary

- S32K: S32 Design Studio only.
- Teensy: PlatformIO if we build Teensy firmware.
- Python: optional PC visualizer only.
- Display: existing S32K display driver, four text rows.
- Fancy UI: not needed for the car; the useful test is stable packet values and clear fail states.
