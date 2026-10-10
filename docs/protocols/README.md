# Protocol Documentation

## S32K-Teensy SPI Communication Link

### Overview
This folder documents the inter-microcontroller communication protocols used in the NXP Cup multi-board vehicle system.

### Teensy Link (S32K ↔ Teensy)

The **Teensy Link SPI protocol** enables real-time bidirectional communication between the S32K144 (main controller) and Teensy 4.1 (sensor/camera host).

**Key Characteristics:**
- **Type**: Full-duplex SPI, synchronous blocking transfers
- **Cadence**: nominal 5 ms (200 Hz) slots, gated by Teensy READY; missed slots are skipped
- **Frame Size**: Fixed 84 bytes (16 B header + 66 B payload + 2 B CRC)
- **Clock**: 500 kHz, SPI Mode 0 (CPOL=0, CPHA=0)
- **Data**: Bidirectional sensor + command exchange
- **Reliability**: CRC-16/CCITT-FALSE error detection, sequence number tracking

**Documentation Files:**

1. **[S32K bring-up notes](https://github.com/michalfrohlich/NXP-Cup-EXPO/blob/main/firmware/s32k144/docs/teensy-link-spi.md)**
   - S32K runtime and standalone test-mode usage
   - OLED page behavior for link, camera, IMU, and logger status
   - Timing and link-health expectations
   - Suggested first hardware checks

2. **[Two-board 84-byte SPI test](teensy-s32k-spi-test.md)**
   - VS Code and S32 Design Studio folders to open
   - S32K to Teensy wiring table
   - PlatformIO commands for the Teensy
   - OLED pages and expected pass/fail behavior
   - Specific bench tests for MOSI, MISO, missing camera, stale, and CRC paths
   - Pointer to the shield V2 pinout in `hardware/shield-v2-pinout.md`

3. **[Shared protocol header](https://github.com/michalfrohlich/NXP-Cup-EXPO/blob/main/shared/protocol/teensy_link_protocol.h)**
   - Versioned 84-byte frame contract
   - Header, payload, flag, and offset definitions
   - Two-camera result slots and IMU/logger fields

4. **[Shared CRC helper](https://github.com/michalfrohlich/NXP-Cup-EXPO/blob/main/shared/protocol/teensy_link_crc.h)**
   - Portable CRC-16/CCITT-FALSE implementation used by both sides

### Quick Facts

| Aspect | Value | Rationale |
|--------|-------|-----------|
| **Wire time** | ~1.344 ms per transfer | 84 bytes at 500 kHz; excludes software overhead and sensor age |
| **Frequency** | 200 Hz target (5 ms slots) | READY and foreground scheduling determine actual transfers |
| **Payload** | 66 B | Control commands + sensor telemetry |
| **Camera identity** | 16-bit sequence per camera | Rejects repeated telemetry as duplicate camera data |
| **Error Detection** | CRC-16/CCITT | Catches bit flips, validates on RX |
| **Sequence Tracking** | 16-bit per direction | Detects dropped frames |
| **Detector tuning** | S32K generation + Teensy applied-generation ACK | Applies camera-0 thresholds asynchronously |
| **Link stale timeout** | 100 ms | Expires link telemetry; camera and IMU validity have separate checks |

## ESP32-S32K UART Web Link

Read [Tuning and drive commands](tuning.md) for mode support, editable values,
request timing, and the separate Teensy configuration acknowledgment.

The ESP32 web UI communicates with the S32K over UART using the shared frame
definitions in `shared/protocol/esp_s32k_uart_protocol.h`.

Current ESP-to-S32K request frames are:

| Purpose | Format | Notes |
|---|---|---|
| Tuning snapshot | `#TnnPxxxxxIxxxxxDxxxxxCxxxLxxxxMxxxxHxxxExxxXcc_` | PID, steering shaping, and line-detector RAM tune values |
| Drive command | `#CnnSxXcc_` | `S0` stop, `S1` start |

The S32K replies to both request types with `#RnnOXcc_` for accepted or
`#RnnEXcc_` for rejected. `nn` is the request sequence and `cc` is CRC-8/ATM
over the frame payload before `X`.

### Implementation References

- **Protocol Header**: `shared/protocol/teensy_link_protocol.h`
- **S32K Driver**: `firmware/s32k144/src/drivers/teensy_link.c`
- **S32K API**: `firmware/s32k144/include/drivers/teensy_link.h`
- **S32K Bring-up Test**: `firmware/s32k144/src/app/modes/bench_teensy_link.c`
- **Teensy PlatformIO Project**: `firmware/teensy/platformio.ini`

---

Protocol v3 carries camera-0 tuning without changing the 84-byte transfer.
The [mode matrix](../system/modes.md) identifies which applications service
each link; the existence of a packet field does not imply an active feature.
