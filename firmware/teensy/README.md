# Teensy Firmware

Teensy-side firmware for camera acquisition, camera vision, and the S32K
128-byte SPI telemetry link.

## Layout

- `include/teensy_config.h`: board-level pins, runtime rates, and link debug flags.
- `include/config/`: camera, display, and vision configuration.
- `include/drivers/` and `src/drivers/`: Teensy-local hardware drivers.
- `include/services/` and `src/services/`: camera vision, line detection, and race runtime services.
- `include/comms/` and `src/comms/`: Teensy SPI slave transport.
- `include/telemetry/` and `src/telemetry/`: packing and decoding for the shared 128-byte frame.
- `src/app/`: compile-time app-mode dispatcher and mode implementations.
- `src/main.cpp`: minimal Arduino entry point that dispatches the selected app mode.

The shared packet contract lives in `../../shared/protocol/teensy_link_protocol.h`.

## App Modes

The selected Teensy mode is controlled in `src/app/teensy_app_config.h`.
The current default is camera bench:

```cpp
#define TEENSY_APP_MODE_LINK_BENCH        0
#define TEENSY_APP_MODE_CAMERA_BENCH      1
#define TEENSY_APP_MODE_RACE              2
#define TEENSY_APP_SELECTED_MODE TEENSY_APP_MODE_CAMERA_BENCH
```

| Mode | Purpose | Debug behavior |
|---|---|---|
| `TEENSY_APP_MODE_CAMERA_BENCH` | Camera 0 bring-up and Python plotting without the S32K link runtime. | Binary Python camera stream enabled by camera config; text frame/status output disabled by default. |
| `TEENSY_APP_MODE_LINK_BENCH` | Debug wrapper around the integrated race runtime. | Runs SPI link, camera acquisition, line detection, and optional debug outputs. |
| `TEENSY_APP_MODE_RACE` | Integrated runtime intended for running the car. | Serial text, Python stream, and display debug are disabled. |

## Camera And Vision

Camera 0 uses a TSL1401CL linear CCD on these Teensy 4.1 pins:

| Camera signal | Teensy 4.1 pin |
|---|---:|
| SI | 4 |
| CLK | 6 |
| AO | 15 / A1 |

The current camera timing is:

| Parameter | Value |
|---|---:|
| Pixel clock | 200 kHz |
| Frame rate | 200 Hz |
| Published samples | 128 raw 12-bit ADC values |
| Debug serial baud | 921600 |

The driver uses FlexPWM, XBARA, ADC_ETC, ADC1, and DMA so the CPU does not
service each pixel sample. The foreground loop still has to call the camera
service often enough to start frames and observe DMA completion.

`CameraVision` converts each raw camera frame into telemetry-ready line data:

```text
128 raw ADC samples
  -> trim 2 pixels from each side
  -> 124-pixel detector input
  -> S32K-compatible line detector
  -> Teensy link camera result
```

Detector indices are relative to the trimmed buffer. Add `CAM_TRIM_LEFT_PX`
when overlaying them on the raw 128-sample Python plot.

## Python Camera Stream

The binary Python stream is controlled by `include/config/camera_config.h`.
It is enabled by default for camera bench:

| Setting | Current value |
|---|---:|
| `TEENSY_LINEAR_CAMERA_STREAM_ENABLED` | `true` |
| `TEENSY_LINEAR_CAMERA_STREAM_PERIOD_MS` | `10` |
| Stream rate | 100 Hz |
| Camera bench payload | Raw 128-sample frame with vision marked invalid |

Run the viewer from the repository root:

```bat
python tools\python\live_camera_viewer\live_camera_viewer.py
```

The viewer plots raw ADC values directly in the `0..4095` range. Text status
and `frame8` serial output are disabled by default because they share the same
USB serial port and add avoidable timing noise while the binary stream is active.
The same stream packet format carries detector output when it is sent by the
integrated race runtime.

## OLED Display

The optional SSD1306 OLED debug display is configured in
`include/config/display_config.h`:

| Signal | Teensy 4.1 pin |
|---|---:|
| SCL1 | 16 |
| SDA1 | 17 |

| Setting | Current value |
|---|---:|
| Display size | 128x64 |
| I2C address | `0x3C` |
| I2C clock | 400 kHz |
| Camera display debug | disabled |
| Camera refresh period | 200 ms |

Display transfer is asynchronous and idle-gated: refresh requests snapshot the
framebuffer, then `DisplayService()` sends small I2C chunks only when the camera
reports enough idle time before the next frame. Keep display debug disabled
while validating raw camera stability; enable it later as a slower visual debug
tool.

## S32K Link Runtime

`TEENSY_APP_MODE_LINK_BENCH` and `TEENSY_APP_MODE_RACE` publish the shared
128-byte telemetry frame over SPI. The Teensy is the SPI slave and the S32K
controls the actual transfer cadence. The Teensy updates the telemetry buffer
at `TEENSY_LINK_SENSOR_HZ`, currently 200 Hz.

The current runtime sends real camera 0 vision data and placeholder data for
components that are not implemented yet:

| Component | Current behavior |
|---|---|
| IMU | Deterministic valid placeholder. |
| Camera 0 | Valid when fresh camera vision data has been processed. |
| Camera 1 | Stale/default placeholder. |
| Logger / SD | Not ready placeholder. |
| RX counters | Based on S32K-to-Teensy frames decoded by the transport. |

Keep `TEENSY_LINK_SERIAL_DEBUG_ENABLED` disabled when using the Python camera
stream, because both use USB serial.

## Build And Upload

Use PlatformIO from the Teensy firmware folder:

```bat
cd /d C:\Users\misof\workspaceS32DS.3.6.3\EXPO_03_Nxp_Cup_project\firmware\teensy
pio run
pio run -t upload
```

Full S32K link run instructions are in
`../../docs/protocols/teensy-s32k-128b-spi-test.md`. The shield pinout is in
`../../hardware/shield-v2-pinout.md`.
