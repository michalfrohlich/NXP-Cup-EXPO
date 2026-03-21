# Quick Reference

## Purpose
- `description.txt` describes this as an NXP Cup S32K144 sample project for line following with a linear camera.

## Main entry points
- `src/main.c`: process entry; calls `App_RunSelectedMode()`.
- `src/app/app_modes.c`: top-level mode dispatcher and main loops.
- `src/app/board_init.c`: RTD/MCAL driver bring-up and common hardware init.
- `src/app/car_config.h`: compile-time mode selection and main tuning constants.

## Current mode selection
- Exactly one `APP_TEST_*` flag must be enabled.
- Current repository state selects `APP_TEST_FINAL_DUMMY = 1`.

## Major modules
- App orchestration: `src/app/app_modes.c`, `src/app/user_interface.c`, `src/app/vision_debug.c`
- Vision / control: `src/services/vision_linear_v2.c`, `src/services/steering_control_linear.c`, `src/services/braking.c`
- Hardware-facing modules: `src/linear_camera.c`, `src/esc.c`, `src/servo.c`, `src/onboard_pot.c`, `src/ultrasonic.c`, `src/receiver.c`, `src/display.c`, `src/buttons.c`, `src/rgb_led.c`, `src/timebase.c`, `src/hbridge.c`

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
- Whether the current `APP_TEST_FINAL_DUMMY` mode is the intended default for future work
