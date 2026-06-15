#pragma once

#include <Arduino.h>

#include "telemetry/teensy_link_telemetry.h"

class Mpu6050Imu
{
public:
    bool begin();
    bool sample(uint32_t nowUs);

    bool isPresent() const;
    bool isCalibrated() const;
    bool isValid(uint32_t nowMs) const;
    bool isAccelTrusted() const;
    uint8_t address() const;
    uint16_t sequence() const;
    uint16_t sampleDtUs() const;
    uint16_t ageMs(uint32_t nowMs) const;
    uint32_t readErrorCount() const;
    const TeensyLinkImuSample &latest() const;

private:
    bool probe(uint8_t address);
    bool configure();
    bool calibrateGyro();
    bool readRaw(int16_t raw[7]);
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t &value);
    static float wrapDegrees(float degrees);

    TeensyLinkImuSample latest_ = {};
    float gyroBiasX_ = 0.0f;
    float gyroBiasY_ = 0.0f;
    float gyroBiasZ_ = 0.0f;
    uint32_t lastSampleUs_ = 0U;
    uint32_t lastSampleMs_ = 0U;
    uint32_t readErrorCount_ = 0U;
    uint16_t sequence_ = 0U;
    uint16_t sampleDtUs_ = 0U;
    uint8_t address_ = 0U;
    bool present_ = false;
    bool calibrated_ = false;
    bool sampleValid_ = false;
    bool accelTrusted_ = false;
    bool filterStarted_ = false;
};
