#pragma once

#include <Arduino.h>

static constexpr uint32_t TEENSY_SERIAL_BAUD = 115200UL;

static constexpr uint8_t TEENSY_LINK_SPI_CS_PIN = 10U;
static constexpr uint8_t TEENSY_LINK_SPI_MOSI_PIN = 11U;
static constexpr uint8_t TEENSY_LINK_SPI_MISO_PIN = 12U;
static constexpr uint8_t TEENSY_LINK_SPI_SCK_PIN = 13U;
static constexpr uint8_t TEENSY_LINK_READY_PIN = 31U;

static constexpr uint8_t TEENSY_IMU_SDA_PIN = 18U;
static constexpr uint8_t TEENSY_IMU_SCL_PIN = 19U;
static constexpr uint8_t TEENSY_IMU_INT_PIN = 30U;

static constexpr uint8_t TEENSY_DISPLAY_SDA_PIN = 17U;
static constexpr uint8_t TEENSY_DISPLAY_SCL_PIN = 16U;

static constexpr uint8_t TEENSY_POT_PIN = 27U;
static constexpr uint8_t TEENSY_BUTTON_1_PIN = 28U;
static constexpr uint8_t TEENSY_BUTTON_2_PIN = 29U;
static constexpr uint8_t TEENSY_ANALOG_READ_BITS = 12U;

/* Current ZJY_M208_25664-4P assumption: SSD1362 I2C at separate addresses. */
static constexpr uint8_t TEENSY_SECONDARY_DISPLAY_1_ADDRESS = 0x3CU;
static constexpr uint8_t TEENSY_SECONDARY_DISPLAY_2_ADDRESS = 0x3DU;
static constexpr uint32_t TEENSY_SECONDARY_DISPLAY_I2C_HZ = 400000UL;
static constexpr uint32_t TEENSY_SECONDARY_DISPLAY_UPDATE_MS = 250UL;

static constexpr uint8_t TEENSY_CAM1_SI_PIN = 4U;
static constexpr uint8_t TEENSY_CAM2_SI_PIN = 5U;
static constexpr uint8_t TEENSY_CAM1_CLK_PIN = 6U;
static constexpr uint8_t TEENSY_CAM2_CLK_PIN = 7U;
static constexpr uint8_t TEENSY_CAM1_ANALOG_PIN = 15U;
static constexpr uint8_t TEENSY_CAM2_ANALOG_PIN = 14U;

/* Matches the generated S32K LPSPI1 setup for the 128-byte bench link. */
static constexpr uint32_t TEENSY_LINK_S32K_SPI_HZ = 2000000UL;
static constexpr uint32_t TEENSY_LINK_SENSOR_HZ = 100UL;
static constexpr uint32_t TEENSY_LINK_SENSOR_INTERVAL_US = 1000000UL / TEENSY_LINK_SENSOR_HZ;
static constexpr uint32_t TEENSY_LINK_SERIAL_PERIOD_MS = 100UL;

static constexpr uint8_t TEENSY_LINK_CAMERA_STATUS_TRACK_LOST = 0U;
static constexpr uint8_t TEENSY_LINK_CAMERA_STATUS_TRACK_OK = 1U;

static constexpr uint16_t TEENSY_LINK_COMPONENT_IMU = 1U << 0;
static constexpr uint16_t TEENSY_LINK_COMPONENT_SD = 1U << 1;
static constexpr uint16_t TEENSY_LINK_COMPONENT_CAMERA0 = 1U << 2;
static constexpr uint16_t TEENSY_LINK_COMPONENT_CAMERA1 = 1U << 3;

static constexpr uint16_t TEENSY_LINK_STATUS_IMU_PRESENT = 1U << 0;
static constexpr uint16_t TEENSY_LINK_STATUS_IMU_CALIBRATED = 1U << 1;
static constexpr uint16_t TEENSY_LINK_STATUS_IMU_VALID = 1U << 2;
static constexpr uint16_t TEENSY_LINK_STATUS_ACCEL_TRUSTED = 1U << 3;
static constexpr uint16_t TEENSY_LINK_STATUS_YAW_RELATIVE = 1U << 4;
