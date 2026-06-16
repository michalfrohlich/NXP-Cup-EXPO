#include "drivers/mpu6050_imu.h"

#include <Wire.h>
#include <math.h>

#include "teensy_config.h"

static constexpr uint8_t MPU6050_ADDRESS_LOW = 0x68U;
static constexpr uint8_t MPU6050_ADDRESS_HIGH = 0x69U;
static constexpr uint8_t MPU6050_REG_SMPLRT_DIV = 0x19U;
static constexpr uint8_t MPU6050_REG_CONFIG = 0x1AU;
static constexpr uint8_t MPU6050_REG_GYRO_CONFIG = 0x1BU;
static constexpr uint8_t MPU6050_REG_ACCEL_CONFIG = 0x1CU;
static constexpr uint8_t MPU6050_REG_INT_PIN_CFG = 0x37U;
static constexpr uint8_t MPU6050_REG_INT_ENABLE = 0x38U;
static constexpr uint8_t MPU6050_REG_ACCEL_XOUT_H = 0x3BU;
static constexpr uint8_t MPU6050_REG_PWR_MGMT_1 = 0x6BU;
static constexpr uint8_t MPU6050_REG_WHO_AM_I = 0x75U;

static constexpr float MPU6050_ACCEL_LSB_PER_G = 8192.0f;
static constexpr float MPU6050_GYRO_LSB_PER_DPS = 65.5f;
static constexpr float MPU6050_RAD_TO_DEG = 57.2957795f;
static constexpr float MPU6050_FILTER_ALPHA = 0.98f;
static constexpr float MPU6050_ACCEL_TRUST_MIN_G = 0.80f;
static constexpr float MPU6050_ACCEL_TRUST_MAX_G = 1.20f;
static constexpr uint16_t MPU6050_CALIBRATION_SAMPLES = 200U;
static constexpr float MPU6050_CALIBRATION_MAX_GYRO_DPS = 8.0f;

static int16_t readS16Be(const uint8_t *bytes)
{
    return (int16_t)(((uint16_t)bytes[0] << 8U) | bytes[1]);
}

bool Mpu6050Imu::begin()
{
    Wire.setSDA(TEENSY_IMU_SDA_PIN);
    Wire.setSCL(TEENSY_IMU_SCL_PIN);
    Wire.begin();
    Wire.setClock(TEENSY_IMU_I2C_HZ);

    present_ = probe(MPU6050_ADDRESS_LOW) || probe(MPU6050_ADDRESS_HIGH);
    if (!present_)
    {
        return false;
    }

    if (!configure())
    {
        present_ = false;
        return false;
    }

    pinMode(TEENSY_IMU_INT_PIN, INPUT);
    delay(100U);
    calibrated_ = calibrateGyro();
    return true;
}

