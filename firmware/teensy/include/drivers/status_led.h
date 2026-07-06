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
void StatusLed_SetBrightness(StatusLedColor color, uint8_t brightness);
void StatusLed_SetRgb(uint8_t red, uint8_t green, uint8_t blue);
void StatusLed_BootSweep();
