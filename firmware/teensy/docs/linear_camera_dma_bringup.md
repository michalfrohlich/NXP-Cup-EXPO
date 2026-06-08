# Teensy TSL1401CL DMA Bring-Up

This bring-up mode validates camera 0 on the Teensy 4.1 using the TSL1401CL
linear CCD.

## Wiring

| Camera signal | Teensy 4.1 pin |
|---|---:|
| SI | 4 |
| CLK | 6 |
| AO | 15 / A1 |
| GND | GND |
| VDD | 3.3 V |

## Current Timing

The active bring-up timing is configured in
`include/config/camera_config.h`.

| Parameter | Value |
|---|---:|
| Pixel clock | 200 kHz |
| Frame rate | 200 Hz |
| Frame period | 5000 us |
| Readout clocks | 129 |
| Readout window | about 645 us |
| Published pixels | 128 |

The extra readout clock is kept so the driver can discard the trailing transfer
sample while publishing the 128 camera pixels.

## Hardware Path

The driver uses the Teensy 4.1 hardware trigger path:

```text
FlexPWM2 SM2 CLK on pin 6
  -> XBARA1
  -> ADC_ETC trigger 0
  -> ADC1 channel for pin 15 / A1
  -> DMA buffer
  -> 128-pixel published frame
```

The CPU does not service every pixel. `LinearCamera::service()` starts a frame
when the frame period expires, returns immediately after SI/CLK/DMA are armed,
then later observes DMA completion and publishes the frame.

One important limitation remains: CLK is stopped in software when
`service()` observes DMA completion. The PWM burst is therefore expected to be
stable only if the foreground loop calls `service()` frequently. The debug
counters expose late service as a larger `capUs` / `capClk` value or as
`overrun` increments.

## Bring-Up Mode

The selected app mode is controlled in `src/app/teensy_app_config.h`.

```cpp
#define TEENSY_APP_MODE_CAMERA_BRINGUP    1
#define TEENSY_APP_SELECTED_MODE TEENSY_APP_MODE_CAMERA_BRINGUP
```

The camera debug serial controls are in `include/config/camera_config.h`:

```cpp
TEENSY_LINEAR_CAMERA_DEBUG_ENABLED
TEENSY_LINEAR_CAMERA_DEBUG_SERIAL_BAUD
TEENSY_LINEAR_CAMERA_DEBUG_PRINT_PERIOD_MS
TEENSY_LINEAR_CAMERA_DEBUG_PRINT_FRAME8
```

For line-following integration, disable serial camera debug with:

```cpp
static constexpr bool TEENSY_LINEAR_CAMERA_DEBUG_ENABLED = false;
```

## Expected Debug Output

At 200 Hz, consecutive 1 Hz debug lines should show:

- `req`, `si`, `win`, `ready`, and `dma` increasing by about 200.
- `samples` increasing by `200 * 128 = 25600`.
- `drop=0`, `overrun=0`, `adcErr=0`, and `dmaErr=0`.
- `capUs` close to the readout window plus small software overhead.
- `capClk` close to 129. A value around 130 is acceptable because it is derived
  from software-side capture duration, not a direct pulse counter.

Bright light should push ADC values near full scale (`max` close to 4095 and
`frame8` near 254/255). Fully dark input should produce a low, mostly uniform
baseline.

## Scope Checks

With the current config:

- SI period: 5 ms.
- CLK frequency: 200 kHz.
- CLK burst length: about 645 us.
- Exactly one CLK rising edge occurs while SI is high.
- AO changes with illumination and stays within the Teensy ADC input range.