bool Mpu6050Imu::sample(uint32_t nowUs)
{
    int16_t raw[7];
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float accelRoll;
    float accelPitch;
    float dtSeconds;
    uint32_t dtUs;

    if (!present_)
    {
        sampleValid_ = false;
        return false;
    }
    if (!readRaw(raw))
    {
        sampleValid_ = false;
        readErrorCount_++;
        return false;
    }

    ax = (float)raw[0] / MPU6050_ACCEL_LSB_PER_G;
    ay = (float)raw[1] / MPU6050_ACCEL_LSB_PER_G;
    az = (float)raw[2] / MPU6050_ACCEL_LSB_PER_G;
    gx = ((float)raw[4] / MPU6050_GYRO_LSB_PER_DPS) - gyroBiasX_;
    gy = ((float)raw[5] / MPU6050_GYRO_LSB_PER_DPS) - gyroBiasY_;
    gz = ((float)raw[6] / MPU6050_GYRO_LSB_PER_DPS) - gyroBiasZ_;

    latest_.axG = ax;
    latest_.ayG = ay;
    latest_.azG = az;
    latest_.gxDps = gx;
    latest_.gyDps = gy;
    latest_.gzDps = gz;
    latest_.accelNormG = sqrtf((ax * ax) + (ay * ay) + (az * az));
    latest_.lateralG = ay;
    latest_.tempC = ((float)raw[3] / 340.0f) + 36.53f;

    accelTrusted_ = (latest_.accelNormG >= MPU6050_ACCEL_TRUST_MIN_G) &&
                    (latest_.accelNormG <= MPU6050_ACCEL_TRUST_MAX_G);
    accelRoll = atan2f(ay, az) * MPU6050_RAD_TO_DEG;
    accelPitch = atan2f(-ax, sqrtf((ay * ay) + (az * az))) * MPU6050_RAD_TO_DEG;

    dtUs = filterStarted_ ? (uint32_t)(nowUs - lastSampleUs_) : TEENSY_LINK_TELEMETRY_INTERVAL_US;
    if ((dtUs == 0U) || (dtUs > 100000U))
    {
        dtUs = TEENSY_LINK_TELEMETRY_INTERVAL_US;
    }
    sampleDtUs_ = (dtUs > 65535U) ? 65535U : (uint16_t)dtUs;
    dtSeconds = (float)dtUs * 0.000001f;

    if (!filterStarted_)
    {
        latest_.rollDeg = accelRoll;
        latest_.pitchDeg = accelPitch;
        latest_.yawDeg = 0.0f;
        filterStarted_ = true;
    }
    else
    {
        latest_.rollDeg += gx * dtSeconds;
        latest_.pitchDeg += gy * dtSeconds;
        latest_.yawDeg = wrapDegrees(latest_.yawDeg + (gz * dtSeconds));

        if (accelTrusted_)
        {
            latest_.rollDeg = (MPU6050_FILTER_ALPHA * latest_.rollDeg) +
                              ((1.0f - MPU6050_FILTER_ALPHA) * accelRoll);
            latest_.pitchDeg = (MPU6050_FILTER_ALPHA * latest_.pitchDeg) +
                               ((1.0f - MPU6050_FILTER_ALPHA) * accelPitch);
        }
        latest_.rollDeg = wrapDegrees(latest_.rollDeg);
        latest_.pitchDeg = wrapDegrees(latest_.pitchDeg);
    }

    lastSampleUs_ = nowUs;
    lastSampleMs_ = millis();
    sequence_++;
    sampleValid_ = calibrated_;
    return sampleValid_;
}

bool Mpu6050Imu::isPresent() const
{
    return present_;
}

bool Mpu6050Imu::isCalibrated() const
{
    return calibrated_;
}

bool Mpu6050Imu::isValid(uint32_t nowMs) const
{
    return sampleValid_ && (ageMs(nowMs) <= TEENSY_IMU_STALE_MS);
}

bool Mpu6050Imu::isAccelTrusted() const
{
    return accelTrusted_;
}

uint8_t Mpu6050Imu::address() const
{
    return address_;
}

uint16_t Mpu6050Imu::sequence() const
{
    return sequence_;
}

uint16_t Mpu6050Imu::sampleDtUs() const
{
    return sampleDtUs_;
}

uint16_t Mpu6050Imu::ageMs(uint32_t nowMs) const
{
    uint32_t age = sampleValid_ ? (uint32_t)(nowMs - lastSampleMs_) : 65535U;
    return (age > 65535U) ? 65535U : (uint16_t)age;
}

uint32_t Mpu6050Imu::readErrorCount() const
{
    return readErrorCount_;
}

const Mpu6050ImuSample &Mpu6050Imu::latest() const
{
    return latest_;
}

bool Mpu6050Imu::probe(uint8_t address)
{
    uint8_t whoAmI;

    address_ = address;
    if (!readRegister(MPU6050_REG_WHO_AM_I, whoAmI))
    {
        return false;
    }

    if ((whoAmI != MPU6050_ADDRESS_LOW) && (whoAmI != MPU6050_ADDRESS_HIGH))
    {
        return false;
    }
    return true;
}

