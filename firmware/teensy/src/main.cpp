/*
Commands to run in terminal for teensy

cd /d C:\Users\Navif\workspaceS32DS.3.6.3\NXP_Cup\firmware\teensy
pio run
pio run -t upload
pio device list
pio device monitor -p COM3 -b 115200

Must correct COM number and change directrory
*/



#include <Arduino.h>

#include "teensy_app_config.h"

#if TEENSY_APP_HARDWARE_TEST

#include "modes/mode_hardware_test.h"

void setup()
{
    HardwareTest_Setup();
}

void loop()
{
    HardwareTest_Loop();
}

#else

#include "comms/teensy_link_spi_slave.h"
#include "drivers/mpu6050_imu.h"
#include "drivers/secondary_displays.h"
#include "drivers/status_led.h"
#include "logging/teensy_sd_logger.h"
#include "teensy_config.h"
#include "telemetry/teensy_link_telemetry.h"

static TeensyLinkSpiSlave s32kSpi;
static TeensySdLogger sdLogger;
static Mpu6050Imu imu;
static TeensyLinkTelemetryInputs telemetry = {};
static TeensyLinkS32kSnapshot s32kSnapshot = {};
static uint8_t txFrame[TEENSY_LINK_FRAME_BYTES] = {};

static uint16_t teensyFrameSeq = 0U;
static uint16_t sensorSeq = 0U;
static uint32_t nextSensorUs = 0U;
static uint32_t nextSerialMs = 0U;
static uint16_t potRaw = 0U;
static bool button1Pressed = false;
static bool button2Pressed = false;
static uint32_t lastGoodFrameMs = 0U;
static uint16_t lastGoodFrameSeq = 0U;
static bool everGoodFrame = false;

static_assert(TEENSY_LINK_FRAME_BYTES == 128U, "S32K link frame must stay 128 bytes");

