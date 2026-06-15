#pragma once

#include <stdint.h>

#include "teensy_config.h"

static constexpr uint8_t TEENSY_DISPLAY_I2C_ADDRESS = 0x3CU;
static constexpr uint32_t TEENSY_DISPLAY_I2C_HZ = 400000UL;

static constexpr uint8_t TEENSY_DISPLAY_WIDTH_PX = 128U;
static constexpr uint8_t TEENSY_DISPLAY_HEIGHT_PX = 64U;
static constexpr uint8_t TEENSY_DISPLAY_CHARACTER_COLUMNS = 16U;
static constexpr uint8_t TEENSY_DISPLAY_CHARACTER_ROWS =
    TEENSY_DISPLAY_HEIGHT_PX / 8U;
static constexpr uint8_t TEENSY_DISPLAY_ASYNC_CHUNK_BYTES = 8U;
static constexpr uint32_t TEENSY_DISPLAY_SERVICE_MIN_IDLE_US = 1000UL;

static constexpr bool TEENSY_DISPLAY_CAMERA_DEBUG_ENABLED = false;
static constexpr uint32_t TEENSY_DISPLAY_CAMERA_REFRESH_PERIOD_MS = 200UL;
