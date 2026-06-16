#include "drivers/board_inputs.h"

#include "teensy_config.h"

void BoardInputs_Init()
{
    analogReadResolution(TEENSY_ANALOG_READ_BITS);
    pinMode(TEENSY_POT_PIN, INPUT);
    pinMode(TEENSY_BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(TEENSY_BUTTON_2_PIN, INPUT_PULLUP);
}

BoardInputsSnapshot BoardInputs_Read()
{
    BoardInputsSnapshot inputs = {};

    inputs.potRaw = (uint16_t)analogRead(TEENSY_POT_PIN);
    inputs.button1Pressed = digitalReadFast(TEENSY_BUTTON_1_PIN) == LOW;
    inputs.button2Pressed = digitalReadFast(TEENSY_BUTTON_2_PIN) == LOW;
    return inputs;
}
