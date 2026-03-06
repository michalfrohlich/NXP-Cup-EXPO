#include "vision_debug.h"
#include "display.h"
#include <stdio.h>

/* --- Internal helpers --- */

/* DisplayGraph() expects 0..100 and becomes unsafe at 0% for full 32px height.
   We clamp to 1..100 to avoid shift-by-32 UB in the driver. */
static uint8 clampPct_1_100(uint8 pct)
{
    if (pct == 0U)  { return 1U; }
    if (pct > 100U) { return 100U; }
    return pct;
}

/* FIXED scaling: 0..whiteSat -> 1..100 */
static void ScaleFixed_U16_ToPct(uint8 *dstPct, const uint16 *srcU16, uint8 n, uint16 whiteSat)
{
    uint32 denom = (whiteSat == 0U) ? 1U : (uint32)whiteSat;

    for (uint8 i = 0U; i < n; i++)
    {
        uint32 pct32 = ((uint32)srcU16[i] * 100U) / denom;
        uint8 pct = (pct32 > 100U) ? 100U : (uint8)pct32;
        dstPct[i] = clampPct_1_100(pct);
    }
}

/* AUTO scaling: mn..mx -> 1..100 */
static void ScaleAuto_U16_ToPct(uint8 *dstPct, const uint16 *srcU16, uint8 n, uint16 *outMin, uint16 *outMax)
{
    uint16 mn = 0xFFFFU;
    uint16 mx = 0U;

    for (uint8 i = 0U; i < n; i++)
    {
        uint16 v = srcU16[i];
        if (v < mn) { mn = v; }
        if (v > mx) { mx = v; }
    }

    if (outMin) { *outMin = mn; }
    if (outMax) { *outMax = mx; }

    if (mx <= mn)
    {
        for (uint8 i = 0U; i < n; i++) { dstPct[i] = 100U; }
        return;
    }

    {
        uint32 range = (uint32)(mx - mn);

        for (uint8 i = 0U; i < n; i++)
        {
            uint16 v = srcU16[i];

            if (v <= mn)      { dstPct[i] = 1U; }
            else if (v >= mx) { dstPct[i] = 100U; }
            else
            {
                uint32 num = (uint32)(v - mn) * 100U;
                dstPct[i] = clampPct_1_100((uint8)(num / range));
            }
        }
    }
}

/* Convert scalar to pct using FIXED scale */
static uint8 ScalarFixed_ToPct(uint16 vU16, uint16 whiteSat)
{
    uint32 denom = (whiteSat == 0U) ? 1U : (uint32)whiteSat;
    uint32 pct32 = ((uint32)vU16 * 100U) / denom;
    uint8 pct = (pct32 > 100U) ? 100U : (uint8)pct32;
    return clampPct_1_100(pct);
}

/* Convert scalar to pct using AUTO mn/mx */
static uint8 ScalarAuto_ToPct(uint16 vU16, uint16 mn, uint16 mx)
{
    if (mx <= mn) { return 100U; }
    if (vU16 <= mn) { return 1U; }
    if (vU16 >= mx) { return 100U; }

    {
        uint32 range = (uint32)(mx - mn);
        uint32 num   = (uint32)(vU16 - mn) * 100U;
        return clampPct_1_100((uint8)(num / range));
    }
}

/* DisplayGraph y mapping (y=0 top) */
static uint8 Pct_ToYPx(uint8 pct, uint8 heightPx)
{
    pct = clampPct_1_100(pct);

    uint32 y = ((uint32)(100U - pct) * (uint32)heightPx) / 100U;
    if (heightPx == 0U) { return 0U; }
    if (y >= (uint32)heightPx) { y = (uint32)(heightPx - 1U); }
    return (uint8)y;
}

/* --- Public API --- */

void VisionDebug_Init(VisionDebug_State_t *st, uint16 whiteSat_default)
{
    if (st == NULL) { return; }

    st->screen = VDBG_SCREEN_MAIN;
    st->debugEntryScreen = VDBG_SCREEN_DEBUG_FIXED;
    st->mainTextMode = 0U;
    st->cfg.whiteSat = (whiteSat_default == 0U) ? 1U : whiteSat_default;
}

