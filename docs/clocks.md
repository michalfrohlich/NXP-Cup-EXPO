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
| CORE_CLK | 80 MHz |
| SYS_CLK | 80 MHz |
| BUS_CLK | 40 MHz |
| FLASH_CLK | 20 MHz |
| SPLLDIV1_CLK | 80 MHz |
| SPLLDIV2_CLK | 40 MHz |
| SIRCOUT | 8 MHz |
| SOSCOUT | 8 MHz |

## App-relevant peripheral clocks
| Peripheral clock | Value | Notes |
| --- | --- | --- |
| ADC0_CLK | 48 MHz | Used by camera / pot ADC paths |
| FTM0_CLK | 8 MHz | PWM/H-bridge related generated config exists |
| FTM1_CLK | 8 MHz | ICU config uses FTM1 for ultrasonic echo capture |
| FTM2_CLK | 8 MHz | Linear camera pixel clock PWM uses FTM2 |
| FTM3_CLK | 8 MHz | ESC and servo PWM use FTM3 |
| LPI2C0_CLK | 8 MHz | Display driver uses I2C channel 0 |
| LPIT0_CLK | 8 MHz | GPT channels 1/2/3 map to LPIT0 channels |
| LPTMR0_CLK | 48 MHz | GPT receiver timeout channel uses LPTMR0 |

## Generated GPT channel mapping
| GPT logical channel | ID | Generated IP |
| --- | --- | --- |
| `Receiver_Timeout` | 0 | LPTMR0 channel 0 |
| `LinearCameraShutter` | 1 | LPIT0 channel 0 |
| `EmuTimer_Notification` | 2 | LPIT0 channel 1 |
| `UsTimer` | 3 | LPIT0 channel 2 |

## Generated PWM timing that can be derived reliably
| Logical PWM | Generated mapping | Derived timer rate |
| --- | --- | --- |
| `Esc_Pwm` | FTM3 CH6, period 40000, clk 8 MHz / div 4 | 50 Hz |
| `Servo_Pwm` | FTM3 CH7, period 40000, clk 8 MHz / div 4 | 50 Hz |
| `LinearCamera_Clk` | FTM2 CH0, currently generated as period 1000, clk 8 MHz / div 1 | 8 kHz now; regenerate PWM to period 800 for 10 kHz |

## Confirmed app-side timing assumptions
| Item | Source | Value |
| --- | --- | --- |
| Main clock init config | `board_init.c` + generated MCU clock config | `McuClockSettingConfig_0` |
| Camera shutter GPT logical channel | `src/app/car_config.h` | 1 |
| Camera frame interval ticks | `src/app/car_config.h` | 56700 |
| Camera shutter high ticks | `src/app/car_config.h` | 100 |
| Linear camera helper clock | `include/linear_camera.h` | 8 MHz source / period 800 (10 kHz target after PWM regen) |
| EmuTimer period | `src/timebase.c` | starts GPT ch2 with 8000 ticks for 1 ms |
| UsTimer period base | generated GPT + LPIT config, `src/timebase.c` | GPT ch3 -> LPIT0 CH2 at 8 MHz, so `ticksPerUs = 8` is correct |

## Verified note
- `UsTimer` has been verified against generated config:
  - GPT logical channel `3` maps to `LPIT0 CH2`
  - `LPIT0_CLK` is `8 MHz`
  - `src/timebase.c` using `ticksPerUs = 8` is consistent with generated config
- The previous `48 MHz` wording was a handwritten comment error, not a generated-config mismatch.
