# Architecture

## Top-level structure
- `src/main.c` calls `App_RunSelectedMode()` and never returns.
- `src/app/app_modes.c` selects exactly one compile-time mode from `src/app/car_config.h`.
- `src/app/board_init.c` initializes RTD/MCAL modules before app logic starts.

## Execution model
- Bare-metal, no scheduler, no RTOS.
- Main behavior is implemented as infinite loops inside the selected mode.
- Timing is a mix of:
  - polling against `Timebase_GetMs()`
  - busy waits
  - interrupt callbacks wired to RTD notifications

## Current control flow
1. `main()`
2. `App_RunSelectedMode()`
3. Selected mode initializes required modules
4. Mode loop runs forever

## Current selected mode
- `APP_TEST_FINAL_DUMMY` is enabled in `src/app/car_config.h`.
- In `mode_final_dummy()`:
  - SW3 switches between `DUMMY_ESC` and `DUMMY_CAM`
  - ESC mode uses the onboard pot for manual speed command
  - CAM mode runs camera -> vision -> steering and ramps motor speed automatically

## Data flow
### Camera steering path
- `linear_camera.c`
  - drives shutter and pixel clock
  - samples 128 pixels through ADC
  - publishes the latest frame through ping-pong buffers
- `services/vision_linear_v2.c`
  - filters the frame
  - computes a 1D gradient
  - selects lane edges / track center
- `services/steering_control_linear.c`
  - converts vision error to steering command
- `servo.c`
  - applies steering through PWM

### Manual / motor path
- `onboard_pot.c`
  - samples the potentiometer through ADC
- `app_modes.c`
  - maps pot value to speed in ESC mode
  - ramps speed in camera mode
- `esc.c`
  - applies motor command through PWM and an internal ESC state machine

### Optional braking path present in repo
- `services/braking.c`
  - uses `ultrasonic.c` and `hbridge.c`
  - not part of the currently selected mode path

## Interrupt / callback-driven pieces
- `timebase.c`: `EmuTimer_Notification()`, `UsTimer_Notification()`
- `linear_camera.c`: `NewCameraFrame()`, `CameraClock()`, `CameraAdcFinished()`
- `ultrasonic.c`: `Icu_TimestampUltrasonicNotification()`
- `receiver.c`: `Icu_SignalNotification()`, `Gpt_Notification()`
- `esc.c`: `Esc_Period_Finished()`

## Key implementation files
- App control: `src/app/app_modes.c`, `src/app/car_config.h`
- Board bring-up: `src/app/board_init.c`
- Vision: `src/services/vision_linear_v2.c`, `src/app/vision_debug.c`
- Steering: `src/services/steering_control_linear.c`, `src/servo.c`
- Motor control: `src/esc.c`, `src/hbridge.c`
- Sensors / IO: `src/linear_camera.c`, `src/onboard_pot.c`, `src/ultrasonic.c`, `src/receiver.c`, `src/buttons.c`, `src/display.c`, `src/rgb_led.c`
