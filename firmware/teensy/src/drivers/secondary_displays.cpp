#include "drivers/secondary_displays.h"

#include <U8x8lib.h>
#include <Wire.h>
#include <stdio.h>
#include <string.h>

#include "teensy_config.h"

static constexpr uint8_t DISPLAY_COLUMNS = 32U;

static U8X8_SSD1362_256X64_2ND_HW_I2C display1(U8X8_PIN_NONE);
static U8X8_SSD1362_256X64_2ND_HW_I2C display2(U8X8_PIN_NONE);

static SecondaryDisplaySnapshot snapshot = {
    SecondaryDisplayState::NotChecked,
    SecondaryDisplayState::NotChecked,
    TEENSY_SECONDARY_DISPLAY_1_ADDRESS,
    TEENSY_SECONDARY_DISPLAY_2_ADDRESS,
    0U};

static uint32_t nextUpdateMs = 0U;
static bool updateDisplay1Next = true;

static bool addressResponds(uint8_t address)
{
    Wire1.beginTransmission(address);
    return Wire1.endTransmission() == 0U;
}

static void writeLine(U8X8 &display, uint8_t row, const char *text)
{
    char line[DISPLAY_COLUMNS + 1U];
    size_t textLength = strlen(text);

    if (textLength > DISPLAY_COLUMNS)
    {
        textLength = DISPLAY_COLUMNS;
    }

    (void)memset(line, ' ', DISPLAY_COLUMNS);
    (void)memcpy(line, text, textLength);
    line[DISPLAY_COLUMNS] = '\0';
    display.drawString(0U, row, line);
}

static void initializeDisplay(U8X8 &display, uint8_t address, const char *title)
{
    display.setI2CAddress((uint8_t)(address << 1U));
    display.setBusClock(TEENSY_SECONDARY_DISPLAY_I2C_HZ);
    display.begin();
    display.setPowerSave(0U);
    display.setFont(u8x8_font_chroma48medium8_r);
    display.clearDisplay();
    writeLine(display, 0U, title);
    writeLine(display, 2U, "TEENSY STARTING");
}

static void drawLinkDashboard(U8X8 &display, const SecondaryDisplayDashboard &dashboard)
{
    char line[DISPLAY_COLUMNS + 1U];

    writeLine(display, 0U, "S32K <-> TEENSY SPI");
    writeLine(display, 1U, dashboard.s32kValid ? "LINK: OK" : "LINK: WAITING");

    (void)snprintf(line, sizeof(line), "RX:%u S32KSEQ:%u",
                   dashboard.rxFrames,
                   dashboard.s32kSequence);
    writeLine(display, 2U, line);

    (void)snprintf(line, sizeof(line), "ERR:%u TIMEOUT:%u",
                   dashboard.rxErrors,
                   dashboard.timeouts);
    writeLine(display, 3U, line);

    (void)snprintf(line, sizeof(line), "APP:%u SPEED:%d/%d",
                   dashboard.appMode,
                   dashboard.targetSpeedPct,
                   dashboard.currentSpeedPct);
    writeLine(display, 4U, line);

    (void)snprintf(line, sizeof(line), "SERVO:%d", dashboard.servoCmd);
    writeLine(display, 5U, line);

    (void)snprintf(line, sizeof(line), "ULTRA:%u.%u cm",
                   dashboard.ultrasonicDistanceCm10 / 10U,
                   dashboard.ultrasonicDistanceCm10 % 10U);
    writeLine(display, 6U, line);
    writeLine(display, 7U, "128 BYTES / READY GATED");
}

