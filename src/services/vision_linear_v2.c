/*==================================================================================================
* vision_linear_v2.c
*
* Stage 1 redesign:
* raw -> filtered -> 1D Sobel derivative -> hysteresis edges -> inner edge selection
*==================================================================================================*/

#include "vision_linear_v2.h"

/* ----------------------------- Internal State ----------------------------- */

static float s_lastTrackCenter = (float)(VISION_LINEAR_BUFFER_SIZE / 2U);
static uint8 s_isLocked = 0U;
static uint8 s_singleEdgeStreak = 0U;

/* ----------------------------- Helpers ----------------------------- */

static uint16 VisionLinear_AbsU16(sint16 value)
{
    return (value < 0) ? (uint16)(-value) : (uint16)value;
}

static uint16 VisionLinear_AbsDiffU8(uint8 a, uint8 b)
{
    return (a >= b) ? (uint16)(a - b) : (uint16)(b - a);
}

static uint8 VisionLinear_ClampSplitPoint(float center)
{
    uint8 splitPoint = (uint8)center;
    uint8 minSplit = VISION_LINEAR_SPLIT_MARGIN_PX;
    uint8 maxSplit = (uint8)(VISION_LINEAR_BUFFER_SIZE - 1U - VISION_LINEAR_SPLIT_MARGIN_PX);

    if (splitPoint < minSplit)
    {
        splitPoint = minSplit;
    }
    if (splitPoint > maxSplit)
    {
        splitPoint = maxSplit;
    }

    return splitPoint;
}

static void VisionLinear_FilterSignal(const uint16 *pixels, uint16 *filtered)
{
    uint16 i;

    filtered[0] = pixels[0];
    filtered[1] = (uint16)(((uint32)pixels[0] + ((uint32)pixels[1] * 2U) + (uint32)pixels[2]) / 4U);

    for (i = 2U; i < (VISION_LINEAR_BUFFER_SIZE - 2U); i++)
    {
        uint32 value =
            (uint32)pixels[i - 2U] +
            ((uint32)pixels[i - 1U] * 4U) +
            ((uint32)pixels[i] * 6U) +
            ((uint32)pixels[i + 1U] * 4U) +
            (uint32)pixels[i + 2U];

        filtered[i] = (uint16)(value / 16U);
    }

    filtered[VISION_LINEAR_BUFFER_SIZE - 2U] =
        (uint16)(((uint32)pixels[VISION_LINEAR_BUFFER_SIZE - 3U] +
                  ((uint32)pixels[VISION_LINEAR_BUFFER_SIZE - 2U] * 2U) +
                  (uint32)pixels[VISION_LINEAR_BUFFER_SIZE - 1U]) / 4U);
    filtered[VISION_LINEAR_BUFFER_SIZE - 1U] = pixels[VISION_LINEAR_BUFFER_SIZE - 1U];
}

static void VisionLinear_ComputeGradient(const uint16 *filtered,
                                         sint16 *gradient,
                                         uint16 *maxAbsGradient)
{
    uint16 i;
    uint16 maxGradient = 0U;

    gradient[0] = 0;
    gradient[1] = 0;

    for (i = 2U; i < (VISION_LINEAR_BUFFER_SIZE - 2U); i++)
    {
        sint32 value =
            -((sint32)filtered[i - 2U]) -
            ((sint32)filtered[i - 1U] * 2) +
            ((sint32)filtered[i + 1U] * 2) +
            (sint32)filtered[i + 2U];

        if (value > 32767)
        {
            value = 32767;
        }
        else if (value < -32768)
        {
            value = -32768;
        }

        gradient[i] = (sint16)value;

        {
            uint16 magnitude = VisionLinear_AbsU16(gradient[i]);
            if (magnitude > maxGradient)
            {
                maxGradient = magnitude;
            }
        }
    }

    gradient[VISION_LINEAR_BUFFER_SIZE - 2U] = 0;
    gradient[VISION_LINEAR_BUFFER_SIZE - 1U] = 0;
    *maxAbsGradient = maxGradient;
}

