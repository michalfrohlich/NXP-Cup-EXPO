#pragma once

#include <Arduino.h>

enum class TeensyCameraState : uint8_t
{
    NotConfigured = 0U,
    NotConnected,
    NoData,
    Ready,
    Error
};

struct TeensyCameraSnapshot
{
    TeensyCameraState state;
    uint16_t analogMin;
    uint16_t analogMax;
    uint16_t sampleCount;
};

void TeensyCameras_Init();
void TeensyCameras_Service();
TeensyCameraSnapshot TeensyCameras_GetSnapshot(uint8_t cameraIndex);
const char *TeensyCameras_StateText(TeensyCameraState state);
