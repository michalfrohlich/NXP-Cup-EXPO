# Teensy Dual-Camera Template

This branch adds the Teensy-side structure for two PCB-connected cameras. It
does not implement a camera protocol because the exact camera model is not yet
confirmed.

The existing S32K linear-camera driver is already implemented separately in
`firmware/s32k144/src/drivers/linear_camera.c`. This Teensy template does not
change, replace, or depend on that working S32K camera.

## Pin Plan

| Function | Teensy 4.1 pin |
|---|---:|
| Camera 1 SI | 4 |
| Camera 1 CLK | 6 |
| Camera 1 analog | 15 |
| Camera 2 SI | 5 |
| Camera 2 CLK | 7 |
| Camera 2 analog | 14 |
| Potentiometer | 27 |
| Button 1 | 28 |
| Button 2 | 29 |

The pin values come from the current shield schematic and Teensy pin plan.
Confirm the camera connector pin order before applying power.

## State Meaning

- `NOT_CONFIGURED`: the camera is intentionally disabled because its model and
  timing are not known.
- `NOT_CONNECTED`: reserved for a future model-specific driver that can
  positively detect a missing camera.
- `NO_DATA`: the camera is enabled but the analog input has no useful activity.
- `READY`: the enabled camera analog input changed enough during the sample
  window. This confirms activity only, not a valid image.
- `ERROR`: invalid driver request or future model-specific fault.

The generic analog test cannot reliably distinguish a disconnected floating
wire from a connected camera. Do not treat `READY` as proof that image capture
is correct.

## Enable And Run

Open:

```text
C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
```

Set this value to `1` in `include/teensy_app_config.h`:

```cpp
#define TEENSY_APP_DUAL_CAMERA_TEST 1
```

The cameras remain `NOT_CONFIGURED` until their models are known. To run the
temporary analog-activity test, set the required camera flag to `true` in
`include/teensy_config.h`:

```cpp
static constexpr bool TEENSY_CAM1_CONFIGURED = true;
static constexpr bool TEENSY_CAM2_CONFIGURED = true;
```

Then run:

```bat
cd /d C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
pio run
pio run -t upload
pio device list
pio device monitor -p COM3 -b 115200
```

Replace `COM3` with the Teensy port. Button 1 selects camera 1 and button 2
selects camera 2. Moving the potentiometer to the lower or upper third also
selects camera 1 or camera 2. The selected camera has a `>` marker:

```text
> camera1 state=NOT_CONFIGURED samples=0 min=- max=-
  camera2 state=NOT_CONFIGURED samples=0 min=- max=-
```

Set `TEENSY_APP_DUAL_CAMERA_TEST` back to `0` before running the normal
S32K-Teensy SPI test.

## Where To Add The Real Driver

- Put model-independent camera state and public functions in
  `include/drivers/teensy_cameras.h`.
- Put Teensy pin sampling and the future model-specific capture sequence in
  `src/drivers/teensy_cameras.cpp`.
- Keep the manual bench behavior in `src/modes/mode_dual_camera_test.cpp`.
- Update the shared 128-byte telemetry only after real camera output is
  validated.

## Documentation Handoff Prompt

```text
Update the Teensy dual-camera documentation using the confirmed camera model,
datasheet, connector pin order, voltage, SI/CLK timing, ADC settings, frame
rate, and observed Serial test results. Replace assumptions with measured
values. Explain how NOT_CONNECTED, NO_DATA, READY, and ERROR are detected.
Document the exact files changed, wiring, build/upload commands, expected test
output, and how validated camera results map into the shared 128-byte
S32K-Teensy packet. Keep the S32K OLED and S32K camera code unchanged.
```

## Files

- `include/teensy_app_config.h`: selects the dual-camera test.
- `include/teensy_config.h`: camera, button, and potentiometer settings.
- `include/drivers/teensy_cameras.h`: camera state interface.
- `src/drivers/teensy_cameras.cpp`: generic analog-activity template.
- `src/modes/mode_dual_camera_test.cpp`: Serial test and camera selection.
- `src/main.cpp`: dispatches to the camera test when enabled.
