#include "services/teensy_race_runtime.h"

#include <Arduino.h>

#include "config/camera_config.h"
#include "debug/camera_stream.h"
#include "teensy_config.h"

static const CameraVisionConfig cameraVision0Config = {
    {
        TEENSY_LINEAR_CAMERA0_SI_PIN,
        TEENSY_LINEAR_CAMERA0_CLK_PIN,
        TEENSY_LINEAR_CAMERA0_ANALOG_PIN,
        TEENSY_LINEAR_CAMERA_PIXEL_CLOCK_HZ,
        TEENSY_LINEAR_CAMERA_FRAME_RATE_HZ,
    },
};

static_assert(TEENSY_LINK_FRAME_BYTES == 128U, "S32K link frame must stay 128 bytes");

void TeensyRaceRuntime::begin(const TeensyRaceRuntimeConfig &config)
{
    config_ = config;

    if ((config_.serialStatusEnabled == true) ||
        (config_.cameraStreamEnabled == true))
    {
        Serial.begin(config_.cameraStreamEnabled
                         ? TEENSY_LINEAR_CAMERA_DEBUG_SERIAL_BAUD
                         : TEENSY_SERIAL_BAUD);
        while (!Serial && (millis() < 1200UL))
        {
        }
    }

    TeensyLinkTelemetry_DefaultCamera(telemetry_.camera[0], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);
    TeensyLinkTelemetry_DefaultCamera(telemetry_.camera[1], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);

    cameraVision0Ready_ = cameraVision0_.begin(cameraVision0Config);
    if (cameraVision0Ready_ == true)
    {
        cameraVision0Ready_ = cameraVision0_.start();
    }

    s32kSpi_.begin(TEENSY_LINK_SPI_CS_PIN,
                   TEENSY_LINK_SPI_SCK_PIN,
                   TEENSY_LINK_SPI_MOSI_PIN,
                   TEENSY_LINK_SPI_MISO_PIN,
                   TEENSY_LINK_READY_PIN);

    updateSensors(micros(), millis());
    publishFrame(millis());

    nextSensorUs_ = micros();
    nextSerialMs_ = millis();
    nextCameraStreamMs_ = millis();

    if (config_.serialStatusEnabled == true)
    {
        Serial.println("Teensy S32K SPI slave v1 link");
        Serial.println("Packet: 128-byte full-duplex teensy_link frame, CRC-16/CCITT-FALSE");
        Serial.print("Camera vision: ");
        Serial.println(cameraVision0Ready_ ? "ready" : "not available");
    }
}

void TeensyRaceRuntime::service()
{
    uint32_t nowUs = micros();
    uint32_t nowMs = millis();

    if (cameraVision0Ready_ == true)
    {
        cameraVision0_.service(nowUs);
    }

    if ((config_.cameraStreamEnabled == true) &&
        (cameraVision0Ready_ == true) &&
        (cameraVision0_.isCameraReadoutActive() != true) &&
        ((int32_t)(nowMs - nextCameraStreamMs_) >= 0))
    {
        streamCameraVision0();
        do
        {
            nextCameraStreamMs_ += TEENSY_LINEAR_CAMERA_STREAM_PERIOD_MS;
        } while ((int32_t)(nowMs - nextCameraStreamMs_) >= 0);
    }

    if (s32kSpi_.service() == true)
    {
        (void)s32kSpi_.getLatestS32kFrame(s32kSnapshot_);
        publishFrame(nowMs);
    }

    if ((int32_t)(nowUs - nextSensorUs_) >= 0)
    {
        updateSensors(nowUs, nowMs);
        publishFrame(nowMs);
        nextSensorUs_ += TEENSY_LINK_SENSOR_INTERVAL_US;
        int32_t scheduleSlipUs = (int32_t)(nowUs - nextSensorUs_);
        if (scheduleSlipUs > (int32_t)TEENSY_LINK_SENSOR_INTERVAL_US)
        {
            nextSensorUs_ = nowUs + TEENSY_LINK_SENSOR_INTERVAL_US;
        }
    }

    if ((config_.serialStatusEnabled == true) &&
        ((uint32_t)(nowMs - nextSerialMs_) >= config_.serialStatusPeriodMs))
    {
        nextSerialMs_ = nowMs;
        printLinkStatus(nowMs);
    }
}

void TeensyRaceRuntime::publishFrame(uint32_t nowMs)
{
    uint16_t ackS32kSeq = s32kSnapshot_.valid ? s32kSnapshot_.frameSeq : 0U;

    telemetry_.sensorAgeMs = (uint16_t)(nowMs - lastSensorMs_);
    TeensyLinkTelemetry_BuildTeensyFrame(txFrame_,
                                         teensyFrameSeq_,
                                         nowMs,
                                         ackS32kSeq,
                                         telemetry_,
                                         s32kSpi_.completedTransfers(),
                                         s32kSpi_.protocolErrorCount());
    teensyFrameSeq_++;
    s32kSpi_.publishFrame(txFrame_);
}

