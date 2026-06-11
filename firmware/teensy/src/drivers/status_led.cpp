#include "drivers/status_led.h"

#include "teensy_config.h"

static StatusLedColor currentColor = StatusLedColor::Off;

static void applyColor(StatusLedColor color)
{
    bool r = false;
    bool g = false;
    bool b = false;

    switch (color)
    {
        case StatusLedColor::Red:
            r = true;
            break;
        case StatusLedColor::Green:
            g = true;
            break;
        case StatusLedColor::Blue:
            b = true;
            break;
        case StatusLedColor::Yellow:
            r = true;
            g = true;
            break;
        case StatusLedColor::Cyan:
            g = true;
            b = true;
            break;
        case StatusLedColor::Magenta:
            r = true;
            b = true;
            break;
        case StatusLedColor::White:
            r = true;
            g = true;
            b = true;
            break;
        case StatusLedColor::Off:
        default:
            break;
    }

    digitalWriteFast(TEENSY_RGB_LED_R_PIN, r ? HIGH : LOW);
    digitalWriteFast(TEENSY_RGB_LED_G_PIN, g ? HIGH : LOW);
    digitalWriteFast(TEENSY_RGB_LED_B_PIN, b ? HIGH : LOW);
}

void StatusLed_Init()
{
    pinMode(TEENSY_RGB_LED_R_PIN, OUTPUT);
    pinMode(TEENSY_RGB_LED_G_PIN, OUTPUT);
    pinMode(TEENSY_RGB_LED_B_PIN, OUTPUT);
    applyColor(StatusLedColor::Off);
    currentColor = StatusLedColor::Off;
}

void StatusLed_Set(StatusLedColor color)
{
    /* Skip the pin writes when nothing changed; this runs every loop. */
    if (color == currentColor)
    {
        return;
    }
    currentColor = color;
    applyColor(color);
}

void StatusLed_BootSweep()
{
    const StatusLedColor sweep[3] = {StatusLedColor::Red,
                                     StatusLedColor::Green,
                                     StatusLedColor::Blue};

    for (uint8_t i = 0U; i < 3U; i++)
    {
        applyColor(sweep[i]);
        delay(150UL);
    }
    applyColor(StatusLedColor::Off);
    currentColor = StatusLedColor::Off;
}
