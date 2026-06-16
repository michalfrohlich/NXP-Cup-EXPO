#pragma once

#include <Arduino.h>

enum class StatusLedColor : uint8_t
{
    Off = 0U,
    Red,
    Green,
    Blue,
    Yellow,
    Cyan,
    Magenta,
    White
};

void StatusLed_Init();
void StatusLed_Set(StatusLedColor color);
void StatusLed_BootSweep();
