# Peripherals

## ADC
| Purpose | Driver/module | Key files |
| --- | --- | --- |
| Linear camera pixel sampling | `Adc` group conversion + notification | `src/linear_camera.c`, `include/linear_camera.h`, `src/app/board_init.c` |
| Onboard potentiometer sampling | `Adc` blocking group conversion | `src/onboard_pot.c`, `include/onboard_pot.h` |

Notes:
- `board_init.c` performs ADC init and calibration.
- `onboard_pot.c` references generated group `AdcGroup_pot`.

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
- Handwritten camera timing now targets a 10 kHz pixel clock with about 50 fps once the FTM2 PWM period is regenerated from 1000 to 800 ticks.

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

## I2C
| Purpose | Driver/module | Key files |
| --- | --- | --- |
| OLED display writes | `CDD_I2c` | `src/display.c`, `include/display.h`, `src/app/board_init.c` |

Notes:
- `display.c` uses I2C address `0x3C`.
- The display code is written for a 128x32 SSD1306-style display.

## DIO / GPIO
| Purpose | Driver/module | Key files |
| --- | --- | --- |
| Buttons SW2/SW3 | `Dio` | `src/buttons.c`, `include/buttons.h` |
| RGB LED | `Dio` | `src/rgb_led.c`, `include/rgb_led.h` |
| Ultrasonic trigger | `Dio` | `src/ultrasonic.c`, `include/ultrasonic.h` |
| Linear camera shutter line | `Dio` | `src/linear_camera.c`, `src/app/car_config.h` |

## Not documented here
- UART / CAN / SPI are not documented because their use was not confirmed in handwritten project code.
