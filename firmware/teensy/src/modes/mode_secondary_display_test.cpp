#include "modes/mode_secondary_display_test.h"

#include <Arduino.h>

#include "drivers/secondary_displays.h"
#include "teensy_config.h"

static uint32_t nextScanMs = 0U;

static void printDisplay(const char *name, uint8_t address, SecondaryDisplayState state)
{
    Serial.print(name);
    Serial.print(" address=0x");
    if (address < 0x10U)
    {
        Serial.print('0');
    }
    Serial.print(address, HEX);
    Serial.print(" state=");
    Serial.println(SecondaryDisplays_StateText(state));
}

void ModeSecondaryDisplayTest_Setup()
{
    Serial.begin(TEENSY_SERIAL_BAUD);
    while (!Serial && (millis() < 1200UL))
    {
    }

    SecondaryDisplays_Init();
    Serial.println("Teensy secondary-display I2C test");
    Serial.println("This test probes the PCB display bus; it does not start the S32K OLED.");
}

void ModeSecondaryDisplayTest_Loop()
{
    uint32_t nowMs = millis();

    if ((int32_t)(nowMs - nextScanMs) < 0)
    {
        return;
    }

    SecondaryDisplays_Scan();
    SecondaryDisplaySnapshot current = SecondaryDisplays_GetSnapshot();

    Serial.print("scan=");
    Serial.println(current.scanCount);
    printDisplay("display1", current.display1Address, current.display1);
    printDisplay("display2", current.display2Address, current.display2);
    Serial.println("Graphics output waits for confirmed controller and address settings.");

    nextScanMs = nowMs + TEENSY_SECONDARY_DISPLAY_SCAN_PERIOD_MS;
}