static void publishFrame(uint32_t nowMs)
{
    uint16_t ackS32kSeq = s32kSnapshot.valid ? s32kSnapshot.frameSeq : 0U;

    telemetry.sensorAgeMs = imu.ageMs(nowMs);
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

static void updateSensors(uint32_t nowMs, uint32_t nowUs)
{
    (void)imu.sample(nowUs);

    sensorSeq = imu.sequence();
    telemetry.sensorSeq = sensorSeq;
    telemetry.sensorDtUs = imu.sampleDtUs();
    telemetry.sensorAgeMs = imu.ageMs(nowMs);
    telemetry.statusFlags = 0U;
    telemetry.componentMask = 0U;
    telemetry.imu = {};

    if (imu.isPresent())
    {
        telemetry.componentMask |= TEENSY_LINK_COMPONENT_IMU;
        telemetry.statusFlags |= TEENSY_LINK_STATUS_IMU_PRESENT |
                                 TEENSY_LINK_STATUS_YAW_RELATIVE;
        telemetry.imu = imu.latest();
    }
    if (imu.isCalibrated())
    {
        telemetry.statusFlags |= TEENSY_LINK_STATUS_IMU_CALIBRATED;
    }
    if (imu.isValid(nowMs))
    {
        telemetry.statusFlags |= TEENSY_LINK_STATUS_IMU_VALID;
    }
    if (imu.isValid(nowMs) && imu.isAccelTrusted())
    {
        telemetry.statusFlags |= TEENSY_LINK_STATUS_ACCEL_TRUSTED;
    }

    /* No Teensy camera acquisition driver exists yet. Report both slots
       missing instead of publishing fabricated track results. */
    TeensyLinkTelemetry_DefaultCamera(telemetry.camera[0], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);
    TeensyLinkTelemetry_DefaultCamera(telemetry.camera[1], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);

    /* Real SD status goes onto the SPI link, so the S32K OLED
       IMU/LOG page shows it live. */
    telemetry.loggerFlags = sdLogger.linkLoggerFlags();
    telemetry.loggerDropCount = sdLogger.dropCount();
    if (sdLogger.isReady())
    {
        telemetry.componentMask |= TEENSY_LINK_COMPONENT_SD;
    }
}

static void updateBoardInputs()
{
    potRaw = (uint16_t)analogRead(TEENSY_POT_PIN);
    button1Pressed = digitalReadFast(TEENSY_BUTTON_1_PIN) == LOW;
    button2Pressed = digitalReadFast(TEENSY_BUTTON_2_PIN) == LOW;
}

/* PCB LED tells the link story at a glance:
   BLUE = waiting for the first S32K frame, GREEN = link OK,
   YELLOW = link OK but SD broken, RED = link was up and died.
   The boards may be powered in any order; this just keeps reporting
   whatever the link does while both sides keep retrying. */
static void updateStatusLed(uint32_t nowMs)
{
    StatusLedColor color;

    if (!everGoodFrame)
    {
        color = StatusLedColor::Blue;
    }
    else if ((uint32_t)(nowMs - lastGoodFrameMs) > TEENSY_LINK_LED_LOST_MS)
    {
        color = StatusLedColor::Red;
    }
    else if (sdLogger.hasError())
    {
        color = StatusLedColor::Yellow;
    }
    else
    {
        color = StatusLedColor::Green;
    }

    StatusLed_Set(color);
}

static SecondaryDisplayDashboard buildDisplayDashboard()
{
    SecondaryDisplayDashboard dashboard = {};

    dashboard.s32kValid = s32kSnapshot.valid;
    dashboard.s32kSequence = s32kSnapshot.valid ? s32kSnapshot.frameSeq : 0U;
    dashboard.rxFrames = s32kSpi.completedTransfers();
    dashboard.rxErrors = s32kSpi.protocolErrorCount();
    dashboard.timeouts = s32kSpi.timeoutCount();
    dashboard.appMode = s32kSnapshot.valid ? s32kSnapshot.appMode : 0U;
    dashboard.targetSpeedPct = s32kSnapshot.valid ? s32kSnapshot.targetSpeedPct : 0;
    dashboard.currentSpeedPct = s32kSnapshot.valid ? s32kSnapshot.currentSpeedPct : 0;
    dashboard.servoCmd = s32kSnapshot.valid ? s32kSnapshot.servoCmd : 0;
    dashboard.ultrasonicDistanceCm10 =
        s32kSnapshot.valid ? s32kSnapshot.ultrasonicDistanceCm10 : 0U;
    dashboard.imuPresent = imu.isPresent();
    dashboard.imuCalibrated = imu.isCalibrated();
    dashboard.imuValid = imu.isValid(millis());
    dashboard.imuYawDeg = (int16_t)telemetry.imu.yawDeg;
    dashboard.imuGzDps = (int16_t)telemetry.imu.gzDps;
    dashboard.sdReady = sdLogger.isReady();
    dashboard.sdError = sdLogger.hasError();
    dashboard.sdDrops = sdLogger.dropCount();
    dashboard.sdBytesWritten = sdLogger.bytesWritten();
    dashboard.sdFileName = sdLogger.fileName();
    dashboard.teensyTxSequence = teensyFrameSeq;
    dashboard.sensorSequence = sensorSeq;
    dashboard.potRaw = potRaw;
    dashboard.button1Pressed = button1Pressed;
    dashboard.button2Pressed = button2Pressed;
    return dashboard;
}

static void printLinkStatus(uint32_t nowMs)
{
    Serial.print("t=");
    Serial.print(nowMs);
    Serial.print(" txSeq=");
    Serial.print(teensyFrameSeq);
    Serial.print(" sensorSeq=");
    Serial.print(sensorSeq);
    Serial.print(" imu=");
    Serial.print(imu.isPresent() ? 'P' : '-');
    Serial.print(imu.isCalibrated() ? 'C' : '-');
    Serial.print(imu.isValid(nowMs) ? 'V' : '-');
    Serial.print((imu.isValid(nowMs) && imu.isAccelTrusted()) ? 'A' : '-');
    Serial.print(" imuErr=");
    Serial.print(imu.readErrorCount());
    Serial.print(" ax=");
    Serial.print(telemetry.imu.axG, 3);
    Serial.print(" ay=");
    Serial.print(telemetry.imu.ayG, 3);
    Serial.print(" az=");
    Serial.print(telemetry.imu.azG, 3);
    Serial.print(" gz=");
    Serial.print(telemetry.imu.gzDps, 2);
    Serial.print(" yaw=");
    Serial.print(telemetry.imu.yawDeg, 1);
    Serial.print(" cam0=");
    Serial.print((telemetry.camera[0].flags & TEENSY_LINK_CAMERA_FLAG_VALID) != 0U ? 'V' : '-');
    Serial.print(" cam1=");
    Serial.print((telemetry.camera[1].flags & TEENSY_LINK_CAMERA_FLAG_VALID) != 0U ? 'V' : '-');
    Serial.print(" s32k=");
    Serial.print(s32kSnapshot.valid ? s32kSnapshot.frameSeq : 0U);
    Serial.print(" rx=");
    Serial.print(s32kSpi.completedTransfers());
    Serial.print(" err=");
    Serial.print(s32kSpi.protocolErrorCount());
    Serial.print(" timeout=");
    Serial.print(s32kSpi.timeoutCount());
    Serial.print(" app=");
    Serial.print(s32kSnapshot.valid ? s32kSnapshot.appMode : 0U);
    Serial.print(" speed=");
    Serial.print(s32kSnapshot.valid ? s32kSnapshot.targetSpeedPct : 0);
    Serial.print("/");
    Serial.print(s32kSnapshot.valid ? s32kSnapshot.currentSpeedPct : 0);
    Serial.print(" servo=");
    Serial.print(s32kSnapshot.valid ? s32kSnapshot.servoCmd : 0);
    Serial.print(" ultra=");
    Serial.print(s32kSnapshot.valid ? s32kSnapshot.ultrasonicDistanceCm10 : 0U);
    Serial.print(" sd=");
    if (sdLogger.hasError())
    {
        Serial.print('E');
    }
    else
    {
        Serial.print(sdLogger.isReady() ? 'R' : '-');
    }
    Serial.print(" drop=");
    Serial.print(sdLogger.dropCount());
    Serial.print(" sdkB=");
    Serial.print(sdLogger.bytesWritten() / 1024U);
    Serial.print(" pot=");
    Serial.print(potRaw);
    Serial.print(" b1=");
    Serial.print(button1Pressed ? 1U : 0U);
    Serial.print(" b2=");
    Serial.print(button2Pressed ? 1U : 0U);

    SecondaryDisplaySnapshot displays = SecondaryDisplays_GetSnapshot();
    Serial.print(" d1=");
    Serial.print(SecondaryDisplays_StateText(displays.display1));
    Serial.print(" d2=");
    Serial.println(SecondaryDisplays_StateText(displays.display2));
}

void setup()
{
    Serial.begin(TEENSY_SERIAL_BAUD);
    while (!Serial && (millis() < 1200UL))
    {
    }

    StatusLed_Init();
    StatusLed_BootSweep();
    StatusLed_Set(StatusLedColor::Blue);

    TeensyLinkTelemetry_DefaultCamera(telemetry.camera[0], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);
    TeensyLinkTelemetry_DefaultCamera(telemetry.camera[1], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);
    analogReadResolution(TEENSY_ANALOG_READ_BITS);
    pinMode(TEENSY_POT_PIN, INPUT);
    pinMode(TEENSY_BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(TEENSY_BUTTON_2_PIN, INPUT_PULLUP);
    updateBoardInputs();
    SecondaryDisplays_Init();

    Serial.println("Keep the car still while the MPU6050 gyro calibrates.");
    if (imu.begin())
    {
        Serial.print("MPU6050 detected at 0x");
        Serial.print(imu.address(), HEX);
        Serial.println(imu.isCalibrated() ? ", calibrated" : ", calibration failed");
    }
    else
    {
        Serial.println("MPU6050 not detected; IMU flags stay invalid");
    }

    /* Mount the SD before the SPI link starts. Blocking is fine here.
       A missing card just means sd=- on serial and we keep running. */
    if (sdLogger.begin())
    {
        Serial.print("SD logger ready, file: ");
        Serial.println(sdLogger.fileName());
    }
    else
    {
        Serial.println("SD logger off (no card or open failed)");
    }

    s32kSpi.begin(TEENSY_LINK_SPI_CS_PIN,
                  TEENSY_LINK_SPI_SCK_PIN,
                  TEENSY_LINK_SPI_MOSI_PIN,
                  TEENSY_LINK_SPI_MISO_PIN,
                  TEENSY_LINK_READY_PIN);

    updateSensors(millis(), micros());
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

        /* A fresh sequence number means a frame really decoded just now
           (a failed CRC leaves the old snapshot untouched). */
        if (s32kSnapshot.valid &&
            (!everGoodFrame || (s32kSnapshot.frameSeq != lastGoodFrameSeq)))
        {
            everGoodFrame = true;
            lastGoodFrameSeq = s32kSnapshot.frameSeq;
            lastGoodFrameMs = nowMs;
        }

        publishFrame(nowMs);
    }

    if ((int32_t)(nowUs - nextSensorUs) >= 0)
    {
        updateBoardInputs();
        updateSensors(nowMs, nowUs);
        publishFrame(nowMs);

        /* One CSV row per sensor sample (100 Hz). Only RAM writes
           happen here, so the SPI slave is not delayed.
           publishFrame() already advanced teensyFrameSeq, so the frame
           that just went out carries teensyFrameSeq - 1. */
        sdLogger.logRow(nowMs,
                        (uint16_t)(teensyFrameSeq - 1U),
                        telemetry,
                        s32kSnapshot,
                        s32kSpi.completedTransfers(),
                        s32kSpi.protocolErrorCount(),
                        s32kSpi.timeoutCount());

        nextSensorUs += TEENSY_LINK_SENSOR_INTERVAL_US;
        if ((uint32_t)(nowUs - nextSensorUs) > TEENSY_LINK_SENSOR_INTERVAL_US)
        {
            nextSensorUs = nowUs + TEENSY_LINK_SENSOR_INTERVAL_US;
        }
    }

    /* Hold READY low only when the logger actually needs a card write.
       The S32K waits instead of starting SPI during the blocking write. */
    if (sdLogger.serviceDue(nowMs))
    {
        s32kSpi.setReady(false);
        if (digitalReadFast(TEENSY_LINK_SPI_CS_PIN) == HIGH)
        {
            sdLogger.service(nowMs);
        }
        s32kSpi.setReady(true);
    }

    if (SecondaryDisplays_UpdateDue(nowMs))
    {
        /* Hold READY low so the S32K cannot start a transfer while
           the blocking I2C text update is in progress. */
        s32kSpi.setReady(false);
        if (digitalReadFast(TEENSY_LINK_SPI_CS_PIN) == HIGH)
        {
            SecondaryDisplays_Service(nowMs, buildDisplayDashboard());
        }
        s32kSpi.setReady(true);
    }

    updateStatusLed(nowMs);

    if ((uint32_t)(nowMs - nextSerialMs) >= TEENSY_LINK_SERIAL_PERIOD_MS)
    {
        nextSerialMs = nowMs;
        printLinkStatus(nowMs);
    }
}

#endif /* TEENSY_APP_HARDWARE_TEST */
