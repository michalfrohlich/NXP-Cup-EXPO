/*==================================================================================================
* line_detector.c
*
* Stage 1 redesign:
* raw -> filtered -> 1D Sobel derivative -> hysteresis edges -> inner edge selection
*==================================================================================================*/

#include "services/line_detector.h"

/* ----------------------------- Internal State ----------------------------- */

static float s_lastTrackCenter = (float)(VISION_LINEAR_BUFFER_SIZE / 2U);
static uint8 s_isLocked = 0U;
static uint8 s_singleEdgeStreak = 0U;

static const LineDetectorParams_t s_defaultParams =
{
    VISION_LINEAR_MIN_CONTRAST,
    VISION_LINEAR_EDGE_HIGH_PCT,
    VISION_LINEAR_EDGE_LOW_PCT
};

/* ----------------------------- Helpers ----------------------------- */

static uint16 LineDetector_AbsU16(sint16 value)
{
    return (value < 0) ? (uint16)(-value) : (uint16)value;
}

static uint16 LineDetector_AbsDiffU8(uint8 a, uint8 b)
{
    return (a >= b) ? (uint16)(a - b) : (uint16)(b - a);
}

static uint8 LineDetector_ScaleToPct(uint16 value, uint16 low, uint16 high)
{
    if (value <= low)
    {
        return 0U;
    }

    if (value >= high)
    {
        return 100U;
    }

    if (high <= low)
    {
        return 100U;
    }

    return (uint8)(((uint32)(value - low) * 100U) / (uint32)(high - low));
}

static const LineDetectorParams_t *LineDetector_SelectParams(const LineDetectorParams_t *params)
{
    return (params != (const LineDetectorParams_t*)0) ? params : &s_defaultParams;
}

static uint8 LineDetector_ComputeContrastConfidence(uint16 contrast, uint16 threshold)
{
    uint16 high = (threshold < 32768U) ? (uint16)(threshold * 2U) : 65535U;

    return LineDetector_ScaleToPct(contrast, threshold, high);
}

static uint8 LineDetector_ComputeEdgeStrengthConfidence(uint16 edgeStrength)
{
    return LineDetector_ScaleToPct(edgeStrength,
                                   VISION_LINEAR_CONF_EDGE_LOW,
                                   VISION_LINEAR_CONF_EDGE_HIGH);
}

static uint8 LineDetector_BlendConfidence(uint8 edgeStrengthConfidence,
                                          uint8 contrastConfidence)
{
    uint16 edgeWeight = VISION_LINEAR_CONF_EDGE_WEIGHT_PCT;
    uint16 contrastWeight = VISION_LINEAR_CONF_CONTRAST_WEIGHT_PCT;
    uint16 weightSum = (uint16)(edgeWeight + contrastWeight);

    if (weightSum == 0U)
    {
        return edgeStrengthConfidence;
    }

    return (uint8)((((uint32)edgeStrengthConfidence * edgeWeight) +
                    ((uint32)contrastConfidence * contrastWeight)) / weightSum);
}

static uint8 LineDetector_ClampSplitPoint(float center)
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

static void LineDetector_FilterSignal(const uint16 *pixels, uint16 *filtered)
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

