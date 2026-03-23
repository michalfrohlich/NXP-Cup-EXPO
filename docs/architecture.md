# Architecture

## Top-level structure
- `src/main.c` calls `App_RunSelectedMode()` and never returns.
- `src/app/app_modes.c` selects exactly one compile-time mode from `src/app/car_config.h`.
- `src/app/board_init.c` initializes RTD/MCAL modules before app logic starts.

## Execution model
- Bare-metal, no scheduler, no RTOS.
- Standalone compile-time modes still own the top-level loop, but the reusable test implementations are internally split into `enter` / `update` / `exit` helpers.
- `APP_TEST_LINEAR_CAMERA_TEST` remains a special-case standalone test mode for direct camera bring-up/debug.
- `APP_TEST_NXP_CUP` is a standalone competition mode with profile selection, ready, ESC rearm, and autonomous run phases.
- `APP_TEST_RACE_MODE` is the standalone production race flow.
- `APP_TEST_NXP_CUP_TESTS` is the compile-time mode that exposes the rest of the individual test screens.
- `APP_TEST_HONOR_LAP` is a separate standalone compile-time mode that boots straight into line following plus ultrasonic speed limiting.
- Invalid or missing `APP_TEST_*` selection is treated as a configuration error; the dispatcher no longer silently falls back to `FINAL_DUMMY`.
- Timing is a mix of:
  - polling against `Timebase_GetMs()`
  - periodic state-machine updates
  - interrupt callbacks wired to RTD notifications

## LED conventions in `app_modes.c`
- Red: error or fault
- Yellow: degraded / paused / missing valid sensor data
- Blue: idle, setup, menu, or manual standby
- Green: primary active run state
- Cyan: secondary active run state (camera-servo / honor-lap style behavior)

## Current control flow
1. `main()`
2. `App_RunSelectedMode()`
3. Exactly one compile-time mode is selected in `car_config.h`
4. That mode performs common runtime init (`Board_InitDrivers()`, timebase, pot, and mode-specific display bring-up when needed)
5. The selected mode runs forever
6. Only `APP_TEST_NXP_CUP_TESTS` can switch between tests at runtime; the other modes boot straight into their own dedicated flows
7. `APP_TEST_RACE_MODE` keeps a fixed execution order of `vision -> ultrasonic -> control`, and only touches the OLED when `swPcb` requests debug output

## Runtime-selectable tests mode
- `APP_TEST_NXP_CUP_TESTS` adds a simple test menu driven by the onboard potentiometer.
- While that compile-time mode is active:
  - the pot selects the highlighted test
  - the dedicated `swPcb` toggle switch enters or leaves the selected test
  - `swPcb` is treated as a maintained level input, not a click-style button event
  - the menu includes an `Ultrasonic` runtime test that, after a short enable delay, classifies the measured distance into `WAIT`, `SCAN`, `CLEAR`, `SLOW`, and `STOP`, with `SW2` resetting the test state
  - the menu includes an `Ultra+ESC` runtime test that uses the ultrasonic stopping logic from honor lap without camera or servo
  - the menu now includes a final `Cam Servo` runtime test that reuses the camera debug flow and adds automatic servo steering from the detected line
- `APP_TEST_FINAL_DUMMY` and `APP_TEST_HONOR_LAP` remain standalone compile-time modes and are not part of the runtime test menu

## NXP Cup mode
- `APP_TEST_NXP_CUP` is a profile-driven competition flow built on the current camera, vision, steering, and ultrasonic paths.
- The user flow is:
  - menu: select `SUPERFAST`, `5050`, or `SLOW` with the pot and confirm with `SW2`
  - ready: `SW3` starts the run sequence, `SW2` goes back
  - ESC rearm: the ESC finishes its arm/beep sequence after profile selection, while camera steering is already running in the background
  - run: camera vision steers, motor speed ramps toward the selected profile target, ultrasonic can force stop/crawl/cutoff behavior
- `SW2` returns to the menu and `SW3` returns to the ready screen from both the rearm and run phases.
- During ultrasonic obstacle handling, steering is held straight instead of continuing camera corrections.

## Race mode
- `APP_TEST_RACE_MODE` is the cleaned-up production successor to `APP_TEST_FINAL_DUMMY`.
- The mode owns ESC, servo, linear camera, vision, and ultrasonic directly instead of switching between sub-modes.
- After startup it:
  - arms the ESC
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
- `app_modes.c`
  - the linear-camera waiting screen consumes the driver debug-counters snapshot instead of several driver-internal counter getters
  - the waiting screen is rendered once before `LinearCameraStartStream()` so the OLED can show a visible startup frame before camera ISR traffic begins
