## Repository context
- This is a multi-board NXP Cup car monorepo.
- Firmware projects live under `firmware/`.
- The active S32K144 firmware project lives in `firmware/s32k144/`.
- Future Teensy and ESP32 firmware projects live in `firmware/teensy/` and `firmware/esp32/`.
- Cross-board protocol definitions belong under `shared/protocol/` once they are stable enough to share.

## Boundaries
- Keep board-specific firmware code inside its board folder.
- Keep S32 Design Studio generated/vendor code inside `firmware/s32k144/`.
- Do not move generated S32K folders (`board/`, `generate/`, `RTD/`) unless the task is explicitly about S32DS project metadata and generated-code paths.
- Shared code must stay portable and must not include S32K RTD, Arduino/Teensy, or ESP-IDF headers.

## Documentation
- Root `docs/` describes the whole multi-board system.
- `firmware/s32k144/docs/` describes the S32K firmware project.
- Update the docs closest to the behavior or boundary being changed.

## Build
- S32K CLI build from the repository root:
  - `C:\msys64\ucrt64\bin\mingw32-make.exe -C firmware/s32k144/Debug_FLASH all`
- S32K CLI build from `firmware/s32k144/`:
  - `C:\msys64\ucrt64\bin\mingw32-make.exe -C Debug_FLASH all`
