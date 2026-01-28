/*==================================================================================================
* vision_linear_v2.c
*
* Split-Tracking Algorithm:
* 1. Smooth & Threshold image to find black blobs.
* 2. define a "Split Point" (usually the previous frame's center).
* 3. Search for the best black blob to the Left of the split -> Left Line.
* 4. Search for the best black blob to the Right of the split -> Right Line.
* 5. Calculate Error:
* - If Both: Average of L and R.
* - If Single: Line Position +/- (NominalWidth / 2).
*==================================================================================================*/

#include "vision_linear_v2.h"

/* ----------------------------- Internal State ----------------------------- */

/* We keep track of the last known center to help classify blobs in the next frame */
static float s_lastTrackCenter = (float)(VISION_LINEAR_BUFFER_SIZE / 2U);
static uint8 s_isLocked = 0U; /* 0 = searching from scratch, 1 = using history */

/* ----------------------------- Helpers ----------------------------- */

typedef struct
{
    uint8 start;
    uint8 end;
    uint8 width;
    uint8 center;
} VisionBlob_t;

static uint8 Vision_AbsDiff(uint8 a, uint8 b)
{
    return (a > b) ? (uint8)(a - b) : (uint8)(b - a);
}

/* ----------------------------- Internal Implementation ----------------------------- */

/* Single source of truth for both APIs:
 * - If dbg == NULL: behaves like ProcessFrame (no debug outputs).
 * - If dbg != NULL: exports only what dbg->mask requests (and only if buffers are provided).
 */
