# Teensy Firmware

Teensy-side firmware for camera acquisition, camera vision, and the S32K
84-byte SPI telemetry link.

## Layout

- `include/teensy_config.h`: board-level pins, runtime rates, and link debug flags.
- `include/config/`: camera, display, and vision configuration.
- `include/drivers/` and `src/drivers/`: Teensy-local hardware drivers, including board inputs, RGB status LED, and the MPU6050 IMU.
- `include/logging/` and `src/logging/`: optional local data logging modules.
- `include/services/` and `src/services/`: camera vision, line detection, and race runtime services.
- `include/comms/` and `src/comms/`: Teensy SPI slave transport.
- `include/telemetry/` and `src/telemetry/`: packing and decoding for the shared 84-byte frame.
- `src/app/`: compile-time app-mode dispatcher and mode implementations.
- `src/main.cpp`: minimal Arduino entry point that dispatches the selected app mode.

The shared packet contract lives in `../../shared/protocol/teensy_link_protocol.h`.

## App Modes

The selected Teensy mode is controlled in `src/app/teensy_app_config.h`.
The current integration default is race:

```cpp
#define TEENSY_APP_MODE_LINK_BENCH        0
#define TEENSY_APP_MODE_CAMERA_BENCH      1
#define TEENSY_APP_MODE_RACE              2
#define TEENSY_APP_MODE_HARDWARE_TEST     3
#define TEENSY_APP_MODE_SD_LOG_TEST       4
#define TEENSY_APP_SELECTED_MODE TEENSY_APP_MODE_RACE
```

| Mode | Purpose | Debug behavior |
|---|---|---|
| `TEENSY_APP_MODE_CAMERA_BENCH` | Camera 0 bring-up and Python plotting without the S32K link runtime. | Binary Python camera stream enabled by camera config; text frame/status output disabled by default. |
| `TEENSY_APP_MODE_LINK_BENCH` | Debug wrapper around the integrated race runtime. | Runs SPI link, camera acquisition, line detection, and optional debug outputs. |
| `TEENSY_APP_MODE_RACE` | Integrated runtime intended for running the car. | Serial text, Python stream, and display debug are disabled. |
| `TEENSY_APP_MODE_HARDWARE_TEST` | Local PCB input and RGB LED self-test. | Prints pot, button, and passive SPI pin levels at `TEENSY_SERIAL_BAUD`; holds READY low so the S32K does not clock the Teensy SPI slave. |
| `TEENSY_APP_MODE_SD_LOG_TEST` | Local built-in SD card logger self-test. | Creates `LOGnnn.CSV`, logs test rows from pot/button snapshots, prints SD status, and holds READY low. |

## PCB Inputs And Status LED

The PCB input and RGB LED pins are configured in `include/teensy_config.h`:

| Signal | Teensy 4.1 pin |
|---|---:|
| Potentiometer ADC | 27 |
| Button 1 | 28 |
| Button 2 | 29 |
| RGB LED red | 1 |
| RGB LED green | 2 |
| RGB LED blue | 3 |

`TEENSY_APP_MODE_HARDWARE_TEST` is the bring-up mode for these pins. It cycles
LED colors while idle. Holding button 1 forces the LED white through the
level-based `BoardInputs_IsPressed()` path. Pressing button 2 once toggles LED
power through the one-shot `BoardInputs_WasPressed()` path.

The Teensy board-input driver debounces both momentary buttons. App code can use
`BoardInputs_IsPressed()` for the current debounced level and
`BoardInputs_WasPressed()` for the one-shot press edge.

## SD Logger

The optional SD logger uses the Teensy 4.1 built-in SD slot through SdFat SDIO.
`TEENSY_APP_MODE_SD_LOG_TEST` validates the logger without the S32K link
runtime: it creates the next `LOGnnn.CSV` file, queues one test row every
`TEENSY_SD_LOG_TEST_PERIOD_MS`, and services the card from the foreground loop.
Use the `teensy41-sdtest` PlatformIO environment to build this mode without
editing `src/app/teensy_app_config.h`.

LED status in SD log test:

| LED | Meaning |
|---|---|
| Green | SD logger ready |
| Yellow | no card or open failed |
| Red | logger error |

The test rows reuse the normal telemetry CSV schema. Pot/button values are
encoded into the synthetic IMU columns only for this standalone SD self-test;
race logging should use real telemetry when it is wired in later.

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
python -m live_camera_viewer --port COM11 --baud 921600
```

Or with a specific GUI backend:

```bat
python -m live_camera_viewer --port COM11 --baud 921600 --gui camera_2d
```

The viewer plots raw ADC values directly in the `0..4095` range. Available GUI
frontends are `classic` (default, 1D plots) and `camera_2d` (2D frame visualization).
Text status and `frame8` serial output are disabled by default because they share
the same USB serial port and add avoidable timing noise while the binary stream
is active. The same stream packet format carries detector output when it is sent
by the integrated race runtime.

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
84-byte telemetry frame over SPI. The Teensy is the SPI slave and the S32K
controls the actual transfer cadence. The Teensy updates the telemetry buffer
at `TEENSY_LINK_TELEMETRY_HZ`, currently 200 Hz.

Protocol v2 adds a 16-bit source sequence to each camera slot. The S32K uses
that sequence to distinguish a newly processed camera result from repeated
telemetry frames. Sequence wrap from `65535` to `0` is valid.

The current runtime sends real camera 0 vision data and placeholder data for
components that are not implemented yet:

| Component | Current behavior |
|---|---|
| IMU | Physical MPU6050 sample when detected and calibrated; otherwise the IMU component/status bits stay invalid. |
| Camera 0 | Valid when fresh camera vision data has been processed. |
| Camera 1 | Stale/default placeholder. |
| Logger / SD | Not ready placeholder. |
| RX counters | Based on S32K-to-Teensy frames decoded by the transport. |

The active SPI slave backend is the fixed-pin, software bit-banged
implementation. There is no DMA/LPSPI4 backend selector in the current code.
At 2 MHz, interrupts are deferred for the bounded 336 us transfer so the
software slave cannot miss SCK edges; camera PWM, ADC triggering, and DMA
continue in hardware.

Keep `TEENSY_LINK_SERIAL_DEBUG_ENABLED` disabled when using the Python camera
stream, because both use USB serial.

## Build And Upload

Use PlatformIO from `firmware/teensy`:

```bat
pio run
pio run -t upload
pio device list
pio device monitor
```

The monitor baud is configured in `platformio.ini` as `921600`. Race mode keeps
serial debug output disabled; select a bench mode before expecting terminal or
Python camera-stream output.

## Wiring Summary

| S32K signal | S32K pin | Teensy 4.1 pin |
|---|---:|---:|
| SCK | PTB14 | 13 |
| MOSI / S32K SOUT | PTB16 | 11 |
| MISO / S32K SIN | PTB15 | 12 |
| CS / PCS3 | PTB17 | 10 |
| READY input | PTD14 | 31 |
| GND | GND | GND |

Both boards use 3.3 V logic and require a common ground.

Full S32K link run instructions are in
`../../docs/protocols/teensy-s32k-spi-test.md`. The shield pinout is in
`../../hardware/shield-v2-pinout.md`.
