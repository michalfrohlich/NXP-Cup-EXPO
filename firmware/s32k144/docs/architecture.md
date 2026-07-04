# Architecture

## Top-level structure
- `src/main.c` runs the common boot banner, then calls `App_RunSelectedMode()` and never returns.
- `src/app/app_modes.c` selects exactly one compile-time mode from `src/app/app_config.h` and dispatches to a dedicated mode module.
- `src/app/board_init.c` initializes RTD/MCAL modules before app logic starts.
- `src/app/app_common.c` owns the common boot banner, shared app-level runtime init helpers after `Board_InitDrivers()`, status LED helpers, and display text helpers.
- `src/app/app_internal.h` is a private app-layer header for shared mode state, helper declarations, and the static mode contract.
- `src/app/app_config.h` owns app build selection and behavior/profile constants.
- `include/config/board_config.h` owns board routing aliases that may reference generated RTD/MCAL symbols.
- `include/config/actuator_config.h` owns servo and ESC physical calibration values.
- `include/config/sensor_config.h` owns sensor geometry, timing, conversion, and validity-limit constants.
- `include/domain/vehicle_types.h` defines the shared vision/control/app handoff packets.
- `include/config/vision_config.h` defines vision detector tunables used by the vision service and debug code.
- `include/config/control_config.h` defines baseline steering-control tunings used by the control service and app runtime tune initialization.
- `SteeringControllerConfig_t` is the app-to-controller tuning boundary. Race
  and camera/servo profiles are converted by the app and applied through
  `SteeringController_ApplyConfig()`; app modes do not write controller tuning
  fields directly. Applying a config preserves PID history, while
  `SteeringController_Reset()` explicitly clears that history at mode entry.
- `src/debug/uart_host_link.c` provides a temporary handwritten UART debug transport that is brought up from `board_init.c` after generated driver init.

## Execution model
- Bare-metal, no scheduler, no RTOS.
- Race-family modes keep the authoritative actuator safety state. Fault detection
  records the first `RaceFaultReason_t`, forces neutral/center immediately, and
  leaves all vehicle-status color selection to one renderer. Cross-board LED
  meanings are documented in `../../../docs/system/status-led-semantics.md`.
- `SpeedModeSetting_t` identifies the shared `SUPERFAST`, `5050`, and `SLOW`
  control profiles. Each profile contains base speed, PID gains, integral clamp,
  steering clamp, output LPF, and steering rate limit. Standalone S32K-camera and
  Teensy-camera Race builds use a common `SELECT -> READY -> START` launcher;
  bench code can apply the same profiles explicitly through
  `RuntimeTune_ApplySpeedMode()` without duplicating constants.
- Every build flavor first runs the same startup banner: initialize common hardware, print the selected `APP_TEST_*` build flag to the host UART, show the selected mode on the OLED for 3 seconds, and run a visible RGB LED color sequence.
- Standalone compile-time modes still own the top-level loop after the boot banner, but the reusable test implementations are internally split into `enter` / `update` / `exit` helpers.
- `app_modes.c` is intentionally only the compile-time flavor dispatcher.
- Dedicated app modules own mode behavior:
  - `modes/bench_menu.c`: runtime test menu host and linear-camera standalone wrapper.
  - `modes/bench_tests.c`: reusable bench test implementations for camera, ESC, servo, ultrasonic, receiver, camera+servo, and simple drive.
  - `modes/bench_teensy_link.c`: S32K-master Teensy SPI runtime test service and OLED viewer.
  - `modes/mode_teensy_link.c`: direct compile-time Teensy SPI link mode.
  - `modes/mode_race.c`: local-camera race flow, Teensy-camera synchronized race flow, and the tune-drive bench wrapper.
  - `modes/mode_servo_rate.c`: standalone servo timing test.
  - `modes/mode_esp_link.c`: standalone ESP32 UART integration test.
  - `modes/bench_serial_tune.c`: UART/OLED runtime tuning UI used by the bench menu.
  - `modes/bench_esp_link.c`: runtime-menu ESP32 tune-frame receive test.
