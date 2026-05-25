# Teensy IMU SPI Display Test In S32DS

This project now keeps the Teensy IMU integration inside the S32K source layout:

- `include/drivers/teensy_imu.h`
- `src/drivers/teensy_imu.c`
- `include/services/imu_motion.h`
- `src/services/imu_motion.c`
- `src/app/modes/bench_teensy_imu.c`

## Why It Is Split This Way

- `drivers`: receives and decodes the 45-byte Teensy packet.
- `services`: converts packet values into engineering units and future slip math.
- `app`: shows the IMU test on the S32K display.

The MPU6050/GY-521 is still connected to the Teensy, not directly to the S32K. The S32K receives filtered IMU values from the Teensy.

## Packet Source

The driver is ready for SPI bytes, but this repo does not yet have generated S32K LPSPI slave code. When that is added, the SPI receive callback should call:

```c
TeensyImu_SubmitRxBytes(rxBytes, TEENSY_IMU_PACKET_BYTES, Timebase_GetMs());
```

The test is available in two ways:

- Runtime test menu: keep `APP_TEST_NXP_CUP_TESTS = 1` and select `Teensy IMU`.
- Direct build mode: set `APP_TEST_TEENSY_IMU_TEST = 1` and all other `APP_TEST_*` flags to `0`.

Full packet and two-board test documentation is in `docs/teensy-imu-spi.md`.

## Display Test

Run `APP_TEST_NXP_CUP_TESTS = 1`, enter the test menu, select `Teensy IMU`, then turn the mode switch on.

While SPI is not connected:

```text
TEENSY IMU
SPI: WAIT
ERR:0000
SW2 DEMO:OFF
```

Press SW2 to toggle demo data. This proves the parser and display screen work before the real SPI callback exists.

With real packets:

```text
IMU OK  SQ:1234
R10:  12 P10: -21
Y10: 450 GZ:   7
C1:S32 C2:NC  S:Y
```

`R10`, `P10`, and `Y10` are degrees x10 so the small display can show decimals without printing floats.

## Data Meaning

- `axMg`, `ayMg`, `azMg`: acceleration in milli-g.
- `gxDps10`, `gyDps10`, `gzDps10`: gyro in degrees/second x10.
- `rollCdeg`, `pitchCdeg`, `yawCdeg`: angle in degrees x100.
- `lateralMg`: current lateral estimate, from Teensy Y acceleration.
- `camera1Status`: currently `S32`, because camera 1 stays on S32K.
- `camera2Status`: currently `NC`, because camera 2 is not connected.
- `SD_READY`: tells the display whether Teensy SD logging is active.

## Drift And Filter Notes

The Teensy should run the IMU filter because it owns the IMU sampling timing. The S32K should consume the filtered packet and use the values for display, logging, or control decisions.

Roll and pitch are Kalman-filtered on Teensy. Yaw is relative because MPU6050 has no magnetometer. For control, yaw rate (`gz`) and lateral acceleration are more useful than absolute yaw.

Future slip check:

```text
expected_lateral_accel = speed_mps * yaw_rate_rad_s
slip_indicator = measured_lateral_accel - expected_lateral_accel
```
