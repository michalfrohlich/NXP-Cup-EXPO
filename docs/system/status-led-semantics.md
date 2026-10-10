# LEDs And Displays

Status LEDs provide immediate local diagnostics. They are board-local: the same
color does not imply that every board detected the same condition.

## S32K Vehicle Status

These meanings apply to `APP_RACE`, `APP_RACE_S32K_ONLY`, Tune Drive, and Stack
Drive. Individual benches have their own rules listed below.
The S32K is authoritative for vehicle motion and actuator safety.

| Color | Meaning | Vehicle response |
|---|---|---|
| White | ESC arming, or the active line source is lost | Neutral while arming. During line-loss grace the line-loss speed policy remains active; after the grace period the line-loss stop is latched. |
| Green | Normal speed run with valid line tracking | Normal control. |
| Yellow | Honor-lap/limited-speed phase | Distance-based limited-speed control. |
| Red | Latched hardware or timing fault | ESC immediately neutral and steering centered. |
| Off | Commanded stop | Steering centered; Stack Drive forces ESC neutral immediately. Standalone race/Tune Drive use their control schedule and normal speed ramp toward zero. |

Line loss is intentionally white rather than red. It is a perception state with
a dedicated recovery window, not proof of a low-level hardware failure. If line
loss persists for `LINE_LOST_COAST_MS`, motion stops and white remains latched.

The first latched S32K fault reason is retained. Stack Drive Start can clear it
and begin another arm cycle; other race variants require reset or bench
exit/re-entry. Currently monitored safety faults are:

- stale S32K camera frames when the S32K camera controls steering;
- stale Teensy camera telemetry when Teensy camera steering is selected;
- a stalled control update;
- a stalled or uninitialized servo PWM period sequence.

Current thresholds are 500 ms for local camera/control/servo progress, and
1000 ms for missing acceptable Teensy camera telemetry in Teensy-steering
modes. Teensy camera samples older than 100 ms are excluded earlier. Persistent
line loss uses a separate 3000 ms grace. In SPEED_RUN that grace targets
`SPEED_LOST_LINE` (currently 20%); in Honor a lost line targets zero. White does
not necessarily mean stopped, and the 20% target is not always below the
selected profile speed.

Ultrasonic timeouts and transient ESP/SPI protocol errors are diagnostics, not
red safety faults. Ultrasonic data affects motion policy in Honor mode, but an
individual timeout cannot distinguish no echo from failed hardware reliably.

The LED is a software status indicator. There is no motor-speed feedback or
mechanical steering verification. If the whole foreground loop stalls, these
foreground checks and the LED renderer cannot run; they are not a watchdog.

## S32K Bench Exceptions

| Mode | Meaning |
|---|---|
| Boot banner | Cycles red/yellow/green/cyan/blue at 250 ms steps for three seconds after common init |
| Bench menu | Blue: selection; yellow: startup switch-release guard |
| Servo | Blue while testing; use displayed RAW/OUT/LIM for behavior |
| ESC | Blue: disarmed; blinking cyan: arming; blinking green: armed neutral; green: running; blinking red: ESTOP state |
| Camera-s32k / Cam+Servo-s32k | White: paused or line lost; red: no fresh frame for 500 ms; green: fresh tracked line. This is not the race fault latch. |
| Drive mode-s32k | White: arming/lost line; off: waiting for SW3 start; green: tracking; red: stale camera, neutral forced. No three-second line-loss stop or latched race fault. |
| Ultrasonic | Blue: startup/wait; green: clear/scanning; yellow: slow region; red: close stop region or capture error. No actuators. |
| Ultra+ESC | Blue: arming; yellow: waiting for valid range or slow region; green: clear/run; red: distance stop or sensor error. These are local test states, not race faults. |
| Cable tune-s32k | Blue waiting for terminal connection; green in connected menus; yellow while editing a value |
| ESP Link bench | Blue: idle; cyan for 300 ms after accepted tune/command; red for 300 ms after a new UART/protocol/TX error, with error priority |
| Standalone ESP test | Same cyan/error pulses; green for a fresh button ACK within 250 ms, otherwise blue |
| Teensy Link | Green if live; otherwise red for any recorded CRC/SPI error or more than 20 protocol errors; otherwise yellow if an old valid frame exists, blue if none |
| Servo rate test | Green: period callback and command latch keeping up; red: missing callback or prior command still buffered |
| Victory Lap | Blue: arming; green: approach; yellow: no valid distance; fanfare sequence alternates cyan/green/yellow then off |

In the SPI test, a green live link can coexist with historical error counters.
After it becomes non-live, those retained counters can make the LED red.
Read the counters and freshness together rather than interpreting every red
as a new error. The ESC blink step is 250 ms (a full on/off cycle is 500 ms).

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