static uint8 VisionLinear_IsLocalMaximum(const sint16 *gradient, uint16 idx)
{
    uint16 current = VisionLinear_AbsU16(gradient[idx]);
    uint16 left = VisionLinear_AbsU16(gradient[idx - 1U]);
    uint16 right = VisionLinear_AbsU16(gradient[idx + 1U]);

    return ((current >= left) && (current > right)) ? 1U : 0U;
}

static uint8 VisionLinear_WidthMatchesFinish(uint8 width, uint8 expectedWidth)
{
    uint16 minWidth;
    uint16 maxWidth;

    if (expectedWidth == 0U)
    {
        return 0U;
    }

    minWidth = ((uint16)expectedWidth * (uint16)VISION_FINISH_WIDTH_MIN_PCT) / 100U;
    if (minWidth < 1U)
    {
        minWidth = 1U;
    }

    maxWidth = ((uint16)expectedWidth * (uint16)VISION_FINISH_WIDTH_MAX_PCT) / 100U;
    if (maxWidth < (uint16)expectedWidth)
    {
        maxWidth = expectedWidth;
    }

    return (((uint16)width >= minWidth) && ((uint16)width <= maxWidth)) ? 1U : 0U;
}

static uint8 VisionLinear_WidthMatchesLane(uint8 width)
{
    uint16 minWidth;
    uint16 maxWidth;

    minWidth = ((uint16)VISION_LINEAR_NOMINAL_LANE_WIDTH *
                (uint16)(100U - VISION_LINEAR_LANE_WIDTH_TOL_PCT)) / 100U;
    maxWidth = ((uint16)VISION_LINEAR_NOMINAL_LANE_WIDTH *
                (uint16)(100U + VISION_LINEAR_LANE_WIDTH_TOL_PCT)) / 100U;

    return (((uint16)width >= minWidth) && ((uint16)width <= maxWidth)) ? 1U : 0U;
}

/* ----------------------------- Internal Implementation ----------------------------- */

