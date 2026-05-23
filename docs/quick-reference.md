# Quick Reference

## Purpose
- `description.txt` describes this as an NXP Cup S32K144 sample project for line following with a linear camera.

## Main entry points
- `src/main.c`: process entry; calls `App_RunSelectedMode()`.
- `src/app/app_modes.c`: compile-time mode dispatcher plus the `APP_TEST_NXP_CUP_TESTS` runtime test menu and the standalone `APP_TEST_NXP_CUP` competition flow.
- `src/app/board_init.c`: RTD/MCAL driver bring-up.
- `src/app/car_config.h`: compile-time mode selection, generated PWM/channel routing aliases, and app/profile constants, including the `HONOR_*` honor-lap parameters and race-mode display / finish-confidence constants.
- `include/domain/main_types.h`: shared domain packets passed between vision, control, and app code.
- `include/config/camera_config.h`: shared camera frame timing and geometry constants used by the camera driver and vision processing.
- `include/config/vision_config.h`: vision detector tuning constants.
- `include/config/control_defaults.h`: default steering PID, steering shaping, and controller speed policy constants.

## Mode selection
- Exactly one `APP_TEST_*` flag must be enabled.
- Invalid or missing selection is a configuration error.
- `APP_TEST_LINEAR_CAMERA_TEST` remains available as a standalone compile-time mode for deterministic camera-only testing.
- `APP_TEST_NXP_CUP` is a standalone competition flow with profile selection, ready screen, ESC rearm, then camera-guided run with ultrasonic obstacle handling.
- `APP_TEST_RACE_MODE` is the standalone production race path: ESC arm, automatic line following, finish-line transition, then honor-lap obstacle stop.
- `APP_TEST_SERVO_RATE_TEST` is a standalone servo timing/debug mode for checking the period-latched servo behavior without camera or ESC activity.
- `APP_TEST_NXP_CUP_TESTS` is the compile-time mode for the rest of the interactive test screens.
- `APP_TEST_HONOR_LAP` is available as a standalone compile-time mode for automatic line following with ultrasonic obstacle slowing/stopping.
- The `APP_TEST_NXP_CUP_TESTS` menu contains the individual test screens: `Camera`, `ESC`, `Servo`, `Ultrasonic`, `Cam+Servo`, `Simple test drv`, `Serial tune`, `Ultra+ESC`, and `Receiver - x`.
- `Servo` uses a setup step where the pot selects `RAW` or `SMOOTH`, `SW2` enters the selected mode, and the live screen then shows raw, filtered, and applied steering values.
- `APP_TEST_SERVO_RATE_TEST` uses `SW2` to cycle command rates `10/50/100/250 Hz`, `SW3` to cycle angle source `POT/FINE SWEEP/STEP SWEEP`, and the OLED shows frequency, mode, command angle, PWM callback count, plus `BUF` when commands outrun the servo latch or `NOISR` if PWM-period callbacks do not arrive.
- `Simple test drv` is the old `FINAL_DUMMY` auto-camera drive path turned into a normal runtime test: entering it initializes ESC plus camera/servo, waits through ESC arm time, then starts only after `SW3` is pressed and ramps to `FULL_AUTO_SPEED_PCT` while the camera steering loop stays active.
- `Serial tune` is the UART proof-of-concept moved into the runtime tests menu; it keeps the same OLED/UART menu flow, polls UART non-blockingly so it can still be exited through `swPcb`, and now stores tuned values in RAM for the current board-on session.
- `Camera` and `Cam+Servo` can now also stream a compact live frame packet over the same handwritten UART path for MATLAB visualization; the MATLAB viewers live in `tools/matlab/linear_camera_dashboard_viewer.m` and `tools/matlab/linear_camera_single_graph_viewer.m`.
  - The camera still runs at full rate; UART debug streaming is intentionally downsampled by `CAM_UART_STREAM_PERIOD_MS` so the PC viewer does not fall behind.
