# Pre-Commit And Formatting

This repository uses `pre-commit` to run quick checks before each commit and
`clang-format` to format handwritten C/C++ code.

## Install

From the repository root:

```powershell
python -m pip install pre-commit
python -m pre_commit install
```

On this workstation, `pre-commit` is installed in the PlatformIO Python
environment:

```powershell
C:\Users\misof\.platformio\penv\Scripts\python.exe -m pre_commit install
```

## Run Checks

Run the checks on staged files:

```powershell
python -m pre_commit run
```

Run the checks on selected files:

```powershell
python -m pre_commit run --files firmware/s32k144/src/app/app_modes.c
```

Avoid `--all-files` while unrelated work is in progress because formatting
hooks may rewrite many files at once.

## Formatting Scope

The `clang-format` hook only targets handwritten or shared C/C++ code:

- `firmware/s32k144/src/`
- `firmware/s32k144/include/`
- `firmware/teensy/src/`
- `firmware/teensy/include/`
- `firmware/esp32/src/`
- `firmware/esp32/include/`
- `shared/protocol/`

Generated and vendor-managed S32K code is intentionally excluded:

- `firmware/s32k144/RTD/`
- `firmware/s32k144/generate/`
- `firmware/s32k144/board/`

Keep formatting-only changes in a separate commit from behavior changes.
