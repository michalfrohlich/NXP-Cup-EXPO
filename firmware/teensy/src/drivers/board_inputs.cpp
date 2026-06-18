#include "drivers/board_inputs.h"

#include "teensy_config.h"

namespace
{
constexpr uint32_t BUTTON_DEBOUNCE_MS = 15UL;
constexpr uint8_t BUTTON_COUNT = (uint8_t)BoardButtonId::Count;

struct ButtonContext
{
    bool stableState;
    bool lastRawState;
    bool pressedEvent;
    uint32_t candidateSinceMs;
};

static ButtonContext buttonState[BUTTON_COUNT];

static uint8_t buttonIndex(BoardButtonId id)
{
    return (uint8_t)id;
}

static bool isValidButton(BoardButtonId id)
{
    return buttonIndex(id) < BUTTON_COUNT;
}

static bool readButtonRaw(BoardButtonId id)
{
    switch (id)
    {
        case BoardButtonId::Button1:
            return digitalReadFast(TEENSY_BUTTON_1_PIN) == LOW;
        case BoardButtonId::Button2:
            return digitalReadFast(TEENSY_BUTTON_2_PIN) == LOW;
        default:
            return false;
    }
}

static void updateButton(BoardButtonId id, uint32_t nowMs)
{
    ButtonContext &button = buttonState[buttonIndex(id)];
    const bool rawPressed = readButtonRaw(id);

    if (rawPressed != button.lastRawState)
    {
        button.lastRawState = rawPressed;
        button.candidateSinceMs = nowMs;
        return;
    }

    if ((rawPressed != button.stableState) &&
        ((uint32_t)(nowMs - button.candidateSinceMs) >= BUTTON_DEBOUNCE_MS))
    {
        const bool previousStable = button.stableState;
        button.stableState = rawPressed;

        if ((previousStable != true) && (button.stableState == true))
        {
            button.pressedEvent = true;
        }
    }
}
} // namespace

void BoardInputs_Init()
{
    analogReadResolution(TEENSY_ANALOG_READ_BITS);
    pinMode(TEENSY_POT_PIN, INPUT);
    pinMode(TEENSY_BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(TEENSY_BUTTON_2_PIN, INPUT_PULLUP);

    const uint32_t nowMs = millis();
    for (uint8_t i = 0U; i < BUTTON_COUNT; i++)
    {
        const BoardButtonId id = (BoardButtonId)i;
        buttonState[i].stableState = false;
        buttonState[i].lastRawState = readButtonRaw(id);
        buttonState[i].pressedEvent = false;
        buttonState[i].candidateSinceMs = nowMs;
    }
}

void BoardInputs_Update()
{
    const uint32_t nowMs = millis();

    for (uint8_t i = 0U; i < BUTTON_COUNT; i++)
    {
        updateButton((BoardButtonId)i, nowMs);
    }
}

BoardInputsSnapshot BoardInputs_Read()
{
    BoardInputsSnapshot inputs = {};

    BoardInputs_Update();
    inputs.potRaw = (uint16_t)analogRead(TEENSY_POT_PIN);
    inputs.button1Pressed = BoardInputs_IsPressed(BoardButtonId::Button1);
    inputs.button2Pressed = BoardInputs_IsPressed(BoardButtonId::Button2);
    return inputs;
}

bool BoardInputs_IsPressed(BoardButtonId id)
{
    if (isValidButton(id) != true)
    {
        return false;
    }

    return buttonState[buttonIndex(id)].stableState;
}

bool BoardInputs_WasPressed(BoardButtonId id)
{
    if (isValidButton(id) != true)
    {
        return false;
    }

    ButtonContext &button = buttonState[buttonIndex(id)];
    if (button.pressedEvent == true)
    {
        button.pressedEvent = false;
        return true;
    }

    return false;
}
