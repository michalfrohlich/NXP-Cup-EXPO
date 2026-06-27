#pragma once

#include <stdint.h>

#include "comms/teensy_link_spi_slave.h"
#include "drivers/mpu6050_imu.h"
#include "services/camera_vision.h"
#include "telemetry/teensy_link_telemetry.h"

struct TeensyRaceRuntimeConfig
{
    bool serialStatusEnabled;
    bool cameraStreamEnabled;
    uint32_t serialStatusPeriodMs;
    bool trackingLedEnabled;
    bool stackDisplayEnabled;
    uint32_t stackDisplayPeriodMs;
};

class TeensyRaceRuntime
{
  public:
    void begin(const TeensyRaceRuntimeConfig &config);
    void service();

  private:
    void publishFrame(uint32_t nowMs);
    void updateSensors(uint32_t nowUs, uint32_t nowMs);
    void updateTrackingLed();
    void initStackDisplay();
    void serviceStackDisplay(uint32_t nowMs);
    void drawStackDisplay();
    void printLinkStatus(uint32_t nowMs);
    void streamCameraVision0();

    TeensyRaceRuntimeConfig config_ = {};
    TeensyLinkSpiSlave s32kSpi_;
    CameraVision cameraVision0_;
    Mpu6050Imu imu_;
    TeensyLinkTelemetryInputs telemetry_ = {};
    TeensyLinkS32kSnapshot s32kSnapshot_ = {};
    uint8_t txFrame_[TEENSY_LINK_FRAME_BYTES] = {};

    uint16_t teensyFrameSeq_ = 0U;
    uint16_t sensorSeq_ = 0U;
    uint32_t lastSensorMs_ = 0U;
    uint32_t nextImuUs_ = 0U;
    uint32_t nextSensorUs_ = 0U;
    uint32_t nextSerialMs_ = 0U;
    uint32_t nextCameraStreamMs_ = 0U;
    uint32_t nextStackDisplayMs_ = 0U;
    bool cameraVision0Ready_ = false;
    bool stackDisplayReady_ = false;
};
