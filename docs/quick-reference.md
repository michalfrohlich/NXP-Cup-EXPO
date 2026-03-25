# Quick Reference

## Purpose
- `description.txt` describes this as an NXP Cup S32K144 sample project for line following with a linear camera.

## Main entry points
- `src/main.c`: process entry; calls `App_RunSelectedMode()`.
- `src/app/app_modes.c`: compile-time mode dispatcher plus the `APP_TEST_NXP_CUP_TESTS` runtime test menu and the standalone `APP_TEST_NXP_CUP` competition flow.
- `src/app/board_init.c`: RTD/MCAL driver bring-up and common hardware init.
- `src/app/car_config.h`: compile-time mode selection and main tuning constants, including the `HONOR_*` honor-lap parameters and race-mode display / finish-confidence constants.

## Mode selection
- Exactly one `APP_TEST_*` flag must be enabled.
- Invalid or missing selection is a configuration error.
- Current repository state selects `APP_TEST_NXP_CUP_TESTS = 1`.
- `APP_TEST_LINEAR_CAMERA_TEST` remains available as a standalone compile-time mode for deterministic camera-only testing.
- `APP_TEST_NXP_CUP` is a standalone competition flow with profile selection, ready screen, ESC rearm, then camera-guided run with ultrasonic obstacle handling.
- `APP_TEST_RACE_MODE` is the standalone production race path: ESC arm, automatic line following, finish-line transition, then honor-lap obstacle stop.
- `APP_TEST_NXP_CUP_TESTS` is the compile-time mode for the rest of the interactive test screens.
- `APP_TEST_HONOR_LAP` is available as a standalone compile-time mode for automatic line following with ultrasonic obstacle slowing/stopping.
- The `APP_TEST_NXP_CUP_TESTS` menu contains the individual test screens: `Camera`, `ESC`, `Servo`, `Ultrasonic`, `Cam+Servo`, `Simple test drv`, `Serial tune`, `Ultra+ESC`, and `Receiver - x`.
- `Servo` uses a setup step where the pot selects `RAW` or `SMOOTH`, `SW2` enters the selected mode, and the live screen then shows raw, filtered, and applied steering values.
- `Simple test drv` is the old `FINAL_DUMMY` auto-camera drive path turned into a normal runtime test: entering it initializes ESC plus camera/servo, waits through ESC arm time, then starts only after `SW3` is pressed and ramps to `FULL_AUTO_SPEED_PCT` while the camera steering loop stays active.
- `Serial tune` is the UART proof-of-concept moved into the runtime tests menu; it keeps the same OLED/UART menu flow, polls UART non-blockingly so it can still be exited through `swPcb`, and now stores tuned values in RAM for the current board-on session.
- `Camera` and `Cam+Servo` can now also stream a compact live frame packet over the same handwritten UART path for MATLAB visualization; the MATLAB viewers live in `tools/matlab/linear_camera_dashboard_viewer.m` and `tools/matlab/linear_camera_single_graph_viewer.m`.
  - The camera still runs at full rate; UART debug streaming is intentionally downsampled by `CAM_UART_STREAM_PERIOD_MS` so the PC viewer does not fall behind.
- `Cam+Servo` and `Simple test drv` now consume the current session runtime tuning block instead of always rebuilding from the compile-time `KP/KI/KD` defaults.
- `Ultrasonic` uses a state-based diagnostic view with `WAIT`, `SCAN`, `CLEAR`, `SLOW`, and `STOP` states driven by enable delay and distance thresholds.
- In `APP_TEST_RACE_MODE`, the OLED debug screen is optional, but it must be enabled during ESC arm; after the race starts, `swPcb` only controls refresh of an already-initialized display.

## Major modules
- App orchestration: `src/app/app_modes.c`, `src/app/user_interface.c`, `src/app/vision_debug.c`
- Vision / control: `src/services/vision_linear_v2.c`, `src/services/steering_control_linear.c`, `src/services/steering_smoothing.c`, `src/services/braking.c`
- Hardware-facing modules: `src/linear_camera.c`, `src/esc.c`, `src/servo.c`, `src/onboard_pot.c`, `src/ultrasonic.c`, `src/receiver.c`, `src/display.c`, `src/buttons.c`, `src/rgb_led.c`, `src/timebase.c`, `src/hbridge.c`, `src/services/serial_debug.c`

## Vision V2 snapshot
- Shared output packet: `VisionOutput_t` in `src/app/main_types.h`
- Processing pipeline in `src/services/vision_linear_v2.c`:
  - filtered signal
  - signed gradient
  - edge candidates
  - lane selection
  - finish-gap detection
- Single-edge recovery keeps estimating lane center, but confidence is reduced after a configurable streak limit.
- Finish-line acceptance is constrained by expected gap width and by gap midpoint staying close to the lane midpoint.
- Debug screens in `src/app/vision_debug.c`: `MAIN`, `FILT`, `GRAD`, `FINISH`
- `MAIN` and graph/debug screens can overlay finish-gap edge markers when the detector finds them.

## Confirmed peripheral usage
- ADC: onboard potentiometer and linear camera sampling
- PWM: ESC, servo, camera pixel clock, H-bridge speed outputs
- GPT: millisecond timebase, microsecond delay, camera shutter timing, receiver timing support
- ICU: ultrasonic echo timestamping, receiver signal capture
- I2C: SSD1306-style OLED display (`0x3C`)
- DIO/GPIO: buttons, RGB LED, ultrasonic trigger, camera shutter

## Important directories
- `src/`, `include/`: handwritten hardware modules
- `src/app/`: application modes and board init
- `src/services/`: vision and control logic
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
