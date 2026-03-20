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
| FTM0_CLK | 8 MHz | PWM/H-bridge related generated config exists |
| FTM1_CLK | 8 MHz | ICU config uses FTM1 for ultrasonic echo capture |
| FTM2_CLK | 8 MHz | Linear camera pixel clock PWM uses FTM2 |
| FTM3_CLK | 8 MHz | ESC and servo PWM use FTM3 |
| LPI2C0_CLK | 8 MHz | Display driver uses I2C channel 0 |
| LPIT0_CLK | 8 MHz | GPT channels 1/2/3 map to LPIT0 channels |
| LPTMR0_CLK | 48 MHz | GPT receiver timeout uses LPTMR0; with configured prescaler divider `/4` (ConfigTools), effective timer tick is 12 MHz (`48 MHz / 4`) |

## Generated GPT channel mapping
| GPT logical channel | ID | Generated IP |
| --- | --- | --- |
| `Receiver_Timeout` | 0 | LPTMR0 channel 0 |
| `LinearCameraShutter` | 1 | LPIT0 channel 0 |
| `EmuTimer_Notification` | 2 | LPIT0 channel 1 |
| `UsTimer` | 3 | LPIT0 channel 2 |

## Generated PWM timing that can be derived reliably
| Logical PWM | Generated mapping | Derived output rate |
| --- | --- | --- |
| `Esc_Pwm` | FTM3 CH6, period 40000, clk 8 MHz / div 4 | `8 MHz / 4 / 40000 = 50 Hz` |
| `Servo_Pwm` | FTM3 CH7, period 40000, clk 8 MHz / div 4 | `8 MHz / 4 / 40000 = 50 Hz` |
| `Motor_1_Speed` | FTM0 CH1, period 2000, clk 8 MHz / div 4 | `8 MHz / 4 / 2000 = 1 kHz` |
| `Motor_2_Speed` | FTM0 CH2, period 2000, clk 8 MHz / div 4 | `8 MHz / 4 / 2000 = 1 kHz` |
| `LinearCamera_Clk` | FTM2 CH0, period 1000, clk 8 MHz / div 1 | `8 MHz / 1 / 1000 = 8 kHz` |

## Confirmed app-side timing assumptions
| Item | Source | Value |
| --- | --- | --- |
| Main clock init config | `board_init.c` + generated MCU clock config | `McuClockSettingConfig_0` |
| Camera shutter GPT logical channel | `src/app/car_config.h` | 1 |
| Camera frame interval ticks | `src/app/car_config.h` | 177778 |
| Camera shutter high ticks | `src/app/car_config.h` | 1000 |
| Linear camera helper clock | `include/linear_camera.h` | 8 MHz source / period 1000 |
| EmuTimer period | `src/timebase.c` | starts GPT ch2 with 8000 ticks for 1 ms |
| UsTimer period base | generated GPT + LPIT config, `src/timebase.c` | GPT ch3 -> LPIT0 CH2 at 8 MHz, so `ticksPerUs = 8` is correct |
| Receiver PPM thresholds | `src/app/app_modes.c` (`ReceiverInit(0U, 0U, 11700U, 17700U, 23700U, 26000U)`) | At effective 12 MHz LPTMR tick: `0.975 / 1.475 / 1.975 / 2.167 ms` |

## Additional derived peripheral timing
| Item | Generated basis | Derived timing / note |
| --- | --- | --- |
| `Receiver_Timeout` effective tick | LPTMR0 clock 48 MHz + receiver channel prescaler enabled with configured divider `/4` | `12 MHz` effective timer tick (`48 MHz / 4`) |
| `Ultrasonic_Echo` ICU timestamp tick | FTM1 clock 8 MHz + ICU FTM prescaler 4 | `2 MHz` timestamp tick (`0.5 us` per tick) |
| `Display_Channel` I2C SCL | LPI2C0 source 8 MHz, prescaler `DIV_8`, `CLKHI=1`, `CLKLO=3` | `~166.7 kHz` (`8e6 / (8 * (1 + 3 + 2))`) |
| Camera ADC sampling cadence | handwritten `linear_camera.c` + generated ADC Group0 SW trigger | Manual one-sample ADC conversion started on each camera clock falling edge after the SI pulse |

## Verified note
- `UsTimer` has been verified against generated config:
  - GPT logical channel `3` maps to `LPIT0 CH2`
  - `LPIT0_CLK` is `8 MHz`
  - `src/timebase.c` using `ticksPerUs = 8` is consistent with generated config
- The previous `48 MHz` wording was a handwritten comment error, not a generated-config mismatch.
