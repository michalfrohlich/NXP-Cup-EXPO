# Shared Protocol

Home for cross-board protocol contracts once they are stable enough to share.

Use this area for:
- S32K <-> Teensy packet definitions.
- Shared packet validation helpers such as CRC implementations.
- S32K <-> ESP32 command and telemetry definitions.
- Versioned packet layout documentation.

Keep this code portable. Do not include board-specific SDK headers here.

Current active protocol:

- `teensy_link_protocol.h`: fixed 128-byte S32K <-> Teensy SPI frame.
- `teensy_link_crc.h`: CRC-16/CCITT-FALSE helper used by both boards.
- `esp_s32k_uart_protocol.h`: temporary S32K <-> ESP32 UART PID and button/ACK frames.

Run and wiring notes are in `../../docs/protocols/teensy-s32k-128b-spi-test.md`.