void TeensyRaceRuntime::updateSensors(uint32_t nowUs, uint32_t nowMs)
{
    float phase = (float)(nowMs % 10000UL) * 0.001f;

    telemetry_.sensorSeq = sensorSeq_;
    telemetry_.sensorDtUs = (uint16_t)TEENSY_LINK_SENSOR_INTERVAL_US;
    telemetry_.sensorAgeMs = 0U;
    telemetry_.statusFlags = TEENSY_LINK_STATUS_IMU_PRESENT |
                             TEENSY_LINK_STATUS_IMU_CALIBRATED |
                             TEENSY_LINK_STATUS_IMU_VALID |
                             TEENSY_LINK_STATUS_ACCEL_TRUSTED |
                             TEENSY_LINK_STATUS_YAW_RELATIVE;
    telemetry_.componentMask = TEENSY_LINK_COMPONENT_IMU;
    if (cameraVision0Ready_ == true)
    {
        telemetry_.componentMask |= TEENSY_LINK_COMPONENT_CAMERA0;
    }

    telemetry_.imu.axG = 0.01f;
    telemetry_.imu.ayG = 0.04f;
    telemetry_.imu.azG = 1.00f;
    telemetry_.imu.gxDps = 0.5f;
    telemetry_.imu.gyDps = -1.2f;
    telemetry_.imu.gzDps = 1.8f;
    telemetry_.imu.rollDeg = 1.2f;
    telemetry_.imu.pitchDeg = -0.4f;
    telemetry_.imu.yawDeg = phase * 18.0f;
    telemetry_.imu.accelNormG = 1.003f;
    telemetry_.imu.lateralG = 0.04f;
    telemetry_.imu.tempC = 27.4f;

    if (cameraVision0Ready_ == true)
    {
        cameraVision0_.getTelemetryResult(telemetry_.camera[0], nowUs);
    }
    else
    {
        TeensyLinkTelemetry_DefaultCamera(telemetry_.camera[0],
                                          TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);
    }

    TeensyLinkTelemetry_DefaultCamera(telemetry_.camera[1], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);

    telemetry_.loggerFlags = 0U;
    telemetry_.loggerDropCount = 0U;
    sensorSeq_++;
    lastSensorMs_ = nowMs;
}

void TeensyRaceRuntime::printLinkStatus(uint32_t nowMs)
{
    Serial.print("t=");
    Serial.print(nowMs);
    Serial.print(" txSeq=");
    Serial.print(teensyFrameSeq_);
    Serial.print(" sensorSeq=");
    Serial.print(sensorSeq_);
    Serial.print(" s32k=");
    Serial.print(s32kSnapshot_.valid ? s32kSnapshot_.frameSeq : 0U);
    Serial.print(" rx=");
    Serial.print(s32kSpi_.completedTransfers());
    Serial.print(" err=");
    Serial.print(s32kSpi_.protocolErrorCount());
    Serial.print(" timeout=");
    Serial.print(s32kSpi_.timeoutCount());
    Serial.print(" app=");
    Serial.print(s32kSnapshot_.valid ? s32kSnapshot_.appMode : 0U);
    Serial.print(" speed=");
    Serial.print(s32kSnapshot_.valid ? s32kSnapshot_.targetSpeedPct : 0);
    Serial.print("/");
    Serial.print(s32kSnapshot_.valid ? s32kSnapshot_.currentSpeedPct : 0);
    Serial.print(" servo=");
    Serial.print(s32kSnapshot_.valid ? s32kSnapshot_.servoCmd : 0);
    Serial.print(" ultra=");
    Serial.print(s32kSnapshot_.valid ? s32kSnapshot_.ultrasonicDistanceCm10 : 0U);
    Serial.print(" cam0=");
    Serial.print(telemetry_.camera[0].status);
    Serial.print("/");
    Serial.print(telemetry_.camera[0].confidence);
    Serial.print(" err=");
    Serial.print(telemetry_.camera[0].errorPct);
    Serial.print(" lr=");
    Serial.print(telemetry_.camera[0].leftLineIdx);
    Serial.print("/");
    Serial.print(telemetry_.camera[0].rightLineIdx);
    Serial.print(" age=");
    Serial.print(telemetry_.camera[0].ageMs);
    Serial.print(" flg=");
    Serial.println(telemetry_.camera[0].flags);
}

void TeensyRaceRuntime::streamCameraVision0()
{
    LinearCameraFrame frame = {};
    VisionOutput vision = {};
    uint32_t sequence = 0U;
    uint32_t timestampUs = 0U;

    if (cameraVision0_.getLatestFrameAndVision(frame, vision, sequence, timestampUs) != true)
    {
        return;
    }

    (void)sequence;
    (void)timestampUs;
    CameraStream_WriteFrameVision(frame, vision, true);
}
