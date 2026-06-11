# Integrated Teensy Display, SPI, And SD Test

This firmware runs the Teensy displays, S32K SPI communication, and SD logger
together. No display mode needs to be selected.

## What Should Always Be Visible

- S32K OLED: controlled by the S32K firmware and shows its selected mode/test.
- Teensy display 1: S32K-Teensy SPI status.
- Teensy display 2: Teensy SD logger status.

If a Teensy display is missing, the rest of the firmware continues running and
Serial reports `NOT_CONNECTED`.

## Connections

S32K to Teensy:

| S32K signal | S32K pin | Teensy 4.1 pin |
|---|---:|---:|
| SCK | PTB14 | 13 |
| MOSI / S32K SOUT | PTB16 | 11 |
| MISO / S32K SIN | PTB15 | 12 |
| CS / PCS3 | PTB17 | 10 |
| READY | PTD14 | 31 |
| Ground | GND | GND |

Teensy displays:

| Signal | Teensy 4.1 pin |
|---|---:|
| Display SDA1 | 17 |
| Display SCL1 | 16 |
| Display power | 3.3 V |
| Display ground | GND |

Insert the SD card into the Teensy 4.1 built-in SD slot. The logger uses SDIO,
not the S32K-Teensy SPI wires.

## Build And Upload Teensy

Open this folder in VS Code:

```text
C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
```

Run:

```bat
cd /d C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
pio run
pio run -t upload
pio device list
pio device monitor -p COM3 -b 115200
```

Replace `COM3` with the Teensy port.

Expected Serial fields:

```text
s32k=12 rx=12 err=0 timeout=0 sd=R drop=0 sdkB=4 d1=DETECTED d2=DETECTED
```

- `s32k` and `rx` increasing: bidirectional SPI communication works.
- `err=0` and `timeout=0`: packets are arriving without detected faults.
- `sd=R`: SD card and log file are ready.
- `sdkB` increasing: CSV bytes are reaching the card.
- `drop=0`: logger is keeping up.
- `d1/d2=DETECTED`: both configured I2C addresses responded.

## Run S32K Communication Test

The current S32K build uses the runtime test menu:

```c
#define APP_TEST_TEENSY_LINK_TEST  0
#define APP_TEST_NXP_CUP_TESTS     1
```

Flash and run the S32K, enter `TEENSY LINK` in the NXP Cup tests menu, then use
SW2 to cycle its OLED pages.

For a direct compile-time communication test, change
`firmware/s32k144/src/app/app_config.h` so:

```c
#define APP_TEST_TEENSY_LINK_TEST  1
#define APP_TEST_NXP_CUP_TESTS     0
```

Build the S32K from the repository root:

```bat
set PATH=C:\NXP\S32DS.3.6.3\S32DS\build_tools\gcc_v10.2\gcc-10.2-arm32-eabi\bin;C:\msys64\mingw64\bin;%PATH%
C:\msys64\mingw64\bin\mingw32-make.exe -C firmware\s32k144\Debug_FLASH all
```

Expected results:

- Teensy display 1 changes from `LINK: WAITING` to `LINK: OK`.
- Teensy display 1 `RX` and `S32KSEQ` increase.
- S32K OLED Teensy-link counters increase.
- Teensy Serial `s32k` and `rx` increase.

## Test SD Card

1. Insert a FAT32 or exFAT card into the Teensy built-in slot.
2. Reset or power-cycle the Teensy.
3. Confirm Teensy display 2 shows `SD: READY`.
4. Confirm `WRITTEN` increases and `DROPS` stays `0`.
5. Power down and open the newest `LOGnnn.CSV` on a PC.
6. In MATLAB, run:

```matlab
T = readtable("LOG000.CSV");
```

Booting without a card should show `SD: NO CARD`; SPI and displays must keep
running.

## Important Limits

- The Teensy display controller is assumed to be SSD1362. Confirm the display
  datasheet if the screen remains blank.
- The two displays need different I2C addresses for different dashboards.
- IMU and Teensy camera fields are still mock/missing values.
- The S32K-Teensy slave transport is still bit-banged. READY gating prevents
  new S32K transfers during display and SD writes, but the final high-rate
  design should use a hardware SPI slave implementation.
