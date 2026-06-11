#include "modes/mode_dual_camera_test.h"

#include <Arduino.h>

#include "drivers/teensy_cameras.h"
#include "teensy_config.h"

static uint8_t selectedCamera = 0U;
static uint32_t nextPrintMs = 0U;
static bool previousButton1 = true;
static bool previousButton2 = true;

static void updateSelection()
{
    bool button1 = digitalRead(TEENSY_BUTTON_1_PIN) != LOW;
    bool button2 = digitalRead(TEENSY_BUTTON_2_PIN) != LOW;

    if (previousButton1 && !button1)
    {
        selectedCamera = 0U;
    }
    if (previousButton2 && !button2)
    {
        selectedCamera = 1U;
    }

    uint16_t pot = analogRead(TEENSY_POT_PIN);
    if (pot < TEENSY_CAMERA_POT_LOW)
    {
        selectedCamera = 0U;
    }
    else if (pot > TEENSY_CAMERA_POT_HIGH)
    {
        selectedCamera = 1U;
    }

    previousButton1 = button1;
    previousButton2 = button2;
}

static void printCamera(uint8_t cameraIndex, TeensyCameraSnapshot camera)
{
    Serial.print(cameraIndex == selectedCamera ? "> " : "  ");
    Serial.print("camera");
    Serial.print(cameraIndex + 1U);
    Serial.print(" state=");
    Serial.print(TeensyCameras_StateText(camera.state));
    Serial.print(" samples=");
    Serial.print(camera.sampleCount);
    if (camera.sampleCount > 0U)
    {
        Serial.print(" min=");
        Serial.print(camera.analogMin);
        Serial.print(" max=");
        Serial.println(camera.analogMax);
    }
    else
    {
        Serial.println(" min=- max=-");
    }
}

void ModeDualCameraTest_Setup()
{
    Serial.begin(TEENSY_SERIAL_BAUD);
    while (!Serial && (millis() < 1200UL))
    {
    }

    pinMode(TEENSY_BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(TEENSY_BUTTON_2_PIN, INPUT_PULLUP);
    pinMode(TEENSY_POT_PIN, INPUT);
    TeensyCameras_Init();

    Serial.println("Teensy dual-camera pin test");
    Serial.println("Button 1 selects camera 1; button 2 selects camera 2.");
}

void ModeDualCameraTest_Loop()
{
    uint32_t nowMs = millis();

    updateSelection();
    TeensyCameras_Service();

    if ((int32_t)(nowMs - nextPrintMs) < 0)
    {
        return;
    }

    printCamera(0U, TeensyCameras_GetSnapshot(0U));
    printCamera(1U, TeensyCameras_GetSnapshot(1U));
    Serial.println();
    nextPrintMs = nowMs + TEENSY_CAMERA_PRINT_PERIOD_MS;
}