- `APP_TEST_LINEAR_CAMERA_TEST` remains a special-case standalone test mode for direct camera bring-up/debug.
- `APP_TEST_ESC_TEST` boots directly into the ESC bench test while reusing the runtime menu ESC test implementation.
- `APP_RACE_S32K_ONLY` is the standalone fallback race flow using the S32K linear camera.
- `APP_RACE` is the full standalone race flow using Teensy camera results over SPI.
- `APP_TEST_SERVO_RATE_TEST` is a standalone servo-only timing test for the period-latched steering output.
- `APP_TEST_NXP_CUP_TESTS` is the compile-time mode that exposes the rest of the individual test screens.
- `APP_TEST_ESP_LINK_TEST` is a standalone UART integration mode. The link
  driver parses CRC-protected button ACK and full tuning frames; the mode
  consumes the latest validated snapshot, updates all eight RAM runtime-tune
  values, queues a compact sequence-matched result, and owns LED diagnostics.
  It also ACKs compact ESP start/stop command frames without moving the
  vehicle.
  The LPUART2 ISR copies received bytes into a 256-byte software ring and
  records hardware error flags; ordered reset markers discard only the damaged
  frame while preserving later retry bytes. Parsing remains in the foreground
  service, and Stack Drive services RX and result TX around its SPI/control work.
  OLED activity is temporarily disabled in this mode while the link is
  validated. Once per second, immediately after a valid ACK, the mode reports
  UART and protocol error counters on the host debug UART.
- ESP tuning frame and drive-command servicing is factored into small app helpers so runtime
  bench modes can consume the same validated PID, steering, and line-detector
  snapshot, or ACK start/stop requests, without duplicating UART parsing or
  tune-state writes.
- The runtime `ESP Link` bench item validates RAM-only tuning frames and ACKs
  start/stop command frames from ESP32 without transmitting S32K button state
  or moving the vehicle.
- Invalid or missing `APP_TEST_*` selection is treated as a configuration error.
- Timing is a mix of:
  - polling against `Timebase_GetMs()`
  - periodic state-machine updates
  - interrupt callbacks wired to RTD notifications
- The Teensy SPI link runs in the direct `APP_TEST_TEENSY_LINK_TEST`, the
  runtime `Teensy Link` bench test, and `APP_RACE`. The race
  path consumes fresh Teensy camera results on a servo-period schedule.

## LED conventions in app modes
- Red: error or fault
- Yellow: degraded / paused / missing valid sensor data
- Blue: idle, setup, menu, or manual standby
- Green: primary active run state
- Cyan: secondary active run state (camera-servo / honor-lap style behavior)

## Current control flow
1. `main()`
2. `App_RunBootBanner()`
3. Common runtime init runs once (`Board_InitDrivers()`, timebase, pot, UART host, display)
4. The selected `APP_TEST_*` build flag is printed to UART and shown on the OLED for 3 seconds while the RGB LED runs the boot color sequence
5. `App_RunSelectedMode()`
6. Exactly one compile-time mode is selected in `app_config.h`
7. `app_modes.c` dispatches to one mode module
8. That mode calls common runtime init through app-owned helpers; repeated calls are idempotent so MCAL/display bring-up is not restarted after the boot banner
9. The selected mode runs forever
10. Only `APP_TEST_NXP_CUP_TESTS` can switch between tests at runtime; the other modes boot straight into their own dedicated flows
11. `APP_RACE_S32K_ONLY` keeps a fixed execution order of `vision -> ultrasonic -> control`, and only touches the OLED when `swPcb` requests debug output
12. `APP_TEST_SERVO_RATE_TEST` initializes only common runtime hardware plus the servo/display path and then sends synthetic `Servo_SetSteer()` commands at a selectable software cadence

## Mode lifecycle
- Standalone mode lifecycle:
  - `App_RunBootBanner()` performs shared startup identification before any mode-specific code starts.
  - `App_RunSelectedMode()` selects one build flavor.
  - The selected `mode_*.c` module calls `App_InitRuntimeCore()` or `App_InitRuntimeCommon()`; these helpers are safe to call after the boot banner and do not re-run MCAL bring-up.
  - The mode performs its own hardware-specific init.
  - The mode enters its forever loop.
- Runtime bench lifecycle:
  - `modes/bench_menu.c` initializes common runtime hardware and draws the pot-driven menu.
  - Entering a menu item calls that test's `enter(nowMs)` helper.
  - The menu host repeatedly calls that test's `update(...)` helper while active.
  - Leaving a menu item calls that test's `exit()` helper and returns to the menu.

