#include "services/line_detector.h"

#include <string.h>

static uint16_t absU16(int16_t value)
{
    return (value < 0) ? (uint16_t)(-value) : (uint16_t)value;
}

static uint16_t absDiffU8(uint8_t a, uint8_t b)
{
    return (a >= b) ? (uint16_t)(a - b) : (uint16_t)(b - a);
}

static uint8_t scaleToPct(uint16_t value, uint16_t low, uint16_t high)
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

    return (uint8_t)((((uint32_t)value - low) * 100UL) / (uint32_t)(high - low));
}

static uint8_t computeContrastConfidence(uint16_t contrast)
{
    uint16_t threshold = VISION_LINEAR_CONF_CONTRAST_THRESHOLD;
    uint16_t high = (threshold < 32768U) ? (uint16_t)(threshold * 2U) : 65535U;

    return scaleToPct(contrast, threshold, high);
}

static uint8_t computeEdgeStrengthConfidence(uint16_t edgeStrength)
{
    return scaleToPct(edgeStrength, VISION_LINEAR_CONF_EDGE_LOW, VISION_LINEAR_CONF_EDGE_HIGH);
}

static uint8_t blendConfidence(uint8_t edgeStrengthConfidence, uint8_t contrastConfidence)
{
    uint16_t edgeWeight = VISION_LINEAR_CONF_EDGE_WEIGHT_PCT;
    uint16_t contrastWeight = VISION_LINEAR_CONF_CONTRAST_WEIGHT_PCT;
    uint16_t weightSum = (uint16_t)(edgeWeight + contrastWeight);

    if (weightSum == 0U)
    {
        return edgeStrengthConfidence;
    }

    return (uint8_t)((((uint32_t)edgeStrengthConfidence * edgeWeight) +
                      ((uint32_t)contrastConfidence * contrastWeight)) /
                     weightSum);
}

