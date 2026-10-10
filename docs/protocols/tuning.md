# Tuning And Drive Commands

S32K owns the volatile runtime tuning state, `g_runtimeTune`. Cable Tune and
ESP requests update that state. SPI v3 distributes its three detector values
to Teensy camera 0 when the selected S32K mode services SPI.

## Editable Values

| Value | ESP UI range | Stored meaning |
|---|---|---|
| KP, KI, KD | 0..99.999 each | Steering PID gains; UART uses integer milli-units |
| Steering clamp | 0..100 | Magnitude limit for steering output |
| Steering LPF | 0..1.000 | Output low-pass alpha; UART uses milli-units |
| `minContrast` | 0..4095 | Minimum detector contrast in ADC counts |
| `edgeHighPct`, `edgeLowPct` | 1..100 each | Detector edge percentages |

Cable Tune exposes the same eight settings through its text menus. Connect
at 115200 8N1 on the S32K **host/cable UART**, enter `c`, then use `p/i/d`,
`s` for servo settings, or `v` for vision. Vision keys are `m/h/l`; integer
input ends with `#` to save or `q` to cancel. Its text parser is more permissive
than the web form for some steering fields; use the meaningful ranges above.
Do not confuse this port with the separate 470588-baud ESP UART.

## Where Values Take Effect

| Mode | Steering tuning | Vision tuning |
|---|---|---|
| Camera-s32k | No steering | Compile-time defaults (`LineDetector_ProcessDebug`) |
| Cam+Servo-s32k | Controller defaults and local shaping | Compile-time defaults; does not read runtime detector values |
| Drive mode-s32k | Stored profile applied at entry | S32K runtime parameters |
| Stack Drive | Stored profile plus live ESP updates | S32K runtime parameters; also forwards to Teensy |
| Tune Drive | Stored profile plus live ESP updates | Forwarded to Teensy camera 0 over SPI v3 |
| `APP_RACE` | Selected startup speed profile | Active/default runtime thresholds forwarded over SPI; no live ESP handler |
| `APP_RACE_S32K_ONLY` | Selected startup speed profile | Local runtime thresholds; no live ESP handler |
| Teensy Link | No actuator tuning applied | Forwards S32K's stored detector values |
| ESP Link / Cable Tune | Stores settings for subsequent consumers | Stores values; does not itself service Teensy SPI |
| Teensy Camera Bench | No vehicle steering | Local Teensy configuration only |

The shared profile also contains base speed, integral clamp, and rate limit;
these are not additional editable fields in the current ESP page. Selecting a
standalone race speed profile replaces the steering/speed profile. Runtime
values survive bench changes until a reset; no flash/SD persistence exists.

## PC To S32K

The browser posts a complete snapshot to `/tune`, or a drive request to
`/drive`. ESP validates the form, encodes the shared ASCII/CRC-8 packet, sends
it over UART, and waits for a matching sequence result. On S32K, the ISR copies
bytes into a 256-byte ring; foreground parsing validates complete frames.
Application helpers store/apply the tune or consume the command and queue the
result. Parsing alone does not execute a vehicle command.

| Frame | Bytes | Role |
|---|---:|---|
| `#T...Xcc_` | 48 | Complete tuning snapshot |
| `#CnnSxXcc_` | 10 | Drive request: `S0` Stop, `S1` Start |
| `#RnnOXcc_` / `#RnnEXcc_` | 9 | Accepted/rejected result |
| `#B...Xcc_` / `#A...Xcc_` | 17 | Separate button/ACK transport diagnostics |

At 470588 baud, 8N1, a tune frame uses about 1.02 ms of wire time and a drive
request about 0.21 ms. ESP waits 150 ms per attempt and retries once. HTTP
success requires a matched accepted result; timeout returns 504, rejection
422, malformed form 400, and transport failure 500. Browser latency also
includes Wi-Fi, scheduling, and possible request serialization.

Start/Stop changes motion only in Stack Drive. The two ESP tests acknowledge
it without motion. Tune Drive services tune snapshots but not drive-command
consumption. Standalone race services neither. A disconnected ESP is not a
vehicle heartbeat fault, and a web Stop is not a hardware emergency stop.

## S32K To Teensy

1. `RuntimeTune_SetLineDetectorParams()` validates contrast 0..4095 and both
   percentages 1..100. It changes generation only if values change, skipping
   zero on 16-bit wrap.
2. Each serviced S32K SPI frame includes the active generation and values.
3. Teensy validates and applies them to camera 0 without resetting tracking
   history. Invalid requests retain the previous config and acknowledgment.
4. A subsequently prepared response includes the applied generation. The
   response shifted during the request was already prepared; do not assume a
   same-transfer acknowledgment.
5. The next processed camera frame uses the updated detector settings.

`CFG == ACK` in S32K link diagnostics means Teensy has applied that generation,
not that the car has detected a line or that an image processed with those
parameters has already reached S32K. ESP success means S32K acceptance; it
does not wait for Teensy. Repeated publication lets boards resynchronize after
an independent reset. Both generation and values are checked on Teensy.

Both boards must use **v3**, fixed **84 bytes**. Old v2 firmware is rejected
by version validation. See the [SPI contract](teensy-s32k-spi-test.md).

## Source References

- [Portable UART contract](https://github.com/michalfrohlich/NXP-Cup-EXPO/blob/main/shared/protocol/esp_s32k_uart_protocol.h)
- [Portable SPI contract](https://github.com/michalfrohlich/NXP-Cup-EXPO/blob/main/shared/protocol/teensy_link_protocol.h)
- [S32K runtime tune state](https://github.com/michalfrohlich/NXP-Cup-EXPO/blob/main/firmware/s32k144/src/app/app_common.c)
