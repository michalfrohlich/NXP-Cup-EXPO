#include "drivers/status_led.h"

#include "teensy_config.h"

static StatusLedColor currentColor = StatusLedColor::Off;
static uint8_t currentBrightness = 255U;
static bool currentDiscreteColorValid = false;
static uint8_t currentRed = 0U;
static uint8_t currentGreen = 0U;
static uint8_t currentBlue = 0U;
static bool currentRgbValid = false;

static void applyRgb(uint8_t red, uint8_t green, uint8_t blue)
{
    if ((currentRgbValid == true) && (red == currentRed) && (green == currentGreen) &&
        (blue == currentBlue))
    {
        return;
    }

    currentRed = red;
    currentGreen = green;
    currentBlue = blue;
    currentRgbValid = true;
    analogWrite(TEENSY_RGB_LED_R_PIN, red);
    analogWrite(TEENSY_RGB_LED_G_PIN, green);
    analogWrite(TEENSY_RGB_LED_B_PIN, blue);
}

static void applyColor(StatusLedColor color, uint8_t brightness)
{
    uint8_t red = 0U;
    uint8_t green = 0U;
    uint8_t blue = 0U;

    switch (color)
    {
        case StatusLedColor::Red:
            red = brightness;
            break;
        case StatusLedColor::Green:
            green = brightness;
            break;
        case StatusLedColor::Blue:
            blue = brightness;
            break;
        case StatusLedColor::Yellow:
            red = brightness;
            green = brightness;
            break;
        case StatusLedColor::Cyan:
            green = brightness;
            blue = brightness;
            break;
        case StatusLedColor::Magenta:
            red = brightness;
            blue = brightness;
            break;
        case StatusLedColor::White:
            red = brightness;
            green = brightness;
            blue = brightness;
            break;
        case StatusLedColor::Off:
        default:
            break;
    }

    applyRgb(red, green, blue);
}

void StatusLed_Init()
{
    pinMode(TEENSY_RGB_LED_R_PIN, OUTPUT);
    pinMode(TEENSY_RGB_LED_G_PIN, OUTPUT);
    pinMode(TEENSY_RGB_LED_B_PIN, OUTPUT);
    applyColor(StatusLedColor::Off, 0U);
    currentColor = StatusLedColor::Off;
    currentBrightness = 255U;
    currentDiscreteColorValid = true;
}

void StatusLed_Set(StatusLedColor color)
{
    StatusLed_SetBrightness(color, 255U);
}

void StatusLed_SetBrightness(StatusLedColor color, uint8_t brightness)
{
    if ((currentDiscreteColorValid == true) && (color == currentColor))
    {
        if ((color == StatusLedColor::Off) || (brightness == currentBrightness))
        {
            return;
        }
    }

    currentColor = color;
    currentBrightness = brightness;
    currentDiscreteColorValid = true;
    applyColor(color, brightness);
}

void StatusLed_SetRgb(uint8_t red, uint8_t green, uint8_t blue)
{
    currentDiscreteColorValid = false;
    applyRgb(red, green, blue);
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
        applyColor(sweep[i], 255U);
        delay(150UL);
    }

    applyColor(StatusLedColor::Off, 0U);
    currentColor = StatusLedColor::Off;
    currentBrightness = 255U;
    currentDiscreteColorValid = true;
}
