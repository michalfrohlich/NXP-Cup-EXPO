# NXP Cup Car Firmware

Multi-board firmware repository for the NXP Cup car.

## Projects
- `firmware/s32k144/`: S32K144 bare-metal control firmware built with S32 Design Studio and NXP RTD.
- `firmware/teensy/`: Teensy 4.1 PlatformIO firmware for the S32K SPI link and sensor bring-up.
- `firmware/esp32/`: ESP32 Wi-Fi access point, web tuning UI, and S32K UART bridge firmware built with ESP-IDF/PlatformIO.

## Shared Areas
- `shared/protocol/`: active portable S32K-Teensy SPI and S32K-ESP32 UART packet contracts.
- `tools/`: host-side utilities such as MATLAB telemetry viewers.
- `docs/`: system-level documentation for the multi-board car.
- `hardware/`: hardware notes, pinouts, and schematic references.

## S32K Build
From the repository root:

```powershell
C:\msys64\ucrt64\bin\mingw32-make.exe -C firmware/s32k144/Debug_FLASH all
```

If the ARM compiler is not on `PATH`, prepend the S32DS GCC path documented in `firmware/s32k144/AGENTS.md`.
