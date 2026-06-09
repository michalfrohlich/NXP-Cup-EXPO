#pragma once

#include <stdint.h>

#include "config/vision_config.h"
#include "domain/vehicle_types.h"

struct LineDetectorDebugEdge
{
    uint8_t idx;
    int8_t polarity;
    uint16_t strength;
};

static constexpr uint32_t VLIN_DBG_NONE = 0U;
static constexpr uint32_t VLIN_DBG_FILTERED = 1UL << 0;
static constexpr uint32_t VLIN_DBG_GRADIENT = 1UL << 1;
static constexpr uint32_t VLIN_DBG_STATS = 1UL << 2;
static constexpr uint32_t VLIN_DBG_EDGES = 1UL << 3;

struct LineDetectorDebugOut
{
    uint32_t mask;

    uint16_t *filteredOut;
    int16_t *gradientOut;

    uint16_t minVal;
    uint16_t maxVal;
    uint16_t contrast;
    uint16_t maxAbsGradient;
    uint16_t edgeHighThreshold;
    uint16_t edgeLowThreshold;
    uint8_t splitPoint;
    uint8_t leftInnerEdgeIdx;
    uint8_t rightInnerEdgeIdx;
    uint8_t finishGapLeftEdgeIdx;
    uint8_t finishGapRightEdgeIdx;
    uint8_t laneWidth;
    uint8_t expectedFinishGap;
    uint8_t measuredFinishGap;
    uint8_t finishDetected;

    uint8_t edgeCount;
    LineDetectorDebugEdge edges[VLIN_MAX_EDGE_CANDIDATES];
};

class LineDetector
{
public:
    void init();
    void process(const uint16_t *pixels, VisionOutput &out);
    void processDebug(const uint16_t *pixels,
                      VisionOutput &out,
                      LineDetectorDebugOut &debug);

private:
    void processImpl(const uint16_t *pixels,
                     VisionOutput &out,
                     LineDetectorDebugOut *debug);

    float lastTrackCenter_ = (float)(VISION_LINEAR_BUFFER_SIZE / 2U);
    uint8_t isLocked_ = 0U;
    uint8_t singleEdgeStreak_ = 0U;
};

