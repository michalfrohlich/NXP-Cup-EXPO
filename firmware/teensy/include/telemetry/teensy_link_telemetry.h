#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "../../../../shared/protocol/teensy_link_protocol.h"

struct TeensyLinkCameraResult
{
    int8_t errorPct;
    uint8_t status;
    uint8_t feature;
    uint8_t confidence;
    uint8_t leftLineIdx;
    uint8_t rightLineIdx;
    uint8_t ageMs;
    uint8_t flags;
};

struct TeensyLinkImuSample
{
    float axG;
    float ayG;
    float azG;
    float gxDps;
    float gyDps;
    float gzDps;
    float rollDeg;
    float pitchDeg;
    float yawDeg;
    float accelNormG;
    float lateralG;
    float tempC;
};

struct TeensyLinkTelemetryInputs
{
    uint16_t sensorSeq;
    uint16_t sensorDtUs;
    uint16_t sensorAgeMs;
    uint16_t statusFlags;
    uint16_t componentMask;
    TeensyLinkCameraResult camera[TEENSY_LINK_CAMERA_COUNT];
    TeensyLinkImuSample imu;
    uint16_t loggerFlags;
    uint16_t loggerDropCount;
};

struct TeensyLinkS32kSnapshot
{
    bool valid;
    uint16_t frameSeq;
    uint32_t timeMs;
    uint16_t flags;
    uint16_t ackTeensySeq;
    uint16_t controlLoopSeq;
    uint16_t controlDtUs;
    uint8_t appMode;
    uint8_t appState;
    uint16_t safetyFlags;
    TeensyLinkCameraResult camera[TEENSY_LINK_CAMERA_COUNT];
    int16_t steerRaw;
    int16_t steerFilt;
    int16_t steerOut;
    int16_t targetSpeedPct;
    int16_t currentSpeedPct;
    int16_t escPrimaryLogical;
    int16_t escSecondaryLogical;
    int16_t servoCmd;
    uint16_t actuatorFlags;
    uint16_t ultrasonicDistanceCm10;
    uint16_t ultrasonicAgeMs;
    uint16_t ultrasonicFlags;
    uint16_t diagFlags;
};

void TeensyLinkTelemetry_DefaultCamera(TeensyLinkCameraResult &camera, uint8_t sourceFlag);
void TeensyLinkTelemetry_BuildTeensyFrame(uint8_t frame[TEENSY_LINK_FRAME_BYTES],
                                          uint16_t frameSeq,
                                          uint32_t nowMs,
                                          uint16_t ackS32kSeq,
                                          const TeensyLinkTelemetryInputs &inputs,
                                          uint16_t teensyRxFrames,
                                          uint16_t teensyRxErrors);
bool TeensyLinkTelemetry_DecodeS32kFrame(const uint8_t frame[TEENSY_LINK_FRAME_BYTES],
                                         TeensyLinkS32kSnapshot &outSnapshot);
