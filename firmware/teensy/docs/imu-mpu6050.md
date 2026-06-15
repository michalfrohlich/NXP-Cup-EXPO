# Teensy MPU6050 / GY-521 IMU

## Purpose

The Teensy reads the physical MPU6050 at 100 Hz, filters its orientation, logs
the result to SD, and sends the existing IMU fields in the 128-byte SPI frame.
The packet format did not change.

Driver files:

```text
firmware/teensy/include/drivers/mpu6050_imu.h
firmware/teensy/src/drivers/mpu6050_imu.cpp
```

The normal acquisition/service call is in `firmware/teensy/src/main.cpp`.

## Connections

| GY-521 / MPU6050 | Teensy 4.1 | Notes |
|---|---:|---|
| VCC | 3.3 V | Use 3.3 V logic/power on the shield |
| GND | GND | Common ground |
| SDA | 18 | Teensy `Wire` SDA |
| SCL | 19 | Teensy `Wire` SCL |
| INT | 30 | Data-ready configured; sampling currently uses the 100 Hz scheduler |
| AD0 | GND or open | Address `0x68`; AD0 high selects `0x69` |

The driver probes both `0x68` and `0x69`. The two Teensy displays use `Wire1`
on pins 17/16, so they do not share the IMU bus.

## Sensor Configuration

| Setting | Value |
|---|---|
| I2C speed | 400 kHz |
| Sample rate | 100 Hz |
| Digital low-pass filter | MPU6050 DLPF setting 3 |
| Accelerometer range | +/-4 g |
| Gyroscope range | +/-500 degrees/s |
| Clock source | X-axis gyro PLL |
| Data-ready interrupt | Enabled, pin 30 available |

Every sample is one 14-byte burst read containing accelerometer, temperature,
and gyroscope data.

## Startup Calibration

At every Teensy boot:

1. The driver probes and configures the MPU6050.
2. It collects 200 gyro samples over about two seconds.
3. It averages the three gyro axes and uses them as zero-rate biases.
4. If an I2C read fails or the car moves faster than the calibration limit,
   calibration fails and the IMU is not marked valid.

Keep the car completely still during startup. If serial reports
`calibration failed`, hold it still and reset the Teensy.

Accelerometer offsets are not automatically removed. This avoids silently
assuming the car is perfectly level during boot.

## Orientation Filter

- Roll and pitch start from gravity-based accelerometer angles.
- Gyroscope rates are integrated every sample.
- A complementary filter blends roll/pitch back toward gravity when
  acceleration magnitude is between `0.80 g` and `1.20 g`.
- Yaw is only integrated gyro Z. The MPU6050 has no magnetometer, so yaw is
  relative and slowly drifts.
- `lateralG` currently uses the IMU Y axis.

Mounting assumption:

| IMU axis | Vehicle direction |
|---|---|
| X | forward |
| Y | left/right lateral |
| Z | up |

If the board is mounted differently, change the axis mapping before using
`lateralG` for slip control.

## Honest Status Reporting

The Teensy no longer sends fixed IMU values or always marks the IMU valid.

| Status | Meaning |
|---|---|
| `P` / `IMU_PRESENT` | WHO_AM_I responded and configuration succeeded |
| `C` / `IMU_CALIBRATED` | startup gyro calibration completed |
| `V` / `IMU_VALID` | a fresh calibrated sample exists |
| `A` / `ACCEL_TRUSTED` | acceleration magnitude is suitable for gravity correction |
| `YAW_RELATIVE` | yaw has no absolute magnetic reference |

If the sensor is missing, the component bit and validity flags remain clear.
If a read fails, the last values are not marked fresh and sensor age becomes
stale.

## Run The Physical IMU Test

```bat
cd /d C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
pio run
pio run -t upload
pio device list
pio device monitor -p COM3 -b 115200
```

Replace `COM3` with the Teensy port. Keep the car still during the first two
seconds.

Expected startup:

```text
Keep the car still while the MPU6050 gyro calibrates.
MPU6050 detected at 0x68, calibrated
```

Expected repeating fields:

```text
imu=PCVA imuErr=0 ax=... ay=... az=... gz=... yaw=...
```

Checks:

1. Stationary and level: `az` should be close to `+1 g`; `ax` and `ay` near 0.
2. Tilt the car: roll/pitch values in the SD CSV and S32K packet should change.
3. Rotate the car flat: `gz` and relative yaw should change.
4. Leave it still: gyro values should settle near zero after calibration.
5. Disconnect SDA/SCL: `V` clears, age becomes stale, and `imuErr` increases.
6. Remove the IMU and reboot: serial reports not detected and `imu=----`.

With the S32K Teensy-link test running, open `TLINK IMU/LOG`. Yaw, gyro Z,
lateral acceleration, and age should follow the real Teensy sample. The Teensy
dashboard also shows `IMU:PCV`, yaw, and gyro Z.

The SD CSV already contains all physical IMU fields. Confirm the newest
`LOGnnn.CSV` changes when the car is moved before using the data in MATLAB.

## Current Limits

- Physical behavior still requires testing on the actual mounted sensor.
- Yaw drift is unavoidable without a magnetometer or external correction.
- The data-ready interrupt is configured but not required; the main loop polls
  at 100 Hz.
- Startup calibration is volatile and runs again after every reset.
- Both Teensy camera slots report missing/stale until camera drivers exist.
