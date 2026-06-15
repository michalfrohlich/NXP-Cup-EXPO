#pragma once

#include <stdint.h>

#include "config/camera_config.h"

static constexpr uint16_t CAM_N_PIXELS = TEENSY_LINEAR_CAMERA_PIXEL_COUNT;
static constexpr uint16_t CAM_TRIM_LEFT_PX = 2U;
static constexpr uint16_t CAM_TRIM_RIGHT_PX = 2U;
static constexpr uint16_t CAM_EFFECTIVE_PIXELS =
    CAM_N_PIXELS - CAM_TRIM_LEFT_PX - CAM_TRIM_RIGHT_PX;

static constexpr uint16_t VISION_LINEAR_BUFFER_SIZE = CAM_EFFECTIVE_PIXELS;
static constexpr uint8_t VISION_LINEAR_INVALID_IDX = 255U;

static constexpr uint16_t VISION_LINEAR_CONF_CONTRAST_THRESHOLD = 450U;
static constexpr uint16_t VISION_LINEAR_CONF_EDGE_LOW = 70U;
static constexpr uint16_t VISION_LINEAR_CONF_EDGE_HIGH = 220U;
static constexpr uint16_t VISION_LINEAR_CONF_EDGE_WEIGHT_PCT = 80U;
static constexpr uint16_t VISION_LINEAR_CONF_CONTRAST_WEIGHT_PCT = 20U;

static constexpr uint8_t VLIN_MAX_EDGE_CANDIDATES = 12U;

static constexpr uint16_t VISION_LINEAR_MIN_CONTRAST = 650U;
static constexpr uint16_t VISION_LINEAR_MIN_WEAK_EDGE = 32U;
static constexpr uint16_t VISION_LINEAR_MIN_STRONG_EDGE = 40U;
static constexpr uint16_t VISION_LINEAR_EDGE_HIGH_PCT = 40U;
static constexpr uint16_t VISION_LINEAR_EDGE_LOW_PCT = 55U;

static constexpr uint8_t VISION_LINEAR_NOMINAL_LANE_WIDTH = 90U;
static constexpr uint8_t VISION_LINEAR_LANE_WIDTH_TOL_PCT = 20U;

static constexpr uint8_t VISION_LINEAR_SINGLE_EDGE_STREAK_LIMIT = 3U;
static constexpr uint8_t VISION_LINEAR_SINGLE_EDGE_LOW_CONFIDENCE = 35U;

static constexpr uint8_t VISION_LINEAR_SPLIT_MARGIN_PX = 10U;

static constexpr uint16_t VISION_FINISH_INNER_WIDTH_MM = 530U;
static constexpr uint16_t VISION_FINISH_CENTER_GAP_MM = 74U;
static constexpr uint8_t VISION_FINISH_WIDTH_MIN_PCT = 70U;
static constexpr uint8_t VISION_FINISH_WIDTH_MAX_PCT = 130U;
static constexpr uint8_t VISION_FINISH_CENTER_TOL_PCT = 15U;
