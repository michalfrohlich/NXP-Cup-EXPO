# Teensy Link SPI Bring-Up

This is the S32K master-side bring-up path for the new full-duplex Teensy SPI
link.

## Run Modes

The Teensy link is test-owned for this bring-up step. Normal app modes do not
clock SPI globally. SPI starts in one of two cases:

- `APP_TEST_TEENSY_LINK_TEST` is selected at compile time. The S32K boots
  directly into `mode_teensy_link_test()` from `mode_teensy_link.c`.
- `APP_TEST_NXP_CUP_TESTS` is selected and the `Teensy Link` runtime bench test
  is entered from the OLED menu.

The viewer can run from the normal runtime bench menu. With:

```c
#define APP_TEST_NXP_CUP_TESTS            1
```

select `Teensy Link` on the OLED menu and turn the mode switch on.

The dedicated compile-time mode is also available:

```c
#define APP_TEST_TEENSY_LINK_TEST         1
```

Both paths run the same `teensy_link_test_enter/update/exit` logic. The
standalone mode boots directly into `mode_teensy_link_test()`. Entering the
test calls `TeensyLink_Init()`, then the update loop services the link every
5 ms (the driver schedules the actual transfers, see SPI Timing) and uses SW2
to cycle OLED pages:

- link status,
- camera 0 result,
- camera 1 result,
- IMU/logger status,
- received 128-byte SPI frame counters.

The menu item is named `Teensy Link`. It lives in the runtime test menu, not in
a separate Project Explorer folder.

## Hardware Setup

The generated S32K SPI setup is:

| Signal | S32K pin | Generated name |
|---|---:|---|
| SCK | PTB14 | `TeensySpiSck` |
| MISO / S32K SIN | PTB15 | `TeensySpiMiso` |
| MOSI / S32K SOUT | PTB16 | `TeensySpiMosi` |
| CS | PTB17 / PCS3 | `TeensySpiCs` |
| READY | PTD14 | `TeensyReady` |

READY gates the transfer schedule: the S32K only starts a normal transfer
while READY is high. It never busy-waits on the pin — when READY is low the
service call simply returns and counts a skip (see SPI Timing below).

Connect it to the Teensy 4.1 like this:

| S32K signal | S32K pin | Teensy 4.1 pin | Direction |
|---|---:|---:|---|
| SCK | PTB14 | 13 | S32K to Teensy |
| MOSI / S32K SOUT | PTB16 | 11 | S32K to Teensy |
| MISO / S32K SIN | PTB15 | 12 | Teensy to S32K |
| CS / PCS3 | PTB17 | 10 | S32K to Teensy |
| READY | PTD14 | 31 | Teensy to S32K |
| GND | GND | GND | common reference |

Both boards use 3.3 V logic. Do not route a 5 V signal into either SPI pin.

## SPI Timing

- S32K is master.
- Teensy is slave.
- SPI mode 0.
- 8-bit words, MSB first.
- 2 MHz SCK.
- Synchronous blocking transfer for v1.

The service is still called every 5 ms, but the driver schedules the actual
128-byte transfers itself (`TEENSY_LINK_TRANSFER_PERIOD_MS` and
`TEENSY_LINK_HEARTBEAT_MS` in `include/drivers/teensy_link.h`):

| Condition | Action |
|---|---|
| Less than 10 ms since the last transfer | wait (Teensy sensor data only refreshes at 100 Hz) |
| Due and READY high | normal 128-byte transfer |
| Due and READY low | skip and count it (`notReadySkipCount`) |
| READY low for 25 ms | probe transfer anyway (`heartbeatProbeCount`) so a broken READY wire cannot kill the link |

The very first service call after `TeensyLink_Init()` always transfers.
CRC/stale handling is unchanged: a failed probe keeps the last good data and
the stale timeout (100 ms) still drives the OK/STALE/WAIT status. The 25 ms
probe period leaves room for several probes per stale window, so a single
lost probe does not flash STALE.

What READY means today: the Teensy raises it once after boot, when its first
frame is published, and never lowers it. So the gate currently protects
against an absent or still-booting Teensy. A busy Teensy (serial print, future
SD writes) still keeps READY high and is still clocked; corrupted frames from
that case are caught by CRC exactly as before. If the Teensy firmware later
lowers READY while busy, this master already respects it.

At 2 MHz, the 128-byte blocking transfer is about 512 us of wire time, a
roughly 5 percent CPU duty cycle on the 10 ms schedule (512 us / 10 ms) —
half of the old every-5-ms load — and near-zero wire activity while the
Teensy is absent (only the 25 ms probes).

If the first bench setup is unstable with long jumper wires, lower the S32K
`TeensySpiDevice` baud rate in S32 Design Studio, regenerate, and test the same
128-byte frame again. Keep the packet format unchanged.

## Frame Contract

The portable constants live in:

```text
shared/protocol/teensy_link_protocol.h
```

Frame layout:

| Byte range | Meaning |
|---:|---|
| 0..15 | common header |
| 16..125 | 110-byte payload |
| 126..127 | CRC-16/CCITT-FALSE over bytes 0..125 |

Both directions include exactly two camera result slots. Each slot mirrors the
current `VisionOutput_t` semantics in compact wire form:

```text
error_pct_s8, status_u8, feature_u8, confidence_u8,
left_idx_u8, right_idx_u8, age_ms_u8, flags_u8
```

Unavailable camera data is sent as track lost, confidence 0, indexes 255, and
the stale/invalid flag set.

## S32K Driver

The S32K driver API is in:

```text
include/drivers/teensy_link.h
src/drivers/teensy_link.c
```

The bench/direct test update path calls:

```c
TeensyLink_Service5ms(nowMs, &input);
```

The driver uses:

- `Spi_SetupEB(SpiConf_SpiChannel_TeensySpiFrameChannel, ...)`,
- `Spi_SyncTransmit(SpiConf_SpiSequence_TeensySpiSequence)`,
- `Dio_ReadChannel(DioConf_DioChannel_TeensyReady)`.

`Board_InitDrivers()` initializes the generated SPI driver with `Spi_Init(NULL_PTR)`.
`src/app/modes/bench_teensy_link.c` owns the test state, input packet, and
periodic service call. `src/app/modes/mode_teensy_link.c` is only the
compile-time direct wrapper around that same test.

## OLED Acceptance Test

Use the S32K OLED while the Teensy serial monitor is open.

1. Upload the Teensy PlatformIO project from `firmware/teensy`.
2. Build and flash the S32K project from `firmware/s32k144`.
3. Either enable `APP_TEST_TEENSY_LINK_TEST` for direct boot, or leave
   `APP_TEST_NXP_CUP_TESTS` enabled and enter the `Teensy Link` menu item.
4. Check the Teensy serial monitor. `s32k` should become nonzero and `rx`
   should increase while the test is running.
5. Press SW2 to cycle pages.

Expected OLED behavior:

| Page | What it proves |
|---|---|
| `TLINK` | Link status, S32K sequence, Teensy sequence, READY, errors (`E:`) and READY-low skips (`SK:`) |
| `TLINK CAM0` | Teensy camera 0 slot decodes correctly |
| `TLINK CAM1` | Missing/stale camera data is handled without breaking the link |
| `TLINK IMU/LOG` | IMU and logger fields decode correctly |
| `TLINK RX 128B` | 128-byte RX counters, ACK, CRC, and SPI errors |

The RGB LED is green when the S32K sees live valid frames. Blue means no frame
yet, yellow means stale data, and red means repeated protocol/SPI errors.
