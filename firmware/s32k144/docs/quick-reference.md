# Quick Reference

## Purpose
- `description.txt` describes this as an NXP Cup S32K144 sample project for line following with a linear camera.

## Main entry points
- `src/main.c`: process entry; calls `App_RunSelectedMode()`.
- `src/app/app_modes.c`: compile-time mode dispatcher.
- `src/app/app_common.c`: shared app runtime init, status LED helpers, display text helper, and ESC logical-speed helper.
- `src/app/modes/bench_menu.c`: `APP_TEST_NXP_CUP_TESTS` runtime test menu host and standalone linear-camera wrapper.
- `src/app/modes/bench_tests.c`: reusable bench test implementations used by the runtime menu.
- `src/app/modes/bench_serial_tune.c`: UART/OLED tuning test used by the runtime menu.
- `src/app/modes/bench_teensy_link.c`: S32K-master Teensy SPI runtime test service and OLED viewer.
- `src/app/modes/mode_teensy_link.c`: direct compile-time Teensy SPI link mode for `APP_TEST_TEENSY_LINK_TEST`.
- `src/app/modes/mode_esp_link.c`: direct compile-time ESP32 UART link mode for button/ACK traffic and RAM-only full tuning receive testing.
- `src/app/modes/mode_nxp_cup.c`, `src/app/modes/mode_honor_lap.c`, `src/app/modes/mode_race.c`, `src/app/modes/mode_servo_rate.c`: standalone app mode implementations.
- `src/app/board_init.c`: RTD/MCAL driver bring-up.
- `src/app/app_config.h`: compile-time mode selection and app behavior/profile constants, including the `HONOR_*` honor-lap parameters and race-mode display / finish-confidence constants.
- `include/config/board_config.h`: generated PWM/channel routing aliases and board-specific input/device addresses.
- `include/config/actuator_config.h`: servo and ESC physical calibration values.
- `include/config/sensor_config.h`: camera geometry/timing and ultrasonic sensor calibration/validity limits.
- `include/domain/vehicle_types.h`: shared domain packets passed between vision, control, and app code.
- `include/config/vision_config.h`: vision detector tuning constants.
- `include/config/control_config.h`: baseline steering PID, steering shaping, and controller speed policy constants.

## Mode selection
- Exactly one `APP_TEST_*` flag must be enabled.
- Invalid or missing selection is a configuration error.
- `APP_TEST_LINEAR_CAMERA_TEST` remains available as a standalone compile-time mode for deterministic camera-only testing.
- `APP_TEST_NXP_CUP` is a standalone competition flow with profile selection, ready screen, ESC rearm, then camera-guided run with ultrasonic obstacle handling.
- `APP_TEST_RACE_MODE` is the standalone production race path: ESC arm, automatic line following, finish-line transition, then honor-lap obstacle stop.
- `APP_TEST_SERVO_RATE_TEST` is a standalone servo timing/debug mode for checking the period-latched servo behavior without camera or ESC activity.
- `APP_TEST_NXP_CUP_TESTS` is the compile-time mode for the rest of the interactive test screens.
- `APP_TEST_HONOR_LAP` is available as a standalone compile-time mode for automatic line following with ultrasonic obstacle slowing/stopping.
- `APP_TEST_ESP_LINK_TEST` sends CRC-protected button state to the ESP32 and
  receives one validated snapshot containing PID, servo clamp/LPF, and
  line-detector values. It stores all eight values in `g_runtimeTune` and
  replies with a compact sequence-matched result. OLED activity is temporarily
  disabled for transport isolation. Cyan indicates an accepted snapshot; red
  indicates a UART hardware, protocol, or transmit error.
- The `APP_TEST_NXP_CUP_TESTS` menu contains the individual test screens: `Camera`, `ESC`, `Servo`, `Ultrasonic`, `Cam+Servo`, `Simple test drv`, `Serial tune`, `Ultra+ESC`, `Receiver - x`, `Teensy Link`, and `Victory Lap`.
- `Servo` uses a setup step where the pot selects `RAW` or `SMOOTH`, `SW2` enters the selected mode, and the live screen then shows raw, filtered, and applied steering values.
- `APP_TEST_SERVO_RATE_TEST` uses `SW2` to cycle command rates `10/50/100/250 Hz`, `SW3` to cycle angle source `POT/FINE SWEEP/STEP SWEEP`, and the OLED shows frequency, mode, command angle, PWM callback count, plus `BUF` when commands outrun the servo latch or `NOISR` if PWM-period callbacks do not arrive.
- `Simple test drv` is the old `FINAL_DUMMY` auto-camera drive path turned into a normal runtime test: entering it initializes ESC plus camera/servo, waits through ESC arm time, then starts only after `SW3` is pressed and ramps to `FULL_AUTO_SPEED_PCT` while the camera steering loop stays active.
- `Serial tune` is the UART proof-of-concept moved into the runtime tests menu; it keeps the same OLED/UART menu flow, polls UART non-blockingly so it can still be exited through `swPcb`, and now stores tuned values in RAM for the current board-on session.
- `Camera` and `Cam+Servo` can now also stream a compact live frame packet over the same handwritten UART path for MATLAB visualization; the MATLAB viewers live in the repository-level `tools/matlab/` directory.
  - The camera still runs at full rate; UART debug streaming is intentionally downsampled by `CAM_UART_STREAM_PERIOD_MS` so the PC viewer does not fall behind.
