#include "vision_debug.h"
#include "display.h"
#include <stdio.h>
#include <stdarg.h>

#define VDBG_SENSOR_MAX_U12    (4095U)

/* --- Internal helpers --- */

static uint8 clampPct_1_100(uint8 pct)
{
    if (pct == 0U)  { return 1U; }
    if (pct > 100U) { return 100U; }
    return pct;
}

static void ScaleFixed_U16_ToPct(uint8 *dstPct, const uint16 *srcU16, uint8 n, uint16 whiteSat)
{
    uint32 denom = (whiteSat == 0U) ? 1U : (uint32)whiteSat;
    uint8 i;

    for (i = 0U; i < n; i++)
    {
        uint32 pct32 = ((uint32)srcU16[i] * 100U) / denom;
        uint8 pct = (pct32 > 100U) ? 100U : (uint8)pct32;
        dstPct[i] = clampPct_1_100(pct);
    }
}

static uint8 VisionDebug_SignedThresholdToYPx(uint16 threshold,
                                              uint16 scaleAbs,
                                              uint8 heightPx,
                                              boolean positive)
{
    uint8 baselinePx;
    uint32 offset;
    uint32 y;

    if (heightPx == 0U)
    {
        return 0U;
    }

    baselinePx = (uint8)(heightPx / 2U);
    if (scaleAbs == 0U)
    {
        scaleAbs = 1U;
    }
    if (threshold > scaleAbs)
    {
        threshold = scaleAbs;
    }

    offset = ((uint32)threshold * (uint32)baselinePx) / (uint32)scaleAbs;
    y = (positive == TRUE)
        ? ((uint32)baselinePx - offset)
        : ((uint32)baselinePx + offset);

    if (y >= (uint32)heightPx)
    {
        y = (uint32)(heightPx - 1U);
    }

    return (uint8)y;
}

static void VisionDebug_FormatLine(char line[17], const char *fmt, ...)
{
    va_list args;
    int len;

    if (line == NULL)
    {
        return;
    }

    va_start(args, fmt);
    len = vsnprintf(line, 17U, fmt, args);
    va_end(args);

    if (len < 0)
    {
        len = 0;
    }
    if (len > 16)
    {
        len = 16;
    }

    while (len < 16)
    {
        line[len] = ' ';
        len++;
    }
    line[16] = '\0';
}

/* --- Public API --- */

void VisionDebug_Init(VisionDebug_State_t *st, uint16 whiteSat_default)
{
    if (st == NULL) { return; }

    st->screen = VDBG_SCREEN_MAIN;
    st->debugEntryScreen = VDBG_SCREEN_DEBUG_FIXED;
    st->mainTextMode = 0U;
    st->cfg.whiteSat = (whiteSat_default == 0U) ? 1U : whiteSat_default;
    st->cfg.paused = FALSE;
}

void VisionDebug_OnTick(VisionDebug_State_t *st,
                        boolean screenTogglePressed,
                        boolean modeNextPressed)
{
    if (st == NULL) { return; }

    if (screenTogglePressed == TRUE)
    {
        if (st->screen == VDBG_SCREEN_MAIN)
        {
            st->screen = st->debugEntryScreen;
        }
        else
        {
            st->screen = VDBG_SCREEN_MAIN;
        }
    }

    if (modeNextPressed == TRUE)
    {
        if (st->screen == VDBG_SCREEN_MAIN)
        {
            st->mainTextMode = (st->mainTextMode == 0U) ? 1U : 0U;
        }
        else if (st->screen == VDBG_SCREEN_DEBUG_FIXED)
        {
            st->screen = VDBG_SCREEN_DEBUG_AUTO;
        }
        else if (st->screen == VDBG_SCREEN_DEBUG_AUTO)
        {
            st->screen = VDBG_SCREEN_DEBUG_FINISH;
        }
        else
        {
            st->screen = VDBG_SCREEN_DEBUG_FIXED;
        }
    }
}

boolean VisionDebug_WantsVisionDebugData(const VisionDebug_State_t *st)
{
    if (st == NULL) { return FALSE; }

    if (st->screen != VDBG_SCREEN_MAIN)
    {
        return TRUE;
    }

    return (st->mainTextMode != 0U) ? TRUE : FALSE;
}

void VisionDebug_PrepareVisionDbg(const VisionDebug_State_t *st,
                                  VisionLinear_DebugOut_t *dbg,
                                  uint16 *filteredOutBuf,
                                  sint16 *gradientOutBuf)
{
    if ((st == NULL) || (dbg == NULL)) { return; }

    dbg->mask = (uint32)VLIN_DBG_NONE;
    dbg->filteredOut = (uint16*)0;
    dbg->gradientOut = (sint16*)0;

    if (st->screen == VDBG_SCREEN_MAIN)
    {
        if (st->mainTextMode == 1U)
        {
            dbg->mask = (uint32)VLIN_DBG_STATS;
        }
    }
    else
    {
        dbg->mask = (uint32)(VLIN_DBG_FILTERED | VLIN_DBG_GRADIENT | VLIN_DBG_STATS | VLIN_DBG_EDGES);
        dbg->filteredOut = filteredOutBuf;
        dbg->gradientOut = gradientOutBuf;
    }
}

