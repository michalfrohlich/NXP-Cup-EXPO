# Architecture

## Top-level structure
- `src/main.c` calls `App_RunSelectedMode()` and never returns.
- `src/app/app_modes.c` selects exactly one compile-time mode from `src/app/app_config.h` and dispatches to a dedicated mode module.
- `src/app/board_init.c` initializes RTD/MCAL modules before app logic starts.
- `src/app/app_common.c` owns shared app-level runtime init helpers after `Board_InitDrivers()`.
- `src/app/app_internal.h` is a private app-layer header for shared mode state, helper declarations, and the static mode contract.
- `src/app/app_config.h` owns app build selection and behavior/profile constants.
- `include/config/board_config.h` owns board routing aliases that may reference generated RTD/MCAL symbols.
- `include/config/actuator_config.h` owns servo and ESC physical calibration values.
- `include/config/sensor_config.h` owns sensor geometry, timing, conversion, and validity-limit constants.
- `include/domain/vehicle_types.h` defines the shared vision/control/app handoff packets.
- `include/config/vision_config.h` defines vision detector tunables used by the vision service and debug code.
- `include/config/control_config.h` defines baseline steering-control tunings used by the control service and app runtime tune initialization.
- `src/debug/uart_host_link.c` provides a temporary handwritten UART debug transport that is brought up from `board_init.c` after generated driver init.

## Execution model
- Bare-metal, no scheduler, no RTOS.
- Standalone compile-time modes still own the top-level loop, but the reusable test implementations are internally split into `enter` / `update` / `exit` helpers.
- `app_modes.c` is intentionally only the compile-time flavor dispatcher.
- Dedicated app modules own mode behavior:
  - `modes/bench_menu.c`: runtime test menu host and linear-camera standalone wrapper.
  - `modes/bench_tests.c`: reusable bench test implementations for camera, ESC, servo, ultrasonic, receiver, camera+servo, and simple drive.
  - `modes/bench_teensy_link.c`: S32K-master Teensy SPI runtime test service and OLED viewer.
  - `modes/mode_teensy_link.c`: direct compile-time Teensy SPI link mode.
  - `modes/mode_nxp_cup.c`: NXP Cup profile, ready, rearm, run, and ultrasonic obstacle state machine.
  - `modes/mode_honor_lap.c`: standalone honor-lap line-following plus ultrasonic speed policy.
  - `modes/mode_race.c`: standalone race/honor-lap production flow.
  - `modes/mode_servo_rate.c`: standalone servo timing test.
  - `modes/bench_serial_tune.c`: UART/OLED runtime tuning UI used by the bench menu.
- `APP_TEST_LINEAR_CAMERA_TEST` remains a special-case standalone test mode for direct camera bring-up/debug.
- `APP_TEST_NXP_CUP` is a standalone competition mode with profile selection, ready, dual-ESC rearm, and autonomous run phases.
- `APP_TEST_RACE_MODE` is the standalone production race flow.
- `APP_TEST_SERVO_RATE_TEST` is a standalone servo-only timing test for the period-latched steering output.
- `APP_TEST_NXP_CUP_TESTS` is the compile-time mode that exposes the rest of the individual test screens.
- `APP_TEST_HONOR_LAP` is a separate standalone compile-time mode that boots straight into line following plus ultrasonic speed limiting.
- Invalid or missing `APP_TEST_*` selection is treated as a configuration error.
- Timing is a mix of:
  - polling against `Timebase_GetMs()`
  - periodic state-machine updates
  - interrupt callbacks wired to RTD notifications
- The Teensy SPI link is test-owned in the current bring-up. It runs when
  `APP_TEST_TEENSY_LINK_TEST` boots directly into the link mode, or when the
  `Teensy Link` runtime bench test is entered.

## LED conventions in app modes
- Red: error or fault
- Yellow: degraded / paused / missing valid sensor data
- Blue: idle, setup, menu, or manual standby
- Green: primary active run state
- Cyan: secondary active run state (camera-servo / honor-lap style behavior)

## Current control flow
1. `main()`
2. `App_RunSelectedMode()`
3. Exactly one compile-time mode is selected in `app_config.h`
4. `app_modes.c` dispatches to one mode module
5. That mode performs common runtime init through app-owned helpers (`Board_InitDrivers()`, timebase, pot, and mode-specific display bring-up when needed)
6. The selected mode runs forever
7. Only `APP_TEST_NXP_CUP_TESTS` can switch between tests at runtime; the other modes boot straight into their own dedicated flows
8. `APP_TEST_RACE_MODE` keeps a fixed execution order of `vision -> ultrasonic -> control`, and only touches the OLED when `swPcb` requests debug output
9. `APP_TEST_SERVO_RATE_TEST` initializes only common runtime hardware plus the servo/display path and then sends synthetic `Servo_SetSteer()` commands at a selectable software cadence

## Mode lifecycle
- Standalone mode lifecycle:
  - `App_RunSelectedMode()` selects one build flavor.
  - The selected `mode_*.c` module calls `App_InitRuntimeCore()` or `App_InitRuntimeCommon()`.
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
  - the menu includes `Serial tune`, which reuses the existing UART/OLED tuning UI as a runtime test and polls UART non-blockingly so the test can still be exited with the shared `swPcb` wrapper
  - serial tuning writes into a shared RAM-backed runtime tune block that persists for the current power cycle, and the generic camera/servo drive path now reads that block when entering `Cam Servo` and `Simple test drv`
- `APP_TEST_HONOR_LAP` remains a standalone compile-time mode and is not part of the runtime test menu

