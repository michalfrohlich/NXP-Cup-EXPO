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
test calls `TeensyLink_Init()`, then the update loop clocks the SPI exchange
every 5 ms and uses SW2 to cycle OLED pages:

- link status,
- camera 0 result,
- camera 1 result,
- IMU/logger status,
- received 84-byte SPI frame counters.

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

READY gates each transaction. The S32K skips service while READY is low.

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
- Fixed 84-byte full-duplex transfer every 5 ms.
- Synchronous blocking transfer for protocol v2.

At 2 MHz, the 84-byte transfer is about 0.336 ms of wire time, or 6.72 percent
of each 5 ms service period.

The Teensy uses one fixed-pin software bit-banged backend. The failed
DMA/LPSPI4 experiment is not selectable in the current code. The S32K remains
the hardware-SPI master.

If the first bench setup is unstable with long jumper wires, lower the S32K
`TeensySpiDevice` baud rate in S32 Design Studio, regenerate, and test the same
84-byte frame again. Keep the packet format unchanged.

## Frame Contract

The portable constants live in:

```text
shared/protocol/teensy_link_protocol.h
```

Protocol v2 is not wire-compatible with the previous 80-byte v1 frame. Flash
the matching S32K and Teensy builds together.

Frame layout:

| Byte range | Meaning |
|---:|---|
| 0..15 | common header |
| 16..81 | 66-byte payload |
| 82..83 | CRC-16/CCITT-FALSE over bytes 0..81 |

Both directions include exactly two camera result slots. Each slot mirrors the
current `VisionOutput_t` semantics in compact wire form:

```text
error_pct_s8, status_u8, feature_u8, confidence_u8,
left_idx_u8, right_idx_u8, age_ms_u8, flags_u8, sequence_u16
```

Unavailable camera data is sent as track lost, confidence 0, indexes 255, and
the stale/invalid flag set.

The camera sequence increments when that camera produces a new processed
result. Repeated SPI telemetry can therefore carry the same camera sequence
without being treated as a new control sample.

## Deterministic Teensy-Camera Race Schedule

`APP_TEST_TEENSY_CAM0_RACE` uses the 50 Hz servo PWM rising edge as the master
20 ms epoch:

| PWM phase | Work |
|---:|---|
| 0, 5, 10, 15 ms | one SPI exchange; missed slots are skipped |
| 17 ms | average up to four unique camera results and run control once |
| `17 <= phase < 19 ms` | stage at most one servo duty update |
| 20 ms | hardware starts the next PWM period |

The estimator rejects invalid, stale, and track-lost samples. With three or
four samples it may reject one sample whose error is the largest deviation
from the median and exceeds 0.20 normalized error. It then computes a
confidence-weighted mean, or the accepted-sample median if all confidence
weights are zero. No prediction is used.

If control has not run by phase 19 ms, the previous PWM command is held and a
deadline miss is recorded. OLED refresh is kept out of active race phases.

## S32K Driver

The S32K driver API is in:

```text
include/drivers/teensy_link.h
src/drivers/teensy_link.c
```

The bench/direct test update path calls:

```c
TeensyLink_Service(nowMs, &input);
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
| `TLINK` | Link status, S32K sequence, Teensy sequence, READY, total errors |
| `TLINK CAM0` | Teensy camera 0 slot decodes correctly |
| `TLINK CAM1` | Missing/stale camera data is handled without breaking the link |
| `TLINK IMU/LOG` | IMU and logger fields decode correctly |
| `TLINK RX 84B` | 84-byte RX counters, ACK, CRC, and SPI errors |
| `TLINK RAW HEADER` | Header bytes match the expected Teensy frame |

The RGB LED is green when the S32K sees live valid frames. Blue means no frame
yet, yellow means stale data, and red means repeated protocol/SPI errors.
