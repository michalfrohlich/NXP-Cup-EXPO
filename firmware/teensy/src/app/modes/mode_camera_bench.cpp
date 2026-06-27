#include "app/teensy_app_modes.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "config/camera_config.h"
#include "config/display_config.h"
#include "debug/camera_stream.h"
#include "drivers/display.h"
#include "drivers/linear_camera.h"
#include "drivers/status_led.h"
#include "services/line_detector.h"
#include "teensy_config.h"

static LinearCamera camera0;
static LineDetector trackingLedDetector;
static uint32_t nextDebugPrintMs = 0U;
static uint32_t nextDisplayRefreshMs = 0U;
static uint32_t nextStreamMs = 0U;
static LinearCameraFrame latestFrame = {};
static uint16_t trackingLedPixels[VISION_LINEAR_BUFFER_SIZE] = {};
static VisionOutput trackingLedVision = {};
static bool latestFrameValid = false;
static bool displayReady = false;
static uint8_t displayFramePercent[TEENSY_LINEAR_CAMERA_PIXEL_COUNT] = {};

static const LinearCameraConfig camera0Config = {
    TEENSY_LINEAR_CAMERA0_SI_PIN,       TEENSY_LINEAR_CAMERA0_CLK_PIN,
    TEENSY_LINEAR_CAMERA0_ANALOG_PIN,   TEENSY_LINEAR_CAMERA_PIXEL_CLOCK_HZ,
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
    static constexpr uint32_t adcMax = (1UL << TEENSY_LINEAR_CAMERA_ADC_BITS) - 1UL;

    return (uint8_t)((((uint32_t)sample * 255UL) + (adcMax / 2UL)) / adcMax);
}

static uint8_t scaleAdcSampleToPercent(uint16_t sample)
{
    static constexpr uint32_t adcMax = (1UL << TEENSY_LINEAR_CAMERA_ADC_BITS) - 1UL;

    return (uint8_t)((((uint32_t)sample * 100UL) + (adcMax / 2UL)) / adcMax);
}

static void streamLatestFrame()
{
    if ((TEENSY_LINEAR_CAMERA_STREAM_ENABLED != true) || (latestFrameValid != true))
    {
        return;
    }

    CameraStream_WriteRawFrame(latestFrame);
}

static void updateTrackingLed()
{
    if (latestFrameValid != true)
    {
        StatusLed_SetRgb(128U, 0U, 0U);
        return;
    }

    memcpy(trackingLedPixels, &latestFrame.values[CAM_TRIM_LEFT_PX], sizeof(trackingLedPixels));
    trackingLedDetector.process(trackingLedPixels, trackingLedVision);

    if (trackingLedVision.status == VISION_TRACK_LOST)
    {
        StatusLed_SetRgb(128U, 0U, 0U);
        return;
    }

    float error = trackingLedVision.error;
    if (error > 1.0f)
    {
        error = 1.0f;
    }
    else if (error < -1.0f)
    {
        error = -1.0f;
    }

    const uint8_t green = (error < 0.0f) ? (uint8_t)(((-error) * 255.0f) + 0.5f) : 0U;
    const uint8_t blue = (error > 0.0f) ? (uint8_t)((error * 255.0f) + 0.5f) : 0U;
    StatusLed_SetRgb(0U, green, blue);
}

static void refreshCameraDisplay()
{
    if ((displayReady != true) || (latestFrameValid != true))
    {
        return;
    }

    uint16_t minValue = latestFrame.values[0];
    uint16_t maxValue = latestFrame.values[0];

    for (uint16_t i = 0U; i < TEENSY_LINEAR_CAMERA_PIXEL_COUNT; i++)
    {
        const uint16_t value = latestFrame.values[i];
        if (value < minValue)
        {
            minValue = value;
        }
        if (value > maxValue)
        {
            maxValue = value;
        }
        displayFramePercent[i] = scaleAdcSampleToPercent(value);
    }

    char header[TEENSY_DISPLAY_CHARACTER_COLUMNS + 1U];
    memset(header, ' ', sizeof(header));
    header[TEENSY_DISPLAY_CHARACTER_COLUMNS] = '\0';
    snprintf(header, sizeof(header), "%04u %04u", (unsigned)minValue, (unsigned)maxValue);

    DisplayClear();
    DisplayText(0U, header, TEENSY_DISPLAY_CHARACTER_COLUMNS, 0U);
    DisplayGraph(1U, displayFramePercent, TEENSY_LINEAR_CAMERA_PIXEL_COUNT,
                 (uint8_t)(TEENSY_DISPLAY_CHARACTER_ROWS - 1U));
    DisplayRefresh();
}

static void printScaledFrame8()
{
    if (TEENSY_LINEAR_CAMERA_DEBUG_PRINT_FRAME8 != true)
    {
        return;
    }

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
    if ((TEENSY_LINEAR_CAMERA_DEBUG_ENABLED != true) ||
        ((TEENSY_LINEAR_CAMERA_DEBUG_PRINT_STATUS != true) &&
         (TEENSY_LINEAR_CAMERA_DEBUG_PRINT_FRAME8 != true)))
    {
        return;
    }

    LinearCameraDebugCounters counters = {};

    camera0.getDebugCounters(counters);

    Serial.print("t=");
    Serial.print(nowMs);
    Serial.print(" mode=cam-bench");

    if (TEENSY_LINEAR_CAMERA_DEBUG_PRINT_STATUS == true)
    {
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
        Serial.print(" capClk=");
        Serial.print(
            (uint32_t)((((uint64_t)counters.lastCaptureUs * camera0.pixelClockHz()) + 500000ULL) /
                       1000000ULL));
        Serial.print(" adcErr=");
        Serial.print(counters.adcErrorCount);
        Serial.print(" dma=");
        Serial.print(counters.dmaFrameCount);
        Serial.print(" dmaErr=");
        Serial.print(counters.dmaErrorCount);
    }

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
        if (TEENSY_LINEAR_CAMERA_DEBUG_PRINT_STATUS == true)
        {
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
    }

    printScaledFrame8();
    Serial.println();
}

