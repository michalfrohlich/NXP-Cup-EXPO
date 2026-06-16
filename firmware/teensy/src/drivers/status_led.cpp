#include "drivers/status_led.h"

#include "teensy_config.h"

static StatusLedColor currentColor = StatusLedColor::Off;

static void applyColor(StatusLedColor color)
{
    bool red = false;
    bool green = false;
    bool blue = false;

    switch (color)
    {
        case StatusLedColor::Red:
            red = true;
            break;
        case StatusLedColor::Green:
            green = true;
            break;
        case StatusLedColor::Blue:
            blue = true;
            break;
        case StatusLedColor::Yellow:
            red = true;
            green = true;
            break;
        case StatusLedColor::Cyan:
            green = true;
            blue = true;
            break;
        case StatusLedColor::Magenta:
            red = true;
            blue = true;
            break;
        case StatusLedColor::White:
            red = true;
            green = true;
            blue = true;
            break;
        case StatusLedColor::Off:
        default:
            break;
    }

    digitalWriteFast(TEENSY_RGB_LED_R_PIN, red ? HIGH : LOW);
    digitalWriteFast(TEENSY_RGB_LED_G_PIN, green ? HIGH : LOW);
    digitalWriteFast(TEENSY_RGB_LED_B_PIN, blue ? HIGH : LOW);
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
    if (color == currentColor)
    {
        return;
    }

    currentColor = color;
    applyColor(color);
}

void StatusLed_BootSweep()
{
    static const StatusLedColor sweep[] = {
        StatusLedColor::Red,
        StatusLedColor::Green,
        StatusLedColor::Blue,
    };

    for (uint8_t i = 0U; i < (uint8_t)(sizeof(sweep) / sizeof(sweep[0])); i++)
    {
        applyColor(sweep[i]);
        delay(150UL);
    }

    applyColor(StatusLedColor::Off);
    currentColor = StatusLedColor::Off;
}
