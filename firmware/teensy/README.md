# Teensy Firmware

Teensy-side firmware for the S32K-master SPI link.

Current layout:
- `include/teensy_config.h`: Teensy pins and timing constants.
- `include/comms/` and `src/comms/`: Teensy SPI slave transport.
- `include/telemetry/` and `src/telemetry/`: packing and decoding for the shared 128-byte `teensy_link` frame.
- `src/main.cpp`: bring-up loop with mock IMU/camera data.

The shared packet contract lives in `../../shared/protocol/teensy_link_protocol.h`.
Do not reintroduce the old 45-byte IMU packet as active code.

The current `TeensyLinkSpiSlave` keeps the polling slave structure from the
bring-up sketch, but now transfers the v1 128-byte frame and decodes MOSI from
the S32K. For the configured 2 MHz S32K link, replace this transport with a
hardware SPI slave implementation while keeping the same public API.
