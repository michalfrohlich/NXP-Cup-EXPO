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

/* FIXED scaling: 0..whiteSatU8 -> 1..100 */
static void ScaleFixed_U8_ToPct(uint8 *dstPct, const uint8 *srcU8, uint8 n, uint8 whiteSatU8)
{
    uint16 denom = (whiteSatU8 == 0U) ? 1U : (uint16)whiteSatU8;

    for (uint8 i = 0U; i < n; i++)
    {
        uint16 pct16 = ((uint16)srcU8[i] * 100U) / denom;
        uint8 pct = (pct16 > 100U) ? 100U : (uint8)pct16;
        dstPct[i] = clampPct_1_100(pct);
    }
}

/* AUTO scaling: mn..mx -> 1..100 */
static void ScaleAuto_U8_ToPct(uint8 *dstPct, const uint8 *srcU8, uint8 n, uint8 *outMin, uint8 *outMax)
{
    uint8 mn = 255U;
    uint8 mx = 0U;

    for (uint8 i = 0U; i < n; i++)
    {
        uint8 v = srcU8[i];
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

    uint16 range = (uint16)(mx - mn);

    for (uint8 i = 0U; i < n; i++)
    {
        uint8 v = srcU8[i];

        if (v <= mn)      { dstPct[i] = 1U; }
        else if (v >= mx) { dstPct[i] = 100U; }
        else
        {
            uint16 num = (uint16)(v - mn) * 100U;
            dstPct[i] = clampPct_1_100((uint8)(num / range));
        }
    }
}

/* Convert scalar to pct using FIXED scale */
static uint8 ScalarFixed_ToPct(uint8 vU8, uint8 whiteSatU8)
{
    uint16 denom = (whiteSatU8 == 0U) ? 1U : (uint16)whiteSatU8;
    uint16 pct16 = ((uint16)vU8 * 100U) / denom;
    uint8 pct = (pct16 > 100U) ? 100U : (uint8)pct16;
    return clampPct_1_100(pct);
}

/* Convert scalar to pct using AUTO mn/mx */
static uint8 ScalarAuto_ToPct(uint8 vU8, uint8 mn, uint8 mx)
{
    if (mx <= mn) { return 100U; }
    if (vU8 <= mn) { return 1U; }
    if (vU8 >= mx) { return 100U; }

    uint16 range = (uint16)(mx - mn);
    uint16 num   = (uint16)(vU8 - mn) * 100U;
    return clampPct_1_100((uint8)(num / range));
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

void VisionDebug_Init(VisionDebug_State_t *st, uint8 whiteSatU8_default)
{
    if (st == NULL) { return; }

    st->screen = VDBG_SCREEN_MAIN;
    st->debugEntryScreen = VDBG_SCREEN_DEBUG_FIXED;

    st->cfg.whiteSatU8 = (whiteSatU8_default == 0U) ? 1U : whiteSatU8_default;
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

    /* SW1/SW3: cycle debug modes (only active inside DEBUG) */
    if ((st->screen != VDBG_SCREEN_MAIN) && (modeNextPressed == TRUE))
    {
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

boolean VisionDebug_WantsVisionDebugData(const VisionDebug_State_t *st)
{
    if (st == NULL) { return FALSE; }
    return (st->screen != VDBG_SCREEN_MAIN) ? TRUE : FALSE;
}

void VisionDebug_PrepareVisionDbg(const VisionDebug_State_t *st,
                                  VisionLinear_DebugOut_t *dbg,
                                  uint8 *smoothOutBuf)
{
    if ((st == NULL) || (dbg == NULL)) { return; }

    dbg->mask = (uint32)VLIN_DBG_NONE;
    dbg->smoothOut = (uint8*)0;

    if (st->screen != VDBG_SCREEN_MAIN)
    {
        dbg->mask = (uint32)(VLIN_DBG_SMOOTH | VLIN_DBG_STATS | VLIN_DBG_BLOBS);
        dbg->smoothOut = smoothOutBuf;
    }
}

void VisionDebug_Draw(const VisionDebug_State_t *st,
                      const uint8 *rawU8,
                      const uint8 *smoothU8,
                      const VisionLinear_ResultType *result,
                      const VisionLinear_DebugOut_t *dbg)
{
    if ((st == NULL) || (rawU8 == NULL) || (result == NULL)) { return; }

    /* OLED is 128x32 => 4 pages, 32 px max */
    static uint8 plotPct[VISION_LINEAR_BUFFER_SIZE];

    DisplayClear();

    if (st->screen == VDBG_SCREEN_MAIN)
    {
        /* MAIN: 2 lines text + raw graph (2 pages) */
        char line0[17];
        char line1[17];

        sint16 errPct = (sint16)(result->Error * 100.0f);

        (void)snprintf(line0, sizeof(line0), "C:%3u E:%+4d",
                       (unsigned)result->Confidence, (int)errPct);
        DisplayText(0U, line0, 16U, 0U);

        (void)snprintf(line1, sizeof(line1), "L:%03u R:%03u",
                       (unsigned)result->LeftLineIdx, (unsigned)result->RightLineIdx);
        DisplayText(1U, line1, 16U, 0U);

        /* Scale raw for DisplayGraph() */
        ScaleFixed_U8_ToPct(plotPct, rawU8, VISION_LINEAR_BUFFER_SIZE, st->cfg.whiteSatU8);
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
    if ((smoothU8 == NULL) || (dbg == NULL))
    {
        /* Fallback: plot raw only */
        ScaleFixed_U8_ToPct(plotPct, rawU8, VISION_LINEAR_BUFFER_SIZE, st->cfg.whiteSatU8);
        DisplayGraph(0U, plotPct, VISION_LINEAR_BUFFER_SIZE, 4U);
        DisplayRefresh();
        return;
    }

    const uint8 graphStartLine = 0U;
    const uint8 graphLinesSpan = 4U;
    const uint8 graphHeightPx  = 32U;

    uint8 autoMin = 0U, autoMax = 255U;

    if (st->screen == VDBG_SCREEN_DEBUG_FIXED)
    {
        ScaleFixed_U8_ToPct(plotPct, smoothU8, VISION_LINEAR_BUFFER_SIZE, st->cfg.whiteSatU8);
    }
    else /* AUTO */
    {
        ScaleAuto_U8_ToPct(plotPct, smoothU8, VISION_LINEAR_BUFFER_SIZE, &autoMin, &autoMax);
    }

    DisplayGraph(graphStartLine, plotPct, VISION_LINEAR_BUFFER_SIZE, graphLinesSpan);

    /* Threshold line */
    uint8 thrPct;
    if (st->screen == VDBG_SCREEN_DEBUG_FIXED)
    {
        thrPct = ScalarFixed_ToPct(dbg->threshold, st->cfg.whiteSatU8);
    }
    else
    {
        thrPct = ScalarAuto_ToPct(dbg->threshold, autoMin, autoMax);
    }
    uint8 yThresh = Pct_ToYPx(thrPct, graphHeightPx);
    DisplayOverlayHorizontalLine(graphStartLine, graphLinesSpan, yThresh);

    /* Blob segments at bottom pixel row */
    uint8 yBlob = (uint8)(graphHeightPx - 1U);
    for (uint8 k = 0U; k < dbg->blobCount; k++)
    {
        uint8 x0 = dbg->blobs[k].start;
        uint8 x1 = dbg->blobs[k].end;

        if (x0 >= 128U) { continue; }
        if (x1 >= 128U) { x1 = 127U; }
        if (x0 > x1)    { continue; }

        DisplayOverlayHorizontalSegment(graphStartLine, graphLinesSpan, yBlob, x0, x1);
    }

    /* L/R markers */
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
