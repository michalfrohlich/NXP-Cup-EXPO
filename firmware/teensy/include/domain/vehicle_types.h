#pragma once

#include <stdint.h>

enum VisionTrackStatus : uint8_t
{
    VISION_TRACK_LOST = 0U,
    VISION_TRACK_BOTH = 1U,
    VISION_TRACK_LEFT = 2U,
    VISION_TRACK_RIGHT = 3U,
};

enum VisionFeature : uint8_t
{
    VISION_FEATURE_NONE = 0U,
    VISION_FEATURE_FINISH_LINE = 1U,
};

struct VisionOutput
{
    float error;
    VisionTrackStatus status;
    VisionFeature feature;
    uint8_t confidence;
    uint8_t leftLineIdx;
    uint8_t rightLineIdx;
};