- Camera consumers process a newly completed frame when `LinearCameraGetLatestFrame()` reports one ready; the 5 ms app constants now describe UI/control housekeeping rather than camera frame polling.
- `Cam+Servo` and `Simple test drv` now consume the current session runtime tuning block instead of always rebuilding from the compile-time `KP/KI/KD` defaults.
- Steering PID uses the active `KP/KD/KI/ITERM_CLAMP` values plus the shaping knobs `STEER_CENTER_ERR_DEADBAND`, `STEER_ERROR_LPF_ALPHA`, `STEER_D_INPUT_ALPHA`, `STEER_DTERM_LPF_ALPHA`, and `STEER_DTERM_CLAMP`; these filter the vision error and derivative inside `SteeringLinear_UpdateV2()` before app-level servo smoothing is applied.
- `Ultrasonic` uses a state-based diagnostic view with `WAIT`, `SCAN`, `CLEAR`, `SLOW`, and `STOP` states driven by enable delay and distance thresholds.
- In `APP_TEST_RACE_MODE`, the OLED debug screen is optional, but it must be enabled during ESC arm; after the race starts, `swPcb` only controls refresh of an already-initialized display.

## Major modules
- App orchestration: `src/app/app_modes.c`, `src/app/vision_debug.c`
- Vision / control: `src/services/vision_linear_v2.c`, `src/services/steering_control_linear.c`, `src/services/steering_smoothing.c`
- Hardware-facing modules: `src/drivers/linear_camera.c`, `src/drivers/esc.c`, `src/drivers/servo.c`, `src/drivers/onboard_pot.c`, `src/drivers/ultrasonic.c`, `src/drivers/receiver.c`, `src/drivers/display.c`, `src/drivers/buttons.c`, `src/drivers/rgb_led.c`, `src/drivers/timebase.c`
- Debug transport: `src/debug/serial_debug.c`
- Unused retained modules: `src/unused/user_interface.c` and `src/unused/display_async.c` are excluded from the current CLI build; active runtime menu/HUD code lives in `src/app/app_modes.c`, and active OLED writes use `src/drivers/display.c`.

## Vision V2 snapshot
- Shared output packet: `VisionOutput_t` in `include/domain/main_types.h`
- Processing pipeline in `src/services/vision_linear_v2.c`:
  - filtered signal
  - signed gradient
  - edge candidates
  - lane selection
  - confidence from selected-edge strength, contrast, and line status
  - finish-gap detection
- Finish-line acceptance is constrained by expected gap width and by gap midpoint staying close to the lane midpoint.
- Debug screens in `src/app/vision_debug.c`: `MAIN`, `FILT`, `GRAD`, `FINISH`
- `MAIN` and graph/debug screens can overlay finish-gap edge markers when the detector finds them.

## Confirmed peripheral usage
- ADC: onboard potentiometer and linear camera sampling
- PWM: dual ESC outputs, servo, camera pixel clock
- GPT: millisecond timebase, microsecond delay, camera shutter timing, receiver timing support
- ICU: ultrasonic echo timestamping, receiver signal capture
- I2C: SSD1306-style OLED display (`0x3C`)
- DIO/GPIO: buttons, RGB LED, ultrasonic trigger, camera shutter

## Important directories
- `src/`, `include/`: handwritten hardware modules
- `src/drivers/`: hardware-facing driver implementations
- `src/app/`: application modes and board init
- `src/services/`: vision and control logic
- `src/debug/`: debug/tuning transports and instrumentation helpers
- `include/drivers/`: public hardware-facing module APIs
- `include/services/`: public service/algorithm APIs
- `include/debug/`: public debug/tuning transport APIs
- `include/domain/`: shared domain packets
- `include/config/`: shared compile-time configuration and defaults
- `src/unused/`: retained inactive modules excluded from the current managed make build
- `generate/`: generated RTD/MCAL configuration
- `board/`: generated board/pin configuration
- `RTD/`: local RTD driver sources/headers
- `Project_Settings/`: linker, startup, debugger files

## Generated / config files
- `Nxp_Cup.mex`: S32 Config Tools source for pins, clocks, peripherals
- `.cproject`, `.project`: S32 Design Studio project configuration
- `generate/include`, `generate/src`, `board/`: generated files

## Unknowns to verify manually
- Exact physical wiring for each logical channel beyond what is named in code/config
- Whether all present modules are wired on the current hardware build (`receiver.c` notes the receiver path is not physically connected yet)
- Whether `swPcb` active/on corresponds to a high or low logic level on the assembled board; current handwritten code assumes active-high
