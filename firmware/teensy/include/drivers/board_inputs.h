#pragma once

#include <Arduino.h>

struct BoardInputsSnapshot
{
    uint16_t potRaw;
    bool button1Pressed;
    bool button2Pressed;
};

enum class BoardButtonId : uint8_t
{
    Button1 = 0U,
    Button2,
    Count
};

void BoardInputs_Init();
void BoardInputs_Update();
BoardInputsSnapshot BoardInputs_Read();
bool BoardInputs_IsPressed(BoardButtonId id);
bool BoardInputs_WasPressed(BoardButtonId id);