void ModeCameraBench_Setup()
{
    StatusLed_Init();
    StatusLed_SetRgb(128U, 0U, 0U);
    trackingLedDetector.init();

    if (TEENSY_DISPLAY_CAMERA_DEBUG_ENABLED == true)
    {
        displayReady = DisplayInit();
    }

    if ((TEENSY_LINEAR_CAMERA_DEBUG_ENABLED == true) ||
        (TEENSY_LINEAR_CAMERA_STREAM_ENABLED == true))
    {
        Serial.begin(TEENSY_LINEAR_CAMERA_DEBUG_SERIAL_BAUD);
        while (!Serial && (millis() < 1200UL))
        {
        }

        Serial.println("Teensy TSL1401CL camera bench");
        Serial.print("Camera 0: SI=4 CLK=6 AO=A1/pin15, FlexPWM/ADC_ETC ");
        Serial.print(TEENSY_LINEAR_CAMERA_PIXEL_CLOCK_HZ);
        Serial.print(" Hz, ");
        Serial.print(TEENSY_LINEAR_CAMERA_FRAME_RATE_HZ);
        Serial.println(" Hz DMA acquisition");
        Serial.print("Debug: baud=");
        Serial.print(TEENSY_LINEAR_CAMERA_DEBUG_SERIAL_BAUD);
        Serial.print(" periodMs=");
        Serial.print(TEENSY_LINEAR_CAMERA_DEBUG_PRINT_PERIOD_MS);
        Serial.print(" status=");
        Serial.print(TEENSY_LINEAR_CAMERA_DEBUG_PRINT_STATUS ? "on" : "off");
        Serial.print(" frame8=");
        Serial.print(TEENSY_LINEAR_CAMERA_DEBUG_PRINT_FRAME8 ? "on" : "off");
        Serial.print(" stream=");
        Serial.println(TEENSY_LINEAR_CAMERA_STREAM_ENABLED ? "on" : "off");
        Serial.print("Display: ");
        Serial.print(displayReady ? "ready" : "not found");
        Serial.print(" I2C=Wire1 SDA=");
        Serial.print(TEENSY_DISPLAY_SDA_PIN);
        Serial.print(" SCL=");
        Serial.print(TEENSY_DISPLAY_SCL_PIN);
        Serial.print(" addr=0x");
        Serial.println(TEENSY_DISPLAY_I2C_ADDRESS, HEX);
    }

    if (camera0.begin(camera0Config) != true)
    {
        if (TEENSY_LINEAR_CAMERA_DEBUG_ENABLED == true)
        {
            Serial.println("Camera init failed");
        }
        return;
    }

    if (camera0.start() != true)
    {
        if (TEENSY_LINEAR_CAMERA_DEBUG_ENABLED == true)
        {
            Serial.println("Camera start failed");
        }
        return;
    }

    nextDebugPrintMs = millis();
    nextDisplayRefreshMs = nextDebugPrintMs;
    nextStreamMs = nextDebugPrintMs;
}

void ModeCameraBench_Loop()
{
    uint32_t nowUs = micros();
    uint32_t nowMs = millis();
    camera0.service(nowUs);
    if (camera0.getLatestFrame(latestFrame) == true)
    {
        latestFrameValid = true;
        updateTrackingLed();
    }

    if ((displayReady == true) &&
        (camera0.hasIdleTimeBeforeNextFrame(nowUs, TEENSY_DISPLAY_SERVICE_MIN_IDLE_US) == true))
    {
        DisplayService();
    }

    if (((int32_t)(nowMs - nextDisplayRefreshMs) >= 0) &&
        (camera0.hasIdleTimeBeforeNextFrame(nowUs, TEENSY_DISPLAY_SERVICE_MIN_IDLE_US) == true))
    {
        refreshCameraDisplay();
        do
        {
            nextDisplayRefreshMs += TEENSY_DISPLAY_CAMERA_REFRESH_PERIOD_MS;
        } while ((int32_t)(nowMs - nextDisplayRefreshMs) >= 0);
    }

    if (((int32_t)(nowMs - nextStreamMs) >= 0) && (camera0.isReadoutActive() != true))
    {
        streamLatestFrame();
        do
        {
            nextStreamMs += TEENSY_LINEAR_CAMERA_STREAM_PERIOD_MS;
        } while ((int32_t)(nowMs - nextStreamMs) >= 0);
    }

    if (((int32_t)(nowMs - nextDebugPrintMs) >= 0) && (camera0.isReadoutActive() != true))
    {
        if (TEENSY_LINEAR_CAMERA_DEBUG_ENABLED == true)
        {
            printCameraDebugFrame(nowMs);
        }
        nextDebugPrintMs += TEENSY_LINEAR_CAMERA_DEBUG_PRINT_PERIOD_MS;
    }
}
