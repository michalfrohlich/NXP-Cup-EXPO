/*==================================================================================================
* vision_linear_v2.h
*
* Linear camera (1x128) vision processing service.
* "Split-Tracking" Version.
*
* Purpose:
* - Detect Left and Right lane edges independently.
* - Report tracking status (Left only, Right only, Both, or Lost).
* - Provide exact line pixel positions for visualization.
* - Calculate steering error based on whatever lines are visible.
*
* Intended usage:
* - Call VisionLinear_InitV2() once at startup.
* - Each loop: VisionLinear_ProcessFrame(frame.Values, &result)
*==================================================================================================*/

#ifndef VISION_LINEAR_V2_H
#define VISION_LINEAR_V2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h"

/* ----------------------------- Tunables ----------------------------- */

#ifndef VISION_LINEAR_BUFFER_SIZE
#define VISION_LINEAR_BUFFER_SIZE            (128U)
#endif

/* Dynamic threshold = min + (contrast * frac/100) */
#ifndef VISION_LINEAR_THRESH_FRAC_PCT
#define VISION_LINEAR_THRESH_FRAC_PCT        (50U)
#endif

/* Minimum contrast to consider the frame valid (0-100) */
#ifndef VISION_LINEAR_MIN_CONTRAST
#define VISION_LINEAR_MIN_CONTRAST           (10U)
#endif

/* Ignore blobs smaller than this (noise filtering) */
#ifndef VISION_LINEAR_MIN_BLOB_WIDTH
#define VISION_LINEAR_MIN_BLOB_WIDTH         (2U)
#endif

/* Nominal lane width in pixels. Used to "guess" the center when only one line is seen.
 * Calibrate this by placing the car in the center of the track and checking (Right - Left).
 */
#ifndef VISION_LINEAR_NOMINAL_LANE_WIDTH
#define VISION_LINEAR_NOMINAL_LANE_WIDTH     (70U)
#endif

/* Sentinel value for "Line Not Found" */
#define VISION_LINEAR_INVALID_IDX            (255U)

/* ----------------------------- Output types ----------------------------- */

typedef enum
{
    VISION_LINEAR_LOST        = 0U,   /* No lines found */
    VISION_LINEAR_TRACK_BOTH  = 1U,   /* Both Left and Right lines are visible */
    VISION_LINEAR_TRACK_LEFT  = 2U,   /* Only Left line is visible */
    VISION_LINEAR_TRACK_RIGHT = 3U    /* Only Right line is visible */
} VisionLinear_StatusType;

typedef struct
{
    /* Steering Error: -1.0 (Left) .. 0.0 (Center) .. +1.0 (Right) */
    float Error;

    /* Tracking State */
    VisionLinear_StatusType Status;

    /* Quality of detection (0-100) */
    uint8 Confidence;

    /* Pixel Index of the detected lines (0-127).
     * Set to VISION_LINEAR_INVALID_IDX (255) if that specific line is not seen. */
    uint8 LeftLineIdx;
    uint8 RightLineIdx;

} VisionLinear_ResultType;

/* ----------------------------- API ----------------------------- */

void VisionLinear_InitV2(void);

/* Process one 128-pixel frame. */
void VisionLinear_ProcessFrame(const uint8 *pixels, VisionLinear_ResultType *out);

#ifdef __cplusplus
}
#endif

#endif /* VISION_LINEAR_V2_H */
