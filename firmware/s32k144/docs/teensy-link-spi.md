# Teensy Link SPI Bring-Up

This is the S32K master-side bring-up path for the new full-duplex Teensy SPI
link.

## Run Modes

The link test can run from the normal runtime bench menu. With:

```c
#define APP_TEST_NXP_CUP_TESTS            1
```

select `Teensy Link` on the OLED menu and turn the mode switch on.

The dedicated compile-time mode is also available:

```c
#define APP_TEST_TEENSY_LINK_TEST         1
```

Both paths run the same `teensy_link_test_enter/update/exit` logic. The
standalone mode boots directly into `mode_teensy_link_test()`. The test runs
the SPI exchange every 5 ms and uses SW2 to cycle OLED pages:

- link status,
- camera 0 result,
- camera 1 result,
- IMU/logger status.

## Hardware Setup

The generated S32K SPI setup is:

| Signal | S32K pin | Generated name |
|---|---:|---|
| SCK | PTB14 | `TeensySpiSck` |
| MISO / S32K SIN | PTB15 | `TeensySpiMiso` |
| MOSI / S32K SOUT | PTB16 | `TeensySpiMosi` |
| CS | PTB17 / PCS3 | `TeensySpiCs` |
| READY | PTD14 | `TeensyReady` |

READY is diagnostic only. The S32K never blocks waiting for it.

## SPI Timing

- S32K is master.
- Teensy is slave.
- SPI mode 0.
- 8-bit words, MSB first.
- 2 MHz SCK.
- Fixed 128-byte full-duplex transfer every 5 ms.
- Synchronous blocking transfer for v1.

At 2 MHz, the 128-byte transfer is about 512 us of wire time.

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

The bring-up mode calls:

```c
TeensyLink_Service5ms(nowMs, &input);
```

The driver uses:

- `Spi_SetupEB(SpiConf_SpiChannel_TeensySpiFrameChannel, ...)`,
- `Spi_SyncTransmit(SpiConf_SpiSequence_TeensySpiSequence)`,
- `Dio_ReadChannel(DioConf_DioChannel_TeensyReady)`.

`Board_InitDrivers()` initializes the generated SPI driver with `Spi_Init(NULL_PTR)`.
