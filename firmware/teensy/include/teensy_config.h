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
static constexpr uint32_t TEENSY_IMU_I2C_HZ = 400000UL;
static constexpr uint32_t TEENSY_IMU_SAMPLE_HZ = 100UL;
static constexpr uint32_t TEENSY_IMU_SAMPLE_INTERVAL_US = 1000000UL / TEENSY_IMU_SAMPLE_HZ;
static constexpr uint32_t TEENSY_IMU_STALE_MS = 100UL;

static constexpr uint8_t TEENSY_DISPLAY_SDA_PIN = 17U;
static constexpr uint8_t TEENSY_DISPLAY_SCL_PIN = 16U;

static constexpr uint8_t TEENSY_POT_PIN = 27U;
static constexpr uint8_t TEENSY_BUTTON_1_PIN = 28U;
static constexpr uint8_t TEENSY_BUTTON_2_PIN = 29U;
static constexpr uint8_t TEENSY_ANALOG_READ_BITS = 12U;

/* PCB RGB LED D1: common cathode, active HIGH. */
static constexpr uint8_t TEENSY_RGB_LED_R_PIN = 1U;
static constexpr uint8_t TEENSY_RGB_LED_G_PIN = 2U;
static constexpr uint8_t TEENSY_RGB_LED_B_PIN = 3U;

static constexpr uint8_t TEENSY_CAM1_SI_PIN = 4U;
static constexpr uint8_t TEENSY_CAM2_SI_PIN = 5U;
static constexpr uint8_t TEENSY_CAM1_CLK_PIN = 6U;
static constexpr uint8_t TEENSY_CAM2_CLK_PIN = 7U;
static constexpr uint8_t TEENSY_CAM1_ANALOG_PIN = 15U;
static constexpr uint8_t TEENSY_CAM2_ANALOG_PIN = 14U;

/* Matches the generated S32K LPSPI1 setup for the fixed-size link frame. */
static constexpr uint32_t TEENSY_LINK_S32K_SPI_HZ = 500000UL;
static constexpr uint32_t TEENSY_LINK_TELEMETRY_HZ = 200UL;
static constexpr uint32_t TEENSY_LINK_TELEMETRY_INTERVAL_US = 1000000UL / TEENSY_LINK_TELEMETRY_HZ;
static constexpr bool TEENSY_LINK_SERIAL_DEBUG_ENABLED = false;
static constexpr uint32_t TEENSY_LINK_SERIAL_PERIOD_MS = 100UL;
static constexpr uint32_t TEENSY_SD_LOG_TEST_PERIOD_MS = 100UL;
static constexpr uint32_t TEENSY_SD_LOG_TEST_PRINT_PERIOD_MS = 500UL;

static constexpr uint8_t TEENSY_LINK_CAMERA_STATUS_TRACK_LOST = 0U;
static constexpr uint8_t TEENSY_LINK_CAMERA_STATUS_TRACK_BOTH = 1U;
static constexpr uint8_t TEENSY_LINK_CAMERA_STATUS_TRACK_LEFT = 2U;
static constexpr uint8_t TEENSY_LINK_CAMERA_STATUS_TRACK_RIGHT = 3U;
