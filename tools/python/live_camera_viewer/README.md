# Teensy Camera Live Viewer

Python live graph for the current Teensy TSL1401CL binary camera stream.

The viewer reads fixed-size `TCM1` packets from the serial byte stream. Each
packet contains the `LinearCameraFrame` sequence, timestamp, and all 128 raw
12-bit ADC samples. Version 2 packets also include the line detector output.
The GUI plots raw samples directly on a `0..4095` axis and overlays the detected
left/right line positions.

The current dashboard is a compact vision-system view:

- raw 128-pixel camera signal with the S32K-compatible trim region shaded
- detector left/right line overlays on the raw camera plot
- detector track-center overlay derived from the reported error
- history of vision error and confidence
- history of raw min/avg/max ADC values
- decoded status, feature, confidence, error, and trimmed detector positions

The stream currently carries compact `VisionOutput` only. Matlab-style filtered
signal, gradient, thresholds, split point, edge candidates, and finish-gap
internals require a future richer firmware debug packet.

## Install

From the repository root:

```powershell
python -m pip install -r tools/python/live_camera_viewer/requirements.txt
```

## Firmware Settings

For full camera + detector testing, use the integrated link mode:

```cpp
TEENSY_APP_SELECTED_MODE = TEENSY_APP_MODE_LINK_BENCH
```

For raw camera testing without detector output, use camera bench mode instead:

```cpp
TEENSY_APP_SELECTED_MODE = TEENSY_APP_MODE_CAMERA_BENCH
```

The stream should have:

```cpp
TEENSY_LINEAR_CAMERA_DEBUG_ENABLED = true
TEENSY_LINEAR_CAMERA_DEBUG_PRINT_STATUS = false
TEENSY_LINEAR_CAMERA_DEBUG_PRINT_FRAME8 = false
TEENSY_LINEAR_CAMERA_STREAM_ENABLED = true
TEENSY_LINEAR_CAMERA_STREAM_PERIOD_MS = 10
TEENSY_LINEAR_CAMERA_DEBUG_SERIAL_BAUD = 921600
```

In camera bench mode, `TCM1` v2 packets contain raw samples and `visionFlags=0`,
so the GUI will report no valid detector output. In link mode, the same viewer
also shows detector output when `TEENSY_LINEAR_CAMERA_STREAM_ENABLED` is true.

The current tested camera timing is:

```text
CLK:   200 kHz
Frame: 200 Hz
```

## Run

Explicit port:

```powershell
python tools/python/live_camera_viewer/main.py --port COM11 --baud 921600
```

Auto-detect port:

```powershell
python tools/python/live_camera_viewer/main.py
```

List ports:

```powershell
python tools/python/live_camera_viewer/main.py --list-ports
```

Useful options:

```powershell
python tools/python/live_camera_viewer/main.py --port COM11 --display-hz 60 --history 300
```

The firmware stream defaults to 100 Hz. The GUI redraw defaults to 30 Hz and
uses the latest complete binary frame, so it does not need to render every
received packet.

## What To Check

The graph should move as the camera sees bright/dark input.

When line detection is active:

- `status` should move between `LOST`, `BOTH`, `LEFT`, and `RIGHT`.
- `confidence` should rise when the track edges are clear.
- `error` should change sign as the track center moves left/right.
- left/right positions shown in the text panel are trimmed 124-pixel detector
  positions.
- the shaded first/last two pixels are not used by the detector.

The viewer redraws on a GUI timer and keeps only the latest complete binary
frame for display, so GUI redraws do not run once per serial packet.
