# Clocks

## Reliability
- This file is based on generated clock outputs in `ClockYaml.txt`, `ClockConfigurationMappings.txt`, `Nxp_Cup.mex`, and generated RTD config.
- It is useful for clock-tree orientation and a few named timer/PWM periods.
- Do not treat it as the source of truth for pin routing or for handwritten timing code that contradicts generated config.

## Active generated clock configuration
- Mapping file selects `BOARD_BootClockRUN` for `VS_0`.

## Primary sources
| Item | Value |
| --- | --- |
| SOSC external source | 8 MHz |
| RTC external input | 32.768 kHz |

## Key generated clock outputs
| Clock | Value |
| --- | --- |
| CORE_CLK | 48 MHz |
| SYS_CLK | 48 MHz |
| BUS_CLK | 48 MHz |
| FLASH_CLK | 12 MHz |
| SPLLDIV1_CLK | 48 MHz |
| SPLLDIV2_CLK | 24 MHz |
| SIRCOUT | 8 MHz |
| SOSCOUT | 8 MHz |

## App-relevant peripheral clocks
| Peripheral clock | Value | Notes |
| --- | --- | --- |
| ADC0_CLK | 48 MHz | Used by camera / pot ADC paths |
| FTM0_CLK | 8 MHz | Generated timer clock available for FTM0 PWM channels |
| FTM1_CLK | 8 MHz | ICU config uses FTM1 for ultrasonic echo capture |
| FTM2_CLK | 8 MHz | Linear camera pixel clock PWM uses FTM2 |
| FTM3_CLK | 8 MHz | Dual ESC outputs and servo PWM use FTM3 |
| LPI2C0_CLK | 8 MHz | Display driver uses I2C channel 0 |
| LPIT0_CLK | 8 MHz | GPT channels 0/1/2 map to LPIT0 channels |

## Generated GPT channel mapping
| GPT logical channel | ID | Generated IP |
| --- | --- | --- |
| `LinearCameraShutter` | 0 | LPIT0 channel 0 |
| `EmuTimer_Notification` | 1 | LPIT0 channel 1 |
| `UsTimer` | 2 | LPIT0 channel 2 |

Handwritten code uses the generated `GptConf_GptChannelConfiguration_*` symbols
through `board_config.h`, not numeric channel literals.

## Generated PWM timing that can be derived reliably
| Logical PWM | Generated mapping | Derived output rate |
| --- | --- | --- |
| `Esc_Pwm` | FTM3 CH6, period 40000, clk 8 MHz / div 4 | `8 MHz / 4 / 40000 = 50 Hz` |
| `Esc2_Pwm` | FTM3 CH2, period 40000, clk 8 MHz / div 4 | `8 MHz / 4 / 40000 = 50 Hz` |
| `Servo_Pwm` | FTM3 CH7, period 40000, clk 8 MHz / div 4 | `8 MHz / 4 / 40000 = 50 Hz` |
| `LinearCamera_Clk` | FTM2 CH0, period 1000, clk 8 MHz / div 1 | `8 MHz / 1 / 1000 = 8 kHz` |

## Confirmed app-side timing assumptions
| Item | Source | Value |
| --- | --- | --- |
| Main clock init config | `board_init.c` + generated MCU clock config | `McuClockSettingConfig_0` |
| Buttons polling period | `src/app/app_config.h` | `5 ms` |
| Camera debug UI period | `src/app/app_config.h` | `5 ms` |
| Cam+Servo control housekeeping period | `src/app/app_config.h` | `5 ms` |
| Default display refresh period | `src/app/app_config.h` | `20 ms` |
| Camera debug display period | `src/app/app_config.h` | `100 ms` |
| Legacy/local-camera steering update period | `src/app/app_config.h` | `10 ms` |
| Teensy-camera race steering update | `src/app/modes/mode_race.c` | once per `20 ms` PWM period at phase `17 ms` |
| Servo handwritten commit window | `src/drivers/servo.c` | once per `50 Hz` PWM period while `17 <= phase < 19 ms` |
| Servo rate test software command cadence | `src/app/modes/mode_servo_rate.c` | selectable `10 / 50 / 100 / 250 Hz`; physical PWM output remains `50 Hz` |
| Race display period | `src/app/app_config.h` | `200 ms` |
| Camera shutter GPT logical channel | `include/config/board_config.h` | `LinearCameraShutter` (0) |
| Camera frame interval ticks | `include/config/sensor_config.h` | 160000 |
| Linear camera helper clock | `include/drivers/linear_camera.h` | 8 MHz source / period 1000 = 8 kHz |
| EmuTimer period | `src/drivers/timebase.c` | starts `MS_TIMER_CHANNEL` (1) with 8000 ticks for 1 ms |
| UsTimer period base | generated GPT + LPIT config, `src/drivers/timebase.c` | `US_TIMER_CHANNEL` (2) -> LPIT0 CH2 at 8 MHz; `ticksPerUs = 8` |
| Servo test update period | `src/app/app_config.h` | `5 ms` |
| Camera steering hold after line loss | `src/app/app_config.h` | `350 ms` |
| ESC arm time | `src/app/app_config.h` | `3000 ms` |
| Full-auto speed ramp period | `src/app/app_config.h` | `20 ms` |
| Ultrasonic trigger period | `include/config/sensor_config.h` | `60 ms` |
| Race-family line-loss stop threshold | `src/app/app_config.h` | `3000 ms` |

## Additional derived peripheral timing
| Item | Generated basis | Derived timing / note |
| --- | --- | --- |
| `Ultrasonic_Echo` ICU timestamp tick | FTM1 clock 8 MHz + ICU FTM prescaler 4 | `2 MHz` timestamp tick (`0.5 us` per tick) |
| `Display_Channel` I2C SCL | LPI2C0 source 8 MHz, prescaler `DIV_8`, `CLKHI=1`, `CLKLO=3` | `~166.7 kHz` (`8e6 / (8 * (1 + 3 + 2))`) |
| Camera ADC sampling cadence | handwritten `linear_camera.c` + generated ADC Group0 SW trigger | Manual one-sample ADC conversion starts on each falling camera-clock edge after the two-edge SI pulse; PWM notifications are disabled again between frames |

## Verified note
- `UsTimer` has been verified against generated config:
  - GPT logical channel `2` maps to `LPIT0 CH2`
  - `LPIT0_CLK` is `8 MHz`
  - `src/drivers/timebase.c` using `ticksPerUs = 8` is consistent with generated config
- The previous `48 MHz` wording was a handwritten comment error, not a generated-config mismatch.
