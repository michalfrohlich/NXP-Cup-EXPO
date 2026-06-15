# Teensy Linear Camera DMA Bench

This bench validates camera 0 on the Teensy 4.1 with the TSL1401CL linear CCD.
It is selected in `src/app/teensy_app_config.h` as
`TEENSY_APP_MODE_CAMERA_BENCH`.

## Wiring

| Camera signal | Teensy 4.1 pin |
|---|---:|
| SI | 4 |
| CLK | 6 |
| AO | 15 / A1 |
| GND | GND |
| VDD | 3.3 V |

## Timing

The active timing is configured in `include/config/camera_config.h`.

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

```text
FlexPWM2 SM2 CLK on pin 6
  -> XBARA1
  -> ADC_ETC trigger 0
  -> ADC1 channel for pin 15 / A1
  -> DMA buffer
  -> 128-pixel published frame
```

The CPU does not service every pixel. `LinearCamera::service()` starts a frame
when the frame period expires, returns after SI/CLK/DMA are armed, then later
observes DMA completion and publishes the frame.

One limitation remains: CLK is stopped in software when `service()` observes DMA
completion. The PWM burst is therefore stable only if the foreground loop calls
`service()` frequently. Late service shows up as larger `capUs` / `capClk`
values or as `overrun` increments.

## Bench Outputs

The camera bench is intended for raw camera validation without the S32K link
runtime. Current defaults:

| Output | Default | Notes |
|---|---:|---|
| Binary `TCM1` Python stream | enabled | Sends a raw 128-sample frame every 10 ms. |
| Text status line | disabled | Avoids extra USB serial load. |
| `frame8` text dump | disabled | Avoids long formatted serial bursts. |
| OLED camera debug | disabled | Avoids I2C timing disturbance while validating signal stability. |

The Python stream uses raw 12-bit ADC values in the `0..4095` range. In camera
bench mode the stream health fields are valid and the detector fields are
present, but vision validity is only meaningful in the integrated runtime where
`CameraVision` is serviced.

Run the viewer from the repository root:

```bat
python -m live_camera_viewer --port COM12 --baud 921600
```

Or specify a different GUI backend:

```bat
python -m live_camera_viewer --port COM12 --baud 921600 --gui camera_2d
```

Available GUI frontends:
- `classic`: Traditional 1D line plot visualization (plots.py)
- `camera_2d`: 2D camera frame visualization (camera_2d_view.py)

The selected GUI keeps the latest received frame, redraws at its own display
cadence, and plots stream health so missed packets or slow redraws are visible.

## Vision Runtime

`TEENSY_APP_MODE_LINK_BENCH` and `TEENSY_APP_MODE_RACE` use the same camera
driver, then process frames through `CameraVision`:

```text
128-pixel DMA frame
  -> trim pixels [2..125]
  -> 124-pixel S32K-compatible detector input
  -> LineDetector output
  -> TeensyLinkCameraResult camera[0]
```

Line detector values match the S32K-compatible telemetry contract:

| Field | Values |
|---|---|
| status | `0=LOST`, `1=BOTH`, `2=LEFT`, `3=RIGHT` |
| feature | `0=NONE`, `1=FINISH_LINE` |
| invalid line index | `255` |

Detector indices are relative to the trimmed 124-pixel buffer. Add
`CAM_TRIM_LEFT_PX` to overlay them on the raw 128-pixel plot.

## Expected Checks

At 200 Hz, consecutive 1 Hz debug lines, when enabled, should show:

- `req`, `si`, `win`, `ready`, and `dma` increasing by about 200.
- `samples` increasing by `200 * 128 = 25600`.
- `drop=0`, `overrun=0`, `adcErr=0`, and `dmaErr=0`.
- `capUs` close to the readout window plus small software overhead.
- `capClk` close to 129. A value around 130 is acceptable because it is derived
  from software-side capture duration, not a direct pulse counter.

Scope checks with the current config:

- SI period: 5 ms.
- CLK frequency: 200 kHz.
- CLK burst length: about 645 us.
- Exactly one CLK rising edge occurs while SI is high.
- AO changes with illumination and stays within the Teensy ADC input range.
