/*==================================================================================================
* vision_linear_v2.h
*
* Stage 1 redesign pipeline for the 1x128 linear camera:
* - receive raw ADC pixels
* - spatially filter the signal
* - compute 1D Sobel derivative
* - run simple Canny-style hysteresis edge selection
* - detect the inner track edges (left rising edge, right falling edge)
*==================================================================================================*/

#ifndef VISION_LINEAR_V2_H
#define VISION_LINEAR_V2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h"
#include "../app/main_types.h"
#include "../app/car_config.h"

/* ----------------------------- Tunables ----------------------------- */

#ifndef VISION_LINEAR_BUFFER_SIZE
#define VISION_LINEAR_BUFFER_SIZE            (CAM_EFFECTIVE_PIXELS)
#endif

#ifndef VISION_LINEAR_MIN_CONTRAST
#define VISION_LINEAR_MIN_CONTRAST           (450U)
#endif

#ifndef VISION_LINEAR_MIN_WEAK_EDGE
#define VISION_LINEAR_MIN_WEAK_EDGE          (80U)
#endif

#ifndef VISION_LINEAR_MIN_STRONG_EDGE
#define VISION_LINEAR_MIN_STRONG_EDGE        (140U)
#endif

#ifndef VISION_LINEAR_EDGE_HIGH_PCT
#define VISION_LINEAR_EDGE_HIGH_PCT          (45U)
#endif

#ifndef VISION_LINEAR_EDGE_LOW_PCT
#define VISION_LINEAR_EDGE_LOW_PCT           (55U)
#endif

#ifndef VISION_LINEAR_NOMINAL_LANE_WIDTH
#define VISION_LINEAR_NOMINAL_LANE_WIDTH     (82U)
#endif

#ifndef VISION_LINEAR_LANE_WIDTH_TOL_PCT
#define VISION_LINEAR_LANE_WIDTH_TOL_PCT     (20U)
#endif

#ifndef VISION_LINEAR_SPLIT_MARGIN_PX
#define VISION_LINEAR_SPLIT_MARGIN_PX        (10U)
#endif

#ifndef VISION_LINEAR_SINGLE_EDGE_STREAK_LIMIT
#define VISION_LINEAR_SINGLE_EDGE_STREAK_LIMIT (3U)
#endif

#ifndef VISION_LINEAR_SINGLE_EDGE_LOW_CONFIDENCE
#define VISION_LINEAR_SINGLE_EDGE_LOW_CONFIDENCE (35U)
#endif

#ifndef VISION_FINISH_WIDTH_MIN_PCT
#define VISION_FINISH_WIDTH_MIN_PCT          (50U)
#endif

#ifndef VISION_FINISH_WIDTH_MAX_PCT
#define VISION_FINISH_WIDTH_MAX_PCT          (150U)
#endif

#ifndef VISION_FINISH_INNER_WIDTH_MM
#define VISION_FINISH_INNER_WIDTH_MM         (510U)
#endif

#ifndef VISION_FINISH_BAR_WIDTH_MM
#define VISION_FINISH_BAR_WIDTH_MM           (94U)
#endif

#ifndef VISION_FINISH_CENTER_GAP_MM
#define VISION_FINISH_CENTER_GAP_MM          (74U)
#endif

#define VISION_LINEAR_INVALID_IDX            (255U)

#ifndef VLIN_MAX_EDGE_CANDIDATES
#define VLIN_MAX_EDGE_CANDIDATES             (12U)
#endif

/* ----------------------------- Output types ----------------------------- */

typedef struct
{
    uint8 idx;
    sint8 polarity; /* +1 rising, -1 falling */
    uint16 strength;
} VisionLinear_DebugEdge_t;

typedef enum
{
    VLIN_DBG_NONE      = 0u,
    VLIN_DBG_FILTERED  = 1u << 0,
    VLIN_DBG_GRADIENT  = 1u << 1,
    VLIN_DBG_STATS     = 1u << 2,
    VLIN_DBG_EDGES     = 1u << 3
} VisionLinear_DebugMask_t;

typedef struct
{
    uint32 mask;

    /* Optional caller-owned buffers [VISION_LINEAR_BUFFER_SIZE] */
    uint16 *filteredOut;
    sint16 *gradientOut;

    /* Scalars (valid if VLIN_DBG_STATS) */
    uint16 minVal;
    uint16 maxVal;
    uint16 contrast;
    uint16 maxAbsGradient;
    uint16 edgeHighThreshold;
    uint16 edgeLowThreshold;
    uint8 splitPoint;
    uint8 leftInnerEdgeIdx;
    uint8 rightInnerEdgeIdx;
    uint8 finishGapLeftEdgeIdx;
    uint8 finishGapRightEdgeIdx;
    uint8 laneWidth;
    uint8 expectedFinishWidth;
    uint8 expectedFinishGap;
    uint8 measuredFinishGap;
    uint8 finishDetected;

    /* Edge candidates (valid if VLIN_DBG_EDGES) */
    uint8 edgeCount;
    VisionLinear_DebugEdge_t edges[VLIN_MAX_EDGE_CANDIDATES];
} VisionLinear_DebugOut_t;

/* ----------------------------- API ----------------------------- */

void VisionLinear_InitV2(void);
void VisionLinear_ProcessFrame(const uint16 *pixels, VisionOutput_t *out);
void VisionLinear_ProcessFrameEx(const uint16 *pixels,
                                 VisionOutput_t *out,
                                 VisionLinear_DebugOut_t *dbg);

#ifdef __cplusplus
}
#endif

#endif /* VISION_LINEAR_V2_H */
