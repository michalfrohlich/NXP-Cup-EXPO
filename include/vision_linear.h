/*==================================================================================================
*  vision_linear.h
*
*  Linear camera (1x128) vision processing service.
*
*  Assumptions (per NXP Cup track spec):
*  - The camera always delivers 128 samples.
*  - The track has two black edge lines on a white surface.
*  - Intersections can occur (45deg / 90deg), which may create extra black blobs.
*
*  Purpose:
*    - Provide a stable, simple interface for steering: a normalized lateral error (-1..+1)
*      representing the track center between the two edge lines.
*    - Provide a minimal status and a confidence metric for gating control decisions.
*
*  Intended usage:
*    - Call VisionLinear_Init() once at startup.
*    - Each loop: VisionLinear_ProcessFrame(frame.Values, &result)
*    - Steering uses result.Error.
*==================================================================================================*/

#ifndef VISION_LINEAR_H
#define VISION_LINEAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h"

/* ----------------------------- Tunables (override if needed) ----------------------------- */

#ifndef VISION_LINEAR_BUFFER_SIZE
#define VISION_LINEAR_BUFFER_SIZE            (128U)
#endif

/* Pixel range expected from your current linear_camera.c scaling */
#ifndef VISION_LINEAR_PIXEL_MAX
#define VISION_LINEAR_PIXEL_MAX              (100U)
#endif

/* Dynamic threshold = min + (contrast * frac/100) */
#ifndef VISION_LINEAR_THRESH_FRAC_PCT
#define VISION_LINEAR_THRESH_FRAC_PCT        (50U)  /* midpoint */
#endif

/* If contrast below this -> unreliable frame */
#ifndef VISION_LINEAR_MIN_CONTRAST
#define VISION_LINEAR_MIN_CONTRAST           (12U)  /* in 0..100 domain */
#endif

/* Ignore black blobs smaller than this (noise) */
#ifndef VISION_LINEAR_MIN_BLOB_WIDTH
#define VISION_LINEAR_MIN_BLOB_WIDTH         (2U)
#endif

/* If a black blob wider than this appears -> likely intersection/crossing */
#ifndef VISION_LINEAR_INTERSECTION_BLOB_WIDTH
#define VISION_LINEAR_INTERSECTION_BLOB_WIDTH (60U)
#endif

/* Lane width plausibility window (distance between the two detected line centers, in pixels) */
#ifndef VISION_LINEAR_MIN_LANE_WIDTH
#define VISION_LINEAR_MIN_LANE_WIDTH         (12U)
#endif

#ifndef VISION_LINEAR_MAX_LANE_WIDTH
#define VISION_LINEAR_MAX_LANE_WIDTH         (120U)
#endif

/* Max allowed track-center jump per frame (pixels). Above this -> reject as implausible. */
#ifndef VISION_LINEAR_MAX_CENTER_JUMP
#define VISION_LINEAR_MAX_CENTER_JUMP        (20U)
#endif

/* If we detect >= this many blobs, treat as intersection-ish environment */
#ifndef VISION_LINEAR_INTERSECTION_BLOB_COUNT
#define VISION_LINEAR_INTERSECTION_BLOB_COUNT (3U)
#endif

/* ----------------------------- Output types ----------------------------- */

typedef enum
{
    VISION_LINEAR_LOST = 0U,          /* Could not reliably detect the two edge lines */
    VISION_LINEAR_TRACKING = 1U,      /* Two edge lines detected; Error is valid */
    VISION_LINEAR_INTERSECTION = 2U   /* Likely crossing/intersection; Error should be handled carefully */
} VisionLinear_StatusType;

/* Minimal, steering-friendly output */
typedef struct
{
    float Error;                  /* -1.0 (track center left) .. +1.0 (track center right), 0.0 is centered */
    VisionLinear_StatusType Status;
    uint8 Confidence;             /* 0..100 (based on contrast + edge strength) */
    uint8 LaneWidthPx;            /* distance between left/right detected line centers (pixels), 0 if not valid */
} VisionLinear_ResultType;

/* ----------------------------- API ----------------------------- */

void VisionLinear_Init(void);

/* Process one 128-pixel frame. 'pixels' must point to 128 uint8 samples (0..100). */
void VisionLinear_ProcessFrame(const uint8 *pixels, VisionLinear_ResultType *out);

#ifdef __cplusplus
}
#endif

#endif /* VISION_LINEAR_H */
