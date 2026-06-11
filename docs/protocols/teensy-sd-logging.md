# Teensy SD Logging (SdFat MVP)

## Goal

The Teensy 4.1 logs the car's telemetry to its built-in SD card slot so the
data can be loaded into MATLAB later for PID tuning and calibration.

The logger must never disturb the SPI link or control timing. The Teensy SPI
slave is bit-banged, so a long blocking SD write inside `loop()` would make
the Teensy miss an S32K transfer. The whole design below exists to keep SD
writes tiny and out of the way.

## How it works

```text
sensor update (100 Hz)          loop(), every pass
        |                               |
   logRow(): format CSV row     service(): if CS idle and card not busy,
   into a RAM ring buffer  -->  write ONE 512-byte chunk, sync every 2 s
   (no SD access at all)        (the only place SD is touched)
```

- `logRow()` only formats text into a 32 KiB RAM ring buffer (about 1.5 s of
  rows at 100 Hz). It never touches the card.
- `service()` runs from `loop()` only, and only while the SPI chip-select
  line is idle. It writes at most one 512-byte sector per call and skips the
  call entirely while the card reports busy.
- The file is opened once at boot, the CSV header is written once, and the
  file space is pre-allocated (64 MiB) so writes never stop to allocate
  clusters mid-run. There is no open/close per row.
- `file.sync()` runs every 2 seconds, so pulling the power loses at most the
  last 2 seconds of rows.
- **No SD card:** the firmware keeps running, serial shows `sd=-`, and the
  link logger flags stay clear.
- **Card falls behind or breaks:** rows are dropped (never blocking) and
  counted; serial shows `sd=E` on error.

## Files

| Area | File |
|---|---|
| Logger API | `firmware/teensy/include/logging/teensy_sd_logger.h` |
| Logger implementation | `firmware/teensy/src/logging/teensy_sd_logger.cpp` |
| Hook-up (setup/loop/serial) | `firmware/teensy/src/main.cpp` |

SdFat 2.1.2 ships inside the Teensy Arduino framework used by PlatformIO, so
`platformio.ini` needs no extra library line. The logger uses `SdFs` +
`SdioConfig(FIFO_SDIO)` (the fast built-in SDIO slot, not SPI-mode SD).

## Log files

Each boot creates the next free file: `LOG000.CSV`, `LOG001.CSV`, ...
`LOG999.CSV` in the card root.

## CSV columns

One row per Teensy sensor sample (100 Hz). All multi-byte values are decoded
from the live link state at the moment the row is queued.

| Column(s) | Source | Notes |
|---|---|---|
| `time_ms` | Teensy `millis()` | row timestamp |
| `log_seq` | logger | row counter, gaps = dropped rows |
| `tx_seq`, `sensor_seq`, `sensor_dt_us`, `sensor_age_ms` | Teensy | link + sensor sequences |
| `s32k_valid` | Teensy decode | **1 = the s32k_* columns are real; 0 = no valid S32K frame yet and they are all 0** |
| `s32k_seq`, `rx_frames`, `rx_errors`, `timeouts` | SPI slave | link health |
| `s32k_app_mode`, `s32k_app_state`, `s32k_control_seq`, `s32k_control_dt_us`, `s32k_safety_flags` | S32K frame | control-loop correlation IDs |
| `ax_g .. imu_temp_c` (12 cols) | Teensy IMU struct | **currently MOCK data — there is no real IMU driver yet** |
| `cam0_*`, `cam1_*` (7 cols each) | Teensy camera slots | currently mock (cam0) and intentionally-missing (cam1) |
| `ultra_dist_cm10`, `ultra_age_ms`, `ultra_flags` | S32K frame | zeros in the bench test (S32K sends no ultrasonic data in the link test) |
| `steer_raw/filt/out`, `target/current_speed_pct`, `esc_primary/secondary`, `servo_cmd`, `actuator_flags` | S32K frame | zeros in the bench test for the same reason |
| `logger_flags`, `logger_drops` | logger | same values sent to the S32K over SPI |

Honest-data rule: nothing is invented. Columns whose producer does not exist
yet (real IMU, real cameras, race-mode actuator values) log their current
real value, which today is mock or zero. The `s32k_valid` column says whether
the S32K block ever meant anything.

## Status visibility

- **Teensy serial** (every 100 ms), appended to the existing line:
  `... sd=R drop=0 sdkB=12` → `R` ready, `-` no card, `E` error;
  `drop` = dropped rows; `sdkB` = KiB written so far.
- **S32K OLED**, Teensy Link test, IMU/LOG page: `SD: Y/N` and `DROP:` come
  from the same `loggerFlags`/`loggerDropCount` protocol fields, which now
  carry real values. No S32K change was needed.

## How to run

```bat
cd /d C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
pio run
pio run -t upload
pio device monitor -p COM3 -b 115200
```

1. Put a FAT32/exFAT SD card in the Teensy 4.1 built-in slot.
2. Upload and open the monitor: expect `SD logger ready, file: LOGnnn.CSV`.
3. Run the S32K Teensy Link test as usual (see
   `teensy-s32k-128b-spi-test.md`) if you want S32K columns filled.
4. Power down, put the card in a PC, open `LOGnnn.CSV` — MATLAB:
   `T = readtable("LOG000.CSV");`

## Limitations (MVP)

- IMU/camera columns contain mock data until real drivers land.
- A normal shutdown is "pull the power": the file keeps everything up to the
  last 2-second sync. The file may show a 64 MiB pre-allocated size on some
  tools until properly closed; the CSV content ends at the real data.
- A single 512-byte write occasionally takes longer than the gap between two
  S32K transfers. If chip-select asserts mid-write, that frame is lost; the
  S32K side already tolerates this (CRC counter + keeps last good data). The
  proper fix is the planned hardware SPI slave.
- Counters (`drop`, `rx`, sequences) are 16-bit and wrap at 65535.
