#pragma once

#include <Arduino.h>

static constexpr uint32_t TEENSY_SERIAL_BAUD = 115200UL;

static constexpr uint8_t TEENSY_LINK_SPI_CS_PIN = 10U;
static constexpr uint8_t TEENSY_LINK_SPI_MOSI_PIN = 11U;
static constexpr uint8_t TEENSY_LINK_SPI_MISO_PIN = 12U;
static constexpr uint8_t TEENSY_LINK_SPI_SCK_PIN = 13U;
static constexpr uint8_t TEENSY_LINK_READY_PIN = 31U;

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
