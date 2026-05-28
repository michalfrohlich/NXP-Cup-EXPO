#include <Arduino.h>

#include "comms/teensy_link_spi_slave.h"
#include "teensy_config.h"
#include "telemetry/teensy_link_telemetry.h"

static TeensyLinkSpiSlave s32kSpi;
static TeensyLinkTelemetryInputs telemetry = {};
static TeensyLinkS32kSnapshot s32kSnapshot = {};
static uint8_t txFrame[TEENSY_LINK_FRAME_BYTES] = {};

static uint16_t teensyFrameSeq = 0U;
static uint16_t sensorSeq = 0U;
static uint32_t lastSensorMs = 0U;
static uint32_t nextSensorUs = 0U;
static uint32_t nextSerialMs = 0U;

static void publishFrame(uint32_t nowMs)
{
    uint16_t ackS32kSeq = s32kSnapshot.valid ? s32kSnapshot.frameSeq : 0U;

    telemetry.sensorAgeMs = (uint16_t)(nowMs - lastSensorMs);
    TeensyLinkTelemetry_BuildTeensyFrame(txFrame,
                                         teensyFrameSeq,
                                         nowMs,
                                         ackS32kSeq,
                                         telemetry,
                                         s32kSpi.completedTransfers(),
                                         s32kSpi.protocolErrorCount());
    teensyFrameSeq++;
    s32kSpi.publishFrame(txFrame);
}

static void updateMockSensors(uint32_t nowMs)
{
    float phase = (float)(nowMs % 10000UL) * 0.001f;

    telemetry.sensorSeq = sensorSeq;
    telemetry.sensorDtUs = (uint16_t)TEENSY_LINK_SENSOR_INTERVAL_US;
    telemetry.sensorAgeMs = 0U;
    telemetry.statusFlags = TEENSY_LINK_STATUS_IMU_PRESENT |
                            TEENSY_LINK_STATUS_IMU_CALIBRATED |
                            TEENSY_LINK_STATUS_IMU_VALID |
                            TEENSY_LINK_STATUS_ACCEL_TRUSTED |
                            TEENSY_LINK_STATUS_YAW_RELATIVE;
    telemetry.componentMask = TEENSY_LINK_COMPONENT_IMU |
                              TEENSY_LINK_COMPONENT_CAMERA0 |
                              TEENSY_LINK_COMPONENT_CAMERA1;

    telemetry.imu.axG = 0.01f;
    telemetry.imu.ayG = 0.04f;
    telemetry.imu.azG = 1.00f;
    telemetry.imu.gxDps = 0.5f;
    telemetry.imu.gyDps = -1.2f;
    telemetry.imu.gzDps = 1.8f;
    telemetry.imu.rollDeg = 1.2f;
    telemetry.imu.pitchDeg = -0.4f;
    telemetry.imu.yawDeg = phase * 18.0f;
    telemetry.imu.accelNormG = 1.003f;
    telemetry.imu.lateralG = 0.04f;
    telemetry.imu.tempC = 27.4f;

    telemetry.camera[0].errorPct = -10;
    telemetry.camera[0].status = TEENSY_LINK_CAMERA_STATUS_TRACK_OK;
    telemetry.camera[0].feature = 2U;
    telemetry.camera[0].confidence = 94U;
    telemetry.camera[0].leftLineIdx = 43U;
    telemetry.camera[0].rightLineIdx = 85U;
    telemetry.camera[0].ageMs = 3U;
    telemetry.camera[0].flags = TEENSY_LINK_CAMERA_FLAG_VALID | TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY;

    telemetry.camera[1].errorPct = 6;
    telemetry.camera[1].status = TEENSY_LINK_CAMERA_STATUS_TRACK_OK;
    telemetry.camera[1].feature = 3U;
    telemetry.camera[1].confidence = 88U;
    telemetry.camera[1].leftLineIdx = 40U;
    telemetry.camera[1].rightLineIdx = 88U;
    telemetry.camera[1].ageMs = 4U;
    telemetry.camera[1].flags = TEENSY_LINK_CAMERA_FLAG_VALID | TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY;

    telemetry.loggerFlags = 0U;
    telemetry.loggerDropCount = 0U;
    sensorSeq++;
    lastSensorMs = nowMs;
}

static void printLinkStatus(uint32_t nowMs)
{
    Serial.print("t=");
    Serial.print(nowMs);
    Serial.print(" txSeq=");
    Serial.print(teensyFrameSeq);
    Serial.print(" sensorSeq=");
    Serial.print(sensorSeq);
    Serial.print(" s32k=");
    Serial.print(s32kSnapshot.valid ? s32kSnapshot.frameSeq : 0U);
    Serial.print(" rx=");
    Serial.print(s32kSpi.completedTransfers());
    Serial.print(" err=");
    Serial.print(s32kSpi.protocolErrorCount());
    Serial.print(" timeout=");
    Serial.println(s32kSpi.timeoutCount());
}

void setup()
{
    Serial.begin(TEENSY_SERIAL_BAUD);
    while (!Serial && (millis() < 1200UL))
    {
    }

    TeensyLinkTelemetry_DefaultCamera(telemetry.camera[0], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);
    TeensyLinkTelemetry_DefaultCamera(telemetry.camera[1], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);

    s32kSpi.begin(TEENSY_LINK_SPI_CS_PIN,
                  TEENSY_LINK_SPI_SCK_PIN,
                  TEENSY_LINK_SPI_MOSI_PIN,
                  TEENSY_LINK_SPI_MISO_PIN,
                  TEENSY_LINK_READY_PIN);

    updateMockSensors(millis());
    publishFrame(millis());

    nextSensorUs = micros();
    nextSerialMs = millis();

    Serial.println("Teensy S32K SPI slave v1 link");
    Serial.println("Packet: 128-byte full-duplex teensy_link frame, CRC-16/CCITT-FALSE");
}

void loop()
{
    uint32_t nowUs = micros();
    uint32_t nowMs = millis();

    if (s32kSpi.service() == true)
    {
        (void)s32kSpi.getLatestS32kFrame(s32kSnapshot);
        publishFrame(nowMs);
    }

    if ((int32_t)(nowUs - nextSensorUs) >= 0)
    {
        updateMockSensors(nowMs);
        publishFrame(nowMs);
        nextSensorUs += TEENSY_LINK_SENSOR_INTERVAL_US;
        if ((uint32_t)(nowUs - nextSensorUs) > TEENSY_LINK_SENSOR_INTERVAL_US)
        {
            nextSensorUs = nowUs + TEENSY_LINK_SENSOR_INTERVAL_US;
        }
    }

    if ((uint32_t)(nowMs - nextSerialMs) >= TEENSY_LINK_SERIAL_PERIOD_MS)
    {
        nextSerialMs = nowMs;
        printLinkStatus(nowMs);
    }
}