The tracking indication is evaluated at 20 Hz. A requested non-fault state must
persist for 100 ms before changing the LED. Correction starts beyond +/-0.12
normalized error and persists until back within +/-0.08. Brightness is 128/255.
Camera failure or data older than 100 ms bypasses the hold at the next LED
evaluation. It does not stop the S32K directly.

Race has no LED toggle; Stack Drive button 2 toggles it without stopping sensors
or SPI. Link Bench disables tracking indication. Hardware Test uses button 1
to cycle R/G/B, button 2 for on/off, and pot brightness. SD Test uses green for
ready, yellow for inactive/no card/open failure, red for logger error, also with
pot brightness. Zero brightness can make any of these colors appear off.

## ESP32 Command Status

The ESP32 onboard blue LED turns on while a web command is waiting for the S32K
result. It remains visible for at least 100 ms even when the response is faster,
then turns off. The minimum pulse uses an ESP timer and does not delay the HTTP
response or UART task.

This is the GPIO2 LED. Button frames, including `swPcb`, do not set its color.
Off means the waiting indication ended, not proof of success: inspect the
HTTP result and counters.

## Displays By Mode

| Board/mode | Display behavior |
|---|---|
| S32K common boot | SSD1306-style OLED, build name for three seconds; serial prints full macro |
| S32K standalone races | Speed selector/READY first. Enable `swPcb` during arming to enable race diagnostics. S32K-camera race refreshes at a 200 ms target; Teensy-camera race suppresses refresh in RUN/HONOR, allowing it in ARM/STOP/FAULT. |
| S32K Tune Drive | Static entry screen with SW3 stop/exit instructions; no periodic race dashboard in this wrapper |
| S32K Stack Drive | Four pages, SW2 cycles, 150 ms refresh target: drive state, local camera, ESP tuning, SPI/CFG/ACK |
| S32K camera benches | MAIN/FILT/GRAD/FINISH graphs, waiting/capture counters; nominal 100 ms display refresh |
| S32K ESP Link bench | `ESP TUNE SAFE`, RX/error counts, KP/KD, min/high/low; 500 ms periodic target, accepted requests request earlier refresh when TX is idle |
| S32K standalone ESP test | Per-mode OLED output disabled (`ESP_LINK_DISPLAY_ENABLED=0`); common boot banner still runs |
| S32K Teensy Link | Six pages at 100 ms target: link/config, CAM0, CAM1, IMU/log, RX errors, raw header; SW2 cycles |
| Teensy Race | Display disabled |
| Teensy Stack Drive | SH1122 256x64, 250 ms target; camera/SPI/IMU/S32K/config/LED summary |
| Teensy Camera Bench | Optional SSD1306 128x64 graph disabled by default; separate from SH1122 dashboard |
| ESP32 | SH1122 256x64; 1.5 s self-test then 50 ms refresh target, configured 800 kHz I2C |

Refresh periods are foreground targets, not guaranteed rates. An unchanged race
screen may be intentional suppression. A missing S32K boot banner together
with no boot LED sequence is different: common initialization may not have
completed. Check generated timer aliases, timebase progress, and display bus
initialization before assuming another board prevents boot.

## Reading The Fields

- `CFG` / `ACK`: requested/applied detector generation, not camera frame count.
- SPI `READY`: slave permits a transfer; it does not guarantee valid vision.
- SPI `LIVE` / `OK` on S32K: recently decoded valid telemetry; camera validity
  and sensor age still need checking. `WAIT`: no valid frame yet.
- Teensy dashboard `S32K:OK`: a valid snapshot has been decoded; this renderer
  does not expire the label by age. Check advancing RX/sequence counters.
- Camera `LOST`: detector lacks an accepted track, not necessarily missing
  hardware. Look at raw contrast, frame age, and acquisition counters.
- IMU `P/C/V/A`: present, calibrated, valid/fresh, acceleration trusted.
  Relative yaw can drift even with all flags set.
- Ultrasonic `DRV ST: BUSY/NEW/ERR/IDLE`: acquisition state; normal transitions
  need not match the retained distance classification.
- ESP `wifi:ap`: AP ready, no associated client; `wifi:pc`: client associated;
  `wifi:init`: initializing; `wifi:err`: network startup error. None proves a
  S32K request succeeded.
- ESP `FRAME/BUTTONS/FIELDS`: latest button transport data, not car state.
  `TUNE` counters also include drive-command results through the shared result
  handler. High-level vehicle telemetry is not implemented on this screen.

See the [mode matrix](modes.md) and [test sequence](testing.md) before comparing
indicators between different firmware builds.