void VisionDebug_Draw(const VisionDebug_State_t *st,
                      const uint16 *rawU16,
                      const uint16 *filteredU16,
                      const sint16 *gradientS16,
                      const VisionOutput_t *result,
                      const VisionLinear_DebugOut_t *dbg)
{
    static uint8 plotPct[VISION_LINEAR_BUFFER_SIZE];

    if ((st == NULL) || (rawU16 == NULL) || (result == NULL)) { return; }

    DisplayClear();

    if (st->cfg.paused == TRUE)
    {
        char line0[17];
        char line1[17];

        VisionDebug_FormatLine(line0, "PAUSED");
        VisionDebug_FormatLine(line1, "whiteMax: %4u",
                               (unsigned)st->cfg.whiteSat);
        DisplayText(0U, line0, 16U, 0U);
        DisplayText(1U, line1, 16U, 0U);
        DisplayRefresh();
        return;
    }

    if (st->screen == VDBG_SCREEN_MAIN)
    {
        char line0[17];
        char line1[17];
        const char *statusStr;
        sint16 errPct = (sint16)(result->error * 100.0f);

        if (result->feature == VISION_FEATURE_FINISH_LINE)
        {
            statusStr = "F";
        }
        else switch (result->status)
        {
            case VISION_TRACK_BOTH:  statusStr = "B"; break;
            case VISION_TRACK_LEFT:  statusStr = "L"; break;
            case VISION_TRACK_RIGHT: statusStr = "R"; break;
            default:                        statusStr = "X"; break;
        }

        if (st->mainTextMode == 0U)
        {
            VisionDebug_FormatLine(line0, "S:%s C:%3u E:%+3d",
                                   statusStr,
                                   (unsigned)result->confidence,
                                   (int)errPct);

            VisionDebug_FormatLine(line1, "L:%03u R:%03u",
                                   (unsigned)result->leftLineIdx,
                                   (unsigned)result->rightLineIdx);
        }
        else if (dbg != NULL)
        {
            VisionDebug_FormatLine(line0, "C:%4u G:%4u",
                                   (unsigned)dbg->contrast,
                                   (unsigned)dbg->maxAbsGradient);

            VisionDebug_FormatLine(line1, "Hi:%3u Lo:%3u",
                                   (unsigned)dbg->edgeHighThreshold,
                                   (unsigned)dbg->edgeLowThreshold);
        }
        else
        {
            VisionDebug_FormatLine(line0, "NO DBG");
            VisionDebug_FormatLine(line1, "");
        }

        DisplayText(0U, line0, 16U, 0U);
        DisplayText(1U, line1, 16U, 0U);

        ScaleFixed_U16_ToPct(plotPct, rawU16, VISION_LINEAR_BUFFER_SIZE, VDBG_SENSOR_MAX_U12);
        DisplayGraph(2U, plotPct, VISION_LINEAR_BUFFER_SIZE, 2U);

        if (result->leftLineIdx != VISION_LINEAR_INVALID_IDX)
        {
            DisplayOverlayVerticalLine(2U, 2U, result->leftLineIdx);
        }
        if (result->rightLineIdx != VISION_LINEAR_INVALID_IDX)
        {
            DisplayOverlayVerticalLine(2U, 2U, result->rightLineIdx);
        }
        if ((dbg != NULL) && (dbg->finishGapLeftEdgeIdx != VISION_LINEAR_INVALID_IDX))
        {
            DisplayOverlayVerticalLine(2U, 2U, dbg->finishGapLeftEdgeIdx);
        }
        if ((dbg != NULL) && (dbg->finishGapRightEdgeIdx != VISION_LINEAR_INVALID_IDX))
        {
            DisplayOverlayVerticalLine(2U, 2U, dbg->finishGapRightEdgeIdx);
        }

        DisplayRefresh();
        return;
    }

    if ((filteredU16 == NULL) || (gradientS16 == NULL) || (dbg == NULL))
    {
        ScaleFixed_U16_ToPct(plotPct, rawU16, VISION_LINEAR_BUFFER_SIZE, st->cfg.whiteSat);
        DisplayGraph(0U, plotPct, VISION_LINEAR_BUFFER_SIZE, 4U);
        DisplayRefresh();
        return;
    }

    {
        uint8 graphStartLine = 0U;
        uint8 graphLinesSpan = 4U;
        uint8 graphHeightPx = 32U;
        uint8 b;

        if (st->screen == VDBG_SCREEN_DEBUG_FIXED)
        {
            char line0[17];

            VisionDebug_FormatLine(line0, "FILT %4u",
                                   (unsigned)st->cfg.whiteSat);
            DisplayText(0U, line0, 16U, 0U);
            graphStartLine = 1U;
            graphLinesSpan = 3U;
            graphHeightPx = 24U;

            ScaleFixed_U16_ToPct(plotPct, filteredU16, VISION_LINEAR_BUFFER_SIZE, st->cfg.whiteSat);
            DisplayGraph(graphStartLine, plotPct, VISION_LINEAR_BUFFER_SIZE, graphLinesSpan);
        }
        else if (st->screen == VDBG_SCREEN_DEBUG_AUTO)
        {
            uint16 scaleAbs = dbg->maxAbsGradient;
            uint8 yWeakPos;
            uint8 yWeakNeg;
            uint8 yStrongPos;
            uint8 yStrongNeg;

            DisplaySignedGraph(graphStartLine,
                               gradientS16,
                               VISION_LINEAR_BUFFER_SIZE,
                               graphLinesSpan,
                               scaleAbs);

            yWeakPos = VisionDebug_SignedThresholdToYPx(dbg->edgeLowThreshold,
                                                        scaleAbs,
                                                        graphHeightPx,
                                                        TRUE);
            yWeakNeg = VisionDebug_SignedThresholdToYPx(dbg->edgeLowThreshold,
                                                        scaleAbs,
                                                        graphHeightPx,
                                                        FALSE);
            yStrongPos = VisionDebug_SignedThresholdToYPx(dbg->edgeHighThreshold,
                                                          scaleAbs,
                                                          graphHeightPx,
                                                          TRUE);
            yStrongNeg = VisionDebug_SignedThresholdToYPx(dbg->edgeHighThreshold,
                                                          scaleAbs,
                                                          graphHeightPx,
                                                          FALSE);

            DisplayOverlayHorizontalLine(graphStartLine, graphLinesSpan, yWeakPos);
            DisplayOverlayHorizontalLine(graphStartLine, graphLinesSpan, yWeakNeg);
            DisplayOverlayHorizontalLine(graphStartLine, graphLinesSpan, yStrongPos);
            DisplayOverlayHorizontalLine(graphStartLine, graphLinesSpan, yStrongNeg);
        }
        else
        {
            char line0[17];
            char line1[17];

            VisionDebug_FormatLine(line0, "FIN:%c W:%03u G:%02u",
                                   (result->feature == VISION_FEATURE_FINISH_LINE) ? 'Y' : 'N',
                                   (unsigned)dbg->laneWidth,
                                   (unsigned)dbg->measuredFinishGap);
            VisionDebug_FormatLine(line1, "EG:%02u L:%03u %03u",
                                   (unsigned)dbg->expectedFinishGap,
                                   (unsigned)dbg->finishGapLeftEdgeIdx,
                                   (unsigned)dbg->finishGapRightEdgeIdx);
            DisplayText(0U, line0, 16U, 0U);
            DisplayText(1U, line1, 16U, 0U);

            graphStartLine = 2U;
            graphLinesSpan = 2U;
            graphHeightPx = 16U;

            ScaleFixed_U16_ToPct(plotPct, filteredU16, VISION_LINEAR_BUFFER_SIZE, st->cfg.whiteSat);
            DisplayGraph(graphStartLine, plotPct, VISION_LINEAR_BUFFER_SIZE, graphLinesSpan);
        }

        if (st->screen != VDBG_SCREEN_DEBUG_FINISH)
        {
            for (b = 0U; b < dbg->edgeCount; b++)
            {
                DisplayOverlayHorizontalSegment(graphStartLine,
                                                graphLinesSpan,
                                                (uint8)(graphHeightPx - 1U),
                                                dbg->edges[b].idx,
                                                dbg->edges[b].idx);
            }
        }

        if (dbg->leftInnerEdgeIdx != VISION_LINEAR_INVALID_IDX)
        {
            DisplayOverlayVerticalLine(graphStartLine, graphLinesSpan, dbg->leftInnerEdgeIdx);
        }
        if (dbg->rightInnerEdgeIdx != VISION_LINEAR_INVALID_IDX)
        {
            DisplayOverlayVerticalLine(graphStartLine, graphLinesSpan, dbg->rightInnerEdgeIdx);
        }
        if ((st->screen == VDBG_SCREEN_DEBUG_FINISH) &&
            (dbg->finishGapLeftEdgeIdx != VISION_LINEAR_INVALID_IDX))
        {
            DisplayOverlayVerticalLine(graphStartLine, graphLinesSpan, dbg->finishGapLeftEdgeIdx);
        }
        if ((st->screen == VDBG_SCREEN_DEBUG_FINISH) &&
            (dbg->finishGapRightEdgeIdx != VISION_LINEAR_INVALID_IDX))
        {
            DisplayOverlayVerticalLine(graphStartLine, graphLinesSpan, dbg->finishGapRightEdgeIdx);
        }

        DisplayRefresh();
    }
}