- Camera consumers process a newly completed frame when `LinearCameraGetLatestFrame()` reports one ready; the 5 ms app constants now describe UI/control housekeeping rather than camera frame polling.
- `Cam+Servo` and `Simple test drv` now consume the current session runtime tuning block instead of always rebuilding from the compile-time `KP/KI/KD` defaults.
- Steering PID uses the active `KP/KD/KI/ITERM_CLAMP` values plus the shaping knobs `STEER_CENTER_ERR_DEADBAND`, `STEER_ERROR_LPF_ALPHA`, `STEER_D_INPUT_ALPHA`, `STEER_DTERM_LPF_ALPHA`, and `STEER_DTERM_CLAMP`; these filter the vision error and derivative inside `SteeringController_Update()` before app-level servo smoothing is applied.
- `Ultrasonic` uses a state-based diagnostic view with `WAIT`, `SCAN`, `CLEAR`, `SLOW`, and `STOP` states driven by enable delay and distance thresholds.
- `Ultra+ESC` centers the servo, drives both motors at 50% when clear, slows at 45 cm, and stops at 8 cm.
- `Victory Lap` is a separate pole-detection test: it approaches slowly, stops at 8 cm, then steps through a Mario-style victory note table for future BLDC music support.
- Honor/race ultrasonic handling now also holds the steering straight once the object is inside the 45 cm slow zone.
- The Teensy SPI link runs only inside the `Teensy Link` test. Enable `APP_TEST_TEENSY_LINK_TEST` for direct boot, or enter `Teensy Link` from the runtime bench menu.
- In `APP_TEST_RACE_MODE`, the OLED debug screen is optional, but it must be enabled during ESC arm; after the race starts, `swPcb` only controls refresh of an already-initialized display.

## Major modules
- App dispatch/shared glue: `src/app/app_modes.c`, `src/app/app_common.c`, `src/app/app_internal.h`
- App modes: `src/app/modes/bench_menu.c`, `src/app/modes/bench_tests.c`, `src/app/modes/bench_serial_tune.c`, `src/app/modes/bench_teensy_link.c`, `src/app/modes/mode_nxp_cup.c`, `src/app/modes/mode_honor_lap.c`, `src/app/modes/mode_race.c`, `src/app/modes/mode_servo_rate.c`
- App vision debug UI: `src/app/vision_debug.c`
- Vision / control: `src/services/line_detector.c`, `src/services/steering_controller.c`, `src/services/steering_smoothing.c`
- Hardware-facing modules: `src/drivers/linear_camera.c`, `src/drivers/esc.c`, `src/drivers/servo.c`, `src/drivers/onboard_pot.c`, `src/drivers/ultrasonic.c`, `src/drivers/receiver.c`, `src/drivers/display.c`, `src/drivers/buttons.c`, `src/drivers/rgb_led.c`, `src/drivers/timebase.c`
- Debug transport: `src/debug/uart_host_link.c`
- Teensy SPI link: `src/drivers/teensy_link.c`, `src/app/modes/bench_teensy_link.c`, and `src/app/modes/mode_teensy_link.c`
- Unused retained modules: `src/unused/user_interface.c` and `src/unused/display_async.c` are not called by the active runtime; active menu/HUD code lives in `src/app/modes/bench_menu.c` and the mode-specific app modules, and active OLED writes use `src/drivers/display.c`.

## Vision V2 snapshot
- Shared output packet: `VisionOutput_t` in `include/domain/vehicle_types.h`
- Processing pipeline in `src/services/line_detector.c`:
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
- `src/app/`: application dispatcher, shared app runtime helpers, private app header, app config, and board init
- `src/app/modes/`: standalone app modes and runtime bench-mode implementations
- `src/services/`: vision and control logic
- `src/debug/`: debug/tuning transports and instrumentation helpers
- `include/drivers/`: public hardware-facing module APIs
- `include/services/`: public service/algorithm APIs
- `include/debug/`: public debug/tuning transport APIs
- `include/domain/`: shared domain packets
- `include/config/`: shared compile-time configuration and defaults
- `src/unused/`: retained inactive modules not called by the active runtime
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
