# Status LED Semantics

Status LEDs provide immediate local diagnostics. They are board-local: the same
color does not imply that every board detected the same condition.

## S32K Vehicle Status

These meanings apply to Race, Teensy Camera Race, Tune Drive, Stack Drive, and
the S32K camera/control bench modes where the corresponding state exists.
The S32K is authoritative for vehicle motion and actuator safety.

| Color | Meaning | Vehicle response |
|---|---|---|
| White | ESC arming, or the active line source is lost | Neutral while arming. During line-loss grace the configured reduced-speed policy remains active; after the grace period the line-loss stop is latched. |
| Green | Normal speed run with valid line tracking | Normal control. |
| Yellow | Honor-lap/limited-speed phase | Distance-based limited-speed control. |
| Red | Latched hardware or timing fault | ESC immediately neutral and steering centered. |
| Off | Commanded stop | ESC neutral and steering centered. |

Line loss is intentionally white rather than red. It is a perception state with
a dedicated recovery window, not proof of a low-level hardware failure. If line
loss persists for `LINE_LOST_COAST_MS`, motion stops and white remains latched.

The first latched S32K fault reason is retained until a new Start command begins
another arm cycle. Currently monitored safety faults are:

- stale S32K camera frames when the S32K camera controls steering;
- stale Teensy camera telemetry when Teensy camera steering is selected;
- a stalled control update;
- a stalled or uninitialized servo PWM period sequence.

Ultrasonic timeouts and transient ESP/SPI protocol errors are diagnostics, not
red safety faults. Ultrasonic data affects motion policy in Honor mode, but an
individual timeout cannot distinguish no echo from failed hardware reliably.

## Teensy Tracking Status

These meanings apply to Camera Bench, Race, and Stack Drive when tracking LED
output is enabled.

| Color | Meaning |
|---|---|
| White | Camera/vision initialization, or camera is alive but the line is lost |
| Red | Camera unavailable or camera data stale |
| Green | Line centered within the tracking deadband |
| Cyan | Steering correction toward negative detector error |
| Blue | Steering correction toward positive detector error |
| Off | Tracking indication disabled by the Stack Drive button toggle |

The tracking indication is evaluated at 20 Hz. A 100 ms state hold and a small
center hysteresis prevent frame-rate flicker. Camera failure bypasses the hold
and changes to red immediately. Hardware and SD tests retain their own explicit
test-result color schemes.

## ESP32 Command Status

The ESP32 onboard blue LED turns on while a web command is waiting for the S32K
result. It remains visible for at least 100 ms even when the response is faster,
then turns off. The minimum pulse uses an ESP timer and does not delay the HTTP
response or UART task.