bool Mpu6050Imu::configure()
{
    if (!writeRegister(MPU6050_REG_PWR_MGMT_1, 0x80U))
    {
        return false;
    }
    delay(100U);

    return writeRegister(MPU6050_REG_PWR_MGMT_1, 0x01U) &&
           writeRegister(MPU6050_REG_SMPLRT_DIV, 9U) && writeRegister(MPU6050_REG_CONFIG, 3U) &&
           writeRegister(MPU6050_REG_GYRO_CONFIG, 0x08U) &&
           writeRegister(MPU6050_REG_ACCEL_CONFIG, 0x08U) &&
           writeRegister(MPU6050_REG_INT_PIN_CFG, 0x00U) &&
           writeRegister(MPU6050_REG_INT_ENABLE, 0x01U);
}

bool Mpu6050Imu::calibrateGyro()
{
    int16_t raw[7];
    float sumX = 0.0f;
    float sumY = 0.0f;
    float sumZ = 0.0f;

    for (uint16_t i = 0U; i < MPU6050_CALIBRATION_SAMPLES; i++)
    {
        if (!readRaw(raw))
        {
            readErrorCount_++;
            return false;
        }

        float gx = (float)raw[4] / MPU6050_GYRO_LSB_PER_DPS;
        float gy = (float)raw[5] / MPU6050_GYRO_LSB_PER_DPS;
        float gz = (float)raw[6] / MPU6050_GYRO_LSB_PER_DPS;
        float ax = (float)raw[0] / MPU6050_ACCEL_LSB_PER_G;
        float ay = (float)raw[1] / MPU6050_ACCEL_LSB_PER_G;
        float az = (float)raw[2] / MPU6050_ACCEL_LSB_PER_G;
        float accelNorm = sqrtf((ax * ax) + (ay * ay) + (az * az));
        if ((fabsf(gx) > MPU6050_CALIBRATION_MAX_GYRO_DPS) ||
            (fabsf(gy) > MPU6050_CALIBRATION_MAX_GYRO_DPS) ||
            (fabsf(gz) > MPU6050_CALIBRATION_MAX_GYRO_DPS) ||
            (accelNorm < MPU6050_ACCEL_TRUST_MIN_G) || (accelNorm > MPU6050_ACCEL_TRUST_MAX_G))
        {
            return false;
        }

        sumX += gx;
        sumY += gy;
        sumZ += gz;
        delay(10U);
    }

    gyroBiasX_ = sumX / (float)MPU6050_CALIBRATION_SAMPLES;
    gyroBiasY_ = sumY / (float)MPU6050_CALIBRATION_SAMPLES;
    gyroBiasZ_ = sumZ / (float)MPU6050_CALIBRATION_SAMPLES;
    return true;
}

bool Mpu6050Imu::readRaw(int16_t raw[7])
{
    uint8_t bytes[14];

    Wire.beginTransmission(address_);
    Wire.write(MPU6050_REG_ACCEL_XOUT_H);
    if (Wire.endTransmission(false) != 0U)
    {
        return false;
    }

    if (Wire.requestFrom(address_, (uint8_t)sizeof(bytes), (uint8_t)true) != sizeof(bytes))
    {
        return false;
    }

    for (uint8_t i = 0U; i < sizeof(bytes); i++)
    {
        bytes[i] = (uint8_t)Wire.read();
    }

    for (uint8_t i = 0U; i < 7U; i++)
    {
        raw[i] = readS16Be(&bytes[i * 2U]);
    }
    return true;
}

bool Mpu6050Imu::writeRegister(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(address_);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0U;
}

bool Mpu6050Imu::readRegister(uint8_t reg, uint8_t &value)
{
    Wire.beginTransmission(address_);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0U)
    {
        return false;
    }
    if (Wire.requestFrom(address_, (uint8_t)1U, (uint8_t)true) != 1U)
    {
        return false;
    }
    value = (uint8_t)Wire.read();
    return true;
}

float Mpu6050Imu::wrapDegrees(float degrees)
{
    while (degrees > 180.0f)
    {
        degrees -= 360.0f;
    }
    while (degrees < -180.0f)
    {
        degrees += 360.0f;
    }
    return degrees;
}
