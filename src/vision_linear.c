/*==================================================================================================
* vision_linear.c (Simplified MVP)
* Strategy: Center-Out Scan
*==================================================================================================*/

#include "vision_linear.h"

/* No static history variables needed for MVP vision! */

void VisionLinear_Init(void)
{
    /* Nothing to init in stateless logic */
}

void Vision_Process(const uint8 *pixels, VisionResultType *out)
{
    uint8 i;
    uint8 minVal = 255U;
    uint8 maxVal = 0U;
    uint8 range;
    uint8 threshold;
    uint8 leftIdx = 0U;   /* Default to far left */
    uint8 rightIdx = 127U; /* Default to far right */
    uint8 centerPixel;

    /* 1. Statistics & Smoothing (Basic Min/Max search) */
    /* Note: We skip the separate smoothing pass for speed.
       If noise is bad, re-add the [1,2,1] filter here. */
    for (i = 0U; i < VISION_BUFFER_SIZE; i++)
    {
        uint8 val = pixels[i];
        if (val < minVal) { minVal = val; }
        if (val > maxVal) { maxVal = val; }
    }

    out->MinBrightness = minVal;
    out->MaxBrightness = maxVal;
    range = maxVal - minVal;

    /* 2. Check Validity (Contrast) */
    if (range < VISION_MIN_CONTRAST)
    {
        out->Status = VISION_STATUS_LOST;
        out->Error = 0.0f; /* Keep straight if blind, or use last known value externally */
        out->LaneWidth = 0U;
        return;
    }

    /* 3. Calculate Dynamic Threshold */
    threshold = minVal + (uint8)((range * VISION_THRESH_PCT) / 100U);
    out->ThresholdVal = threshold;

    /* 4. Check Center Pixel (Are we on a crossing?) */
    centerPixel = pixels[VISION_CENTER_IDX];

    if (centerPixel < threshold)
    {
        /* The center of the camera sees BLACK.
           We are either completely off track or on a perpendicular crossing line. */
        out->Status = VISION_STATUS_CROSSING;
        out->Error = 0.0f;
        out->LeftEdgeIdx = VISION_CENTER_IDX;
        out->RightEdgeIdx = VISION_CENTER_IDX;
        out->LaneWidth = 0U;
        return;
    }

    /* 5. Scan LEFT from Center */
    /* Look for transition from White (High) to Black (Low) */
    for (i = VISION_CENTER_IDX; i > 0U; i--)
    {
        if (pixels[i] < threshold)
        {
            leftIdx = i;
            break;
        }
    }

    /* 6. Scan RIGHT from Center */
    for (i = VISION_CENTER_IDX; i < (VISION_BUFFER_SIZE - 1U); i++)
    {
        if (pixels[i] < threshold)
        {
            rightIdx = i;
            break;
        }
    }

    out->LeftEdgeIdx = leftIdx;
    out->RightEdgeIdx = rightIdx;
    out->LaneWidth = rightIdx - leftIdx;

    /* 7. Calculate Error */
    /* Ideal center is (Right + Left) / 2.
       We want that to align with VISION_CENTER_IDX (64). */
    float actualCenter = (float)leftIdx + ((float)(rightIdx - leftIdx) / 2.0f);
    float errorPixels = actualCenter - (float)VISION_CENTER_IDX;

    /* Normalize to -1.0 .. +1.0 roughly.
       64 pixels error = full turn. */
    out->Error = errorPixels / (float)VISION_CENTER_IDX;

    /* 8. Determine Status based on scan results */
    if ((leftIdx == 0U) && (rightIdx == 127U))
    {
        /* Saw NO lines. Track is super wide or we are lost in whitespace. */
        out->Status = VISION_STATUS_LOST;
    }
    else if ((leftIdx == 0U) || (rightIdx == 127U))
    {
        /* Saw only one line. We are drifting out of lane. */
        out->Status = VISION_STATUS_OUT_OF_LANE;
    }
    else
    {
        /* Saw both lines. Good tracking. */
        out->Status = VISION_STATUS_OK;
    }
}
