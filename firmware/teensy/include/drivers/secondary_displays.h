#pragma once

#include <Arduino.h>

enum class SecondaryDisplayState : uint8_t
{
    NotChecked = 0U,
    NotConnected,
    Detected,
    /* Both modules answer on one I2C address. They receive identical
       bytes by hardware, so they automatically show the same picture. */
    Mirrored
};

struct SecondaryDisplaySnapshot
{
    SecondaryDisplayState display1;
    SecondaryDisplayState display2;
    uint8_t display1Address;
    uint8_t display2Address;
    uint32_t scanCount;
};

struct SecondaryDisplayDashboard
{
    bool s32kValid;
    uint16_t s32kSequence;
    uint16_t rxFrames;
    uint16_t rxErrors;
    uint16_t timeouts;
    uint8_t appMode;
    int16_t targetSpeedPct;
    int16_t currentSpeedPct;
    int16_t servoCmd;
    uint16_t ultrasonicDistanceCm10;

    bool imuPresent;
    bool imuCalibrated;
    bool imuValid;
    int16_t imuYawDeg;
    int16_t imuGzDps;
    bool sdReady;
    bool sdError;
    uint16_t sdDrops;
    uint32_t sdBytesWritten;
    const char *sdFileName;
    uint16_t teensyTxSequence;
    uint16_t sensorSequence;
    uint16_t potRaw;
    bool button1Pressed;
    bool button2Pressed;
};

void SecondaryDisplays_Init();
bool SecondaryDisplays_UpdateDue(uint32_t nowMs);
void SecondaryDisplays_Service(uint32_t nowMs, const SecondaryDisplayDashboard &dashboard);
SecondaryDisplaySnapshot SecondaryDisplays_GetSnapshot();
const char *SecondaryDisplays_StateText(SecondaryDisplayState state);