## Runtime-selectable tests mode
- `APP_TEST_NXP_CUP_TESTS` adds a simple test menu driven by the onboard potentiometer.
- While that compile-time mode is active:
  - the pot selects the highlighted test
  - the dedicated `swPcb` toggle switch enters or leaves the selected test
  - `swPcb` is treated as a maintained level input, not a click-style button event
  - the menu includes an `Ultrasonic` runtime test that, after a short enable delay, classifies the measured distance into `WAIT`, `SCAN`, `CLEAR`, `SLOW`, and `STOP`, with `SW2` resetting the test state
  - the menu includes an `Ultra+ESC` runtime test that centers the servo, drives both ESC outputs at 50% when clear, slows at 45 cm, and fully stops at 8 cm
  - the menu includes a `Victory Lap` runtime test that approaches a pole with ultrasonic and then runs a Mario-style victory note sequence for future motor-music support
  - the menu includes a `Cam Servo` runtime test that reuses the camera debug flow and adds automatic servo steering from the detected line
  - the menu includes `Simple test drv`, which reuses the old `FINAL_DUMMY` camera-driving behavior as a normal runtime test with its own enter/update/exit path
  - the menu includes `Tune drive mode`, which runs the Teensy-camera race control path as a bench item and services ESP32 UART tuning frames while driving
  - the menu includes `Stack drive`, which runs the S32K-camera race control path, services ESP32 tuning and start/stop commands, keeps Teensy SPI status service active, and does not hard-stop on missing Teensy camera/SPI data
  - the menu includes `Cable tune`, which reuses the existing UART/OLED tuning UI as a runtime test and polls UART non-blockingly so the test can still be exited with the shared `swPcb` wrapper
  - serial and ESP tuning write into a shared RAM-backed runtime tune block that persists for the current power cycle; `Cam Servo`, `Simple test drv`, `Tune drive mode`, and `Stack drive` read that block when entering or when a new ESP tune frame arrives

## Servo Rate Test
- `APP_TEST_SERVO_RATE_TEST` is meant to isolate servo command timing from camera, ESC, ultrasonic, and steering-controller code.
- `SW2` cycles the software command rate through `10 Hz`, `50 Hz`, `100 Hz`, and `250 Hz`.
- `SW3` cycles the command source through potentiometer control, one-unit fine sweep, and larger step sweep.
- The OLED shows the selected rate, selected command source, current `Servo_SetSteer()` command, PWM-period callback count, and an `OK` / `BUF` / `NOISR` status.
- The RGB LED is green while the servo driver/callback path keeps up, and red if the PWM-period callback is not firing or if a new command slot arrives while the previous command is still buffered.
- The physical PWM frame is still the generated `50 Hz` servo PWM. This test changes how often handwritten code publishes new desired steering commands into the period-latched servo driver.

## Race mode
- `APP_RACE_S32K_ONLY` and `APP_RACE` share the cleaned-up production race state machine.
- The mode owns dual ESC outputs, servo, linear camera, vision, and ultrasonic directly instead of switching between sub-modes.
- After startup it:
  - arms both ESC outputs
  - autostarts line following
  - runs the normal race speed policy until a confirmed finish-line detection
  - transitions once into the honor-lap ultrasonic stopping policy
  - stays stopped on stop/fault conditions
- The race phase and honor-lap phase are differentiated on the RGB LED: green in race run, yellow in honor-lap run, red on hardware/timing fault, white on line loss, and off on commanded stop.
- Finish-line detection switches to honor-lap mode on the first accepted finish-line frame that meets `RACE_FINISH_MIN_CONFIDENCE`.
- The OLED is intentionally kept out of the hot path. In race mode it may be initialized only during ESC arm, and is refreshed later only if `swPcb` is on and the display was already brought up before the race started.

## Data flow
### Camera steering path
- `linear_camera.c`
  - drives shutter and pixel clock
  - requests frame cadence with GPT and aligns the SI pulse to two PWM falling edges
  - keeps the camera clock free-running while FTM falling-edge notifications are enabled only during the active frame window
  - latches a pending frame request if GPT expires during an active frame, then starts the next frame only after the current one is fully published
  - asserts `SI` on the first falling edge and deasserts it on the second falling edge
  - starts one software ADC conversion on each subsequent falling edge until all `128` pixels are captured
  - stores each completed sample in `CameraAdcFinished()` into handwritten ping-pong frame buffers
  - disables PWM notifications again as soon as the frame is complete, so there are no camera clock-edge interrupts between frames
  - exposes a neutral debug-counters snapshot for requested frames, frame starts, capture events, completed frames, and dropped frames
