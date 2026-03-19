# Peripherals

## ADC
| Purpose | Driver/module | Key files |
| --- | --- | --- |
| Linear camera pixel sampling | `Adc` software-triggered single-sample group with per-sample notification | `src/linear_camera.c`, `include/linear_camera.h`, `src/app/board_init.c` |
| Onboard potentiometer sampling | `Adc` blocking group conversion | `src/onboard_pot.c`, `include/onboard_pot.h` |

Notes:
- `board_init.c` performs ADC init and calibration.
- `onboard_pot.c` references generated group `AdcGroup_pot`.
- The linear camera ADC group is back to a software-triggered one-sample conversion path.
- The handwritten camera driver starts one ADC conversion on each falling camera clock edge after the SI pulse.
- If a new GPT frame request arrives before the current frame is done, the driver latches it and starts the next frame only after the current readout completes.
- Each ADC callback stores one pixel into a handwritten ping-pong frame buffer.
- The OLED waiting screen uses a neutral driver debug-counters snapshot instead of exposing ADC/DMA-specific counter getters.

## PWM
| Purpose | Driver/module | Key files |
| --- | --- | --- |
| ESC command output | `Pwm` | `src/esc.c`, `src/app/car_config.h` |
| Servo steering output | `Pwm` | `src/servo.c`, `src/app/car_config.h` |
| Linear camera pixel clock | `Pwm` notification-driven clocking | `src/linear_camera.c`, `src/app/car_config.h` |
| H-bridge motor speed | `Pwm` | `src/hbridge.c`, `include/hbridge.h` |

Notes:
- Logical PWM channel IDs used in handwritten code are defined in `src/app/car_config.h`.
- The underlying timer/IP mapping is generated configuration and should be checked in `generate/` or `Nxp_Cup.mex` before changing it.
- The linear camera PWM output free-runs as the pixel clock; notifications are enabled only briefly to place the SI pulse on two falling edges.

## Timers / capture
| Purpose | Driver/module | Key files |
| --- | --- | --- |
| 1 ms system tick | `Gpt` | `src/timebase.c` |
| Microsecond one-shot delay | `Gpt` | `src/timebase.c` |
| Linear camera shutter timing | `Gpt` | `src/linear_camera.c`, `src/app/car_config.h` |
| Receiver sync / period timing | `Gpt` + `Icu` | `src/receiver.c`, `include/receiver.h` |
| Ultrasonic echo measurement | `Icu` timestamp capture | `src/ultrasonic.c`, `include/ultrasonic.h` |

Notes:
- `timebase.c` documents GPT channel `2` for the ms tick and channel `3` for the microsecond helper.
- `ultrasonic.h` defines `ULTRA_ICU_ECHO_CHANNEL` and trigger DIO channel macros.
- `APP_TEST_HONOR_LAP` uses `ultrasonic.c` directly from `app_modes.c` to trigger measurements, consume fresh distance samples, and slow/stop the ESC command.
- `APP_TEST_RACE_MODE` uses the same ultrasonic driver path, but only after finish detection switches the speed policy into the honor-lap stopping thresholds.
- The standalone ultrasonic test mode and the `APP_TEST_NXP_CUP_TESTS` ultrasonic menu item both use `ultrasonic.c` directly from `app_modes.c` for the 500 ms diagnostic screen.

## I2C
| Purpose | Driver/module | Key files |
| --- | --- | --- |
| OLED display writes | `CDD_I2c` | `src/display.c`, `include/display.h`, `src/app/board_init.c` |
| OLED display non-blocking transport (optional) | `CDD_I2c` async state machine | `src/display_async.c`, `include/display_async.h` |

Notes:
- `display.c` uses I2C address `0x3C`.
- The display code is written for a 128x32 SSD1306-style display.
- `display_async.c` adds double-buffered non-blocking refresh (`I2c_AsyncTransmit` + `I2c_GetStatus`) with latest-frame-wins queuing.
- In `APP_TEST_RACE_MODE`, OLED initialization is allowed only during the ESC-arm phase. After that, `swPcb` can only enable refreshes on an already-initialized display, so the blocking display bring-up cannot stall the active race loop.

## DIO / GPIO
| Purpose | Driver/module | Key files |
| --- | --- | --- |
| Buttons SW2/SW3 and toggle switch `swPcb` | `Dio` | `src/buttons.c`, `include/buttons.h` |
| RGB LED | `Dio` | `src/rgb_led.c`, `include/rgb_led.h` |
| Ultrasonic trigger | `Dio` | `src/ultrasonic.c`, `include/ultrasonic.h` |
| Linear camera shutter line | `Dio` | `src/linear_camera.c`, `src/app/car_config.h` |

Notes:
- `swPcb` is generated as GPIO input on `PTA12` and is consumed from handwritten code through the `buttons` module.
- The `buttons` module now distinguishes momentary buttons from maintained toggle switches; `swPcb` is level-based only and does not publish click events.
- `APP_TEST_NXP_CUP_TESTS` uses `swPcb` to enter/leave the selected test, while `APP_TEST_RACE_MODE` uses it only to enable optional OLED telemetry.

## Not documented here
- UART / CAN / SPI are not documented because their use was not confirmed in handwritten project code.