static void drawSdDashboard(U8X8 &display, const SecondaryDisplayDashboard &dashboard)
{
    char line[DISPLAY_COLUMNS + 1U];
    const char *sdState = dashboard.sdError ? "ERROR" : (dashboard.sdReady ? "READY" : "NO CARD");

    writeLine(display, 0U, "TEENSY + SD LOGGER");

    (void)snprintf(line, sizeof(line), "SD: %s", sdState);
    writeLine(display, 1U, line);

    (void)snprintf(line, sizeof(line), "FILE: %s",
                   dashboard.sdReady ? dashboard.sdFileName : "-");
    writeLine(display, 2U, line);

    (void)snprintf(line, sizeof(line), "WRITTEN: %lu KB",
                   (unsigned long)(dashboard.sdBytesWritten / 1024UL));
    writeLine(display, 3U, line);

    (void)snprintf(line, sizeof(line), "DROPS: %u", dashboard.sdDrops);
    writeLine(display, 4U, line);

    (void)snprintf(line, sizeof(line), "POT:%u B1:%u B2:%u",
                   dashboard.potRaw,
                   dashboard.button1Pressed ? 1U : 0U,
                   dashboard.button2Pressed ? 1U : 0U);
    writeLine(display, 5U, line);

    (void)snprintf(line, sizeof(line), "TX:%u SENSOR:%u",
                   dashboard.teensyTxSequence,
                   dashboard.sensorSequence);
    writeLine(display, 6U, line);

    writeLine(display, 7U, "IMU/CAM: MOCK OR MISSING");
}

void SecondaryDisplays_Init()
{
    bool sameAddress;
    bool display1Responds;
    bool display2Responds;

    Wire1.setSDA(TEENSY_DISPLAY_SDA_PIN);
    Wire1.setSCL(TEENSY_DISPLAY_SCL_PIN);
    Wire1.begin();
    Wire1.setClock(TEENSY_SECONDARY_DISPLAY_I2C_HZ);

    sameAddress = snapshot.display1Address == snapshot.display2Address;
    display1Responds = addressResponds(snapshot.display1Address);
    display2Responds = sameAddress ? display1Responds : addressResponds(snapshot.display2Address);
    snapshot.scanCount++;

    snapshot.display1 = display1Responds ? SecondaryDisplayState::Detected
                                        : SecondaryDisplayState::NotConnected;
    snapshot.display2 = display2Responds ? SecondaryDisplayState::Detected
                                        : SecondaryDisplayState::NotConnected;

    if (sameAddress && display1Responds)
    {
        snapshot.display1 = SecondaryDisplayState::AddressConflict;
        snapshot.display2 = SecondaryDisplayState::AddressConflict;
        initializeDisplay(display1, snapshot.display1Address, "DISPLAY ADDRESS CONFLICT");
        writeLine(display1, 2U, "SET UNIQUE ADDRESSES");
        return;
    }

    if (snapshot.display1 == SecondaryDisplayState::Detected)
    {
        initializeDisplay(display1, snapshot.display1Address, "DISPLAY 1: SPI LINK");
    }
    if (snapshot.display2 == SecondaryDisplayState::Detected)
    {
        initializeDisplay(display2, snapshot.display2Address, "DISPLAY 2: SD LOGGER");
    }
}

bool SecondaryDisplays_UpdateDue(uint32_t nowMs)
{
    return (int32_t)(nowMs - nextUpdateMs) >= 0;
}

void SecondaryDisplays_Service(uint32_t nowMs, const SecondaryDisplayDashboard &dashboard)
{
    if (!SecondaryDisplays_UpdateDue(nowMs))
    {
        return;
    }

    if (snapshot.display1 == SecondaryDisplayState::AddressConflict)
    {
        nextUpdateMs = nowMs + TEENSY_SECONDARY_DISPLAY_UPDATE_MS;
        return;
    }

    if (updateDisplay1Next)
    {
        if (snapshot.display1 == SecondaryDisplayState::Detected)
        {
            drawLinkDashboard(display1, dashboard);
        }
    }
    else if (snapshot.display2 == SecondaryDisplayState::Detected)
    {
        drawSdDashboard(display2, dashboard);
    }

    updateDisplay1Next = !updateDisplay1Next;
    nextUpdateMs = nowMs + TEENSY_SECONDARY_DISPLAY_UPDATE_MS;
}

SecondaryDisplaySnapshot SecondaryDisplays_GetSnapshot()
{
    return snapshot;
}

const char *SecondaryDisplays_StateText(SecondaryDisplayState state)
{
    switch (state)
    {
        case SecondaryDisplayState::NotConnected:
            return "NOT_CONNECTED";
        case SecondaryDisplayState::Detected:
            return "DETECTED";
        case SecondaryDisplayState::AddressConflict:
            return "ADDRESS_CONFLICT";
        default:
            return "NOT_CHECKED";
    }
}