static void LineDetector_ComputeGradient(const uint16 *filtered,
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
            uint16 magnitude = LineDetector_AbsU16(gradient[i]);
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

static uint8 LineDetector_IsLocalMaximum(const sint16 *gradient, uint16 idx)
{
    uint16 current = LineDetector_AbsU16(gradient[idx]);
    uint16 left = LineDetector_AbsU16(gradient[idx - 1U]);
    uint16 right = LineDetector_AbsU16(gradient[idx + 1U]);

    return ((current >= left) && (current > right)) ? 1U : 0U;
}

static uint8 LineDetector_WidthMatchesFinish(uint8 width, uint8 expectedWidth)
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

static uint8 LineDetector_WidthMatchesLane(uint8 width)
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

static void LineDetector_ProcessImpl(const uint16 *pixels,
                                     VisionOutput_t *out,
                                     LineDetector_DebugOut_t *dbg,
                                     const LineDetectorParams_t *params)
{
    const LineDetectorParams_t *cfg = LineDetector_SelectParams(params);
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
    uint16 bestLeftStrength = 0U;
    uint16 bestRightStrength = 0U;
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
    uint8 contrastConfidence = 0U;
    uint8 edgeStrengthConfidence = 0U;
    uint8 baseConfidence = 0U;
    uint8 lineStatusFactor = 0U;

    if ((dbg != (LineDetector_DebugOut_t*)0) &&
        ((dbg->mask & (uint32)VLIN_DBG_FILTERED) != 0u) &&
        (dbg->filteredOut != (uint16*)0))
    {
        filtered = dbg->filteredOut;
    }

    if ((dbg != (LineDetector_DebugOut_t*)0) &&
        ((dbg->mask & (uint32)VLIN_DBG_GRADIENT) != 0u) &&
        (dbg->gradientOut != (sint16*)0))
    {
        gradient = dbg->gradientOut;
    }

    LineDetector_FilterSignal(pixels, filtered);

    for (i = 0U; i < VISION_LINEAR_BUFFER_SIZE; i++)
    {
        if (filtered[i] < minVal) { minVal = filtered[i]; }
        if (filtered[i] > maxVal) { maxVal = filtered[i]; }
    }

    contrast = (uint16)(maxVal - minVal);
    LineDetector_ComputeGradient(filtered, gradient, &maxAbsGradient);
    contrastConfidence = LineDetector_ComputeContrastConfidence(contrast, cfg->minContrast);

    edgeHighThreshold = (uint16)(((uint32)maxAbsGradient * (uint32)cfg->edgeHighPct) / 100U);
    if (edgeHighThreshold < VISION_LINEAR_MIN_STRONG_EDGE)
    {
        edgeHighThreshold = VISION_LINEAR_MIN_STRONG_EDGE;
    }

    edgeLowThreshold = (uint16)(((uint32)edgeHighThreshold * (uint32)cfg->edgeLowPct) / 100U);
    if (edgeLowThreshold < VISION_LINEAR_MIN_WEAK_EDGE)
    {
        edgeLowThreshold = VISION_LINEAR_MIN_WEAK_EDGE;
    }

    splitPoint = s_isLocked ? LineDetector_ClampSplitPoint(s_lastTrackCenter)
                            : (VISION_LINEAR_BUFFER_SIZE / 2U);

    if (dbg != (LineDetector_DebugOut_t*)0)
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

    if ((contrast < cfg->minContrast) || (maxAbsGradient < VISION_LINEAR_MIN_WEAK_EDGE))
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
        uint16 strength = LineDetector_AbsU16(gradient[i]);
        uint8 isCandidate = 0U;

        if ((strength >= edgeLowThreshold) && (LineDetector_IsLocalMaximum(gradient, i) != 0U))
        {
            if (strength >= edgeHighThreshold)
            {
                isCandidate = 1U;
            }
            else
            {
                uint16 leftStrength = LineDetector_AbsU16(gradient[i - 1U]);
                uint16 rightStrength = LineDetector_AbsU16(gradient[i + 1U]);

                if ((leftStrength >= edgeHighThreshold) || (rightStrength >= edgeHighThreshold))
                {
                    isCandidate = 1U;
                }
            }
        }

        if (isCandidate != 0U)
        {
            sint8 polarity = (gradient[i] >= 0) ? 1 : -1;

            if ((dbg != (LineDetector_DebugOut_t*)0) &&
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
                    bestLeftStrength = strength;
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
                    bestRightStrength = strength;
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
        uint16 pairLeftStrength = bestLeftStrength;
        uint16 pairRightStrength = bestRightStrength;

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
                    if (LineDetector_WidthMatchesLane(width) != 0U)
                    {
                        uint16 widthErr = LineDetector_AbsDiffU8(width, (uint8)VISION_LINEAR_NOMINAL_LANE_WIDTH);
                        uint16 centerErr = LineDetector_AbsDiffU8(center, splitPoint);
                        uint16 strengthSum = (uint16)(leftCandidateStrengths[li] + rightCandidateStrengths[ri]);
                        sint32 score = (sint32)((uint32)widthErr * 8UL) +
                                       (sint32)centerErr -
                                       (sint32)(strengthSum / 16U);

                        if (score < bestPairScore)
                        {
                            bestPairScore = score;
                            pairLeft = leftIdx;
                            pairRight = rightIdx;
                            pairLeftStrength = leftCandidateStrengths[li];
                            pairRightStrength = rightCandidateStrengths[ri];
                        }
                    }
                }
            }
        }

        if (bestPairScore != 2147483647L)
        {
            bestLeftIdx = pairLeft;
            bestRightIdx = pairRight;
            bestLeftStrength = pairLeftStrength;
            bestRightStrength = pairRightStrength;
        }
    }

    out->leftLineIdx = bestLeftIdx;
    out->rightLineIdx = bestRightIdx;

    if ((dbg != (LineDetector_DebugOut_t*)0) && ((dbg->mask & (uint32)VLIN_DBG_STATS) != 0u))
    {
        dbg->leftInnerEdgeIdx = bestLeftIdx;
        dbg->rightInnerEdgeIdx = bestRightIdx;
    }

    {
        float midF = (float)(VISION_LINEAR_BUFFER_SIZE - 1U) / 2.0f;
        float trackCenter = midF;

        if ((bestLeftIdx != VISION_LINEAR_INVALID_IDX) && (bestRightIdx != VISION_LINEAR_INVALID_IDX))
        {
            uint16 selectedStrength = (bestLeftStrength < bestRightStrength) ? bestLeftStrength : bestRightStrength;
            edgeStrengthConfidence = LineDetector_ComputeEdgeStrengthConfidence(selectedStrength);
            baseConfidence = LineDetector_BlendConfidence(edgeStrengthConfidence, contrastConfidence);
            lineStatusFactor = 100U;
            out->status = VISION_TRACK_BOTH;
            out->confidence = (uint8)(((uint16)baseConfidence * lineStatusFactor) / 100U);
            trackCenter = ((float)bestLeftIdx + (float)bestRightIdx) / 2.0f;
            s_singleEdgeStreak = 0U;
        }
        else if (bestLeftIdx != VISION_LINEAR_INVALID_IDX)
        {
            float simulatedRight = (float)bestLeftIdx + (float)VISION_LINEAR_NOMINAL_LANE_WIDTH;
            edgeStrengthConfidence = LineDetector_ComputeEdgeStrengthConfidence(bestLeftStrength);
            baseConfidence = LineDetector_BlendConfidence(edgeStrengthConfidence, contrastConfidence);
            lineStatusFactor = 50U;
            out->status = VISION_TRACK_LEFT;
            out->confidence = (uint8)(((uint16)baseConfidence * lineStatusFactor) / 100U);
            trackCenter = ((float)bestLeftIdx + simulatedRight) / 2.0f;
            if (s_singleEdgeStreak < 255U)
            {
                s_singleEdgeStreak++;
            }
        }
        else if (bestRightIdx != VISION_LINEAR_INVALID_IDX)
        {
            float simulatedLeft = (float)bestRightIdx - (float)VISION_LINEAR_NOMINAL_LANE_WIDTH;
            edgeStrengthConfidence = LineDetector_ComputeEdgeStrengthConfidence(bestRightStrength);
            baseConfidence = LineDetector_BlendConfidence(edgeStrengthConfidence, contrastConfidence);
            lineStatusFactor = 50U;
            out->status = VISION_TRACK_RIGHT;
            out->confidence = (uint8)(((uint16)baseConfidence * lineStatusFactor) / 100U);
            trackCenter = (simulatedLeft + (float)bestRightIdx) / 2.0f;
            if (s_singleEdgeStreak < 255U)
            {
                s_singleEdgeStreak++;
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
                    uint16 gapError = LineDetector_AbsDiffU8(gap, expectedFinishGap);
                    uint16 centerError = LineDetector_AbsDiffU8(gapCenter, laneCenter);
                    uint16 maxCenterError = ((uint16)laneWidth * (uint16)VISION_FINISH_CENTER_TOL_PCT) / 100U;

                    if ((gap > 0U) &&
                        (centerError <= maxCenterError) &&
                        (LineDetector_WidthMatchesFinish(gap, expectedFinishGap) != 0U) &&
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

    if ((dbg != (LineDetector_DebugOut_t*)0) && ((dbg->mask & (uint32)VLIN_DBG_STATS) != 0u))
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

void LineDetector_Init(void)
{
    s_lastTrackCenter = (float)(VISION_LINEAR_BUFFER_SIZE / 2U);
    s_isLocked = 0U;
    s_singleEdgeStreak = 0U;
}

void LineDetector_Process(const uint16 *pixels, VisionOutput_t *out)
{
    LineDetector_ProcessImpl(pixels, out, (LineDetector_DebugOut_t*)0, &s_defaultParams);
}

void LineDetector_ProcessWithParams(const uint16 *pixels,
                                    VisionOutput_t *out,
                                    const LineDetectorParams_t *params)
{
    LineDetector_ProcessImpl(pixels, out, (LineDetector_DebugOut_t*)0, params);
}

void LineDetector_ProcessDebug(const uint16 *pixels,
                                 VisionOutput_t *out,
                                 LineDetector_DebugOut_t *dbg)
{
    LineDetector_ProcessImpl(pixels, out, dbg, &s_defaultParams);
}

void LineDetector_ProcessDebugWithParams(const uint16 *pixels,
                                         VisionOutput_t *out,
                                         LineDetector_DebugOut_t *dbg,
                                         const LineDetectorParams_t *params)
{
    LineDetector_ProcessImpl(pixels, out, dbg, params);
}
