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

/* One combined dashboard. Both physical displays show exactly this. */
static void drawCombinedDashboard(U8X8 &display, const SecondaryDisplayDashboard &dashboard)
{
    char line[DISPLAY_COLUMNS + 1U];
    const char *sdState = dashboard.sdError ? "ERROR" : (dashboard.sdReady ? "READY" : "NO CARD");

    writeLine(display, 0U, "S32K LINK + TEENSY + SD");

    (void)snprintf(line, sizeof(line), "LINK:%s RX:%u",
                   dashboard.s32kValid ? "OK" : "WAITING",
                   dashboard.rxFrames);
    writeLine(display, 1U, line);

    (void)snprintf(line, sizeof(line), "ERR:%u TO:%u SEQ:%u",
                   dashboard.rxErrors,
                   dashboard.timeouts,
                   dashboard.s32kSequence);
    writeLine(display, 2U, line);

    (void)snprintf(line, sizeof(line), "APP:%u SPD:%d/%d SRV:%d",
                   dashboard.appMode,
                   dashboard.targetSpeedPct,
                   dashboard.currentSpeedPct,
                   dashboard.servoCmd);
    writeLine(display, 3U, line);

    (void)snprintf(line, sizeof(line), "IMU:%c%c%c Y:%d GZ:%d",
                   dashboard.imuPresent ? 'P' : '-',
                   dashboard.imuCalibrated ? 'C' : '-',
                   dashboard.imuValid ? 'V' : '-',
                   dashboard.imuYawDeg,
                   dashboard.imuGzDps);
    writeLine(display, 4U, line);

    (void)snprintf(line, sizeof(line), "SD:%s %s DROP:%u",
                   sdState,
                   dashboard.sdReady ? dashboard.sdFileName : "-",
                   dashboard.sdDrops);
    writeLine(display, 5U, line);

    (void)snprintf(line, sizeof(line), "POT:%u B1:%u B2:%u",
                   dashboard.potRaw,
                   dashboard.button1Pressed ? 1U : 0U,
                   dashboard.button2Pressed ? 1U : 0U);
    writeLine(display, 6U, line);

    (void)snprintf(line, sizeof(line), "TX:%u SEN:%u KB:%lu",
                   dashboard.teensyTxSequence,
                   dashboard.sensorSequence,
                   (unsigned long)(dashboard.sdBytesWritten / 1024UL));
    writeLine(display, 7U, line);
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
        /* Both modules share one address. Every write through display1
           reaches both panels, so they mirror by hardware. */
        snapshot.display1 = SecondaryDisplayState::Mirrored;
        snapshot.display2 = SecondaryDisplayState::Mirrored;
        initializeDisplay(display1, snapshot.display1Address, "TEENSY DASHBOARD (MIRROR)");
        return;
    }

    if (snapshot.display1 == SecondaryDisplayState::Detected)
    {
        initializeDisplay(display1, snapshot.display1Address, "TEENSY DASHBOARD");
    }
    if (snapshot.display2 == SecondaryDisplayState::Detected)
    {
        initializeDisplay(display2, snapshot.display2Address, "TEENSY DASHBOARD");
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

    /* Both panels show the same combined dashboard. Only one I2C write
       happens per service call so the blocking time stays short:
       - mirrored (same address): one write reaches both panels;
       - separate addresses: alternate which panel is refreshed. */
    if (snapshot.display1 == SecondaryDisplayState::Mirrored)
    {
        drawCombinedDashboard(display1, dashboard);
    }
    else if (updateDisplay1Next)
    {
        if (snapshot.display1 == SecondaryDisplayState::Detected)
        {
            drawCombinedDashboard(display1, dashboard);
        }
    }
    else if (snapshot.display2 == SecondaryDisplayState::Detected)
    {
        drawCombinedDashboard(display2, dashboard);
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
        case SecondaryDisplayState::Mirrored:
            return "MIRRORED";
        default:
            return "NOT_CHECKED";
    }
}
