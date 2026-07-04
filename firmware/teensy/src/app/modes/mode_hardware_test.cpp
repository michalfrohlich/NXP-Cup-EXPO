#include "app/teensy_app_modes.h"

#include <Arduino.h>

#include "drivers/board_inputs.h"
#include "drivers/status_led.h"
#include "teensy_config.h"

static uint32_t nextPrintMs = 0U;
static uint8_t ledStep = 0U;
static bool ledEnabled = true;

static void configureSpiPinsAsInputs()
{
    pinMode(TEENSY_LINK_SPI_CS_PIN, INPUT_PULLUP);
    pinMode(TEENSY_LINK_SPI_SCK_PIN, INPUT_PULLDOWN);
    pinMode(TEENSY_LINK_SPI_MOSI_PIN, INPUT_PULLDOWN);
    pinMode(TEENSY_LINK_READY_PIN, OUTPUT);
    digitalWriteFast(TEENSY_LINK_READY_PIN, LOW);
}

static uint8_t ledBrightnessFromPot(uint16_t potRaw)
{
    return (uint8_t)((potRaw * 255UL) / 4095UL);
}

static void updateLed(const BoardInputsSnapshot &inputs)
{
    static const StatusLedColor cycle[] = {
        StatusLedColor::Red,
        StatusLedColor::Green,
        StatusLedColor::Blue,
    };

    if (BoardInputs_WasPressed(BoardButtonId::Button1))
    {
        ledStep = (uint8_t)((ledStep + 1U) % (uint8_t)(sizeof(cycle) / sizeof(cycle[0])));
    }

    if (BoardInputs_WasPressed(BoardButtonId::Button2))
    {
        ledEnabled = !ledEnabled;
    }

    if (ledEnabled != true)
    {
        StatusLed_Set(StatusLedColor::Off);
        return;
    }

    StatusLed_SetBrightness(cycle[ledStep], ledBrightnessFromPot(inputs.potRaw));
}

static void printStatus(uint32_t nowMs, const BoardInputsSnapshot &inputs)
{
    if ((int32_t)(nowMs - nextPrintMs) < 0)
    {
        return;
    }

    nextPrintMs = nowMs + 200UL;
    Serial.print("HWTEST pot=");
    Serial.print(inputs.potRaw);
    Serial.print(" b1=");
    Serial.print(inputs.button1Pressed ? 1U : 0U);
    Serial.print(" b2=");
    Serial.print(inputs.button2Pressed ? 1U : 0U);
    Serial.print(" led=");
    Serial.print(ledEnabled ? 1U : 0U);
    Serial.print(" color=");
    Serial.print((ledStep == 0U) ? "R" : ((ledStep == 1U) ? "G" : "B"));
    Serial.print(" cs=");
    Serial.print(digitalReadFast(TEENSY_LINK_SPI_CS_PIN));
    Serial.print(" sck=");
    Serial.print(digitalReadFast(TEENSY_LINK_SPI_SCK_PIN));
    Serial.print(" mosi=");
    Serial.println(digitalReadFast(TEENSY_LINK_SPI_MOSI_PIN));
}

void ModeHardwareTest_Setup()
{
    Serial.begin(TEENSY_SERIAL_BAUD);
    while (!Serial && (millis() < 1200UL))
    {
    }

    BoardInputs_Init();
    StatusLed_Init();
    configureSpiPinsAsInputs();

    Serial.println("HWTEST Teensy PCB input/LED self-test");
    Serial.println("HWTEST READY held LOW; S32K SPI transfers are disabled in this mode");
    Serial.println("HWTEST button1 press=next LED R/G/B, button2 press=toggle LED power");
    Serial.println("HWTEST Potentiometer controls LED brightness (PWM)");
}

void ModeHardwareTest_Loop()
{
    uint32_t nowMs = millis();
    BoardInputsSnapshot inputs = BoardInputs_Read();

    updateLed(inputs);
    printStatus(nowMs, inputs);
}
