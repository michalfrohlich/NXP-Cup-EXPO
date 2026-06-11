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
#include "drivers/secondary_displays.h"
#include "drivers/status_led.h"
#include "logging/teensy_sd_logger.h"
#include "teensy_config.h"
#include "telemetry/teensy_link_telemetry.h"

static TeensyLinkSpiSlave s32kSpi;
static TeensySdLogger sdLogger;
static TeensyLinkTelemetryInputs telemetry = {};
static TeensyLinkS32kSnapshot s32kSnapshot = {};
static uint8_t txFrame[TEENSY_LINK_FRAME_BYTES] = {};

static uint16_t teensyFrameSeq = 0U;
static uint16_t sensorSeq = 0U;
static uint32_t lastSensorMs = 0U;
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
                              TEENSY_LINK_COMPONENT_CAMERA0;

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

    /* Camera 1 is intentionally missing for the bench fault path. */
    TeensyLinkTelemetry_DefaultCamera(telemetry.camera[1], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);

    /* Real SD status goes onto the SPI link, so the S32K OLED
       IMU/LOG page shows it live. */
    telemetry.loggerFlags = sdLogger.linkLoggerFlags();
    telemetry.loggerDropCount = sdLogger.dropCount();
    if (sdLogger.isReady())
    {
        telemetry.componentMask |= TEENSY_LINK_COMPONENT_SD;
    }
    sensorSeq++;
    lastSensorMs = nowMs;
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
        updateMockSensors(nowMs);
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
