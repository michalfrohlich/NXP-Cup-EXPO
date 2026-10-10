# Modes And Board Combinations

Select exactly one S32K build flag in `firmware/s32k144/src/app/app_config.h`.
The flags include both `APP_TEST_*` and `APP_RACE*`. Rebuild and flash after
changing one. A bench item is a runtime choice within `APP_TEST_NXP_CUP_TESTS`;
it does not require another flash.

Select Teensy modes with named PlatformIO environments. Each compiles its
selected mode source plus common modules. ESP uses `esp32dev` for all
combinations; its behavior depends on the requests the S32K consumes.

## S32K Build Flavors

| Build flag | Entry behavior | Partner firmware |
|---|---|---|
| `APP_TEST_NXP_CUP_TESTS` | Pot-driven bench menu below | Depends on menu item |
| `APP_TEST_LINEAR_CAMERA_TEST` | Direct Camera-s32k bench | None |
| `APP_TEST_ESC_TEST` | Direct ESC bench | None |
| `APP_TEST_SERVO_RATE_TEST` | Servo timing test: SW2 changes request rate, SW3 changes source | None |
| `APP_TEST_TEENSY_LINK_TEST` | Direct Teensy Link diagnostics | Teensy Race, Link Bench, or Stack Drive |
| `APP_TEST_ESP_LINK_TEST` | UART button/ACK and tuning/command test, no motion | ESP `esp32dev` |
| `APP_RACE_S32K_ONLY` | Speed selector, arm, race using S32K camera | None required |
| `APP_RACE` | Speed selector, arm, race using Teensy camera 0 | Teensy `teensy41-race` |

All S32K flavors first show the same three-second OLED/UART build banner and
RGB sequence. This runs before any per-mode display suppression.

## Bench Menu Operation

After boot, the menu requires `swPcb` in its menu/off position. If it starts
active, the screen says `SWPCB ACTIVE / MOVE SWITCH / TO MENU POS`; no bench
is entered. Move it off, choose an item with the pot, then switch on to enter.
Switch off to call the mode's exit helper and return to the menu.

Holding SW2 and SW3 together for 500 ms is an alternative enter/exit action.
Release them between actions. It can also dismiss the startup switch guard;
the switch is then ignored until seen off. A chord-entered mode uses the chord
to exit, rather than immediately exiting because the switch is off.

## Bench Items In Display Order

Actuator tests can move the car. For first checks, support the chassis with
wheels clear and verify the physical stop/exit controls before applying drive.

| Display item | Behavior and controls | Partners |
|---|---|---|
| `Servo` | Pot selects RAW/SMOOTH; SW2 enters, then pot controls steering and SW2 centers. Shows requested/applied output. | None |
| `ESC` | SW2 hold arms/disarms; SW3 hold cycles 30/50/100% cap. SW2 click changes output mode; SW3 click changes direction while disarmed or neutral. Pot must return to zero after arming. | None |
| `Ultrasonic` | Distance-only; SW2 resets. Shows WAIT/SCAN/CLEAR/SLOW/STOP and `DRV ST` capture state. No motor drive. | None |
| `Ultra+ESC` | Arms, centers steering, runs both ESCs at 50% when clear, slows at 45 cm, stops at 8 cm; missing-data behavior differs from race. | None |
| `Camera-s32k` | Camera graphs/detector debug; SW2 switches view, hold pauses, click resumes; pot changes paused graph scale. Cable telemetry is queued periodically. | None |
| `Cam+Servo-s32k` | Same camera/debug pipeline plus automatic steering; no motor drive. Uses detector/controller defaults and local shaping constants. | None |
| `Drive mode-s32k` | Local camera steering with RAM profile; arms then waits for SW3 to start. Ramps to 25% with current constants. Exit the bench to stop; SW3 is not a run/stop toggle. | None |
| `Tune drive mode` | Teensy-camera race scheduler with live ESP tuning. Arms and starts on entry. SW3 requests stop; exit/re-enter to restart. | Teensy Race; ESP for tuning |
| `Stack drive` | S32K-camera race behavior with ESP tuning/Start/Stop and Teensy telemetry. Arms and starts on entry. SW2 changes diagnostic page; SW3 stops. | ESP; Teensy Stack Drive for full diagnostics |
| `Cable tune-s32k` | PC UART terminal edits PID, servo clamp/LPF, detector thresholds. No motion. Values survive changing bench items, not reset. | PC serial terminal |
| `ESP Link` | Accepts tuning and acknowledges Start/Stop without motion. `ESP TUNE SAFE` screen. Does not send button state. | ESP |
| `Teensy Link` | SPI sensor/configuration diagnostics without drive commands; SW2 cycles six pages. | Teensy Race, Link Bench, or Stack Drive |
| `Victory Lap` | Arms, approaches an obstacle, stops at 8 cm, runs a note-sequence placeholder. Motor-tone generation is not implemented. | None |

