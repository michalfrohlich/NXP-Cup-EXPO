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
#include "domain/main_types.h"
#include "config/camera_config.h"
#include "config/vision_config.h"

#define VISION_LINEAR_INVALID_IDX            (255U)

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