static void VisionLinear_ProcessFrameImpl(const uint8 *pixels,
                                          VisionLinear_ResultType *out,
                                          VisionLinear_DebugOut_t *dbg)
{
	/* Buffers */
	uint8 smoothLocal[VISION_LINEAR_BUFFER_SIZE];
	uint8 *smooth = smoothLocal;
	VisionBlob_t blobs[8];
	uint8 blobCount = 0U;


    /* Variables */
    uint8 i, minVal = 255U, maxVal = 0U;
    uint8 threshold, contrast;
    uint8 bestLeftIdx = VISION_LINEAR_INVALID_IDX;
    uint8 bestRightIdx = VISION_LINEAR_INVALID_IDX;

    /* 1. Spatial Smoothing (Noise Reduction) - Simple 3-tap weighted average
     * If debug wants smooth output and provides a buffer, write smoothing directly into it. */
    if ((dbg != (VisionLinear_DebugOut_t*)0) &&
        ((dbg->mask & (uint32)VLIN_DBG_SMOOTH) != 0u) &&
        (dbg->smoothOut != (uint8*)0))
    {
        smooth = dbg->smoothOut;
    }


    /* 1. Spatial Smoothing (Noise Reduction) - Simple 3-tap weighted average */
    smooth[0] = pixels[0];
    for (i = 1U; i < (VISION_LINEAR_BUFFER_SIZE - 1U); i++)
    {
        uint16 val = (uint16)pixels[i-1] + ((uint16)pixels[i] * 2U) + (uint16)pixels[i+1];
        smooth[i] = (uint8)(val / 4U);
    }
    smooth[VISION_LINEAR_BUFFER_SIZE - 1U] = pixels[VISION_LINEAR_BUFFER_SIZE - 1U];

    /* Optional: output smooth[] only when requested */
    if ((dbg != (VisionLinear_DebugOut_t*)0) &&
        ((dbg->mask & (uint32)VLIN_DBG_SMOOTH) != 0u) &&
        (dbg->smoothOut != (uint8*)0))
    {
        for (i = 0U; i < VISION_LINEAR_BUFFER_SIZE; i++)
        {
            dbg->smoothOut[i] = smooth[i];
        }
    }

    /* 2. Dynamic Threshold Calculation */
    for (i = 0U; i < VISION_LINEAR_BUFFER_SIZE; i++)
    {
        if (smooth[i] < minVal) { minVal = smooth[i]; }
        if (smooth[i] > maxVal) { maxVal = smooth[i]; }
    }

    contrast = (uint8)(maxVal - minVal);

    /* If contrast is too low, the image is garbage */
    if (contrast < VISION_LINEAR_MIN_CONTRAST)
    {
        out->Status = VISION_LINEAR_LOST;
        out->Confidence = 0U;
        out->LeftLineIdx = VISION_LINEAR_INVALID_IDX;
        out->RightLineIdx = VISION_LINEAR_INVALID_IDX;
        /* Keep previous error to avoid steering snap */
        s_isLocked = 0U;

        /* Debug scalars are still useful here */
        if (dbg != (VisionLinear_DebugOut_t*)0)
        {
            if ((dbg->mask & (uint32)VLIN_DBG_STATS) != 0u)
            {
                dbg->minVal       = minVal;
                dbg->maxVal       = maxVal;
                dbg->contrast     = contrast;
                dbg->threshold    = minVal; // threshold not meaningful when contrast fails, but keep deterministic
                dbg->splitPoint   = 0U;
                dbg->bestLeftIdx  = VISION_LINEAR_INVALID_IDX;
                dbg->bestRightIdx = VISION_LINEAR_INVALID_IDX;
            }

            if ((dbg->mask & (uint32)VLIN_DBG_BLOBS) != 0u)
            {
                dbg->blobCount = 0U;
            }
        }
        return;
    }

    /* Threshold formula */
    threshold = minVal + (uint8)(((uint16)contrast * VISION_LINEAR_THRESH_FRAC_PCT) / 100U);

    /* 3. Blob Extraction */
    {
        uint8 inBlob = 0U;
        uint8 start = 0U;

        for (i = 0U; i < VISION_LINEAR_BUFFER_SIZE; i++)
        {
            uint8 isBlack = (smooth[i] < threshold) ? 1U : 0U;

            if (isBlack)
            {
                if (!inBlob) { inBlob = 1U; start = i; }
            }
            else
            {
                if (inBlob)
                {
                    /* Blob ended at i-1 */
                    inBlob = 0U;
                    uint8 width = (i - start);
                    if ((width >= VISION_LINEAR_MIN_BLOB_WIDTH) && (blobCount < 8U))
                    {
                        blobs[blobCount].start = start;
                        blobs[blobCount].end = (uint8)(i - 1U);
                        blobs[blobCount].width = width;
                        blobs[blobCount].center = (start + (i - 1U)) / 2U;
                        blobCount++;
                    }
                }
            }
        }

        /* Check blob at very end of sensor */
        if (inBlob && (blobCount < 8U))
        {
            uint8 width = (VISION_LINEAR_BUFFER_SIZE - start);
            if (width >= VISION_LINEAR_MIN_BLOB_WIDTH)
            {
                blobs[blobCount].start = start;
                blobs[blobCount].end = (uint8)(VISION_LINEAR_BUFFER_SIZE - 1U);
                blobs[blobCount].width = width;
                blobs[blobCount].center = (start + (VISION_LINEAR_BUFFER_SIZE - 1U)) / 2U;
                blobCount++;
            }
        }
    }

    /* 4. Classify Blobs (Left vs Right) using Split Logic */
    /* If we are "Lost", split screen in middle (64).
       If we are "Locked", split screen at last known center. */
    {
        uint8 splitPoint = s_isLocked ? (uint8)s_lastTrackCenter : (VISION_LINEAR_BUFFER_SIZE / 2U);

        /* Clamp split point to avoid edge cases */
        if (splitPoint < 10U) splitPoint = 10U;
        if (splitPoint > 118U) splitPoint = 118U;

        uint8 minDistLeft = 255U;
        uint8 minDistRight = 255U;

        for (i = 0U; i < blobCount; i++)
        {
            uint8 c = blobs[i].center;

            if (c <= splitPoint)
            {
                /* Candidate for LEFT line: Pick the one closest to the split point
                 * (innermost line) if multiple exist */
                uint8 dist = splitPoint - c;
                if (dist < minDistLeft)
                {
                    minDistLeft = dist;
                    bestLeftIdx = c;
                }
            }
            else
            {
                /* Candidate for RIGHT line */
                uint8 dist = c - splitPoint;
                if (dist < minDistRight)
                {
                    minDistRight = dist;
                    bestRightIdx = c;
                }
            }
        }

        /* === Debug outputs (cheap): scalars and blob segments === */
        if (dbg != (VisionLinear_DebugOut_t*)0)
        {
            if ((dbg->mask & (uint32)VLIN_DBG_STATS) != 0u)
            {
                dbg->minVal       = minVal;
                dbg->maxVal       = maxVal;
                dbg->contrast     = contrast;
                dbg->threshold    = threshold;
                dbg->splitPoint   = splitPoint;
                dbg->bestLeftIdx  = bestLeftIdx;
                dbg->bestRightIdx = bestRightIdx;
            }

            if ((dbg->mask & (uint32)VLIN_DBG_BLOBS) != 0u)
            {
                uint8 n = (blobCount > VLIN_MAX_BLOBS) ? (uint8)VLIN_MAX_BLOBS : blobCount;
                dbg->blobCount = n;
                for (i = 0U; i < n; i++)
                {
                    dbg->blobs[i].start  = blobs[i].start;
                    dbg->blobs[i].end    = blobs[i].end;
                    dbg->blobs[i].width  = blobs[i].width;
                    dbg->blobs[i].center = blobs[i].center;
                }
            }
        }
    }

    /* 5. Determine Status & Error */
    {
        float midF = (float)(VISION_LINEAR_BUFFER_SIZE - 1U) / 2.0f;
        float currentTrackCenter = midF; /* Default to middle */

        out->LeftLineIdx = bestLeftIdx;
        out->RightLineIdx = bestRightIdx;

        /* Case A: Both Lines Found */
        if ((bestLeftIdx != VISION_LINEAR_INVALID_IDX) && (bestRightIdx != VISION_LINEAR_INVALID_IDX))
        {
            out->Status = VISION_LINEAR_TRACK_BOTH;
            currentTrackCenter = ((float)bestLeftIdx + (float)bestRightIdx) / 2.0f;
            out->Confidence = 100U;
        }
        /* Case B: Only Left Found */
        else if (bestLeftIdx != VISION_LINEAR_INVALID_IDX)
        {
            out->Status = VISION_LINEAR_TRACK_LEFT;
            /* Guess Right line position based on nominal width */
            float simulatedRight = (float)bestLeftIdx + (float)VISION_LINEAR_NOMINAL_LANE_WIDTH;
            currentTrackCenter = ((float)bestLeftIdx + simulatedRight) / 2.0f;
            out->Confidence = 70U; /* Lower confidence on single line */
        }
        /* Case C: Only Right Found */
        else if (bestRightIdx != VISION_LINEAR_INVALID_IDX)
        {
            out->Status = VISION_LINEAR_TRACK_RIGHT;
            /* Guess Left line position */
            float simulatedLeft = (float)bestRightIdx - (float)VISION_LINEAR_NOMINAL_LANE_WIDTH;
            currentTrackCenter = (simulatedLeft + (float)bestRightIdx) / 2.0f;
            out->Confidence = 70U;
        }
        /* Case D: Lost */
        else
        {
            out->Status = VISION_LINEAR_LOST;
            out->Confidence = 0U;
            /* Don't update s_lastTrackCenter, stick to history */
            return;
        }

        /* 6. Calculate Normalized Error (-1.0 to 1.0) */
        /* Error = (TrackCenter - ImageCenter) / ImageCenter */
        out->Error = (currentTrackCenter - midF) / midF;

        /* Clamp Error */
        if (out->Error < -1.0f) out->Error = -1.0f;
        if (out->Error > 1.0f)  out->Error = 1.0f;

        /* Update History */
        s_lastTrackCenter = currentTrackCenter;
        s_isLocked = 1U;
    }
}

/* ----------------------------- Public API ----------------------------- */

void VisionLinear_InitV2(void)
{
    s_lastTrackCenter = (float)(VISION_LINEAR_BUFFER_SIZE / 2U);
    s_isLocked = 0U;
}

void VisionLinear_ProcessFrame(const uint8 *pixels, VisionLinear_ResultType *out)
{
    VisionLinear_ProcessFrameImpl(pixels, out, (VisionLinear_DebugOut_t*)0);
}

void VisionLinear_ProcessFrameEx(const uint8 *pixels,
                                 VisionLinear_ResultType *out,
                                 VisionLinear_DebugOut_t *dbg)
{
    VisionLinear_ProcessFrameImpl(pixels, out, dbg);
}