void VisionDebug_OnTick(VisionDebug_State_t *st,
                        boolean screenTogglePressed,
                        boolean modeNextPressed)
{
    if (st == NULL) { return; }

    /* SW2: MAIN <-> DEBUG (always enter FIXED first) */
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

    	/* SW3 behavior depends on current screen */
        if (modeNextPressed == TRUE)
        {
            if (st->screen == VDBG_SCREEN_MAIN)
            {
                /* Toggle MAIN text layout */
                st->mainTextMode = (st->mainTextMode == 0U) ? 1U : 0U;
            }
            else
            {
                /* Cycle DEBUG modes */
                if (st->screen == VDBG_SCREEN_DEBUG_FIXED)
                {
                    st->screen = VDBG_SCREEN_DEBUG_AUTO;
                }
                else
                {
                    st->screen = VDBG_SCREEN_DEBUG_FIXED;
                }
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

    /* MAIN stats mode also needs debug scalars */
    return (st->mainTextMode != 0U) ? TRUE : FALSE;
}

void VisionDebug_PrepareVisionDbg(const VisionDebug_State_t *st,
                                  VisionLinear_DebugOut_t *dbg,
                                  uint16 *smoothOutBuf)
{
    if ((st == NULL) || (dbg == NULL)) { return; }

    dbg->mask = (uint32)VLIN_DBG_NONE;
    dbg->smoothOut = (uint16*)0;

    if (st->screen == VDBG_SCREEN_MAIN)
    {
        if (st->mainTextMode == 1U)
        {
            dbg->mask = (uint32)VLIN_DBG_STATS;
        }
    }
    else
    {
        dbg->mask = (uint32)(VLIN_DBG_SMOOTH | VLIN_DBG_STATS | VLIN_DBG_BLOBS);
        dbg->smoothOut = smoothOutBuf;
    }
}

void VisionDebug_Draw(const VisionDebug_State_t *st,
                      const uint16 *rawU16,
                      const uint16 *smoothU16,
                      const VisionLinear_ResultType *result,
                      const VisionLinear_DebugOut_t *dbg)
{
    if ((st == NULL) || (rawU16 == NULL) || (result == NULL)) { return; }

    /* OLED is 128x32 => 4 pages, 32 px max */
    static uint8 plotPct[VISION_LINEAR_BUFFER_SIZE];

    DisplayClear();

    if (st->screen == VDBG_SCREEN_MAIN)
    {
        /* MAIN: 2 lines text + raw graph (2 pages) */
        char line0[17];
        char line1[17];
        uint8 i;
        const char *statusStr;
        sint16 errPct = (sint16)(result->Error * 100.0f);

        for (i = 0U; i < 16U; i++)
        {
            line0[i] = ' ';
            line1[i] = ' ';
        }
        line0[16] = '\0';
        line1[16] = '\0';

        switch (result->Status)
        {
            case VISION_LINEAR_TRACK_BOTH:  statusStr = "B"; break;
            case VISION_LINEAR_TRACK_LEFT:  statusStr = "L"; break;
            case VISION_LINEAR_TRACK_RIGHT: statusStr = "R"; break;
            default:                        statusStr = "X"; break;
        }

        if (st->mainTextMode == 0U)
        {
            /* Screen 1:
               row1: Status, Confidence, Error
               row2: L, R
            */
            (void)snprintf(line0, sizeof(line0), "S:%s C:%3u E:%+3d",
                           statusStr,
                           (unsigned)result->Confidence,
                           (int)errPct);

            (void)snprintf(line1, sizeof(line1), "L:%03u R:%03u",
                           (unsigned)result->LeftLineIdx,
                           (unsigned)result->RightLineIdx);
        }
        else if (dbg != NULL)
        {
            /* Screen 2:
               row1: Contrast, Point
               row2: Min, Max
            */
            (void)snprintf(line0, sizeof(line0), "C:%3u P:%3u",
                           (unsigned)dbg->contrast,
                           (unsigned)dbg->splitPoint);

            (void)snprintf(line1, sizeof(line1), "N:%4u X:%4u",
                           (unsigned)dbg->minVal,
                           (unsigned)dbg->maxVal);
        }
        else
        {
            (void)snprintf(line0, sizeof(line0), "NO DBG          ");
            (void)snprintf(line1, sizeof(line1), "                ");
        }

        DisplayText(0U, line0, 16U, 0U);
        DisplayText(1U, line1, 16U, 0U);

        /* Scale raw for DisplayGraph() */
        ScaleFixed_U16_ToPct(plotPct, rawU16, VISION_LINEAR_BUFFER_SIZE, st->cfg.whiteSat);
        DisplayGraph(2U, plotPct, VISION_LINEAR_BUFFER_SIZE, 2U);

        if (result->LeftLineIdx != VISION_LINEAR_INVALID_IDX)
        {
            DisplayOverlayVerticalLine(2U, 2U, result->LeftLineIdx);
        }
        if (result->RightLineIdx != VISION_LINEAR_INVALID_IDX)
        {
            DisplayOverlayVerticalLine(2U, 2U, result->RightLineIdx);
        }

        DisplayRefresh();
        return;
    }

    /* DEBUG: full height graph (4 pages), overlays at bottom */
    if ((smoothU16 == NULL) || (dbg == NULL))
    {
        /* Fallback: plot raw only */
        ScaleFixed_U16_ToPct(plotPct, rawU16, VISION_LINEAR_BUFFER_SIZE, st->cfg.whiteSat);
        DisplayGraph(0U, plotPct, VISION_LINEAR_BUFFER_SIZE, 4U);
        DisplayRefresh();
        return;
    }

    {
        const uint8 graphStartLine = 0U;
        const uint8 graphLinesSpan = 4U;
        const uint8 graphHeightPx  = 32U;

        uint16 autoMin = 0U, autoMax = 0U;

        if (st->screen == VDBG_SCREEN_DEBUG_FIXED)
        {
            ScaleFixed_U16_ToPct(plotPct, smoothU16, VISION_LINEAR_BUFFER_SIZE, st->cfg.whiteSat);
        }
        else /* AUTO */
        {
            ScaleAuto_U16_ToPct(plotPct, smoothU16, VISION_LINEAR_BUFFER_SIZE, &autoMin, &autoMax);
        }

        DisplayGraph(graphStartLine, plotPct, VISION_LINEAR_BUFFER_SIZE, graphLinesSpan);

        /* Threshold line */
        {
            uint8 thrPct;
            uint8 yThresh;

            if (st->screen == VDBG_SCREEN_DEBUG_FIXED)
            {
                thrPct = ScalarFixed_ToPct(dbg->threshold, st->cfg.whiteSat);
            }
            else
            {
                thrPct = ScalarAuto_ToPct(dbg->threshold, autoMin, autoMax);
            }

            yThresh = Pct_ToYPx(thrPct, graphHeightPx);
            DisplayOverlayHorizontalLine(graphStartLine, graphLinesSpan, yThresh);
        }

        /* Blob segments along the bottom row */
        for (uint8 b = 0U; b < dbg->blobCount; b++)
        {
            uint8 x0 = dbg->blobs[b].start;
            uint8 x1 = dbg->blobs[b].end;
            uint8 y  = (uint8)(graphHeightPx - 1U);

            DisplayOverlayHorizontalSegment(graphStartLine, graphLinesSpan, x0, x1, y);
        }

        /* Final chosen line markers */
        if (result->LeftLineIdx != VISION_LINEAR_INVALID_IDX)
        {
            DisplayOverlayVerticalLine(graphStartLine, graphLinesSpan, result->LeftLineIdx);
        }
        if (result->RightLineIdx != VISION_LINEAR_INVALID_IDX)
        {
            DisplayOverlayVerticalLine(graphStartLine, graphLinesSpan, result->RightLineIdx);
        }

        DisplayRefresh();
    }
}