- `bench_tests.c`
  - the linear-camera waiting screen consumes the driver debug-counters snapshot instead of several driver-internal counter getters
  - the waiting screen is rendered once before `LinearCameraStartStream()` so the OLED can show a visible startup frame before camera ISR traffic begins
  - consumes completed camera frames as soon as `LinearCameraGetLatestFrame()` reports a ready buffer, while UI/debug/control housekeeping remains separately rate-limited
  - the runtime `Camera` / `Cam Servo` tests now opportunistically queue one compact UART telemetry packet per processed frame, dropping packets instead of blocking if the software TX queue is full
- `services/line_detector.c`
  - filters the frame
  - computes a 1D gradient
  - selects lane edges / track center from signed edge candidates
  - computes confidence from selected-edge strength, contrast, and line-status state
  - detects the finish line from the inner white gap
- `services/steering_controller.c`
  - converts vision error to steering command
  - reads baseline PID, steering shaping, and controller speed-policy values from `include/config/control_config.h`
  - applies filtered-error and filtered-derivative shaping (with confidence-aware error filtering) before PID output
  - resets controller memory when vision reports track lost to avoid stale integral/derivative carry-over
  - carries the active integral clamp in controller state, so per-mode and per-profile integral clamp settings are real runtime inputs
- `servo.c`
  - treats `Servo_SetSteer()` as a latest-request mailbox
  - defaults to applying the latest pending request from the PWM falling-edge
    callback, preserving existing mode behavior
  - supports an explicit phase-synchronized policy that timestamps each 50 Hz
    PWM rising edge and exposes the period sequence
  - in the phase-synchronized policy, stages at most one hardware duty update
    while `17 <= phase < 19 ms`
  - discards a phased request that misses its window so it cannot apply one
    period late

### Race mode path
- `mode_race.c`
  - initializes the dual-ESC path, servo, camera, steering, and ultrasonic once at mode entry
  - keeps the original local-camera path on its existing schedule
  - in `APP_RACE`, uses the servo PWM rising edge as the
    20 ms scheduling epoch
  - services SPI at phases 0, 5, 10, and 15 ms, skipping missed slots instead
    of replaying them back-to-back
  - collects only unique camera sequences and runs one robust camera average
    plus one steering-controller update at phase 17 ms
  - rejects stale/lost samples and at most one error outlier above 0.20, then
    confidence-weights the remaining errors
  - holds the previous PWM command if control does not run before phase 19 ms
  - rate-limits the ESC command with the existing ramp constants
  - switches from race-speed policy to honor-lap obstacle-stop policy after confirmed finish detection
  - holds steering straight during the honor phase when ultrasonic sees an obstacle inside the slow threshold
  - only renders OLED telemetry when `swPcb` enables debug display
  - exposes `Tune drive mode` as a runtime bench wrapper around the Teensy-camera race path; live ESP frames update S32K-side PID and steering shaping while line-detector values are stored for future forwarding to the Teensy camera stack

### Teensy SPI link path
- `mode_teensy_link.c`
  - direct compile-time wrapper for `APP_TEST_TEENSY_LINK_TEST`
  - boots straight into the Teensy link test
- `bench_teensy_link.c`
  - calls `TeensyLink_Init()` when the test is entered
  - builds the S32K-to-Teensy test packet
  - calls `TeensyLink_Service()` every 5 ms while the test is active
  - displays RX counters, sequence numbers, stale/live state, and error counts
- `teensy_link.c`
  - builds the fixed 84-byte S32K frame
  - clocks the SPI transfer as S32K master
  - validates CRC/header on the Teensy frame
  - stores the latest decoded snapshot and diagnostics

### Manual / motor path
- `onboard_pot.c`
  - samples the potentiometer through ADC
