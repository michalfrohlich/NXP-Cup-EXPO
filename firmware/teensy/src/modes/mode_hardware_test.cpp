#include "modes/mode_hardware_test.h"

#include <Arduino.h>

#include "drivers/mpu6050_imu.h"
#include "drivers/secondary_displays.h"
#include "drivers/status_led.h"
#include "logging/teensy_sd_logger.h"
#include "teensy_config.h"
#include "telemetry/teensy_link_telemetry.h"

/* Bench self-test for the Teensy side of the PCB. No SPI slave runs here:
   READY is held LOW so a connected S32K skips transfers instead of
   clocking a slave that is not listening.

   What it exercises:
   - RGB LED: continuous color cycle; button 1 forces WHITE, button 2 OFF.
   - Both ZJY displays: live mirrored dashboard with pot/button values.
   - Pot + buttons: shown on serial and on the displays.
   - MPU6050: physical I2C probe, calibration, and live motion values.
   - SD card: one test row per second through the normal logger.
   - SPI pins: CS/SCK/MOSI input levels printed, to debug PCB wiring. */

static TeensySdLogger sdLogger;
static Mpu6050Imu imu;
static TeensyLinkTelemetryInputs zeroTelemetry = {};
static TeensyLinkS32kSnapshot zeroSnapshot = {};

static uint32_t nextLedMs = 0U;
static uint32_t nextPrintMs = 0U;
static uint32_t nextSdRowMs = 0U;
static uint32_t nextImuUs = 0U;
static uint8_t ledStep = 0U;
static uint16_t testSeq = 0U;