`Drive mode-s32k` is simpler than Stack Drive: no Honor phase, ESP service, or
three-second line-loss latch. A stale camera forces neutral but does not create
the race mode's latched fault. Similar colors do not imply equivalent safety.

## Drive-Mode Capabilities

| S32K mode | Steering source | SPI v3 config/telemetry | Live ESP tuning | ESP Start/Stop |
|---|---|---|---|---|
| `APP_RACE` | Teensy camera 0, period-based averaging | Yes; required camera source | No | Not consumed |
| `APP_RACE_S32K_ONLY` | S32K camera | No | No | Not consumed |
| Tune drive mode | Teensy camera 0, race scheduler | Yes; required camera source | Yes | Not consumed |
| Stack drive | S32K camera | Yes; diagnostic, not steering authority | Yes | Changes vehicle state |
| Drive mode-s32k | S32K camera | No | No; uses previously stored tuning | Not consumed |
| ESP Link / standalone ESP test | None | No | Stores in RAM | Acknowledges only, no motion |

Connecting to the ESP page is independent of these choices. An unsupported
command can time out even though Wi-Fi and HTTP are healthy.

## Teensy Environments

| Environment | Camera / IMU / SPI | Display and serial | RGB / buttons |
|---|---|---|---|
| `teensy41-race` (also `teensy41`) | Camera 0, IMU, SPI v3 | Display, text log, camera stream disabled | Tracking LED; no button toggle |
| `teensy41-stack-drive` | Camera 0, IMU, SPI v3 | SH1122 dashboard, 250 ms target; no camera stream or periodic text | Tracking LED; button 2 toggles indication only |
| `teensy41-link-bench` | Shared race sensor/SPI runtime | Configurable debug; current flags enable binary camera stream and disable periodic text | Tracking LED disabled |
| `teensy41-camera-bench` | Camera 0 and local vision; no IMU/SPI service | Binary viewer stream; optional SSD1306 graph disabled by default | Tracking LED |
| `teensy41-hardware-test` | Pot/buttons; passive SPI pin readings, READY low | Serial at 115200; no display | Button 1 cycles R/G/B; button 2 toggles LED; pot brightness |
| `teensy41-sdtest` | SD and pot/buttons; READY low, no SPI/IMU/camera service | Serial at 115200; CSV test rows every 100 ms | Logger state color; pot brightness; hold button 2 for 2 s to close log |

Link Bench's monitor setting is 115200, but its enabled camera stream calls
`Serial.begin(921600)`. Use the Python viewer for that binary output. To obtain
readable periodic link/IMU text, enable `TEENSY_LINK_SERIAL_DEBUG_ENABLED` and
disable `TEENSY_LINEAR_CAMERA_STREAM_ENABLED`, then rebuild Link Bench.

## Suggested Pairings

| Goal | S32K | Teensy | ESP |
|---|---|---|---|
| Inspect camera signal/thresholds on PC | Not needed | Camera Bench | Not needed |
| Verify sensors across SPI without motion | Teensy Link bench or standalone test | Stack Drive for dashboard, or Race | Not needed |
| Verify web tuning/command replies without motion | ESP Link bench | Not needed | `esp32dev` |
| Integrated drive using S32K camera | Stack drive bench | Stack Drive | `esp32dev` |
| Live tune Teensy-camera steering | Tune drive mode bench | Race | `esp32dev` |
| Standalone Teensy-camera competition run | `APP_RACE` | Race | Not required; commands not serviced |

See [testing](testing.md) for commands and verification, and
[LEDs/displays](status-led-semantics.md) for feedback.
