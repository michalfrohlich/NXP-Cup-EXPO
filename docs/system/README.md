# Architecture And Responsibilities

The S32K144 is the authority for steering, motor commands, race phases, and
vehicle fault handling. Teensy camera results and ESP commands are inputs to
that application. Neither remote board directly drives the motors or servo.

[![System interface architecture: controllers, sensors, actuators, displays, and communication paths](../assets/system-interface-architecture.png)](../assets/system-interface-architecture.png)

The diagram includes planned interfaces. SD logging currently runs in a separate
Teensy test; race logging and persistent tuning storage are not integrated.
The ESP display shows link/request diagnostics; PC vehicle telemetry is planned.
The S32K local-camera path used by Stack Drive and S32K-only modes is not shown.
Firmware mode selection determines which peripherals are serviced.

## Board Ownership

| Board | Owns | Does not currently own |
|---|---|---|
| S32K144 | Dual ESC PWM, servo PWM, ultrasonic, local linear camera, race/bench state machines, profiles, runtime tuning, SPI master, UART command consumption | Wi-Fi, Teensy camera acquisition, IMU filtering |
| Teensy 4.1 | Camera 0 acquisition and line detection, MPU6050 acquisition/filtering, SPI slave telemetry, local buttons/RGB/display; separate SD test | Vehicle safety decisions or motor outputs; IMU-based slip correction is not implemented |
| ESP32 | Wi-Fi AP, HTTP server and tuning page, UART requests/results, local diagnostics | Control algorithms, continuous car-status dashboard, persistent settings |
| PC or phone | Browser interaction; optional USB serial camera viewer | A required web server: the ESP32 serves the page itself |

See the [mode combinations](modes.md) before connecting boards: compiling a
driver does not mean the selected application services it.

## Data And Command Paths

**Full race:** Teensy camera -> line detector -> SPI camera result -> S32K
sample selection/averaging -> steering controller -> output shaping -> servo.
S32K race logic separately selects speed, applies its ramp, and commands both
ESCs. Ultrasonic measurements affect the Honor phase.

**Stack Drive:** the S32K camera and local detector replace the Teensy steering
source. Teensy camera/IMU processing and SPI telemetry may continue, but their
absence does not itself stop this mode. Local camera and timing checks remain
required.

**Live tuning:** browser -> ESP HTTP validation -> UART request -> S32K runtime
tune state. S32K applies steering tuning locally and sends detector parameters
to Teensy over SPI v3 when the mode services the link. Teensy reports the applied
generation asynchronously. [Acknowledgment details](../protocols/tuning.md)
distinguish S32K acceptance from Teensy application.

**Start/Stop:** the ESP transports the request; the S32K mode decides what it
means. Stack Drive changes vehicle state; ESP link tests acknowledge without
motion. Standalone race does not consume these requests.

## Software Boundaries

| Area | Responsibility |
|---|---|
| `firmware/s32k144/src/app/` | Mode selection, behavior, orchestration, UI, profiles, actuator safety |
| S32K `src/services/` | Line detection, steering calculation, output filtering |
| S32K `src/drivers/` | Hardware and RTD-facing drivers, including board links |
| S32K `src/debug/` | PC cable UART/debug transport |
| `firmware/teensy/src/app/modes/` | Selects enabled work for Race, Stack Drive, and tests |
| Teensy `src/services/` | Camera processing, shared sensor/link runtime, tracking LED policy |
| Teensy drivers/comms/telemetry/logging | Hardware access, SPI transport, serialization, optional SD logger |
| `firmware/esp32/src/` | Flat app: `main.c`, `pc_web_link.c`, `s32k_uart_link.c`, `esp32_display.c` |
| `shared/protocol/` | Portable packet types, offsets, encoders, validation, CRC; no board SDK headers |

S32K drivers are intentionally RTD-coupled, not a portable HAL.
`SteeringController_Update()` produces a steering request, not hardware writes
or throttle/brake decisions. App code owns the combined vehicle action. An
eventual IMU-based dynamics controller needs an explicit integration step;
sending IMU telemetry does not already provide understeer compensation.

## Execution And Timing

| Board/path | Execution model | Timing implications |
|---|---|---|
| S32K | Bare-metal foreground loop plus peripheral interrupts | Camera/PWM/timebase/UART interrupts run independently; foreground work must be serviced frequently |
| Full race / Tune Drive | Servo-period scheduler | Four nominal SPI slots per 20 ms PWM period, control from phase 17 ms, staging deadline 19 ms |
| Local-camera race / Stack Drive | Camera consumption plus 10 ms control schedule | No four-sample Teensy estimator |
| Teensy | Arduino loop with hardware-assisted acquisition | DMA handles pixels; foreground publishes frames, processes vision, reads IMU, and services SPI |
| ESP32 | ESP-IDF/FreeRTOS tasks | UART task processes responses while HTTP waits and the main task refreshes the display |

SPI blocks the S32K foreground during a transfer. At 500 kHz, 84 bytes need
1.344 ms of clock time plus software/peripheral overhead. Nominal schedules
are not measured worst-case execution bounds. Synchronous OLED work can delay
foreground services; full race suppresses live OLED refresh while driving
from Teensy vision.

Teensy's current SPI slave also blocks: it bit-bangs the transfer with
interrupts masked. Its camera PWM/ADC/DMA hardware continues independently,
but foreground frame completion and other services wait for SPI to finish.

## Configuration Ownership

- S32K `src/app/app_config.h`: build selection, application timing, safety
  thresholds, speed profiles, and mode behavior.
- S32K `include/config/board_config.h`: routing and generated channel symbols.
  RTD IDs must follow regenerated symbols, not copied numeric IDs.
- S32K sensor/actuator configs: geometry, timing, calibration; vision/control
  configs: baseline algorithm parameters. RAM tuning overrides only fields
  explicitly consumed by a mode.
- Teensy `teensy_app_config.h` and `platformio.ini`: mode selection;
  `teensy_config.h`: pins/rates; `include/config/`: camera/display/vision.
- ESP32 `esp32_pin_config.h`: pins/buses; `pc_web_link.c`: fixed AP credentials
  and embedded web UI.

S32K `.mex`, `generate/`, `board/`, and RTD sources remain inside its project.
Change hardware configuration in S32 Config Tools and regenerate. Handwritten
aliases must track generated GPT/ADC/I2C symbols: a wrong timer channel can
stop the millisecond clock before the boot banner is shown.

## Current Limits

Camera 1 is a reserved telemetry slot, not a running second camera pipeline.
Race/Stack Drive do not yet write SD logs. SD Test logs synthetic pot/button
data. ESP does not provide a full live vehicle telemetry feed or read active
tuning back into a newly opened browser. Tuning is volatile. Individual bench
tests do not all implement race safety; see the [mode guide](modes.md).
