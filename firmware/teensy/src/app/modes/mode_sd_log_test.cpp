#include "app/teensy_app_modes.h"

#include "app/teensy_app_config.h"

#if TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_SD_LOG_TEST

#include <Arduino.h>

#include "drivers/board_inputs.h"
#include "drivers/status_led.h"
#include "logging/teensy_sd_logger.h"
#include "teensy_config.h"
#include "telemetry/teensy_link_telemetry.h"

static TeensySdLogger sdLogger;
static TeensyLinkTelemetryInputs telemetry = {};
static TeensyLinkS32kSnapshot s32kSnapshot = {};
static uint32_t nextLogMs = 0U;
static uint32_t nextPrintMs = 0U;
static uint16_t testSeq = 0U;

static void configureSpiReadyLow()
{
    pinMode(TEENSY_LINK_SPI_CS_PIN, INPUT_PULLUP);
    pinMode(TEENSY_LINK_SPI_SCK_PIN, INPUT);
    pinMode(TEENSY_LINK_SPI_MOSI_PIN, INPUT);
    pinMode(TEENSY_LINK_READY_PIN, OUTPUT);
    digitalWriteFast(TEENSY_LINK_READY_PIN, LOW);
}

static void updateLed()
{
    if (sdLogger.hasError())
    {
        StatusLed_Set(StatusLedColor::Red);
    }
    else if (sdLogger.isReady())
    {
        StatusLed_Set(StatusLedColor::Green);
    }
    else
    {
        StatusLed_Set(StatusLedColor::Yellow);
    }
}

static void fillTestTelemetry(uint32_t nowMs, const BoardInputsSnapshot &inputs)
{
    telemetry.sensorSeq = testSeq;
    telemetry.sensorDtUs = (uint16_t)(TEENSY_SD_LOG_TEST_PERIOD_MS * 1000UL);
    telemetry.sensorAgeMs = 0U;
    telemetry.statusFlags = 0U;
    telemetry.componentMask = sdLogger.isReady() ? TEENSY_LINK_COMPONENT_SD : 0U;
    telemetry.imu.axG = (float)inputs.potRaw / (float)((1UL << TEENSY_ANALOG_READ_BITS) - 1UL);
    telemetry.imu.ayG = inputs.button1Pressed ? 1.0f : 0.0f;
    telemetry.imu.azG = inputs.button2Pressed ? 1.0f : 0.0f;
    telemetry.imu.gxDps = 0.0f;
    telemetry.imu.gyDps = 0.0f;
    telemetry.imu.gzDps = 0.0f;
    telemetry.imu.rollDeg = 0.0f;
    telemetry.imu.pitchDeg = 0.0f;
    telemetry.imu.yawDeg = 0.0f;
    telemetry.imu.accelNormG = telemetry.imu.axG;
    telemetry.imu.lateralG = 0.0f;
    telemetry.imu.tempC = 0.0f;
    telemetry.loggerFlags = sdLogger.linkLoggerFlags();
    telemetry.loggerDropCount = sdLogger.dropCount();

    (void)nowMs;
}

static void printStatus(uint32_t nowMs, const BoardInputsSnapshot &inputs)
{
    if ((int32_t)(nowMs - nextPrintMs) < 0)
    {
        return;
    }

    nextPrintMs = nowMs + TEENSY_SD_LOG_TEST_PRINT_PERIOD_MS;
    Serial.print("SDTEST ready=");
    Serial.print(sdLogger.isReady() ? 1U : 0U);
    Serial.print(" err=");
    Serial.print(sdLogger.hasError() ? 1U : 0U);
    Serial.print(" drop=");
    Serial.print(sdLogger.dropCount());
    Serial.print(" file=");
    Serial.print(sdLogger.fileName()[0] != '\0' ? sdLogger.fileName() : "-");
    Serial.print(" bytes=");
    Serial.print(sdLogger.bytesWritten());
    Serial.print(" seq=");
    Serial.print(testSeq);
    Serial.print(" pot=");
    Serial.print(inputs.potRaw);
    Serial.print(" b1=");
    Serial.print(inputs.button1Pressed ? 1U : 0U);
    Serial.print(" b2=");
    Serial.println(inputs.button2Pressed ? 1U : 0U);
}

void ModeSdLogTest_Setup()
{
    Serial.begin(TEENSY_SERIAL_BAUD);
    while (!Serial && (millis() < 1200UL))
    {
    }

    BoardInputs_Init();
    StatusLed_Init();
    StatusLed_BootSweep();
    configureSpiReadyLow();
    TeensyLinkTelemetry_DefaultCamera(telemetry.camera[0], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);
    TeensyLinkTelemetry_DefaultCamera(telemetry.camera[1], TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);

    Serial.println("SDTEST Teensy SD logger self-test");
    Serial.println("SDTEST READY held LOW; S32K SPI transfers are disabled in this mode");
    if (sdLogger.begin())
    {
        Serial.print("SDTEST ready, file=");
        Serial.println(sdLogger.fileName());
    }
    else
    {
        Serial.println("SDTEST off; no card or open failed");
    }

    updateLed();
    nextLogMs = millis();
    nextPrintMs = millis();
}

void ModeSdLogTest_Loop()
{
    uint32_t nowMs = millis();
    BoardInputsSnapshot inputs = BoardInputs_Read();

    if ((int32_t)(nowMs - nextLogMs) >= 0)
    {
        testSeq++;
        fillTestTelemetry(nowMs, inputs);
        sdLogger.logRow(nowMs, testSeq, telemetry, s32kSnapshot, 0U, 0U, 0U);
        do
        {
            nextLogMs += TEENSY_SD_LOG_TEST_PERIOD_MS;
        } while ((int32_t)(nowMs - nextLogMs) >= 0);
    }

    sdLogger.service(nowMs);
    updateLed();
    printStatus(nowMs, inputs);
}

#endif