- `services/vision_linear_v2.c`
  - filters the frame
  - computes a 1D gradient
  - selects lane edges / track center from signed edge candidates
  - detects the finish line from the inner white gap
- `services/steering_control_linear.c`
  - converts vision error to steering command
  - carries the active integral clamp in controller state, so per-mode and per-profile integral clamp settings are real runtime inputs
- `servo.c`
  - applies steering through PWM

### Honor lap path
- `app_modes.c`
  - reuses the linear camera debug/servo path for continuous line following
  - keeps the OLED active in linear-camera debug mode even before first valid frame by showing camera trigger/DMA/ADC counters
  - overrides steering tunings with the `HONOR_*` constants from `car_config.h`
  - arms the ESC at startup, then commands forward speed from the honor-lap distance policy
- `ultrasonic.c`
  - provides periodic obstacle distance samples
  - can reduce commanded speed through the `HONOR_SLOW*` and `HONOR_STOP_*` thresholds

### Race mode path
- `app_modes.c`
  - initializes ESC, servo, camera, steering, and ultrasonic once at mode entry
  - runs a deterministic ordered loop:
    - fetch/process the newest camera frame
    - service ultrasonic timing and capture state
    - update steering and speed commands
  - rate-limits the ESC command with the existing ramp constants
  - switches from race-speed policy to honor-lap obstacle-stop policy after confirmed finish detection
  - only renders OLED telemetry when `swPcb` enables debug display

### Manual / motor path
- `onboard_pot.c`
  - samples the potentiometer through ADC
- `app_modes.c`
  - maps pot value to speed in `FINAL_DUMMY` ESC mode
  - ramps speed in `FINAL_DUMMY` camera mode
  - uses the same manual arm/run ESC flow in standalone ESC test mode and in `FINAL_DUMMY` ESC mode
  - draws an ESC test screen with arm/run state, pot level, and commanded speed in standalone ESC mode
  - exposes a standalone ultrasonic test screen and the same test inside `APP_TEST_NXP_CUP_TESTS`
  - exposes an `Ultra+ESC` runtime test in `APP_TEST_NXP_CUP_TESTS` for tuning honor-lap obstacle stopping
  - exposes a `Cam Servo` runtime test in `APP_TEST_NXP_CUP_TESTS` that follows the linear camera debug flow while also steering automatically
  - exposes a standalone `APP_TEST_NXP_CUP` mode that reuses the current `CamServo` path with a profile menu and dedicated ready / rearm / run state machine
  - uses a two-stage standalone servo test: the potentiometer first selects `RAW` or `SMOOTH`, `SW2` enters the selected mode, and then the live screen shows raw, filtered, and applied steering while `SW2` re-centers the servo
- `esc.c`
  - applies motor command through PWM and an internal ESC state machine

### Optional braking path present in repo
- `services/braking.c`
  - uses `ultrasonic.c` and `hbridge.c`
  - not part of the currently selected mode path

## Interrupt / callback-driven pieces
- `timebase.c`: `EmuTimer_Notification()`, `UsTimer_Notification()`
- `linear_camera.c`: `NewCameraFrame()`, `CameraClock()`, `CameraAdcFinished()`
  - `CameraAdcFinished()` stores one manual ADC sample per clock edge into the current frame buffer
- `ultrasonic.c`: `Icu_TimestampUltrasonicNotification()`
- `receiver.c`: `Icu_SignalNotification()`, `Gpt_Notification()`
- `esc.c`: `Esc_Period_Finished()`

## Key implementation files
- App control: `src/app/app_modes.c`, `src/app/car_config.h`
- Board bring-up: `src/app/board_init.c`
- Vision: `src/services/vision_linear_v2.c`, `src/app/vision_debug.c`
- Steering: `src/services/steering_control_linear.c`, `src/servo.c`
- Motor control: `src/esc.c`, `src/hbridge.c`
- Sensors / IO: `src/linear_camera.c`, `src/onboard_pot.c`, `src/ultrasonic.c`, `src/receiver.c`, `src/buttons.c`, `src/display.c`, `src/rgb_led.c`

## Current vision notes
- The public vision handoff is `VisionOutput_t` in `src/app/main_types.h`.
- The current finish detector is gap-based, not region-based.
- The main debug screens in `vision_debug.c` are `MAIN`, `FILT`, `GRAD`, and `FINISH`.
