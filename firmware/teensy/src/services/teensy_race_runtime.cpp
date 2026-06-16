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

static_assert(TEENSY_LINK_FRAME_BYTES == 84U, "S32K link frame must stay 84 bytes");

static TeensyLinkImuSample toTelemetryImuSample(const Mpu6050ImuSample &sample)
{
    TeensyLinkImuSample out = {};

    out.axG = sample.axG;
    out.ayG = sample.ayG;
    out.azG = sample.azG;
    out.gxDps = sample.gxDps;
    out.gyDps = sample.gyDps;
    out.gzDps = sample.gzDps;
    out.rollDeg = sample.rollDeg;
    out.pitchDeg = sample.pitchDeg;
    out.yawDeg = sample.yawDeg;
    out.accelNormG = sample.accelNormG;
    out.lateralG = sample.lateralG;
    out.tempC = sample.tempC;
    return out;
}

void TeensyRaceRuntime::begin(const TeensyRaceRuntimeConfig &config)
{
    config_ = config;

    if ((config_.serialStatusEnabled == true) || (config_.cameraStreamEnabled == true))
    {
        Serial.begin(config_.cameraStreamEnabled ? TEENSY_LINEAR_CAMERA_DEBUG_SERIAL_BAUD
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

    if (config_.serialStatusEnabled == true)
    {
        Serial.println("Keep the car still while the MPU6050 gyro calibrates.");
    }
    if (imu_.begin())
    {
        if (config_.serialStatusEnabled == true)
        {
            Serial.print("MPU6050 detected at 0x");
            Serial.print(imu_.address(), HEX);
            Serial.println(imu_.isCalibrated() ? ", calibrated" : ", calibration failed");
        }
    }
    else if (config_.serialStatusEnabled == true)
    {
        Serial.println("MPU6050 not detected; IMU flags stay invalid");
    }

    s32kSpi_.begin(TEENSY_LINK_SPI_CS_PIN, TEENSY_LINK_SPI_SCK_PIN, TEENSY_LINK_SPI_MOSI_PIN,
                   TEENSY_LINK_SPI_MISO_PIN, TEENSY_LINK_READY_PIN);

    nextImuUs_ = micros();
    updateSensors(micros(), millis());
    publishFrame(millis());

    nextSensorUs_ = micros();
    nextSerialMs_ = millis();
    nextCameraStreamMs_ = millis();

    if (config_.serialStatusEnabled == true)
    {
        Serial.println("Teensy S32K SPI slave v2 link");
        Serial.println("Packet: 84-byte full-duplex teensy_link v2 frame, CRC-16/CCITT-FALSE");
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

    if ((config_.cameraStreamEnabled == true) && (cameraVision0Ready_ == true) &&
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
        nextSensorUs_ += TEENSY_LINK_TELEMETRY_INTERVAL_US;
        int32_t scheduleSlipUs = (int32_t)(nowUs - nextSensorUs_);
        if (scheduleSlipUs > (int32_t)TEENSY_LINK_TELEMETRY_INTERVAL_US)
        {
            nextSensorUs_ = nowUs + TEENSY_LINK_TELEMETRY_INTERVAL_US;
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

    TeensyLinkTelemetry_BuildTeensyFrame(txFrame_, teensyFrameSeq_, nowMs, ackS32kSeq, telemetry_,
                                         s32kSpi_.completedTransfers(),
                                         s32kSpi_.protocolErrorCount());
    teensyFrameSeq_++;
    s32kSpi_.publishFrame(txFrame_);
}

void TeensyRaceRuntime::updateSensors(uint32_t nowUs, uint32_t nowMs)
{
    if ((int32_t)(nowUs - nextImuUs_) >= 0)
    {
        (void)imu_.sample(nowUs);
        do
        {
            nextImuUs_ += TEENSY_IMU_SAMPLE_INTERVAL_US;
        } while ((int32_t)(nowUs - nextImuUs_) >= 0);

        if ((uint32_t)(nowUs - nextImuUs_) > TEENSY_IMU_SAMPLE_INTERVAL_US)
        {
            nextImuUs_ = nowUs + TEENSY_IMU_SAMPLE_INTERVAL_US;
        }
    }

    telemetry_.sensorSeq = sensorSeq_;
    telemetry_.sensorDtUs = imu_.sampleDtUs();
    telemetry_.sensorAgeMs = imu_.ageMs(nowMs);
    telemetry_.statusFlags = 0U;
    telemetry_.componentMask = 0U;

    if (imu_.isPresent())
    {
        telemetry_.componentMask |= TEENSY_LINK_COMPONENT_IMU;
        telemetry_.statusFlags |= TEENSY_LINK_STATUS_IMU_PRESENT | TEENSY_LINK_STATUS_YAW_RELATIVE;
        telemetry_.imu = toTelemetryImuSample(imu_.latest());
    }
    else
    {
        telemetry_.imu = {};
    }

    if (imu_.isCalibrated())
    {
        telemetry_.statusFlags |= TEENSY_LINK_STATUS_IMU_CALIBRATED;
    }

    if (imu_.isValid(nowMs))
    {
        telemetry_.statusFlags |= TEENSY_LINK_STATUS_IMU_VALID;
    }

    if ((imu_.isValid(nowMs)) && (imu_.isAccelTrusted()))
    {
        telemetry_.statusFlags |= TEENSY_LINK_STATUS_ACCEL_TRUSTED;
    }

    if (cameraVision0Ready_ == true)
    {
        telemetry_.componentMask |= TEENSY_LINK_COMPONENT_CAMERA0;
    }

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
    Serial.print(" imu=");
    Serial.print(imu_.isPresent() ? 'P' : '-');
    Serial.print(imu_.isCalibrated() ? 'C' : '-');
    Serial.print(imu_.isValid(nowMs) ? 'V' : '-');
    Serial.print((imu_.isValid(nowMs) && imu_.isAccelTrusted()) ? 'A' : '-');
    Serial.print(" imuErr=");
    Serial.print(imu_.readErrorCount());
    Serial.print(" ax=");
    Serial.print(telemetry_.imu.axG, 3);
    Serial.print(" ay=");
    Serial.print(telemetry_.imu.ayG, 3);
    Serial.print(" az=");
    Serial.print(telemetry_.imu.azG, 3);
    Serial.print(" gz=");
    Serial.print(telemetry_.imu.gzDps, 2);
    Serial.print(" yaw=");
    Serial.print(telemetry_.imu.yawDeg, 1);
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
