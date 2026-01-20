/*==================================================================================================
*  vision_linear.c
*
*  Linear camera vision processing service (1x128).
*
*  Algorithm (MVP-focused and aligned with track spec: two black edge lines):
*    1) Spatial smoothing (cheap 1D filter) to reduce noise
*    2) Dynamic threshold (per-frame min/max) to handle lighting changes
*    3) Sobel-like edge magnitude (cheap) to compute confidence / edge strength
*    4) Blob extraction on thresholded signal to find black line candidates
*    5) Choose the most plausible LEFT blob and RIGHT blob around the image midpoint
*    6) Compute track center as average of the two line centers, normalize to Error (-1..+1)
*    7) Plausibility check vs previous center/width to reject bad frames
*
*  Notes:
*    - We do NOT try to do full 1D-Canny here; for your constraints, blob logic is the
*      most direct way given the track is always "white + two black edge lines".
*    - Sobel is still used: (a) for confidence and (b) as a robustness signal under glare.
*==================================================================================================*/

#include "vision_linear.h"

/* ----------------------------- Internal helpers ----------------------------- */

typedef struct
{
    uint8 start;
    uint8 end;
    uint8 width;
    uint8 center; /* integer center */
} VisionLinear_BlobType;

static uint8  s_prevValid = 0U;
static uint8  s_prevCenterPx = (VISION_LINEAR_BUFFER_SIZE / 2U);
static uint8  s_prevLaneWidthPx = 0U;
static float  s_prevError = 0.0f;

static uint8 VisionLinear_AbsDiffU8(uint8 a, uint8 b)
{
    return (a >= b) ? (uint8)(a - b) : (uint8)(b - a);
}

static uint8 VisionLinear_ClampU8(sint32 val, uint8 lo, uint8 hi)
{
    if (val < (sint32)lo) { return lo; }
    if (val > (sint32)hi) { return hi; }
    return (uint8)val;
}

void VisionLinear_Init(void)
{
    s_prevValid = 0U;
    s_prevCenterPx = (VISION_LINEAR_BUFFER_SIZE / 2U);
    s_prevLaneWidthPx = 0U;
    s_prevError = 0.0f;
}

static void VisionLinear_SetLost(VisionLinear_ResultType *out, uint8 confidenceOverride)
{
    out->Status = VISION_LINEAR_LOST;
    out->LaneWidthPx = 0U;

    /* For LOST, returning last known error is usually safer than forcing 0.
       Steering/controller can still decide what to do based on Status. */
    out->Error = s_prevError;

    if (confidenceOverride <= 100U)
    {
        out->Confidence = confidenceOverride;
    }
    else
    {
        out->Confidence = 0U;
    }
}