static uint8_t clampSplitPoint(float center)
{
    uint8_t splitPoint = (uint8_t)center;
    uint8_t minSplit = VISION_LINEAR_SPLIT_MARGIN_PX;
    uint8_t maxSplit = (uint8_t)(VISION_LINEAR_BUFFER_SIZE - 1U - VISION_LINEAR_SPLIT_MARGIN_PX);

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

static void filterSignal(const uint16_t *pixels, uint16_t *filtered)
{
    filtered[0] = pixels[0];
    filtered[1] =
        (uint16_t)(((uint32_t)pixels[0] + ((uint32_t)pixels[1] * 2U) + (uint32_t)pixels[2]) / 4U);

    for (uint16_t i = 2U; i < (VISION_LINEAR_BUFFER_SIZE - 2U); i++)
    {
        uint32_t value = (uint32_t)pixels[i - 2U] + ((uint32_t)pixels[i - 1U] * 4U) +
                         ((uint32_t)pixels[i] * 6U) + ((uint32_t)pixels[i + 1U] * 4U) +
                         (uint32_t)pixels[i + 2U];

        filtered[i] = (uint16_t)(value / 16U);
    }

    filtered[VISION_LINEAR_BUFFER_SIZE - 2U] =
        (uint16_t)(((uint32_t)pixels[VISION_LINEAR_BUFFER_SIZE - 3U] +
                    ((uint32_t)pixels[VISION_LINEAR_BUFFER_SIZE - 2U] * 2U) +
                    (uint32_t)pixels[VISION_LINEAR_BUFFER_SIZE - 1U]) /
                   4U);
    filtered[VISION_LINEAR_BUFFER_SIZE - 1U] = pixels[VISION_LINEAR_BUFFER_SIZE - 1U];
}

static void computeGradient(const uint16_t *filtered, int16_t *gradient, uint16_t &maxAbsGradient)
{
    uint16_t maxGradient = 0U;

    gradient[0] = 0;
    gradient[1] = 0;

    for (uint16_t i = 2U; i < (VISION_LINEAR_BUFFER_SIZE - 2U); i++)
    {
        int32_t value = -((int32_t)filtered[i - 2U]) - ((int32_t)filtered[i - 1U] * 2) +
                        ((int32_t)filtered[i + 1U] * 2) + (int32_t)filtered[i + 2U];

        if (value > 32767)
        {
            value = 32767;
        }
        else if (value < -32768)
        {
            value = -32768;
        }

        gradient[i] = (int16_t)value;

        uint16_t magnitude = absU16(gradient[i]);
        if (magnitude > maxGradient)
        {
            maxGradient = magnitude;
        }
    }

    gradient[VISION_LINEAR_BUFFER_SIZE - 2U] = 0;
    gradient[VISION_LINEAR_BUFFER_SIZE - 1U] = 0;
    maxAbsGradient = maxGradient;
}

static uint8_t isLocalMaximum(const int16_t *gradient, uint16_t idx)
{
    uint16_t current = absU16(gradient[idx]);
    uint16_t left = absU16(gradient[idx - 1U]);
    uint16_t right = absU16(gradient[idx + 1U]);

    return ((current >= left) && (current > right)) ? 1U : 0U;
}

static uint8_t widthMatchesFinish(uint8_t width, uint8_t expectedWidth)
{
    uint16_t minWidth;
    uint16_t maxWidth;

    if (expectedWidth == 0U)
    {
        return 0U;
    }

    minWidth = ((uint16_t)expectedWidth * VISION_FINISH_WIDTH_MIN_PCT) / 100U;
    if (minWidth < 1U)
    {
        minWidth = 1U;
    }

    maxWidth = ((uint16_t)expectedWidth * VISION_FINISH_WIDTH_MAX_PCT) / 100U;
    if (maxWidth < (uint16_t)expectedWidth)
    {
        maxWidth = expectedWidth;
    }

    return (((uint16_t)width >= minWidth) && ((uint16_t)width <= maxWidth)) ? 1U : 0U;
}

static uint8_t widthMatchesLane(uint8_t width)
{
    uint16_t minWidth = ((uint16_t)VISION_LINEAR_NOMINAL_LANE_WIDTH *
                         (uint16_t)(100U - VISION_LINEAR_LANE_WIDTH_TOL_PCT)) /
                        100U;
    uint16_t maxWidth = ((uint16_t)VISION_LINEAR_NOMINAL_LANE_WIDTH *
                         (uint16_t)(100U + VISION_LINEAR_LANE_WIDTH_TOL_PCT)) /
                        100U;

    return (((uint16_t)width >= minWidth) && ((uint16_t)width <= maxWidth)) ? 1U : 0U;
}

void LineDetector::init()
{
    lastTrackCenter_ = (float)(VISION_LINEAR_BUFFER_SIZE / 2U);
    isLocked_ = 0U;
    singleEdgeStreak_ = 0U;
}

void LineDetector::process(const uint16_t *pixels, VisionOutput &out)
{
    processImpl(pixels, out, nullptr);
}

void LineDetector::processDebug(const uint16_t *pixels, VisionOutput &out,
                                LineDetectorDebugOut &debug)
{
    processImpl(pixels, out, &debug);
}

void LineDetector::processImpl(const uint16_t *pixels, VisionOutput &out,
                               LineDetectorDebugOut *debug)
{
    uint16_t filteredLocal[VISION_LINEAR_BUFFER_SIZE];
    int16_t gradientLocal[VISION_LINEAR_BUFFER_SIZE];
    uint16_t *filtered = filteredLocal;
    int16_t *gradient = gradientLocal;
    uint16_t minVal = 0xFFFFU;
    uint16_t maxVal = 0U;
    uint16_t contrast;
    uint16_t maxAbsGradient = 0U;
    uint16_t edgeHighThreshold;
    uint16_t edgeLowThreshold;
    uint8_t splitPoint;
    uint8_t bestLeftIdx = VISION_LINEAR_INVALID_IDX;
    uint8_t bestRightIdx = VISION_LINEAR_INVALID_IDX;
    uint16_t bestLeftDist = 255U;
    uint16_t bestRightDist = 255U;
    uint16_t bestLeftStrength = 0U;
    uint16_t bestRightStrength = 0U;
    uint8_t edgeCandidateIdx[VLIN_MAX_EDGE_CANDIDATES];
    int8_t edgeCandidatePolarity[VLIN_MAX_EDGE_CANDIDATES];
    uint8_t edgeCandidateCount = 0U;
    uint8_t leftCandidates[VLIN_MAX_EDGE_CANDIDATES];
    uint16_t leftCandidateStrengths[VLIN_MAX_EDGE_CANDIDATES];
    uint8_t rightCandidates[VLIN_MAX_EDGE_CANDIDATES];
    uint16_t rightCandidateStrengths[VLIN_MAX_EDGE_CANDIDATES];
    uint8_t leftCandidateCount = 0U;
    uint8_t rightCandidateCount = 0U;
    uint8_t laneWidth = 0U;
    uint8_t expectedFinishGap = 0U;
    uint8_t measuredFinishGap = 0U;
    uint8_t finishGapLeftEdgeIdx = VISION_LINEAR_INVALID_IDX;
    uint8_t finishGapRightEdgeIdx = VISION_LINEAR_INVALID_IDX;
    uint8_t finishDetected = 0U;
    uint8_t contrastConfidence = 0U;
    uint8_t edgeStrengthConfidence = 0U;
    uint8_t baseConfidence = 0U;
    uint8_t lineStatusFactor = 0U;

    if (pixels == nullptr)
    {
        out.error = 0.0f;
        out.status = VISION_TRACK_LOST;
        out.feature = VISION_FEATURE_NONE;
        out.confidence = 0U;
        out.leftLineIdx = VISION_LINEAR_INVALID_IDX;
        out.rightLineIdx = VISION_LINEAR_INVALID_IDX;
        init();
        return;
    }

    if ((debug != nullptr) && ((debug->mask & VLIN_DBG_FILTERED) != 0U) &&
        (debug->filteredOut != nullptr))
    {
        filtered = debug->filteredOut;
    }

    if ((debug != nullptr) && ((debug->mask & VLIN_DBG_GRADIENT) != 0U) &&
        (debug->gradientOut != nullptr))
    {
        gradient = debug->gradientOut;
    }

    filterSignal(pixels, filtered);

    for (uint16_t i = 0U; i < VISION_LINEAR_BUFFER_SIZE; i++)
    {
        if (filtered[i] < minVal)
        {
            minVal = filtered[i];
        }
        if (filtered[i] > maxVal)
        {
            maxVal = filtered[i];
        }
    }

    contrast = (uint16_t)(maxVal - minVal);
    computeGradient(filtered, gradient, maxAbsGradient);
    contrastConfidence = computeContrastConfidence(contrast);

    edgeHighThreshold = (uint16_t)(((uint32_t)maxAbsGradient * VISION_LINEAR_EDGE_HIGH_PCT) / 100U);
    if (edgeHighThreshold < VISION_LINEAR_MIN_STRONG_EDGE)
    {
        edgeHighThreshold = VISION_LINEAR_MIN_STRONG_EDGE;
    }

    edgeLowThreshold =
        (uint16_t)(((uint32_t)edgeHighThreshold * VISION_LINEAR_EDGE_LOW_PCT) / 100U);
    if (edgeLowThreshold < VISION_LINEAR_MIN_WEAK_EDGE)
    {
        edgeLowThreshold = VISION_LINEAR_MIN_WEAK_EDGE;
    }

    splitPoint =
        (isLocked_ != 0U) ? clampSplitPoint(lastTrackCenter_) : (VISION_LINEAR_BUFFER_SIZE / 2U);

    if (debug != nullptr)
    {
        if ((debug->mask & VLIN_DBG_STATS) != 0U)
        {
            debug->minVal = minVal;
            debug->maxVal = maxVal;
            debug->contrast = contrast;
            debug->maxAbsGradient = maxAbsGradient;
            debug->edgeHighThreshold = edgeHighThreshold;
            debug->edgeLowThreshold = edgeLowThreshold;
            debug->splitPoint = splitPoint;
            debug->leftInnerEdgeIdx = VISION_LINEAR_INVALID_IDX;
            debug->rightInnerEdgeIdx = VISION_LINEAR_INVALID_IDX;
            debug->finishGapLeftEdgeIdx = VISION_LINEAR_INVALID_IDX;
            debug->finishGapRightEdgeIdx = VISION_LINEAR_INVALID_IDX;
            debug->laneWidth = 0U;
            debug->expectedFinishGap = 0U;
            debug->measuredFinishGap = 0U;
            debug->finishDetected = 0U;
        }

        if ((debug->mask & VLIN_DBG_EDGES) != 0U)
        {
            debug->edgeCount = 0U;
        }
    }

    if (maxAbsGradient < VISION_LINEAR_MIN_WEAK_EDGE)
    {
        out.error = 0.0f;
        out.status = VISION_TRACK_LOST;
        out.feature = VISION_FEATURE_NONE;
        out.confidence = 0U;
        out.leftLineIdx = VISION_LINEAR_INVALID_IDX;
        out.rightLineIdx = VISION_LINEAR_INVALID_IDX;
        isLocked_ = 0U;
        singleEdgeStreak_ = 0U;
        return;
    }

    for (uint16_t i = 2U; i < (VISION_LINEAR_BUFFER_SIZE - 2U); i++)
    {
        uint16_t strength = absU16(gradient[i]);
        uint8_t isCandidate = 0U;

        if ((strength >= edgeLowThreshold) && (isLocalMaximum(gradient, i) != 0U))
        {
            if (strength >= edgeHighThreshold)
            {
                isCandidate = 1U;
            }
            else
            {
                uint16_t leftStrength = absU16(gradient[i - 1U]);
                uint16_t rightStrength = absU16(gradient[i + 1U]);

                if ((leftStrength >= edgeHighThreshold) || (rightStrength >= edgeHighThreshold))
                {
                    isCandidate = 1U;
                }
            }
        }

        if (isCandidate != 0U)
        {
            int8_t polarity = (gradient[i] >= 0) ? 1 : -1;

            if ((debug != nullptr) && ((debug->mask & VLIN_DBG_EDGES) != 0U) &&
                (debug->edgeCount < VLIN_MAX_EDGE_CANDIDATES))
            {
                debug->edges[debug->edgeCount].idx = (uint8_t)i;
                debug->edges[debug->edgeCount].polarity = polarity;
                debug->edges[debug->edgeCount].strength = strength;
                debug->edgeCount++;
            }

            if (edgeCandidateCount < VLIN_MAX_EDGE_CANDIDATES)
            {
                edgeCandidateIdx[edgeCandidateCount] = (uint8_t)i;
                edgeCandidatePolarity[edgeCandidateCount] = polarity;
                edgeCandidateCount++;
            }

            if ((polarity > 0) && ((uint8_t)i <= splitPoint))
            {
                uint16_t distance = (uint16_t)(splitPoint - (uint8_t)i);

                if (leftCandidateCount < VLIN_MAX_EDGE_CANDIDATES)
                {
                    leftCandidates[leftCandidateCount] = (uint8_t)i;
                    leftCandidateStrengths[leftCandidateCount] = strength;
                    leftCandidateCount++;
                }

                if (distance < bestLeftDist)
                {
                    bestLeftDist = distance;
                    bestLeftIdx = (uint8_t)i;
                    bestLeftStrength = strength;
                }
            }
            else if ((polarity < 0) && ((uint8_t)i > splitPoint))
            {
                uint16_t distance = (uint16_t)((uint8_t)i - splitPoint);

                if (rightCandidateCount < VLIN_MAX_EDGE_CANDIDATES)
                {
                    rightCandidates[rightCandidateCount] = (uint8_t)i;
                    rightCandidateStrengths[rightCandidateCount] = strength;
                    rightCandidateCount++;
                }

                if (distance < bestRightDist)
                {
                    bestRightDist = distance;
                    bestRightIdx = (uint8_t)i;
                    bestRightStrength = strength;
                }
            }
        }
    }

    if ((leftCandidateCount > 0U) && (rightCandidateCount > 0U))
    {
        int32_t bestPairScore = 2147483647L;
        uint8_t pairLeft = bestLeftIdx;
        uint8_t pairRight = bestRightIdx;
        uint16_t pairLeftStrength = bestLeftStrength;
        uint16_t pairRightStrength = bestRightStrength;

        for (uint8_t li = 0U; li < leftCandidateCount; li++)
        {
            for (uint8_t ri = 0U; ri < rightCandidateCount; ri++)
            {
                uint8_t leftIdx = leftCandidates[li];
                uint8_t rightIdx = rightCandidates[ri];

                if (rightIdx > leftIdx)
                {
                    uint8_t width = (uint8_t)(rightIdx - leftIdx);
                    uint8_t center = (uint8_t)((leftIdx + rightIdx) / 2U);

                    if (widthMatchesLane(width) != 0U)
                    {
                        uint16_t widthErr = absDiffU8(width, VISION_LINEAR_NOMINAL_LANE_WIDTH);
                        uint16_t centerErr = absDiffU8(center, splitPoint);
                        uint16_t strengthSum =
                            (uint16_t)(leftCandidateStrengths[li] + rightCandidateStrengths[ri]);
                        int32_t score = (int32_t)((uint32_t)widthErr * 8UL) + (int32_t)centerErr -
                                        (int32_t)(strengthSum / 16U);

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

    out.leftLineIdx = bestLeftIdx;
    out.rightLineIdx = bestRightIdx;

    if ((debug != nullptr) && ((debug->mask & VLIN_DBG_STATS) != 0U))
    {
        debug->leftInnerEdgeIdx = bestLeftIdx;
        debug->rightInnerEdgeIdx = bestRightIdx;
    }

    float midF = (float)(VISION_LINEAR_BUFFER_SIZE - 1U) / 2.0f;
    float trackCenter = midF;

    if ((bestLeftIdx != VISION_LINEAR_INVALID_IDX) && (bestRightIdx != VISION_LINEAR_INVALID_IDX))
    {
        uint16_t selectedStrength =
            (bestLeftStrength < bestRightStrength) ? bestLeftStrength : bestRightStrength;
        edgeStrengthConfidence = computeEdgeStrengthConfidence(selectedStrength);
        baseConfidence = blendConfidence(edgeStrengthConfidence, contrastConfidence);
        lineStatusFactor = 100U;
        out.status = VISION_TRACK_BOTH;
        out.confidence = (uint8_t)(((uint16_t)baseConfidence * lineStatusFactor) / 100U);
        trackCenter = ((float)bestLeftIdx + (float)bestRightIdx) / 2.0f;
        singleEdgeStreak_ = 0U;
    }
    else if (bestLeftIdx != VISION_LINEAR_INVALID_IDX)
    {
        float simulatedRight = (float)bestLeftIdx + (float)VISION_LINEAR_NOMINAL_LANE_WIDTH;
        edgeStrengthConfidence = computeEdgeStrengthConfidence(bestLeftStrength);
        baseConfidence = blendConfidence(edgeStrengthConfidence, contrastConfidence);
        lineStatusFactor = 50U;
        out.status = VISION_TRACK_LEFT;
        out.confidence = (uint8_t)(((uint16_t)baseConfidence * lineStatusFactor) / 100U);
        trackCenter = ((float)bestLeftIdx + simulatedRight) / 2.0f;
        if (singleEdgeStreak_ < 255U)
        {
            singleEdgeStreak_++;
        }
    }
    else if (bestRightIdx != VISION_LINEAR_INVALID_IDX)
    {
        float simulatedLeft = (float)bestRightIdx - (float)VISION_LINEAR_NOMINAL_LANE_WIDTH;
        edgeStrengthConfidence = computeEdgeStrengthConfidence(bestRightStrength);
        baseConfidence = blendConfidence(edgeStrengthConfidence, contrastConfidence);
        lineStatusFactor = 50U;
        out.status = VISION_TRACK_RIGHT;
        out.confidence = (uint8_t)(((uint16_t)baseConfidence * lineStatusFactor) / 100U);
        trackCenter = (simulatedLeft + (float)bestRightIdx) / 2.0f;
        if (singleEdgeStreak_ < 255U)
        {
            singleEdgeStreak_++;
        }
    }
    else
    {
        out.error = 0.0f;
        out.status = VISION_TRACK_LOST;
        out.feature = VISION_FEATURE_NONE;
        out.confidence = 0U;
        isLocked_ = 0U;
        singleEdgeStreak_ = 0U;
        return;
    }

    out.feature = VISION_FEATURE_NONE;

    if ((bestLeftIdx != VISION_LINEAR_INVALID_IDX) && (bestRightIdx != VISION_LINEAR_INVALID_IDX) &&
        (bestRightIdx > bestLeftIdx))
    {
        uint16_t bestGapError = 0xFFFFU;
        laneWidth = (uint8_t)(bestRightIdx - bestLeftIdx);
        expectedFinishGap = (uint8_t)(((uint16_t)laneWidth * VISION_FINISH_CENTER_GAP_MM) /
                                      VISION_FINISH_INNER_WIDTH_MM);
        if (expectedFinishGap < 1U)
        {
            expectedFinishGap = 1U;
        }

        for (uint8_t e = 0U; e + 1U < edgeCandidateCount; e++)
        {
            uint8_t leftGapEdge = edgeCandidateIdx[e];
            uint8_t rightGapEdge = edgeCandidateIdx[e + 1U];
            int8_t leftGapPolarity = edgeCandidatePolarity[e];
            int8_t rightGapPolarity = edgeCandidatePolarity[e + 1U];

            if ((leftGapPolarity > 0) && (rightGapPolarity < 0) && (leftGapEdge > bestLeftIdx) &&
                (rightGapEdge < bestRightIdx) && (rightGapEdge > leftGapEdge))
            {
                uint8_t gap = (uint8_t)(rightGapEdge - leftGapEdge);
                uint8_t gapCenter = (uint8_t)((leftGapEdge + rightGapEdge) / 2U);
                uint8_t laneCenter = (uint8_t)((bestLeftIdx + bestRightIdx) / 2U);
                uint16_t gapError = absDiffU8(gap, expectedFinishGap);
                uint16_t centerError = absDiffU8(gapCenter, laneCenter);
                uint16_t maxCenterError =
                    ((uint16_t)laneWidth * VISION_FINISH_CENTER_TOL_PCT) / 100U;

                if ((gap > 0U) && (centerError <= maxCenterError) &&
                    (widthMatchesFinish(gap, expectedFinishGap) != 0U) && (gapError < bestGapError))
                {
                    bestGapError = gapError;
                    measuredFinishGap = gap;
                    finishGapLeftEdgeIdx = leftGapEdge;
                    finishGapRightEdgeIdx = rightGapEdge;
                    finishDetected = 1U;
                    out.feature = VISION_FEATURE_FINISH_LINE;
                }
            }
        }
    }

    out.error = (trackCenter - midF) / midF;
    if (out.error < -1.0f)
    {
        out.error = -1.0f;
    }
    if (out.error > 1.0f)
    {
        out.error = 1.0f;
    }

    lastTrackCenter_ = trackCenter;
    isLocked_ = 1U;

    if ((debug != nullptr) && ((debug->mask & VLIN_DBG_STATS) != 0U))
    {
        debug->laneWidth = laneWidth;
        debug->expectedFinishGap = expectedFinishGap;
        debug->measuredFinishGap = measuredFinishGap;
        debug->finishGapLeftEdgeIdx = finishGapLeftEdgeIdx;
        debug->finishGapRightEdgeIdx = finishGapRightEdgeIdx;
        debug->finishDetected = finishDetected;
    }
}
