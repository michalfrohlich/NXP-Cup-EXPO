#pragma once

#include <Arduino.h>

enum class SecondaryDisplayState : uint8_t
{
    NotChecked = 0U,
    NotConnected,
    Detected,
    AddressConflict
};

struct SecondaryDisplaySnapshot
{
    SecondaryDisplayState display1;
    SecondaryDisplayState display2;
    uint8_t display1Address;
    uint8_t display2Address;
    uint32_t scanCount;
};

void SecondaryDisplays_Init();
void SecondaryDisplays_Scan();
SecondaryDisplaySnapshot SecondaryDisplays_GetSnapshot();
const char *SecondaryDisplays_StateText(SecondaryDisplayState state);