void VisionLinear_ProcessFrame(const uint8 *pixels, VisionLinear_ResultType *out)
{
    uint8 smooth[VISION_LINEAR_BUFFER_SIZE];
    uint8 edge[VISION_LINEAR_BUFFER_SIZE];
    VisionLinear_BlobType blobs[8];

    uint8 i;
    uint8 minVal = 255U;
    uint8 maxVal = 0U;

    uint8 threshold;
    uint8 contrast;

    uint8 blobCount = 0U;
    uint8 inBlob = 0U;
    uint8 blobStart = 0U;

    const uint8 midIdx = (VISION_LINEAR_BUFFER_SIZE / 2U);

    uint8 leftFound = 0U;
    uint8 rightFound = 0U;
    uint8 leftCenter = 0U;
    uint8 rightCenter = 0U;

    uint8 laneWidth;
    float trackCenterF;
    float midF;
    float error;

    uint32 edgeSum = 0U;
    uint32 edgeScore;
    uint32 contrastScore;
    uint32 confidence;

    if ((pixels == (const uint8 *)0) || (out == (VisionLinear_ResultType *)0))
    {
        return;
    }

    /* -------------------------------- 1) Spatial smoothing -------------------------------- */
    /* 1D cheap Gaussian-ish filter:
       smooth[i] = (p[i-1] + 2*p[i] + p[i+1]) / 4 */
    smooth[0] = (uint8)((((uint16)pixels[0] * 2U) + (uint16)pixels[1]) / 3U);
    for (i = 1U; i < (VISION_LINEAR_BUFFER_SIZE - 1U); i++)
    {
        uint16 acc = (uint16)pixels[i - 1U] + ((uint16)pixels[i] * 2U) + (uint16)pixels[i + 1U];
        smooth[i] = (uint8)(acc / 4U);
    }
    smooth[VISION_LINEAR_BUFFER_SIZE - 1U] =
        (uint8)((((uint16)pixels[VISION_LINEAR_BUFFER_SIZE - 1U] * 2U) +
                 (uint16)pixels[VISION_LINEAR_BUFFER_SIZE - 2U]) / 3U);

    /* -------------------------------- 2) Sobel-like edge magnitude ------------------------- */
    edge[0] = 0U;
    for (i = 1U; i < (VISION_LINEAR_BUFFER_SIZE - 1U); i++)
    {
        /* edge ~ |smooth[i+1] - smooth[i-1]| */
        edge[i] = VisionLinear_AbsDiffU8(smooth[i + 1U], smooth[i - 1U]);
    }
    edge[VISION_LINEAR_BUFFER_SIZE - 1U] = 0U;

    /* -------------------------------- 3) Dynamic threshold --------------------------------- */
    for (i = 0U; i < VISION_LINEAR_BUFFER_SIZE; i++)
    {
        if (smooth[i] < minVal) { minVal = smooth[i]; }
        if (smooth[i] > maxVal) { maxVal = smooth[i]; }
    }

    contrast = (uint8)(maxVal - minVal);
    if (contrast < VISION_LINEAR_MIN_CONTRAST)
    {
        /* Very low contrast: likely bad exposure or glare. */
        VisionLinear_SetLost(out, 0U);
        return;
    }

    /* threshold = min + contrast * frac/100 */
    threshold = (uint8)(minVal + (uint8)(((uint16)contrast * (uint16)VISION_LINEAR_THRESH_FRAC_PCT) / 100U));

    /* -------------------------------- 4) Blob extraction (black segments) ------------------ */
    for (i = 0U; i < VISION_LINEAR_BUFFER_SIZE; i++)
    {
        uint8 isBlack = (smooth[i] < threshold) ? 1U : 0U;

        if (isBlack != 0U)
        {
            if (inBlob == 0U)
            {
                inBlob = 1U;
                blobStart = i;
            }
        }
        else
        {
            if (inBlob != 0U)
            {
                uint8 blobEnd = (uint8)(i - 1U);
                uint8 width = (uint8)(blobEnd - blobStart + 1U);

                inBlob = 0U;

                if (width >= VISION_LINEAR_MIN_BLOB_WIDTH)
                {
                    if (blobCount < (uint8)(sizeof(blobs) / sizeof(blobs[0])))
                    {
                        blobs[blobCount].start = blobStart;
                        blobs[blobCount].end = blobEnd;
                        blobs[blobCount].width = width;
                        blobs[blobCount].center = (uint8)((uint16)(blobStart + blobEnd) / 2U);
                        blobCount++;
                    }
                }
            }
        }
    }

    /* Close blob if it reaches the right edge */
    if ((inBlob != 0U) && (blobCount < (uint8)(sizeof(blobs) / sizeof(blobs[0]))))
    {
        uint8 blobEnd = (uint8)(VISION_LINEAR_BUFFER_SIZE - 1U);
        uint8 width = (uint8)(blobEnd - blobStart + 1U);

        if (width >= VISION_LINEAR_MIN_BLOB_WIDTH)
        {
            blobs[blobCount].start = blobStart;
            blobs[blobCount].end = blobEnd;
            blobs[blobCount].width = width;
            blobs[blobCount].center = (uint8)((uint16)(blobStart + blobEnd) / 2U);
            blobCount++;
        }
    }

    /* -------------------------------- 5) Quick intersection heuristics --------------------- */
    for (i = 0U; i < blobCount; i++)
    {
        if (blobs[i].width >= VISION_LINEAR_INTERSECTION_BLOB_WIDTH)
        {
            out->Status = VISION_LINEAR_INTERSECTION;
            out->LaneWidthPx = 0U;
            out->Error = 0.0f;
            out->Confidence = 100U;
            return;
        }
    }

    if (blobCount >= VISION_LINEAR_INTERSECTION_BLOB_COUNT)
    {
        /* Often happens in crossings: extra dark areas appear in the scanline. */
        out->Status = VISION_LINEAR_INTERSECTION;
        out->LaneWidthPx = 0U;
        out->Error = 0.0f;
        /* confidence: still provide some notion, based mainly on contrast */
        contrastScore = ((uint32)contrast * 100U) / (uint32)VISION_LINEAR_PIXEL_MAX;
        out->Confidence = (uint8)VisionLinear_ClampU8((sint32)contrastScore, 0U, 100U);
        return;
    }

    /* -------------------------------- 6) Select LEFT and RIGHT blobs ----------------------- */
    /* Given the spec: two edge lines exist. We pick:
       - left blob: the blob on the left half with the maximum center (closest to mid)
       - right blob: the blob on the right half with the minimum center (closest to mid) */
    for (i = 0U; i < blobCount; i++)
    {
        if (blobs[i].center < midIdx)
        {
            if ((leftFound == 0U) || (blobs[i].center > leftCenter))
            {
                leftCenter = blobs[i].center;
                leftFound = 1U;
            }
        }
        else
        {
            if ((rightFound == 0U) || (blobs[i].center < rightCenter))
            {
                rightCenter = blobs[i].center;
                rightFound = 1U;
            }
        }
    }

    if ((leftFound == 0U) || (rightFound == 0U))
    {
        VisionLinear_SetLost(out, 10U);
        return;
    }

    /* Lane width (between the two line centers) */
    laneWidth = (rightCenter > leftCenter) ? (uint8)(rightCenter - leftCenter) : 0U;

    if ((laneWidth < VISION_LINEAR_MIN_LANE_WIDTH) || (laneWidth > VISION_LINEAR_MAX_LANE_WIDTH))
    {
        /* Too narrow/too wide: likely wrong pairing or crossing scenario. */
        out->Status = VISION_LINEAR_INTERSECTION;
        out->LaneWidthPx = laneWidth;
        out->Error = 0.0f;

        contrastScore = ((uint32)contrast * 100U) / (uint32)VISION_LINEAR_PIXEL_MAX;
        out->Confidence = (uint8)VisionLinear_ClampU8((sint32)contrastScore, 0U, 100U);
        return;
    }

    /* Track center between the two edge lines */
    trackCenterF = ((float)leftCenter + (float)rightCenter) * 0.5f;
    midF = ((float)VISION_LINEAR_BUFFER_SIZE - 1.0f) * 0.5f;

    error = (trackCenterF - midF) / midF; /* normalize to approx -1..+1 */

    /* -------------------------------- 7) Plausibility check vs previous -------------------- */
    if (s_prevValid != 0U)
    {
        uint8 currCenterPx = (uint8)VisionLinear_ClampU8((sint32)(trackCenterF + 0.5f), 0U, (VISION_LINEAR_BUFFER_SIZE - 1U));
        uint8 jump = VisionLinear_AbsDiffU8(currCenterPx, s_prevCenterPx);

        if (jump > VISION_LINEAR_MAX_CENTER_JUMP)
        {
            /* Reject this frame as implausible; keep last error. */
            VisionLinear_SetLost(out, 15U);
            return;
        }
    }

    /* -------------------------------- 8) Confidence (contrast + edge strength) ------------- */
    /* Contrast score: 0..100 */
    contrastScore = ((uint32)contrast * 100U) / (uint32)VISION_LINEAR_PIXEL_MAX;
    contrastScore = (uint32)VisionLinear_ClampU8((sint32)contrastScore, 0U, 100U);

    /* Edge score: sample edge strength around the selected blobs (4 edges total).
       We sample at start and end indices (and one neighbour) to avoid missing peak shift. */
    for (i = 0U; i < blobCount; i++)
    {
        if ((blobs[i].center == leftCenter) || (blobs[i].center == rightCenter))
        {
            uint8 s = blobs[i].start;
            uint8 e = blobs[i].end;

            /* start edge samples */
            if (s > 0U)
            {
                edgeSum += (uint32)edge[s];
                edgeSum += (uint32)edge[s - 1U];
            }
            else
            {
                edgeSum += (uint32)edge[s];
            }

            /* end edge samples */
            if (e < (VISION_LINEAR_BUFFER_SIZE - 1U))
            {
                edgeSum += (uint32)edge[e];
                edgeSum += (uint32)edge[e + 1U];
            }
            else
            {
                edgeSum += (uint32)edge[e];
            }
        }
    }

    /* Normalize edgeSum to 0..100. We sample up to 8 points (2 per edge x 4 edges). */
    edgeScore = (edgeSum * 100U) / (8U * (uint32)VISION_LINEAR_PIXEL_MAX);
    edgeScore = (uint32)VisionLinear_ClampU8((sint32)edgeScore, 0U, 100U);

    /* Weighted confidence: contrast dominates, edges refine */
    confidence = (contrastScore * 70U + edgeScore * 30U) / 100U;
    confidence = (uint32)VisionLinear_ClampU8((sint32)confidence, 0U, 100U);

    /* -------------------------------- 9) Output + update previous -------------------------- */
    out->Status = VISION_LINEAR_TRACKING;
    out->Error = error;
    out->Confidence = (uint8)confidence;
    out->LaneWidthPx = laneWidth;

    s_prevValid = 1U;
    s_prevCenterPx = (uint8)VisionLinear_ClampU8((sint32)(trackCenterF + 0.5f), 0U, (VISION_LINEAR_BUFFER_SIZE - 1U));
    s_prevLaneWidthPx = laneWidth;
    s_prevError = error;
}
