/*==================================================================================================
* vision_linear.h (Simplified MVP)
*==================================================================================================*/
#ifndef VISION_LINEAR_H
#define VISION_LINEAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h"

/* ----------------------------- Tunables ----------------------------- */

#define VISION_BUFFER_SIZE           (128U)
#define VISION_CENTER_IDX            (64U)

/* Determine black vs white: Threshold = Min + (Range * PERCENT / 100) */
/* 50% is standard. Lower it (e.g. 40) if lines are faint. */
#define VISION_THRESH_PCT            (50U)

/* Minimum difference between Min and Max pixel to trust the frame */
#define VISION_MIN_CONTRAST          (15U)

/* ----------------------------- Output types ----------------------------- */

typedef enum
{
    VISION_STATUS_OK          = 0U, /* Found two distinct edges */
    VISION_STATUS_CROSSING    = 1U, /* Center is black or edges too close (Intersection) */
    VISION_STATUS_LOST        = 2U, /* Contrast too low or line missing */
    VISION_STATUS_OUT_OF_LANE = 3U  /* One edge missing (car drifting off) */
} VisionStatusType;

typedef struct
{
    /* The "answer" for the steering controller */
    float Error;            /* -1.0 (Left) to +1.0 (Right). 0.0 is Center. */

    /* Debug info - Check these in FreeMASTER/Debugger! */
    VisionStatusType Status;
    uint8 ThresholdVal;     /* The calculated threshold used for this frame */
    uint8 LeftEdgeIdx;      /* Index where left scan stopped */
    uint8 RightEdgeIdx;     /* Index where right scan stopped */
    uint8 LaneWidth;        /* Pixels between Left and Right */
    uint8 MaxBrightness;    /* Brightest pixel (White level) */
    uint8 MinBrightness;    /* Darkest pixel (Black level) */
} VisionResultType;

/* ----------------------------- API ----------------------------- */

void VisionLinear_Init(void);
void Vision_Process(const uint8 *pixels, VisionResultType *out);

#ifdef __cplusplus
}
#endif

#endif /* VISION_LINEAR_H */
