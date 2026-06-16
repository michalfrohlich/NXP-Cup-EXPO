#pragma once

#include <Arduino.h>

struct BoardInputsSnapshot
{
    uint16_t potRaw;
    bool button1Pressed;
    bool button2Pressed;
};

void BoardInputs_Init();
BoardInputsSnapshot BoardInputs_Read();
