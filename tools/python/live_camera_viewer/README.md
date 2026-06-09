# Teensy Camera Live Viewer

Python live graph for the current Teensy TSL1401CL binary camera stream.

The viewer reads fixed-size `TCM1` packets from the serial byte stream. Each
packet contains the `LinearCameraFrame` sequence, timestamp, and all 128 raw
12-bit ADC samples. Version 2 packets also include the line detector output.
The GUI plots raw samples directly on a `0..4095` axis and overlays the detected
left/right line positions.

## Install

From the repository root:

```powershell
python -m pip install -r tools/python/live_camera_viewer/requirements.txt
```

## Firmware Settings

For raw camera testing, select camera bench mode:

```cpp
TEENSY_APP_SELECTED_MODE = TEENSY_APP_MODE_CAMERA_BENCH
```

The camera bench stream should have:

```cpp
TEENSY_LINEAR_CAMERA_DEBUG_ENABLED = true
TEENSY_LINEAR_CAMERA_DEBUG_PRINT_STATUS = false
TEENSY_LINEAR_CAMERA_DEBUG_PRINT_FRAME8 = false
TEENSY_LINEAR_CAMERA_STREAM_ENABLED = true
TEENSY_LINEAR_CAMERA_STREAM_PERIOD_MS = 10
TEENSY_LINEAR_CAMERA_DEBUG_SERIAL_BAUD = 921600
```

In camera bench mode, `TCM1` v2 packets contain raw samples and no valid detector
overlay. In link bench mode, the same viewer also shows detector output when the
firmware stream includes it.

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

The firmware stream defaults to 100 Hz. The GUI redraw defaults to 60 Hz and
uses the latest complete binary frame, so it does not need to render every
received packet.

## What To Check

The graph should move as the camera sees bright/dark input.

The viewer redraws on a GUI timer and keeps only the latest complete binary
frame for display, so GUI redraws do not run once per serial packet.
