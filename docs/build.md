# Build

## Toolchain / IDE
- S32 Design Studio project (`.project`, `.cproject`)
- Toolchain in `.cproject`: NXP GCC 10.2 for Arm 32-bit Bare-Metal
- Target in `.cproject`: `S32K144` / `CPU_S32K144HFT0VLLT`

## Build configuration visible in repo
- Active managed build configuration: `Debug_FLASH`
- Linker script referenced by `.cproject`:
  - `Project_Settings/Linker_Files/linker_flash_s32k144.ld`

## CLI build
- Use `mingw32-make`, not plain `make`.
- Verified build command:
  - `C:\msys64\ucrt64\bin\mingw32-make.exe -C Debug_FLASH all`
- If `arm-none-eabi-gcc` is not found, add this toolchain directory to `PATH`:
  - `C:\NXP\S32DS.3.6.3\S32DS\build_tools\gcc_v10.2\gcc-10.2-arm32-eabi\bin`
- The shell may also need these MSYS directories on `PATH`:
  - `C:\msys64\usr\bin`
  - `C:\msys64\ucrt64\bin`
- Output ELF:
  - `Debug_FLASH/EXPO_03_Nxp_Cup_project.elf`

## Important project files
- `.cproject`, `.project`: IDE/build metadata
- `Project_Settings/Startup_Code/`: startup, vector table, system init
- `Project_Settings/Linker_Files/`: linker scripts
- `Nxp_Cup.mex`: S32 Config Tools source for generated code

## Generated code locations
- `generate/include/`
- `generate/src/`
- `board/`

## Local RTD sources
- `RTD/include/`
- `RTD/src/`

## Edit warnings
- Treat these as generated or vendor-managed unless the task is specifically about config regeneration or vendor code:
  - `generate/`
  - `board/`
  - `RTD/`
- Prefer changing handwritten code in:
  - `src/`
  - `src/app/`
  - `src/services/`
  - `include/`
- If peripheral configuration must change, the source of truth is likely `Nxp_Cup.mex`, not the generated output files.
- For this temporary handwritten UART proof-of-concept, the runtime code lives in `src/services/serial_debug.c`, but the CLI-managed build also needs the new source listed in `Debug_FLASH/src/services/subdir.mk` until the IDE regenerates the make metadata.
