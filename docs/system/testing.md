# Testing Guide

Work from individual hardware to communication, then controlled motion. Record
the commit, both build selections, wiring/power source, and observed result for
each run. The expectations below come from code; they are not claims that
every combination has already passed on the car.

## Build And Upload

Run these commands from the repository root. Use an installed `pio` on PATH,
or this workstation's executable:

```powershell
$pio = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"
& $pio device list
& $pio run -d firmware/teensy -e teensy41-race -t upload
& $pio run -d firmware/esp32 -e esp32dev -t upload
```

Replace `teensy41-race` with the environment in the [mode table](modes.md).
Add `--upload-port COMx` when multiple boards are attached. A serial monitor:

```powershell
& $pio device monitor --port COMx --baud 115200
```

For S32K select exactly one build flag in `src/app/app_config.h`, then build
from the repository root and flash the resulting image through S32DS:

```powershell
C:\msys64\ucrt64\bin\mingw32-make.exe -C firmware/s32k144/Debug_FLASH all
```

If ARM GCC is absent from PATH, follow the
[S32K build notes](https://github.com/michalfrohlich/NXP-Cup-EXPO/blob/main/firmware/s32k144/docs/build.md).
Changing a host viewer GUI does not require reflashing; changing the firmware
mode or enabling its stream does.

## 1. Boot And Local Inputs

Use the S32K bench build and Teensy Hardware Test, independently.

- S32K: expect the three-second build banner and boot colors. Boot with `swPcb`
  active: no bench should start; the switch-release prompt should appear.
  Move it off, verify pot selection and enter/exit. Also test the two-button
  500 ms chord with release between actions.
- Teensy: button 1 cycles red/green/blue, button 2 toggles on/off, pot sets
  brightness. Confirm the serial button states and READY low. No SPI traffic
  is expected in Hardware Test.
- Repeat cold starts with the intended battery supply as well as USB. Record
  differences; a USB-only pass does not verify the battery wiring/power path.

## 2. Sensors And Actuators In Isolation

Support the chassis with wheels clear for actuator tests.

| Test | Expected checks and edge cases |
|---|---|
| S32K Servo | RAW/SMOOTH selection, pot sweep, SW2 center, sensible limits; exit returns control to menu |
| S32K Servo Rate build | Try 10/50/100/250 Hz request rates and POT/FINE/STEP sources; compare OK/BUF/NOISR and PWM count; physical PWM stays 50 Hz |
| S32K ESC | Hold arm, verify neutral and zero-pot gate; test 30/50/100 caps, drive routing and direction at neutral; hold disarm and test menu exit |
| S32K Ultrasonic | Targets around 45 cm and 8 cm; no echo; confirm distance and DRV ST differ as acquisition advances |
| S32K Camera / Cam+Servo | Verify capture counters and graphs; move track left/right; pause/resume; cover camera; Cam+Servo moves steering only |
| Teensy Camera Bench | Raw signal, track/error/confidence, white on LOST, red only for unavailable/stale acquisition; verify flat input does not imply hardware presence |

Python viewer, from the repository root:

```powershell
python -m pip install -r tools/python/live_camera_viewer/requirements.txt
python tools/python/live_camera_viewer/main.py --list-ports
python tools/python/live_camera_viewer/main.py --port COMx --baud 921600 --gui classic
```

Use `--gui camera_2d` for the alternative view. Close another serial monitor
before opening the same port. From `firmware/teensy`, the script path starts
`../../tools/`, not `tools/`. At 200 Hz acquisition and 100 Hz streaming,
sequence deltas of two can be deliberate decimation, not UART corruption;
check parse errors separately.

## 3. Teensy SPI And IMU Without Motion

Pair S32K Teensy Link with Teensy Stack Drive (dashboard) or Race. For readable
Link Bench text, use the debug flags described in the [mode guide](modes.md).
Keep the car still through IMU startup calibration.

1. Verify READY, increasing sequences and good-frame counters, and `CFG == ACK`.
2. SW2 cycles six S32K pages. CAM0 follows the track; CAM1 stays missing/stale.
   Missing camera 1 must not be interpreted as a failed SPI transport.
3. Tilt/rotate the IMU: yaw/GZ/lateral fields change. Check P/C/V/A on Teensy;
   yaw is relative and can drift.
4. Reset each board separately and confirm valid frames/config ACK recover.
5. In this motionless test, disconnect the link and verify freshness expires;
   reconnect and confirm recovery. Record CRC/protocol/SPI counter changes.

See the [SPI test](../protocols/teensy-s32k-spi-test.md) for pins, raw header,
MOSI/MISO checks, and counter interpretation. Fresh telemetry alone does not
prove the detector sees a usable line.

## 4. Web And Tuning Without Motion

Pair S32K **ESP Link bench** with ESP `esp32dev`. Follow
[Wi-Fi connection steps](../firmware/esp32.md#connect).

1. Connect/disconnect a browser and check `wifi:ap`/`wifi:pc` on ESP.
2. Submit KP/KI/KD, clamp, LPF, minimum contrast, and both edge percentages.
   Expect a successful response, S32K cyan pulse, and changed stored values.
3. Start/Stop should receive replies without moving any actuator in this test.
4. Disconnect UART: the page should report timeout; an ESP blue pulse alone
   does not imply reception. Reconnect and verify a later request succeeds.
5. Check malformed/out-of-range requests are rejected. The web UI constrains
   normal entry; direct HTTP requests are needed to exercise invalid forms.
6. For a motionless end-to-end vision-config check: apply via ESP Link, exit,
   enter Teensy Link, and verify its requested/applied generations agree.
   Repeat using Cable Tune. Reset S32K and confirm defaults are redistributed.

Do not expect live ESP requests while the S32K is sitting in Teensy Link or
Cable Tune; only one bench item executes at a time.

## 5. Controlled Drive Checks

Start with wheels clear, then use a controlled track and the lowest suitable
profile. Keep the physical exit/stop accessible; web Stop only works in the
modes listed in the capability table.

| Mode | Verify |
|---|---|
| Drive mode-s32k | Waits for SW3 after arming, ramps to configured speed, uses previously stored steering/vision tuning. Exit stops. It has no race line-loss latch. |
| Ultra+ESC | After arming: 50% clear, slow at 45 cm, stop at 8 cm; no echo/error behavior; exit neutral |
| Victory Lap | Approach then pole stop; note table/LED sequence, not actual synthesized motor music |
| Stack Drive + Teensy Stack + ESP | Automatic run after arm; SW2 pages; web Stop neutral/off, Start re-arms white then resumes; SW3 local stop; live PID/vision changes; SPI CFG/ACK follows |
| Tune Drive + Teensy Race + ESP | Teensy steering and live tuning; CFG/ACK via diagnostics; SW3 stop/exit. Do not rely on web Start/Stop here. |
| Standalone races | Profile SELECT/READY/START, three-second arm, correct camera source, finish transition to Honor, obstacle speed targets and stop behavior |

For race-family edge cases, first use a stationary setup: line lost briefly
(white), line lost beyond three seconds (white latched stop), local camera
stale or required Teensy camera unavailable (red fault). In Stack Drive, missing
Teensy telemetry alone must not stop local-camera driving. Exercise repeated
Start/Stop in Stack Drive, independent board resets, and settings reset.

Verify display behavior per [diagnostics](status-led-semantics.md): full
Teensy-camera race suppresses OLED updates while running; a static screen is
not by itself evidence of a stalled controller.

## 6. SD Test Separately

Use `teensy41-sdtest`, a FAT32/exFAT card, and serial at 115200. Expect a new
`LOGnnn.CSV`, 100 ms synthetic test rows, green ready status and increasing
bytes. Try absent card and button/pot changes. Hold button 2 for two seconds,
confirm close with no logger error, then power down before removing the card.
Verify the CSV on the PC. No real race logging or S32K dependency is expected.

## Record A Result

For each test record: firmware commit, S32K flag, Teensy environment/debug
flags, ESP environment, supply, expected behavior, observed behavior, counters,
and pass/fail. Preserve failures with the exact combination instead of marking
a whole board tested from a single mode.
