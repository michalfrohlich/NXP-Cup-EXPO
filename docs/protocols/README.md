# Protocol Documentation

## S32K-Teensy SPI Communication Link

### Overview
This folder documents the inter-microcontroller communication protocols used in the NXP Cup multi-board vehicle system.

### Teensy Link (S32K ↔ Teensy)

The **Teensy Link SPI protocol** enables real-time bidirectional communication between the S32K144 (main controller) and Teensy 4.1 (sensor/camera host).

**Key Characteristics:**
- **Type**: Full-duplex SPI, synchronous blocking transfers
- **Cadence**: 5 ms (200 Hz) fixed service interval
- **Frame Size**: Fixed 128 bytes (16 B header + 110 B payload + 2 B CRC)
- **Clock**: 2 MHz, SPI Mode 0 (CPOL=0, CPHA=0)
- **Data**: Bidirectional sensor + command exchange
- **Reliability**: CRC-16/CCITT-FALSE error detection, sequence number tracking

**Documentation Files:**

1. **[S32K bring-up notes](../../firmware/s32k144/docs/teensy-link-spi.md)** - **START HERE**
   - S32K runtime and standalone test-mode usage
   - OLED page behavior for link, camera, IMU, and logger status
   - Timing and link-health expectations
   - Suggested first hardware checks

2. **[Two-board 128-byte SPI test](teensy-s32k-128b-spi-test.md)**
   - VS Code and S32 Design Studio folders to open
   - S32K to Teensy wiring table
   - PlatformIO commands for the Teensy
   - OLED pages and expected pass/fail behavior
   - Specific bench tests for MOSI, MISO, missing camera, stale, and CRC paths
   - Pointer to the shield V2 pinout in `hardware/shield-v2-pinout.md`

3. **[Shared protocol header](../../shared/protocol/teensy_link_protocol.h)**
   - Versioned 128-byte frame contract
   - Header, payload, flag, and offset definitions
   - Two-camera result slots and IMU/logger fields

4. **[Shared CRC helper](../../shared/protocol/teensy_link_crc.h)**
   - Portable CRC-16/CCITT-FALSE implementation used by both sides

### Quick Facts

| Aspect | Value | Rationale |
|--------|-------|-----------|
| **Latency** | ~0.5 ms per frame | 512 μs wire time (128 B ÷ 2 MHz) |
| **Frequency** | 200 Hz (5 ms) | Matches control loop + sensor rates |
| **Payload Used** | ~70 B of 110 B | Control commands + sensor telemetry |
| **Unused Padding** | ~40 B | Reserved for future expansion |
| **Error Detection** | CRC-16/CCITT | Catches bit flips, validates on RX |
| **Sequence Tracking** | 16-bit per direction | Detects dropped frames |
| **Stale Timeout** | 100 ms | Marks old sensors as unreliable |

### For Your Engineering Report

**Include:**
1. Executive summary of the S32K-master, Teensy-slave decision.
2. Physical layer specs and pin assignments.
3. Fixed frame structure: 16-byte header, 110-byte payload, 2-byte CRC.
4. Rationale for fixed-size full-duplex polling at 5 ms.
5. Pros/cons against UART, I2C, variable-length SPI, and Teensy-master SPI.
6. Timing budget: about 512 us wire time at 2 MHz inside a 5 ms service slot.
7. Acceptance behavior: valid CRC/sequence updates, OLED leaves WAIT, stale within 100 ms.

**Recommended Order for Report:**
1. Start with a block diagram of the S32K master polling the Teensy slave.
2. Explain the current frame layout from `teensy_link_protocol.h`.
3. Explain the 5 ms schedule and 100 ms stale timeout.
4. Show the OLED bring-up mode as the verification path.
5. Summarize trade-offs and alternatives considered.
6. Conclude with why this is the V1 competition protocol.

### Implementation References

- **Protocol Header**: `shared/protocol/teensy_link_protocol.h`
- **S32K Driver**: `firmware/s32k144/src/drivers/teensy_link.c`
- **S32K API**: `firmware/s32k144/include/drivers/teensy_link.h`
- **S32K Bring-up Test**: `firmware/s32k144/src/app/modes/bench_teensy_link.c`
- **Teensy PlatformIO Project**: `firmware/teensy/platformio.ini`

---

**Status**: Complete for V1 bench bring-up | Last Updated: June 2026
