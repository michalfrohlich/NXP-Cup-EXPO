#include "app/teensy_app_modes.h"

#include <Arduino.h>

#include "config/camera_config.h"
#include "drivers/linear_camera.h"
#include "teensy_config.h"

static LinearCamera camera0;
static uint32_t nextDebugPrintMs = 0U;
static LinearCameraFrame latestFrame = {};
static bool latestFrameValid = false;

static const LinearCameraConfig camera0Config = {
    TEENSY_LINEAR_CAMERA0_SI_PIN,
    TEENSY_LINEAR_CAMERA0_CLK_PIN,
    TEENSY_LINEAR_CAMERA0_ANALOG_PIN,
    TEENSY_LINEAR_CAMERA_PIXEL_CLOCK_HZ,
    TEENSY_LINEAR_CAMERA_FRAME_RATE_HZ,
};

static const char *statusName(LinearCameraStatus status)
{
    switch (status)
    {
        case LinearCameraStatus::Idle:
            return "IDLE";
        case LinearCameraStatus::Running:
            return "RUN";
        case LinearCameraStatus::Fault:
            return "FAULT";
        default:
            return "?";
    }
}

static uint8_t scaleAdcSampleToByte(uint16_t sample)
{
    static constexpr uint32_t adcMax =
        (1UL << TEENSY_LINEAR_CAMERA_ADC_BITS) - 1UL;

    return (uint8_t)((((uint32_t)sample * 255UL) + (adcMax / 2UL)) / adcMax);
}

static void printScaledFrame8()
{
    Serial.print(" frame8=");
    if (latestFrameValid != true)
    {
        Serial.print("[]");
        return;
    }

    Serial.print("[");
    for (uint16_t i = 0U; i < TEENSY_LINEAR_CAMERA_PIXEL_COUNT; i++)
    {
        if (i != 0U)
        {
            Serial.print(",");
        }
        Serial.print(scaleAdcSampleToByte(latestFrame.values[i]));
    }
    Serial.print("]");
}

static void printCameraDebugFrame(uint32_t nowMs)
{
    LinearCameraDebugCounters counters = {};

    camera0.getDebugCounters(counters);

    Serial.print("t=");
    Serial.print(nowMs);
    Serial.print(" mode=cam-bringup");
    Serial.print(" status=");
    Serial.print(statusName(camera0.status()));
    Serial.print(" clkHz=");
    Serial.print(camera0.pixelClockHz());
    Serial.print(" frameHz=");
    Serial.print(camera0.frameRateHz());
    Serial.print(" frameUs=");
    Serial.print(camera0.framePeriodUs());
    Serial.print(" readoutUs=");
    Serial.print(camera0.readoutWindowUs());
    Serial.print(" req=");
    Serial.print(counters.frameRequestCount);
    Serial.print(" si=");
    Serial.print(counters.siPulseCount);
    Serial.print(" win=");
    Serial.print(counters.readoutWindowCount);
    Serial.print(" ready=");
    Serial.print(counters.frameReadyCount);
    Serial.print(" drop=");
    Serial.print(counters.droppedFrameCount);
    Serial.print(" samples=");
    Serial.print(counters.adcSampleCount);
    Serial.print(" overrun=");
    Serial.print(counters.timingOverrunCount);
    Serial.print(" capUs=");
    Serial.print(counters.lastCaptureUs);
    Serial.print("/");
    Serial.print(counters.maxCaptureUs);
    Serial.print(" adcErr=");
    Serial.print(counters.adcErrorCount);

    if (latestFrameValid == true)
    {
        uint16_t minValue = latestFrame.values[0];
        uint16_t maxValue = latestFrame.values[0];
        uint32_t sum = 0U;

        for (uint16_t i = 0U; i < TEENSY_LINEAR_CAMERA_PIXEL_COUNT; i++)
        {
            uint16_t value = latestFrame.values[i];
            if (value < minValue)
            {
                minValue = value;
            }
            if (value > maxValue)
            {
                maxValue = value;
            }
            sum += value;
        }

        Serial.print(" seq=");
        Serial.print(latestFrame.sequence);
        Serial.print(" min=");
        Serial.print(minValue);
        Serial.print(" max=");
        Serial.print(maxValue);
        Serial.print(" avg=");
        Serial.print(sum / TEENSY_LINEAR_CAMERA_PIXEL_COUNT);
        Serial.print(" p0=");
        Serial.print(latestFrame.values[0]);
        Serial.print(" p64=");
        Serial.print(latestFrame.values[64]);
        Serial.print(" p127=");
        Serial.print(latestFrame.values[127]);
    }

    printScaledFrame8();
    Serial.println();
}

void ModeCameraBringup_Setup()
{
    Serial.begin(TEENSY_SERIAL_BAUD);
    while (!Serial && (millis() < 1200UL))
    {
    }

    Serial.println("Teensy TSL1401CL camera bring-up");
    Serial.println("Camera 0: SI=4 CLK=6 AO=A1/pin15, FlexPWM/ADC_ETC 50 kHz, 100 Hz raw acquisition");

    if (camera0.begin(camera0Config) != true)
    {
        Serial.println("Camera init failed");
        return;
    }

    if (camera0.start() != true)
    {
        Serial.println("Camera start failed");
        return;
    }

    nextDebugPrintMs = millis();
}

void ModeCameraBringup_Loop()
{
    uint32_t nowUs = micros();
    uint32_t nowMs = millis();
    camera0.service(nowUs);
    if (camera0.getLatestFrame(latestFrame) == true)
    {
        latestFrameValid = true;
    }

    if (((int32_t)(nowMs - nextDebugPrintMs) >= 0) &&
        (camera0.isReadoutActive() != true))
    {
        printCameraDebugFrame(nowMs);
        nextDebugPrintMs += TEENSY_LINEAR_CAMERA_DEBUG_PRINT_PERIOD_MS;
    }
}
