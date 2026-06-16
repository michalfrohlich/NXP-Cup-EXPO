# Teensy SD Logger

The Teensy SD logger writes telemetry-style CSV rows to the Teensy 4.1 built-in
SD card slot using SdFat SDIO.

## Files

| Area | File |
|---|---|
| Logger API | `firmware/teensy/include/logging/teensy_sd_logger.h` |
| Logger implementation | `firmware/teensy/src/logging/teensy_sd_logger.cpp` |
| Standalone test mode | `firmware/teensy/src/app/modes/mode_sd_log_test.cpp` |

## Standalone Test

The standalone test mode is selected by the `teensy41-sdtest` PlatformIO
environment:

```bat
cd firmware\teensy
pio run --environment teensy41-sdtest
pio run --environment teensy41-sdtest -t upload
pio device monitor -b 115200
```

Expected behavior:

- A FAT32 or exFAT card in the built-in Teensy slot creates `LOGnnn.CSV`.
- Serial prints `SDTEST ready=1 err=0 ...` while rows are being queued and
  written.
- The RGB LED is green when logging is ready, yellow when no card/open failed,
  and red after a logger error.
- The mode holds READY low because it does not run the S32K SPI slave.

The test rows use the normal telemetry CSV schema. Pot/button values are encoded
into synthetic IMU fields only for this standalone test; race logging should
publish real runtime telemetry when it is wired in later.

## Timing Model

`logRow()` formats a row into a RAM ring buffer and does not access the card.
The logger reserves 768 bytes of free ring-buffer space before starting a row,
which is deliberately larger than the current CSV row size so rows are not
partially written during normal operation.
`service()` performs the card work from the foreground loop:

- writes at most one 512-byte chunk per call;
- skips while the card reports busy;
- syncs the file every 2 seconds;
- drops rows instead of blocking when the RAM buffer is full.

This keeps the logger suitable for later race-runtime integration, where SD
writes must be kept away from the time-critical S32K SPI transfer window.
