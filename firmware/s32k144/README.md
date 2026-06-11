# S32K144 Firmware

Bare-metal S32K144 control firmware for the NXP Cup car.

## Role
- Vehicle control and mode orchestration.
- Servo and ESC actuation.
- Current line-camera processing and steering control.
- UART debug/tuning transport.
- S32K-side integration points for future Teensy and ESP32 boards.

## Main Paths
- `src/`: handwritten C source.
- `include/`: handwritten public headers.
- `board/`, `generate/`, `RTD/`, `Nxp_Cup.mex`: generated/vendor-managed S32DS and RTD files.
- `Project_Settings/`: linker, startup, and debugger files.
- `Debug_FLASH/`: S32DS managed build directory.
- `docs/`: S32K-specific architecture, build, clock, and peripheral notes.

## Build
From this directory:

```powershell
C:\msys64\ucrt64\bin\mingw32-make.exe -C Debug_FLASH all
```

From the repository root:

```powershell
C:\msys64\ucrt64\bin\mingw32-make.exe -C firmware/s32k144/Debug_FLASH all
```