void HardwareTest_Setup()
{
    Serial.begin(TEENSY_SERIAL_BAUD);
    while (!Serial && (millis() < 1200UL))
    {
    }

    StatusLed_Init();
    StatusLed_BootSweep();

    analogReadResolution(TEENSY_ANALOG_READ_BITS);
    pinMode(TEENSY_POT_PIN, INPUT);
    pinMode(TEENSY_BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(TEENSY_BUTTON_2_PIN, INPUT_PULLUP);

    /* Watch the S32K SPI lines without acting as a slave. */
    pinMode(TEENSY_LINK_SPI_CS_PIN, INPUT_PULLUP);
    pinMode(TEENSY_LINK_SPI_SCK_PIN, INPUT);
    pinMode(TEENSY_LINK_SPI_MOSI_PIN, INPUT);
    pinMode(TEENSY_LINK_READY_PIN, OUTPUT);
    digitalWriteFast(TEENSY_LINK_READY_PIN, LOW);

    SecondaryDisplays_Init();

    Serial.println("HWTEST keep the car still while the MPU6050 gyro calibrates.");
    if (imu.begin())
    {
        Serial.print("HWTEST MPU6050 detected at 0x");
        Serial.print(imu.address(), HEX);
        Serial.println(imu.isCalibrated() ? ", calibrated" : ", calibration failed");
    }
    else
    {
        Serial.println("HWTEST MPU6050 not detected");
    }

    if (sdLogger.begin())
    {
        Serial.print("HWTEST SD ready, file: ");
        Serial.println(sdLogger.fileName());
    }
    else
    {
        Serial.println("HWTEST SD off (no card or open failed)");
    }

    Serial.println("HWTEST Teensy hardware self-test (no SPI slave, READY held LOW)");
}

void HardwareTest_Loop()
{
    uint32_t nowMs = millis();
    uint32_t nowUs = micros();
    uint16_t potRaw = (uint16_t)analogRead(TEENSY_POT_PIN);
    bool b1 = digitalReadFast(TEENSY_BUTTON_1_PIN) == LOW;
    bool b2 = digitalReadFast(TEENSY_BUTTON_2_PIN) == LOW;

    if ((int32_t)(nowUs - nextImuUs) >= 0)
    {
        (void)imu.sample(nowUs);
        zeroTelemetry.sensorSeq = imu.sequence();
        zeroTelemetry.sensorDtUs = imu.sampleDtUs();
        zeroTelemetry.sensorAgeMs = imu.ageMs(nowMs);
        zeroTelemetry.imu = imu.isPresent() ? imu.latest() : TeensyLinkImuSample{};
        zeroTelemetry.statusFlags = 0U;
        zeroTelemetry.componentMask = 0U;

        if (imu.isPresent())
        {
            zeroTelemetry.componentMask |= TEENSY_LINK_COMPONENT_IMU;
            zeroTelemetry.statusFlags |= TEENSY_LINK_STATUS_IMU_PRESENT |
                                         TEENSY_LINK_STATUS_YAW_RELATIVE;
        }
        if (imu.isCalibrated())
        {
            zeroTelemetry.statusFlags |= TEENSY_LINK_STATUS_IMU_CALIBRATED;
        }
        if (imu.isValid(nowMs))
        {
            zeroTelemetry.statusFlags |= TEENSY_LINK_STATUS_IMU_VALID;
        }
        if (imu.isValid(nowMs) && imu.isAccelTrusted())
        {
            zeroTelemetry.statusFlags |= TEENSY_LINK_STATUS_ACCEL_TRUSTED;
        }

        nextImuUs += TEENSY_LINK_SENSOR_INTERVAL_US;
        if ((uint32_t)(nowUs - nextImuUs) > TEENSY_LINK_SENSOR_INTERVAL_US)
        {
            nextImuUs = nowUs + TEENSY_LINK_SENSOR_INTERVAL_US;
        }
    }

    /* LED: slow color cycle proves every channel; buttons override it so
       a press gives instant visible feedback. */
    if (b1)
    {
        StatusLed_Set(StatusLedColor::White);
    }
    else if (b2)
    {
        StatusLed_Set(StatusLedColor::Off);
    }
    else
    {
        static const StatusLedColor cycle[6] = {
            StatusLedColor::Red, StatusLedColor::Green, StatusLedColor::Blue,
            StatusLedColor::Yellow, StatusLedColor::Cyan, StatusLedColor::Magenta};

        if ((int32_t)(nowMs - nextLedMs) >= 0)
        {
            nextLedMs = nowMs + 1000UL;
            ledStep = (uint8_t)((ledStep + 1U) % 6U);
        }
        StatusLed_Set(cycle[ledStep]);
    }

    /* One SD row per second through the real logger path. */
    if ((int32_t)(nowMs - nextSdRowMs) >= 0)
    {
        nextSdRowMs = nowMs + 1000UL;
        testSeq++;
        sdLogger.logRow(nowMs, testSeq, zeroTelemetry, zeroSnapshot, 0U, 0U, 0U);
    }
    sdLogger.service(nowMs);

    /* Both displays show the same live dashboard. */
    if (SecondaryDisplays_UpdateDue(nowMs))
    {
        SecondaryDisplayDashboard dashboard = {};
        dashboard.sdReady = sdLogger.isReady();
        dashboard.sdError = sdLogger.hasError();
        dashboard.imuPresent = imu.isPresent();
        dashboard.imuCalibrated = imu.isCalibrated();
        dashboard.imuValid = imu.isValid(nowMs);
        dashboard.imuYawDeg = (int16_t)zeroTelemetry.imu.yawDeg;
        dashboard.imuGzDps = (int16_t)zeroTelemetry.imu.gzDps;
        dashboard.sdDrops = sdLogger.dropCount();
        dashboard.sdBytesWritten = sdLogger.bytesWritten();
        dashboard.sdFileName = sdLogger.fileName();
        dashboard.teensyTxSequence = testSeq;
        dashboard.sensorSequence = testSeq;
        dashboard.potRaw = potRaw;
        dashboard.button1Pressed = b1;
        dashboard.button2Pressed = b2;
        SecondaryDisplays_Service(nowMs, dashboard);
    }

    if ((int32_t)(nowMs - nextPrintMs) >= 0)
    {
        nextPrintMs = nowMs + 200UL;

        SecondaryDisplaySnapshot displays = SecondaryDisplays_GetSnapshot();
        Serial.print("HWTEST pot=");
        Serial.print(potRaw);
        Serial.print(" b1=");
        Serial.print(b1 ? 1U : 0U);
        Serial.print(" b2=");
        Serial.print(b2 ? 1U : 0U);
        Serial.print(" imu=");
        Serial.print(imu.isPresent() ? 'P' : '-');
        Serial.print(imu.isCalibrated() ? 'C' : '-');
        Serial.print(imu.isValid(nowMs) ? 'V' : '-');
        Serial.print((imu.isValid(nowMs) && imu.isAccelTrusted()) ? 'A' : '-');
        Serial.print(" imuErr=");
        Serial.print(imu.readErrorCount());
        Serial.print(" ax=");
        Serial.print(zeroTelemetry.imu.axG, 3);
        Serial.print(" ay=");
        Serial.print(zeroTelemetry.imu.ayG, 3);
        Serial.print(" az=");
        Serial.print(zeroTelemetry.imu.azG, 3);
        Serial.print(" gz=");
        Serial.print(zeroTelemetry.imu.gzDps, 2);
        Serial.print(" yaw=");
        Serial.print(zeroTelemetry.imu.yawDeg, 1);
        Serial.print(" sd=");
        Serial.print(sdLogger.hasError() ? 'E' : (sdLogger.isReady() ? 'R' : '-'));
        Serial.print(" sdkB=");
        Serial.print(sdLogger.bytesWritten() / 1024U);
        Serial.print(" cs=");
        Serial.print(digitalReadFast(TEENSY_LINK_SPI_CS_PIN));
        Serial.print(" sck=");
        Serial.print(digitalReadFast(TEENSY_LINK_SPI_SCK_PIN));
        Serial.print(" mosi=");
        Serial.print(digitalReadFast(TEENSY_LINK_SPI_MOSI_PIN));
        Serial.print(" d1=");
        Serial.print(SecondaryDisplays_StateText(displays.display1));
        Serial.print(" d2=");
        Serial.println(SecondaryDisplays_StateText(displays.display2));
    }
}
