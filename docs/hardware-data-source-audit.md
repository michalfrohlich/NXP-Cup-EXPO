# Hardware Data Source Audit

This audit identifies which integrated values come from physical hardware,
which are derived commands/estimates, and which are unavailable. The firmware
must never mark unavailable hardware data as valid.

## Teensy Sources

| Value | Source | Classification | Missing/failure behavior |
|---|---|---|---|
| MPU6050 accel, gyro, temperature | Physical I2C burst reads on pins 18/19 | Physical measurement | IMU valid clears; age becomes stale |
| Roll and pitch | Complementary filter using physical accel/gyro | Derived estimate | Invalid when no fresh calibrated IMU sample |
| Yaw | Integrated physical gyro Z | Relative estimate | Marked yaw-relative; drifts without magnetometer |
| Lateral acceleration | Physical MPU6050 Y axis | Physical measurement with mounting assumption | Invalid with IMU |
| Potentiometer | Teensy ADC pin 27 | Physical measurement | Raw ADC value only |
| Buttons | Teensy GPIO pins 28/29 | Physical input | Pull-up reads unpressed when open |
| Secondary display presence | I2C address probe on `Wire1` | Physical boot-time probe | Reports `NOT_CONNECTED` when no response at boot |
| SD status and bytes | SdFat built-in SDIO operations | Physical storage state | Ready clears or error sets |
| SPI RX/error/timeout counters | Actual Teensy slave transfers | Physical communication diagnostics | Counters remain zero without S32K clocks |
| Camera 0 and camera 1 | No Teensy acquisition driver yet | Unavailable | Both report lost/stale, confidence 0, indexes 255, no component bits |

## S32K Sources

| Value | Source | Classification | Important meaning |
|---|---|---|---|
| Linear camera frame | Physical S32K ADC/PWM/GPT capture | Physical measurement | Used by S32K camera tests/race modes |
| Missing lane boundary | Detected boundary plus nominal lane width | Derived inference | Confidence is reduced; it is not a second measured edge |
| Ultrasonic distance | Physical trigger plus ICU echo timestamps | Physical measurement | Invalid/error when timeout or outside configured range |
| Receiver channels | Physical ICU/GPT PPM decode | Physical input | Unsynced returns safe zero |
| Potentiometer/buttons | Physical ADC/GPIO | Physical input | Used by tests and menus |
| `targetSpeedPct` | Controller request | Command | Not measured speed |
| `currentSpeedPct` | Rate-limited ESC command | Command | Not measured vehicle speed; no wheel encoder feedback exists |
| ESC primary/secondary | Commands sent to ESC outputs | Command | Not motor RPM or measured torque |
| Servo command | Command sent to servo output | Command | Not measured steering angle |
| Steering raw/filtered/output | Camera/controller calculations | Derived control values | Not separate physical sensors |

## Link-Test Behavior

The S32K Teensy-link bench mode intentionally does not run the race actuators,
ultrasonic service, or S32K camera acquisition. Its outgoing actuator,
ultrasonic, and camera fields therefore use safe unavailable/zero states. Those
zeroes are not measurements.

The Teensy now reports both Teensy camera slots unavailable. The S32K OLED
`TLINK CAM0` and `TLINK CAM1` pages should show lost/stale until real Teensy
camera drivers are added.

Bench modes may intentionally generate actuator commands, LED patterns, or
servo sweeps to test outputs. Those are explicit test commands, not fabricated
sensor measurements.

## MATLAB Validity Rules

Use these CSV columns before analyzing measurements:

- `teensy_status_flags`: check IMU present/calibrated/valid bits.
- `teensy_component_mask`: check which Teensy components physically exist.
- `s32k_valid`: check whether S32K packet columns came from a decoded frame.
- `cam0_flags` / `cam1_flags`: only accept a camera slot when valid is set and
  stale is clear.
- `ultra_flags` and `ultra_age_ms`: reject stale or unavailable ultrasonic data.

Do not treat command fields such as `current_speed_pct`, `esc_primary`, or
`servo_cmd` as measured vehicle response.
