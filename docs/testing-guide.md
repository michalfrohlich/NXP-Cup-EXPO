# Bench Testing Guide (S32K + Teensy + Link)

Every test here runs one subsystem alone, like the existing S32K servo test.
Assume the PCB connections are in place.

## Power-up order (important until the PCB link issue is solved)

The boards back-power each other through the SPI/I2C signal lines: a powered
board drives its outputs high and that current leaks through the unpowered
board's protection diodes into its 3.3 V rail. That is a hardware property,
not a firmware bug. The firmware tolerates any order — the S32K probes a
silent Teensy every 500 ms forever and the link self-recovers — but the
clean bring-up sequence is:

1. Power the S32K (USB/debugger).
2. Power the Teensy (USB).
3. If a board was connected while running and the link looks dead, just
   wait: it recovers on the next answered probe. Worst case, power-cycle
   the Teensy; nothing needs reflashing.

## Teensy PCB LED (D1) colors — normal link app

| Color | Meaning |
|---|---|
| R → G → B sweep at boot | LED self-test, all three channels work |
| BLUE | powered, waiting for the first valid S32K frame |
| GREEN | link OK (valid frames arriving) |
| YELLOW | link OK but the SD logger reported an error |
| RED | link was alive and died (no valid frame for 1 s) |

Cyan / magenta / white are reserved for future states (white/off are used as
button feedback inside the hardware self-test mode).

## S32K tests (OLED bench menu)

Flash the S32K with `APP_TEST_NXP_CUP_TESTS = 1` in
`firmware/s32k144/src/app/app_config.h` (the default). The OLED shows the
menu; the pot scrolls, the mode switch enters/leaves a test.

| Menu item | What it tests alone | Controls |
|---|---|---|
| Linear Camera | S32K camera + vision + optional servo | SW2 pause, pot = white threshold zoom |
| ESC | both motors, modes BOTH/ESC1/ESC2/DIFF | SW2 arm/start, SW3 mode, pot = speed |
| Servo | steering servo alone | pot = angle, SW2 smoothing on/off |
| Ultrasonic | distance sensor alone | shows live cm, LED color by range |
| CamServo | camera steering the servo | SW2 pause |
| Simple Drive | slow full auto drive | SW2 start/stop |
| Serial Tune | live PID tuning over UART | terminal: `c` connect, `#` commit |
| Ultra+ESC | obstacle slow/stop with motors | runs by itself, approach an object |
| Receiver | RC receiver inputs | shows channel values |
| **Teensy Link** | the 128-byte SPI link (DMA path) | SW2 cycles 6 pages, see below |
| Victory Lap | pole approach + fanfare | runs by itself |

Teensy Link OLED pages (SW2 to cycle): 0 `TLINK` status/E:/SK:, 1 CAM0,
2 CAM1, 3 IMU/LOG (SD status from the Teensy), 4 RX 128B counters,
5 `TLINK DMA` (engine, busy, starts, timeouts).

For SPI-only boot without the menu use `APP_TEST_TEENSY_LINK_TEST = 1`.

## Teensy tests

### Normal link app (default, `TEENSY_APP_HARDWARE_TEST 0`)

```bat
cd /d C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
pio run -t upload
pio device monitor -p COM3 -b 115200
```

Serial line shows everything at 10 Hz: link (`s32k/rx/err/timeout`), SD
(`sd/drop/sdkB`), inputs (`pot/b1/b2`), displays (`d1/d2`). Both ZJY
displays show the same combined dashboard (this is intentional: two panels
on one I2C address mirror by hardware and show identical content either
way). LED colors as in the table above.

### Hardware self-test mode (tests the Teensy PCB alone, no S32K needed)

Set `TEENSY_APP_HARDWARE_TEST 1` in
`firmware/teensy/include/teensy_app_config.h`, then `pio run -t upload`.

What you should see:
- LED cycles red→green→blue→yellow→cyan→magenta, 1 s each.
- Hold button 1: LED goes WHITE. Hold button 2: LED goes OFF.
- Both displays show the live dashboard with the pot value moving as you
  turn it.
- Serial (5 Hz): `pot/b1/b2`, SD state and KiB written (one test row per
  second goes to a real LOGnnn.CSV), display detect states, and the live
  CS/SCK/MOSI pin levels — use those to debug the PCB SPI wiring: with an
  idle powered S32K connected, `cs=1 sck=0` is correct.
- READY is held LOW in this mode, so a connected S32K skips transfers
  (its `SK:` counter climbs) instead of clocking garbage.

Set the flag back to 0 and re-upload for the normal app.

## Link + DMA bench test (both boards)

1. Flash Teensy (normal app) and open the serial monitor.
2. Flash S32K, enter `Teensy Link` in the menu.
3. Teensy serial: `rx` climbs ~100/s, `err=0 timeout=0`, LED GREEN.
4. S32K OLED page 0: `TLINK OK`, `SK:` ticking slowly (display/SD busy
   windows), page 5: `ST` climbing, `TO:0`.
5. Pull SCK or CS mid-run: S32K LED red/yellow, Teensy LED RED after 1 s,
   `TO`/`E:` count up — reconnect and both recover without reset.
6. SD proof: pull the card after a run, `LOGnnn.CSV` opens in MATLAB with
   `readtable`.

## Still needs physical validation

- DMA path on real boards (DMAMUX sources 16/17, CS held across the whole
  128-byte frame, abort/recovery) — bench-built and reviewed, never run on
  hardware.
- ZJY_M208_25664-4P controller assumption (SSD1362 at 0x3C/0x3D over I2C):
  if serial says `DETECTED`/`MIRRORED` but panels stay dark, the controller
  or interface assumption is wrong — report what the silkscreen/jumpers say.
- The PCB-vs-jumpers link instability: after this branch, compare `E:`/`SK:`
  counters between jumper wiring and PCB wiring; if the PCB still fails,
  lower the SPI baud rate in S32DS ConfigTools (TeensySpiDevice) and retest.
- The second display header (J25) is on the ESP32 per the schematic — it is
  intentionally not driven by the Teensy ("space for later").
