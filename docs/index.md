# NXP Cup Car Firmware

This site collects engineering documentation for the multi-board NXP Cup car
firmware repository.

## Main Areas

- S32K144 control firmware lives in `firmware/s32k144/`.
- Teensy firmware lives in `firmware/teensy/`.
- ESP32 firmware lives in `firmware/esp32/`.
- Shared cross-board packet contracts live in `shared/protocol/`.
- Hardware notes live in `hardware/`.

## Board Interaction

```mermaid
flowchart LR
    S32K["S32K144 control firmware"]
    Teensy["Teensy sensor firmware"]
    ESP32["ESP32 bridge firmware"]
    Shared["shared/protocol"]

    S32K <-- "SPI teensy_link frame" --> Teensy
    S32K <-- "UART bridge frames" --> ESP32
    Shared --> S32K
    Shared --> Teensy
    Shared --> ESP32
```

Start with the system and protocol pages, then use the board-specific README
files under `firmware/` for build and bring-up details.
