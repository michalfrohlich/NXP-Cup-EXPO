# Teensy Secondary Displays

This branch adds a Teensy-only hardware test for the two planned
`ZJY_M208_25664-4P` 256x64 displays. It does not change or use the S32K OLED.

## Current Limit

The exact display controller and I2C address are not confirmed. Similar 256x64
modules use different controllers, including SSD1362 and SH1122. Sending
controller-specific graphics commands before confirming the part can produce a
blank screen or incorrect behavior.

The current test therefore verifies the electrical I2C connection first. It
reports `DETECTED`, `NOT_CONNECTED`, or `ADDRESS_CONFLICT` for each configured
display. Add graphics initialization only after confirming:

- controller model;
- I2C address or address-selection method;
- voltage and connector pin order;
- whether both displays can use different addresses.

Two displays with the same fixed I2C address cannot be controlled independently
on the same SDA/SCL wires. Use selectable addresses or an I2C multiplexer such
as a TCA9548A.

## Wiring

| Teensy signal | Teensy 4.1 pin | Display signal |
|---|---:|---|
| SDA1 | 17 | SDA |
| SCL1 | 16 | SCL |
| 3.3 V | 3.3 V | VCC |
| Ground | GND | GND |

The PCB schematic routes pins 17 and 16 to the Teensy display connector.
Do not use the S32K display connector for this test.

## Enable And Run

Open:

```text
C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
```

Set this value to `1` in `include/teensy_app_config.h`:

```cpp
#define TEENSY_APP_SECONDARY_DISPLAY_TEST 1
```

Then run:

```bat
cd /d C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
pio run
pio run -t upload
pio device list
pio device monitor -p COM3 -b 115200
```

Replace `COM3` with the Teensy port. Expected Serial output:

```text
scan=1
display1 address=0x3C state=DETECTED
display2 address=0x3D state=NOT_CONNECTED
```

Update the two configured addresses in `include/teensy_config.h` after checking
the display datasheet. If both addresses are configured the same and a device
responds, the test reports `ADDRESS_CONFLICT` because it cannot distinguish two
devices at one address.

Set `TEENSY_APP_SECONDARY_DISPLAY_TEST` back to `0` before running the normal
S32K-Teensy SPI test.

## Documentation Handoff Prompt

```text
Update the Teensy secondary-display documentation using the confirmed
ZJY_M208_25664-4P datasheet. Record the controller model, connector pin order,
voltage, I2C addresses, address-selection method, and whether an I2C
multiplexer is required for two displays. Replace the probe-only test with a
minimal graphics test that writes a different label to each display. Document
the exact files changed, wiring, build/upload commands, expected Serial and
screen output, and measured results. Keep the S32K OLED code unchanged.
```

## Files

- `include/teensy_app_config.h`: selects the Teensy display test.
- `include/teensy_config.h`: display pins, addresses, bus speed, and scan rate.
- `include/drivers/secondary_displays.h`: display probe state interface.
- `src/drivers/secondary_displays.cpp`: Teensy Wire1 I2C probe.
- `src/modes/mode_secondary_display_test.cpp`: periodic Serial test output.
- `src/main.cpp`: dispatches to the display test when enabled.
