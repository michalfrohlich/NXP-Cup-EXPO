#include "drivers/secondary_displays.h"

#include <Wire.h>

#include "teensy_config.h"

static SecondaryDisplaySnapshot snapshot = {
    SecondaryDisplayState::NotChecked,
    SecondaryDisplayState::NotChecked,
    TEENSY_SECONDARY_DISPLAY_1_ADDRESS,
    TEENSY_SECONDARY_DISPLAY_2_ADDRESS,
    0U};

static bool addressResponds(uint8_t address)
{
    Wire1.beginTransmission(address);
    return Wire1.endTransmission() == 0U;
}

void SecondaryDisplays_Init()
{
    Wire1.setSDA(TEENSY_DISPLAY_SDA_PIN);
    Wire1.setSCL(TEENSY_DISPLAY_SCL_PIN);
    Wire1.begin();
    Wire1.setClock(TEENSY_SECONDARY_DISPLAY_I2C_HZ);
}

void SecondaryDisplays_Scan()
{
    bool sameAddress = snapshot.display1Address == snapshot.display2Address;
    bool display1Responds = addressResponds(snapshot.display1Address);
    bool display2Responds = sameAddress ? display1Responds : addressResponds(snapshot.display2Address);

    snapshot.display1 = display1Responds ? SecondaryDisplayState::Detected
                                        : SecondaryDisplayState::NotConnected;
    snapshot.display2 = display2Responds ? SecondaryDisplayState::Detected
                                        : SecondaryDisplayState::NotConnected;

    if (sameAddress && display1Responds)
    {
        snapshot.display1 = SecondaryDisplayState::AddressConflict;
        snapshot.display2 = SecondaryDisplayState::AddressConflict;
    }

    snapshot.scanCount++;
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
