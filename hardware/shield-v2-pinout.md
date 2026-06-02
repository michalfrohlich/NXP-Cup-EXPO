# NXP Car Shield V2 Pinout

This note records the current PCB shield pin plan from the KiCad schematic
screenshots.

## Teensy 4.1

| Function | Teensy pin |
|---|---:|
| S32K SPI CS | 10 |
| S32K SPI MOSI | 11 |
| S32K SPI MISO | 12 |
| S32K SPI SCK | 13 |
| Camera 1 analog | 15 |
| Camera 2 analog | 14 |
| Display SDA | 17 |
| Display SCL | 16 |
| IMU SDA | 18 |
| IMU SCL | 19 |
| Camera 1 SI | 4 |
| Camera 2 SI | 5 |
| Camera 1 CLK | 6 |
| Camera 2 CLK | 7 |
| IMU interrupt | 30 |
| Ready GPIO to S32K | 31 |
| 5 V input | VIN |
| Ground | GND |

The active Teensy constants are in:

```text
firmware/teensy/include/teensy_config.h
```

## S32K To Teensy SPI

| S32K signal | S32K pin | Teensy 4.1 pin |
|---|---:|---:|
| SCK | PTB14 | 13 |
| MOSI / S32K SOUT | PTB16 | 11 |
| MISO / S32K SIN | PTB15 | 12 |
| CS / PCS3 | PTB17 | 10 |
| READY | PTD14 | 31 |
| GND | GND | GND |

The S32K is the SPI master. The Teensy is the SPI slave.

## ESP32

| Function | ESP32 GPIO / connector pin |
|---|---:|
| UART RX | GPIO16 |
| UART TX | GPIO17 |
| BTN1 | GPIO18 |
| BTN2 | GPIO19 |
| I2C data / SDA | GPIO21 |
| I2C clock / SCL | GPIO22 |
| GND | connector pin 2 |
| VIN | connector pin 1 |
| Screen power | 3V3 |
| Screen ground | GND |

The ESP32 constants are in:

```text
firmware/esp32/include/esp32_pin_config.h
```

## Notes

- The screen power pin is 3V3, not VIN.
- Keep S32K/Teensy SPI wiring short during the first 128-byte packet test.
- Share ground between every board before checking any data signal.