static void VisionLinear_ProcessFrameImpl(const uint16 *pixels,
                                          VisionOutput_t *out,
                                          VisionLinear_DebugOut_t *dbg)
{
    uint16 filteredLocal[VISION_LINEAR_BUFFER_SIZE];
    sint16 gradientLocal[VISION_LINEAR_BUFFER_SIZE];
    uint16 *filtered = filteredLocal;
    sint16 *gradient = gradientLocal;
    uint16 i;
    uint16 minVal = 0xFFFFU;
    uint16 maxVal = 0U;
    uint16 contrast;
    uint16 maxAbsGradient = 0U;
    uint16 edgeHighThreshold;
    uint16 edgeLowThreshold;
    uint8 splitPoint;
    uint8 bestLeftIdx = VISION_LINEAR_INVALID_IDX;
    uint8 bestRightIdx = VISION_LINEAR_INVALID_IDX;
    uint16 bestLeftDist = 255U;
    uint16 bestRightDist = 255U;
    uint8 edgeCandidateIdx[VLIN_MAX_EDGE_CANDIDATES];
    sint8 edgeCandidatePolarity[VLIN_MAX_EDGE_CANDIDATES];
    uint8 edgeCandidateCount = 0U;
    uint8 leftCandidates[VLIN_MAX_EDGE_CANDIDATES];
    uint16 leftCandidateStrengths[VLIN_MAX_EDGE_CANDIDATES];
    uint8 rightCandidates[VLIN_MAX_EDGE_CANDIDATES];
    uint16 rightCandidateStrengths[VLIN_MAX_EDGE_CANDIDATES];
    uint8 leftCandidateCount = 0U;
    uint8 rightCandidateCount = 0U;
    uint8 laneWidth = 0U;
    uint8 expectedFinishGap = 0U;
    uint8 measuredFinishGap = 0U;
    uint8 finishGapLeftEdgeIdx = VISION_LINEAR_INVALID_IDX;
    uint8 finishGapRightEdgeIdx = VISION_LINEAR_INVALID_IDX;
    uint8 finishDetected = 0U;

    if ((dbg != (VisionLinear_DebugOut_t*)0) &&
        ((dbg->mask & (uint32)VLIN_DBG_FILTERED) != 0u) &&
        (dbg->filteredOut != (uint16*)0))
    {
        filtered = dbg->filteredOut;
    }

    if ((dbg != (VisionLinear_DebugOut_t*)0) &&
        ((dbg->mask & (uint32)VLIN_DBG_GRADIENT) != 0u) &&
        (dbg->gradientOut != (sint16*)0))
    {
        gradient = dbg->gradientOut;
    }

    VisionLinear_FilterSignal(pixels, filtered);

    for (i = 0U; i < VISION_LINEAR_BUFFER_SIZE; i++)
    {
        if (filtered[i] < minVal) { minVal = filtered[i]; }
        if (filtered[i] > maxVal) { maxVal = filtered[i]; }
    }

    contrast = (uint16)(maxVal - minVal);
    VisionLinear_ComputeGradient(filtered, gradient, &maxAbsGradient);

    edgeHighThreshold = (uint16)(((uint32)maxAbsGradient * VISION_LINEAR_EDGE_HIGH_PCT) / 100U);
    if (edgeHighThreshold < VISION_LINEAR_MIN_STRONG_EDGE)
    {
        edgeHighThreshold = VISION_LINEAR_MIN_STRONG_EDGE;
    }

    edgeLowThreshold = (uint16)(((uint32)edgeHighThreshold * VISION_LINEAR_EDGE_LOW_PCT) / 100U);
    if (edgeLowThreshold < VISION_LINEAR_MIN_WEAK_EDGE)
    {
        edgeLowThreshold = VISION_LINEAR_MIN_WEAK_EDGE;
    }

    splitPoint = s_isLocked ? VisionLinear_ClampSplitPoint(s_lastTrackCenter)
                            : (VISION_LINEAR_BUFFER_SIZE / 2U);

    if (dbg != (VisionLinear_DebugOut_t*)0)
    {
        if ((dbg->mask & (uint32)VLIN_DBG_STATS) != 0u)
        {
            dbg->minVal = minVal;
            dbg->maxVal = maxVal;
            dbg->contrast = contrast;
            dbg->maxAbsGradient = maxAbsGradient;
            dbg->edgeHighThreshold = edgeHighThreshold;
            dbg->edgeLowThreshold = edgeLowThreshold;
            dbg->splitPoint = splitPoint;
            dbg->leftInnerEdgeIdx = VISION_LINEAR_INVALID_IDX;
            dbg->rightInnerEdgeIdx = VISION_LINEAR_INVALID_IDX;
            dbg->finishGapLeftEdgeIdx = VISION_LINEAR_INVALID_IDX;
            dbg->finishGapRightEdgeIdx = VISION_LINEAR_INVALID_IDX;
            dbg->laneWidth = 0U;
            dbg->expectedFinishGap = 0U;
            dbg->measuredFinishGap = 0U;
            dbg->finishDetected = 0U;
        }

        if ((dbg->mask & (uint32)VLIN_DBG_EDGES) != 0u)
        {
            dbg->edgeCount = 0U;
        }
    }

    if ((contrast < VISION_LINEAR_MIN_CONTRAST) || (maxAbsGradient < VISION_LINEAR_MIN_WEAK_EDGE))
    {
        out->error = 0.0f;
        out->status = VISION_TRACK_LOST;
        out->feature = VISION_FEATURE_NONE;
        out->confidence = 0U;
        out->leftLineIdx = VISION_LINEAR_INVALID_IDX;
        out->rightLineIdx = VISION_LINEAR_INVALID_IDX;
        s_isLocked = 0U;
        s_singleEdgeStreak = 0U;
        return;
    }

    for (i = 2U; i < (VISION_LINEAR_BUFFER_SIZE - 2U); i++)
    {
        uint16 strength = VisionLinear_AbsU16(gradient[i]);
        uint8 isCandidate = 0U;

        if ((strength >= edgeLowThreshold) && (VisionLinear_IsLocalMaximum(gradient, i) != 0U))
        {
            if (strength >= edgeHighThreshold)
            {
                isCandidate = 1U;
            }
            else
            {
                uint16 leftStrength = VisionLinear_AbsU16(gradient[i - 1U]);
                uint16 rightStrength = VisionLinear_AbsU16(gradient[i + 1U]);

                if ((leftStrength >= edgeHighThreshold) || (rightStrength >= edgeHighThreshold))
                {
                    isCandidate = 1U;
                }
            }
        }

        if (isCandidate != 0U)
        {
            sint8 polarity = (gradient[i] >= 0) ? 1 : -1;

            if ((dbg != (VisionLinear_DebugOut_t*)0) &&
                ((dbg->mask & (uint32)VLIN_DBG_EDGES) != 0u) &&
                (dbg->edgeCount < VLIN_MAX_EDGE_CANDIDATES))
            {
                dbg->edges[dbg->edgeCount].idx = (uint8)i;
                dbg->edges[dbg->edgeCount].polarity = polarity;
                dbg->edges[dbg->edgeCount].strength = strength;
                dbg->edgeCount++;
            }

            if (edgeCandidateCount < VLIN_MAX_EDGE_CANDIDATES)
            {
                edgeCandidateIdx[edgeCandidateCount] = (uint8)i;
                edgeCandidatePolarity[edgeCandidateCount] = polarity;
                edgeCandidateCount++;
            }

            if ((polarity > 0) && ((uint8)i <= splitPoint))
            {
                uint16 distance = (uint16)(splitPoint - (uint8)i);

                if (leftCandidateCount < VLIN_MAX_EDGE_CANDIDATES)
                {
                    leftCandidates[leftCandidateCount] = (uint8)i;
                    leftCandidateStrengths[leftCandidateCount] = strength;
                    leftCandidateCount++;
                }

                if (distance < bestLeftDist)
                {
                    bestLeftDist = distance;
                    bestLeftIdx = (uint8)i;
                }
            }
            else if ((polarity < 0) && ((uint8)i > splitPoint))
            {
                uint16 distance = (uint16)((uint8)i - splitPoint);

                if (rightCandidateCount < VLIN_MAX_EDGE_CANDIDATES)
                {
                    rightCandidates[rightCandidateCount] = (uint8)i;
                    rightCandidateStrengths[rightCandidateCount] = strength;
                    rightCandidateCount++;
                }

                if (distance < bestRightDist)
                {
                    bestRightDist = distance;
                    bestRightIdx = (uint8)i;
                }
            }
        }
    }

    if ((leftCandidateCount > 0U) && (rightCandidateCount > 0U))
    {
        uint8 li;
        uint8 ri;
        sint32 bestPairScore = 2147483647L;
        uint8 pairLeft = bestLeftIdx;
        uint8 pairRight = bestRightIdx;

        for (li = 0U; li < leftCandidateCount; li++)
        {
            for (ri = 0U; ri < rightCandidateCount; ri++)
            {
                uint8 leftIdx = leftCandidates[li];
                uint8 rightIdx = rightCandidates[ri];

                if (rightIdx > leftIdx)
                {
                    uint8 width = (uint8)(rightIdx - leftIdx);
                    uint8 center = (uint8)((leftIdx + rightIdx) / 2U);
                    if (VisionLinear_WidthMatchesLane(width) != 0U)
                    {
                        uint16 widthErr = VisionLinear_AbsDiffU8(width, (uint8)VISION_LINEAR_NOMINAL_LANE_WIDTH);
                        uint16 centerErr = VisionLinear_AbsDiffU8(center, splitPoint);
                        uint16 strengthSum = (uint16)(leftCandidateStrengths[li] + rightCandidateStrengths[ri]);
                        sint32 score = (sint32)((uint32)widthErr * 8UL) +
                                       (sint32)centerErr -
                                       (sint32)(strengthSum / 16U);

                        if (score < bestPairScore)
                        {
                            bestPairScore = score;
                            pairLeft = leftIdx;
                            pairRight = rightIdx;
                        }
                    }
                }
            }
        }

        if (bestPairScore != 2147483647L)
        {
            bestLeftIdx = pairLeft;
            bestRightIdx = pairRight;
        }
    }

    out->leftLineIdx = bestLeftIdx;
    out->rightLineIdx = bestRightIdx;

    if ((dbg != (VisionLinear_DebugOut_t*)0) && ((dbg->mask & (uint32)VLIN_DBG_STATS) != 0u))
    {
        dbg->leftInnerEdgeIdx = bestLeftIdx;
        dbg->rightInnerEdgeIdx = bestRightIdx;
    }

    {
        float midF = (float)(VISION_LINEAR_BUFFER_SIZE - 1U) / 2.0f;
        float trackCenter = midF;

        if ((bestLeftIdx != VISION_LINEAR_INVALID_IDX) && (bestRightIdx != VISION_LINEAR_INVALID_IDX))
        {
            out->status = VISION_TRACK_BOTH;
            out->confidence = 100U;
            trackCenter = ((float)bestLeftIdx + (float)bestRightIdx) / 2.0f;
            s_singleEdgeStreak = 0U;
        }
        else if (bestLeftIdx != VISION_LINEAR_INVALID_IDX)
        {
            float simulatedRight = (float)bestLeftIdx + (float)VISION_LINEAR_NOMINAL_LANE_WIDTH;
            out->status = VISION_TRACK_LEFT;
            out->confidence = 60U;
            trackCenter = ((float)bestLeftIdx + simulatedRight) / 2.0f;
            if (s_singleEdgeStreak < 255U)
            {
                s_singleEdgeStreak++;
            }
            if (s_singleEdgeStreak > VISION_LINEAR_SINGLE_EDGE_STREAK_LIMIT)
            {
                out->confidence = VISION_LINEAR_SINGLE_EDGE_LOW_CONFIDENCE;
            }
        }
        else if (bestRightIdx != VISION_LINEAR_INVALID_IDX)
        {
            float simulatedLeft = (float)bestRightIdx - (float)VISION_LINEAR_NOMINAL_LANE_WIDTH;
            out->status = VISION_TRACK_RIGHT;
            out->confidence = 60U;
            trackCenter = (simulatedLeft + (float)bestRightIdx) / 2.0f;
            if (s_singleEdgeStreak < 255U)
            {
                s_singleEdgeStreak++;
            }
            if (s_singleEdgeStreak > VISION_LINEAR_SINGLE_EDGE_STREAK_LIMIT)
            {
                out->confidence = VISION_LINEAR_SINGLE_EDGE_LOW_CONFIDENCE;
            }
        }
        else
        {
            out->error = 0.0f;
            out->status = VISION_TRACK_LOST;
            out->feature = VISION_FEATURE_NONE;
            out->confidence = 0U;
            s_isLocked = 0U;
            s_singleEdgeStreak = 0U;
            return;
        }

        out->feature = VISION_FEATURE_NONE;

        if ((bestLeftIdx != VISION_LINEAR_INVALID_IDX) &&
            (bestRightIdx != VISION_LINEAR_INVALID_IDX) &&
            (bestRightIdx > bestLeftIdx))
        {
            uint8 e;
            uint16 bestGapError = 0xFFFFU;

            laneWidth = (uint8)(bestRightIdx - bestLeftIdx);
            expectedFinishGap = (uint8)(((uint16)laneWidth * (uint16)VISION_FINISH_CENTER_GAP_MM) /
                                        (uint16)VISION_FINISH_INNER_WIDTH_MM);
            if (expectedFinishGap < 1U)
            {
                expectedFinishGap = 1U;
            }

            for (e = 0U; e + 1U < edgeCandidateCount; e++)
            {
                uint8 leftGapEdge = edgeCandidateIdx[e];
                uint8 rightGapEdge = edgeCandidateIdx[e + 1U];
                sint8 leftGapPolarity = edgeCandidatePolarity[e];
                sint8 rightGapPolarity = edgeCandidatePolarity[e + 1U];

                if ((leftGapPolarity > 0) &&
                    (rightGapPolarity < 0) &&
                    (leftGapEdge > bestLeftIdx) &&
                    (rightGapEdge < bestRightIdx) &&
                    (rightGapEdge > leftGapEdge))
                {
                    uint8 gap = (uint8)(rightGapEdge - leftGapEdge);
                    uint8 gapCenter = (uint8)((leftGapEdge + rightGapEdge) / 2U);
                    uint8 laneCenter = (uint8)((bestLeftIdx + bestRightIdx) / 2U);
                    uint16 gapError = VisionLinear_AbsDiffU8(gap, expectedFinishGap);
                    uint16 centerError = VisionLinear_AbsDiffU8(gapCenter, laneCenter);
                    uint16 maxCenterError = ((uint16)laneWidth * (uint16)VISION_FINISH_CENTER_TOL_PCT) / 100U;

                    if ((gap > 0U) &&
                        (centerError <= maxCenterError) &&
                        (VisionLinear_WidthMatchesFinish(gap, expectedFinishGap) != 0U) &&
                        (gapError < bestGapError))
                    {
                        bestGapError = gapError;
                        measuredFinishGap = gap;
                        finishGapLeftEdgeIdx = leftGapEdge;
                        finishGapRightEdgeIdx = rightGapEdge;
                        finishDetected = 1U;
                        out->feature = VISION_FEATURE_FINISH_LINE;
                    }
                }
            }

        }

        out->error = (trackCenter - midF) / midF;
        if (out->error < -1.0f) { out->error = -1.0f; }
        if (out->error > 1.0f)  { out->error = 1.0f; }

        s_lastTrackCenter = trackCenter;
        s_isLocked = 1U;
    }

    if ((dbg != (VisionLinear_DebugOut_t*)0) && ((dbg->mask & (uint32)VLIN_DBG_STATS) != 0u))
    {
        dbg->laneWidth = laneWidth;
        dbg->expectedFinishGap = expectedFinishGap;
        dbg->measuredFinishGap = measuredFinishGap;
        dbg->finishGapLeftEdgeIdx = finishGapLeftEdgeIdx;
        dbg->finishGapRightEdgeIdx = finishGapRightEdgeIdx;
        dbg->finishDetected = finishDetected;
    }

}

/* ----------------------------- Public API ----------------------------- */

void VisionLinear_InitV2(void)
{
    s_lastTrackCenter = (float)(VISION_LINEAR_BUFFER_SIZE / 2U);
    s_isLocked = 0U;
    s_singleEdgeStreak = 0U;
}

void VisionLinear_ProcessFrame(const uint16 *pixels, VisionOutput_t *out)
{
    VisionLinear_ProcessFrameImpl(pixels, out, (VisionLinear_DebugOut_t*)0);
}

void VisionLinear_ProcessFrameEx(const uint16 *pixels,
                                 VisionOutput_t *out,
                                 VisionLinear_DebugOut_t *dbg)
{
    VisionLinear_ProcessFrameImpl(pixels, out, dbg);
}