- `bench_menu.c` and `bench_tests.c`
  - uses a shared ESC bench test implementation for both the runtime menu and `APP_TEST_ESC_TEST`; `SW2` hold arms/disarms, `SW3` hold cycles the `30/50/100%` speed cap, and the potentiometer maps into the selected cap
  - draws an ESC test screen with state, selected output mode, direction, pot level, and both ESC commands
  - exposes a standalone ultrasonic test screen and the same test inside `APP_TEST_NXP_CUP_TESTS`
  - exposes an `Ultra+ESC` runtime test in `APP_TEST_NXP_CUP_TESTS` for tuning obstacle slowing and stop behavior
  - exposes a `Victory Lap` runtime test in `APP_TEST_NXP_CUP_TESTS` for pole-triggered fanfare behavior
  - exposes a `Cam Servo` runtime test in `APP_TEST_NXP_CUP_TESTS` that follows the linear camera debug flow while also steering automatically
  - exposes a `Simple test drv` runtime test in `APP_TEST_NXP_CUP_TESTS` that initializes ESC, camera, and servo together, waits through ESC arm time, and then starts only after `SW3` is pressed before ramping to `FULL_AUTO_SPEED_PCT` while camera steering stays active
  - exposes an `ESP Link` runtime test in `APP_TEST_NXP_CUP_TESTS` that validates RAM-only tuning frames from ESP32 without transmitting S32K button state
- `mode_race.c`
  - exposes `Tune drive mode` in `APP_TEST_NXP_CUP_TESTS`, reusing the Teensy-camera race scheduler with ESP32 UART tuning enabled
  - exposes `Stack drive` in `APP_TEST_NXP_CUP_TESTS`, using the S32K camera for steering while ESP tuning/drive commands and Teensy SPI status service continue to run
- `bench_tests.c`
  - uses a two-stage standalone servo test: the potentiometer first selects `RAW` or `SMOOTH`, `SW2` enters the selected mode, and then the live screen shows raw, filtered, and applied steering while `SW2` re-centers the servo
- `debug/uart_host_link.c`
  - still provides the blocking shell-style serial path used by `Cable tune`
  - now also exposes a small non-blocking TX queue so camera telemetry can be drained in the background from the runtime camera tests
- `esc.c`
  - applies motor command through PWM and an internal ESC state machine on both configured ESC outputs
  - exposes one explicit dual-output API: `Esc_Init(primary, secondary, ...)`, `Esc_SetSpeed(primary, secondary)`, and `Esc_SetBrake(primary, secondary)`

## Interrupt / callback-driven pieces
- `timebase.c`: `EmuTimer_Notification()`, `UsTimer_Notification()`
- `linear_camera.c`: `NewCameraFrame()`, `CameraClock()`, `CameraAdcFinished()`
  - `CameraAdcFinished()` stores one manual ADC sample per clock edge into the current frame buffer
- `ultrasonic.c`: `Icu_TimestampUltrasonicNotification()`
- `receiver.c`: `Icu_SignalNotification()`, `Gpt_Notification()`
- `esc.c`: `Esc_Period_Finished()`

## Key implementation files
- App dispatch, config, and shared glue: `src/app/app_modes.c`, `src/app/app_common.c`, `src/app/app_internal.h`, `src/app/app_config.h`, `include/config/board_config.h`, `include/config/actuator_config.h`, `include/config/sensor_config.h`
- App modes: `src/app/modes/bench_menu.c`, `src/app/modes/bench_tests.c`, `src/app/modes/bench_serial_tune.c`, `src/app/modes/bench_teensy_link.c`, `src/app/modes/mode_race.c`, `src/app/modes/mode_servo_rate.c`
- Board bring-up: `src/app/board_init.c`
- Vision: `src/services/line_detector.c`, `src/app/vision_debug.c`
- Steering: `src/services/steering_controller.c`, `src/drivers/servo.c`
- Motor control: `src/drivers/esc.c`
- Serial debug: `src/debug/uart_host_link.c`
- Sensors / IO: `src/drivers/linear_camera.c`, `src/drivers/onboard_pot.c`, `src/drivers/ultrasonic.c`, `src/drivers/receiver.c`, `src/drivers/buttons.c`, `src/drivers/display.c`, `src/drivers/rgb_led.c`

## Current vision notes
- The public vision handoff is `VisionOutput_t` in `include/domain/vehicle_types.h`.
- The current finish detector is gap-based, not region-based.
- The main debug screens in `vision_debug.c` are `MAIN`, `FILT`, `GRAD`, and `FINISH`.
- `src/unused/user_interface.c` is a retained legacy UI module not called by the active runtime; the active menu/HUD code is implemented in `src/app/modes/bench_menu.c` and the mode-specific app modules.
