# Engineering Findings And Next Steps

The V2 iteration improved electrical packaging, camera acquisition, steering
conditioning, and debugging. The next integration priority is reliable
battery-powered communication, followed by repeatable full-car testing.

## Development Outcomes

| Area | Outcome |
|---|---|
| Electrical assembly | Custom shield combines the three controllers; RJ45 connections simplify module replacement and cable routing. |
| Mechanical integration | Dedicated battery, lighting, and ultrasonic mounts organize and secure the hardware. |
| Lighting | Two focused red LEDs replaced the earlier six-LED arrangement; earlier lighting tests observed more consistent camera input under shadows. |
| Camera acquisition | Teensy hardware-triggered ADC and DMA provide a 200 Hz acquisition target without per-pixel CPU callbacks. Foreground code still stops the clock and publishes frames. |
| Vision | Edge-based lane selection replaces black-region midpoint tracking; the Python viewer exposes raw signals, detected edges, error, and confidence. The recorder exports camera scans to CSV. |
| Steering | PID output filtering, rate limiting, and clamping improved stability in earlier driving tests. |
| Diagnostics | Board displays, LEDs, cable tuning, web tuning, and link counters support isolated bring-up and integrated testing. |

## Test Observations

These observations describe earlier hardware integration runs. Record new runs
with the firmware revision and mode combination using the [testing guide](testing.md).

| Test area | Observation | Follow-up |
|---|---|---|
| Autonomous driving | Camera-guided full laps and steering response were demonstrated in earlier runs. | Measure lap time and repeatability with the current integrated build. |
| SPI supply dependence | Communication worked on a breadboard and on the final PCB powered through S32K USB. Battery-powered operation produced communication failures and visible signal noise. | Compare supply rails and signal integrity under the same firmware and load. |
| Vision | Lane-position jitter remained; finish-line detection was unreliable under race conditions. | Record difficult frames and tune geometry, thresholds, and confidence. |
| Ultrasonic stopping | Obstacle response generally worked in earlier tests. | Verify the current Honor policy, invalid-distance behavior, and stopping distance. |
| SD logging | Early logging tests identified write-speed limitations. | Measure worst-case write and sync duration, buffer occupancy, and dropped rows. |

The 4.1 s lap-time goal, 9/10 completed laps, and ten-minute endurance target
remain acceptance targets, not established performance figures.

## Electrical And Mechanical Priorities

The V2 single-layer PCB required jumper wires. Assembly corrections addressed
the S32K UART routing, camera supply selection, and a missing S32K-Teensy
handshake connection. Incorporate these corrections into the schematic and
next PCB revision, with connector-to-firmware pin checks before manufacture.

Battery-powered SPI failures may involve supply capacity, voltage drop, ground
return paths, or signal integrity. Earlier observations identified the shared
S32K-board 5 V regulator as a possible bottleneck; the root cause remains open.
Measure rail voltage and ripple at each board, current demand, regulator
temperature, and SPI levels under camera/display/ESP load. Repeat with motors
enabled only after communication is stable with stationary actuators.

A multilayer revision can provide continuous return paths and reduce jumpers.
Retain accessible test points and verify connector pin assignments, voltage
levels, and current ratings. Standardize remaining module connections where
practical.

Mechanical refinements are a battery holder supporting multiple pack sizes,
easier ultrasonic-sensor retention, and stronger LED attachment to the existing
mount. Keep sensor position repeatable: camera height changes apparent lane
width and requires detector recalibration.

## Software Priorities

| Priority | Work | Completion check |
|---|---|---|
| 1 | Repeat link and vehicle regression tests after hardware corrections | Stable battery-powered counters, correct Start/Stop and fault responses, recorded lap and endurance results |
| 2 | Tune vision jitter, confidence, and finish detection | Recorded difficult frames produce consistent lane estimates and repeatable finish transitions |
| 3 | Extend camera debug export with gradients, thresholds, candidates, and finish geometry | PC captures explain why a candidate was accepted or rejected |
| 4 | Make camera clock termination hardware-controlled | Scope verifies bounded readout bursts under SPI/display load |
| 5 | Integrate buffered SD logging with real sensor/vehicle snapshots | Bounded foreground latency, explicit overflow counters, valid CSV and reliable close behavior |
| 6 | Refine Honor speed transitions and distance filtering | Repeatable obstacle stopping and defined behavior for missing/stale measurements |

The SD logger already uses a RAM ring buffer and chunked writes. Optimize the
existing path based on latency measurements before selecting a different DMA
or storage design. The current SD Test records synthetic inputs; Race and
Stack Drive do not yet log their runtime telemetry.

## Further Extensions

- **Second camera:** add look-ahead perception and define how two estimates
  affect steering and speed; the protocol already reserves a second camera slot.
- **Finish sensing:** evaluate a secondary sensor if camera geometry and temporal
  filtering do not provide repeatable detection.
- **Differential drive:** use the existing two ESC channels for independent
  rear-wheel commands, with a coordinated steering/speed policy.
- **IMU feedback:** establish calibration, coordinate conventions, and useful
  vehicle-dynamics measurements before applying understeer/oversteer correction.
- **PC telemetry:** add high-level state and active-parameter readback to the web
  interface; the present UI sends requests and receives command results.

Rear-wheel steering or a different propulsion layout would require a separate
mechanical/control evaluation and is lower priority than baseline reliability.

## Current Implementation Reference

The active SPI contract is **v3, 84 bytes**, with blocking transfers. Steering
in Teensy-camera race runs at **50 Hz**; 200 Hz camera acquisition and link
slots are separate schedules. Race-family line loss has a three-second stop
threshold; the simpler drive bench has different behavior. The current Honor
policy uses distance-based speed targets, not the removed stop-hold/crawl
state machine.

Use the [mode matrix](modes.md), [S32K application guide](../firmware/s32k.md),
and [tuning protocol](../protocols/tuning.md) for current behavior and packet formats.
