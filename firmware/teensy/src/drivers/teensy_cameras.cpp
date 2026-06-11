#include "drivers/teensy_cameras.h"

#include "teensy_config.h"

static TeensyCameraSnapshot cameras[2] = {};
static const uint8_t analogPins[2] = {
    TEENSY_CAM1_ANALOG_PIN,
    TEENSY_CAM2_ANALOG_PIN};
static const bool configured[2] = {
    TEENSY_CAM1_CONFIGURED,
    TEENSY_CAM2_CONFIGURED};

static void resetWindow(uint8_t cameraIndex)
{
    cameras[cameraIndex].analogMin = UINT16_MAX;
    cameras[cameraIndex].analogMax = 0U;
    cameras[cameraIndex].sampleCount = 0U;
}

void TeensyCameras_Init()
{
    analogReadResolution(TEENSY_CAMERA_ANALOG_BITS);

    for (uint8_t cameraIndex = 0U; cameraIndex < 2U; cameraIndex++)
    {
        pinMode(analogPins[cameraIndex], INPUT);
        cameras[cameraIndex].state = configured[cameraIndex]
                                         ? TeensyCameraState::NoData
                                         : TeensyCameraState::NotConfigured;
        resetWindow(cameraIndex);
    }
}

void TeensyCameras_Service()
{
    for (uint8_t cameraIndex = 0U; cameraIndex < 2U; cameraIndex++)
    {
        if (!configured[cameraIndex])
        {
            continue;
        }

        uint16_t sample = analogRead(analogPins[cameraIndex]);
        cameras[cameraIndex].analogMin = min(cameras[cameraIndex].analogMin, sample);
        cameras[cameraIndex].analogMax = max(cameras[cameraIndex].analogMax, sample);
        cameras[cameraIndex].sampleCount++;

        uint16_t range = cameras[cameraIndex].analogMax - cameras[cameraIndex].analogMin;
        cameras[cameraIndex].state = range >= TEENSY_CAMERA_ACTIVITY_RANGE
                                         ? TeensyCameraState::Ready
                                         : TeensyCameraState::NoData;
    }
}

TeensyCameraSnapshot TeensyCameras_GetSnapshot(uint8_t cameraIndex)
{
    if (cameraIndex >= 2U)
    {
        TeensyCameraSnapshot invalid = {};
        invalid.state = TeensyCameraState::Error;
        return invalid;
    }

    TeensyCameraSnapshot current = cameras[cameraIndex];
    resetWindow(cameraIndex);
    return current;
}

const char *TeensyCameras_StateText(TeensyCameraState state)
{
    switch (state)
    {
        case TeensyCameraState::NotConfigured:
            return "NOT_CONFIGURED";
        case TeensyCameraState::NotConnected:
            return "NOT_CONNECTED";
        case TeensyCameraState::NoData:
            return "NO_DATA";
        case TeensyCameraState::Ready:
            return "READY";
        case TeensyCameraState::Error:
            return "ERROR";
        default:
            return "ERROR";
    }
}