## Servo Rate Test
- `APP_TEST_SERVO_RATE_TEST` is meant to isolate servo command timing from camera, ESC, ultrasonic, and steering-controller code.
- `SW2` cycles the software command rate through `10 Hz`, `50 Hz`, `100 Hz`, and `250 Hz`.
- `SW3` cycles the command source through potentiometer control, one-unit fine sweep, and larger step sweep.
- The OLED shows the selected rate, selected command source, current `Servo_SetSteer()` command, PWM-period callback count, and an `OK` / `BUF` / `NOISR` status.
- The RGB LED is green while the servo driver/callback path keeps up, and red if the PWM-period callback is not firing or if a new command slot arrives while the previous command is still buffered.
- The physical PWM frame is still the generated `50 Hz` servo PWM. This test changes how often handwritten code publishes new desired steering commands into the period-latched servo driver.

## NXP Cup mode
- `APP_TEST_NXP_CUP` is a profile-driven competition flow built on the current camera, vision, steering, and ultrasonic paths.
- The user flow is:
  - menu: select `SUPERFAST`, `5050`, or `SLOW` with the pot and confirm with `SW2`
  - ready: `SW3` starts the run sequence, `SW2` goes back
  - ESC rearm: both ESC outputs finish their arm/beep sequence after profile selection, while camera steering is already running in the background
  - run: camera vision steers, motor speed ramps toward the selected profile target, ultrasonic can force stop/crawl/cutoff behavior
- `SW2` returns to the menu and `SW3` returns to the ready screen from both the rearm and run phases.
- During ultrasonic obstacle handling, steering is held straight instead of continuing camera corrections.

## Race mode
- `APP_TEST_RACE_MODE` is the cleaned-up production successor to `APP_TEST_FINAL_DUMMY`.
- The mode owns dual ESC outputs, servo, linear camera, vision, and ultrasonic directly instead of switching between sub-modes.
- After startup it:
  - arms both ESC outputs
  - autostarts line following
  - runs the normal race speed policy until a confirmed finish-line detection
  - transitions once into the honor-lap ultrasonic stopping policy
  - stays stopped on stop/fault conditions
- The race phase and honor-lap phase are also differentiated on the RGB LED: green in race run, cyan in honor-lap run, red on fault.
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
  - applies steering through PWM

### Honor lap path
- `mode_honor_lap.c`
  - reuses the linear camera debug/servo path for continuous line following
  - keeps the OLED active in linear-camera debug mode even before first valid frame by showing camera trigger/DMA/ADC counters
  - overrides steering tunings with the `HONOR_*` constants from `app_config.h`
  - arms both ESC outputs at startup, then commands forward speed from the honor-lap distance policy
- `ultrasonic.c`
  - provides periodic obstacle distance samples
  - can reduce commanded speed through the `HONOR_SLOW*` and `HONOR_STOP_*` thresholds
  - holds steering straight once the obstacle is inside the first slow threshold

### Race mode path
- `mode_race.c`
  - initializes the dual-ESC path, servo, camera, steering, and ultrasonic once at mode entry
  - runs a deterministic ordered loop:
    - consume the newest completed camera frame if one is ready
    - service ultrasonic timing and capture state
    - update steering and speed commands
  - rate-limits the ESC command with the existing ramp constants
  - switches from race-speed policy to honor-lap obstacle-stop policy after confirmed finish detection
  - holds steering straight during the honor phase when ultrasonic sees an obstacle inside the slow threshold
  - only renders OLED telemetry when `swPcb` enables debug display

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
  - builds the fixed 80-byte S32K frame
  - clocks the SPI transfer as S32K master
  - validates CRC/header on the Teensy frame
  - stores the latest decoded snapshot and diagnostics

### Manual / motor path
- `onboard_pot.c`
  - samples the potentiometer through ADC
- `bench_menu.c` and `bench_tests.c`
  - uses a dedicated standalone ESC test with hold-to-arm/disarm, hold-to-stop, selectable both/ESC1/ESC2/differential output modes, and a low capped potentiometer speed command
  - draws an ESC test screen with state, selected output mode, direction, pot level, and both ESC commands
  - exposes a standalone ultrasonic test screen and the same test inside `APP_TEST_NXP_CUP_TESTS`
  - exposes an `Ultra+ESC` runtime test in `APP_TEST_NXP_CUP_TESTS` for tuning obstacle slowing and stop behavior
  - exposes a `Victory Lap` runtime test in `APP_TEST_NXP_CUP_TESTS` for pole-triggered fanfare behavior
  - exposes a `Cam Servo` runtime test in `APP_TEST_NXP_CUP_TESTS` that follows the linear camera debug flow while also steering automatically
  - exposes a `Simple test drv` runtime test in `APP_TEST_NXP_CUP_TESTS` that initializes ESC, camera, and servo together, waits through ESC arm time, and then starts only after `SW3` is pressed before ramping to `FULL_AUTO_SPEED_PCT` while camera steering stays active
- `mode_nxp_cup.c`
  - exposes a standalone `APP_TEST_NXP_CUP` mode that reuses the current `CamServo` path with a profile menu and dedicated ready / rearm / run state machine
- `bench_tests.c`
  - uses a two-stage standalone servo test: the potentiometer first selects `RAW` or `SMOOTH`, `SW2` enters the selected mode, and then the live screen shows raw, filtered, and applied steering while `SW2` re-centers the servo
- `debug/uart_host_link.c`
  - still provides the blocking shell-style serial path used by `Serial tune`
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
- App modes: `src/app/modes/bench_menu.c`, `src/app/modes/bench_tests.c`, `src/app/modes/bench_serial_tune.c`, `src/app/modes/bench_teensy_link.c`, `src/app/modes/mode_nxp_cup.c`, `src/app/modes/mode_honor_lap.c`, `src/app/modes/mode_race.c`, `src/app/modes/mode_servo_rate.c`
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
