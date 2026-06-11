#pragma once

#include <Arduino.h>

/* PCB RGB LED (D1, common cathode on Teensy pins 1/2/3).

   Color meanings in the normal link app:
   - BLUE    powered up, no valid S32K frame seen yet (waiting/connecting)
   - GREEN   link OK (valid S32K frames arriving)
   - YELLOW  link OK but the SD logger reported an error
   - RED     link lost (frames used to arrive, none for a while)
   White / cyan / magenta are free for future states. */
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

/* Short blocking R -> G -> B sweep so a human can verify all three LED
   channels on the PCB. Call from setup() only. */
void StatusLed_BootSweep();
