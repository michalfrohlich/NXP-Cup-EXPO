## Technical context
- Target: NXP S32K144 (Cortex-M4), S32 Design Studio, bare-metal GCC build.
- Stack: NXP RTD 2.0.0 / AUTOSAR 4.7 MCAL-style drivers with generated config.
- Architecture: non-RTE, non-AUTOSAR application code on top of RTD/MCAL; no FreeRTOS.

## Code boundaries
- Handwritten code: `src/`, `src/app/`, `src/drivers/`, `src/services/`, `src/debug/`, `src/unused/`, `include/`.
- App layer: `src/app/app_modes.c` is the compile-time dispatcher, `src/app/app_common.c` holds shared runtime glue, and `src/app/modes/` holds standalone modes plus runtime bench menu/tests.
- Generated/vendor code: `generate/`, `board/`, `RTD/`, `Nxp_Cup.mex`.

## Editing guidance
- Prefer changes in handwritten application code.
- Avoid manual edits to generated/vendor files unless the task is explicitly config/driver-related.
- For HW/peripheral changes, distinguish between config-tool-generated changes (`Nxp_Cup.mex`, `generate/`, `board/`) and runtime logic changes (`src/`, `include/`).
- When modifying documented areas, update the matching files in `docs/` as part of the same task:
  - `docs/quick-reference.md` for project purpose, entry points, major modules, important directories, generated/config files, or major unknowns.
  - `docs/architecture.md` for module structure, control flow, selected app mode, interrupt/callback flow, or data flow changes.
  - `docs/peripherals.md` for confirmed peripheral usage, driver/module usage, or key hardware-facing file changes.
  - `docs/build.md` for toolchain, build configuration, linker/startup layout, generated-code locations, or edit-boundary changes.
  - `docs/clocks.md` for clock tree, generated clock outputs, timer/PWM rates, or timing assumptions derived from generated config.
-When updating docs keep it factual and not story-like. Do not mention/update the active compile time mode.

## Build
- In this workspace, use `mingw32-make`, not plain `make`.
- Managed build directory: `Debug_FLASH/`.
- Working CLI build command from `firmware/s32k144/`:
  - `C:\msys64\ucrt64\bin\mingw32-make.exe -C Debug_FLASH all`
- Working CLI build command from the repository root:
  - `C:\msys64\ucrt64\bin\mingw32-make.exe -C firmware/s32k144/Debug_FLASH all`
- If the shell does not know the compiler, prepend this to `PATH` before building:
  - `C:\NXP\S32DS.3.6.3\S32DS\build_tools\gcc_v10.2\gcc-10.2-arm32-eabi\bin`
- `mingw32-make` may also need:
  - `C:\msys64\usr\bin`
  - `C:\msys64\ucrt64\bin`
