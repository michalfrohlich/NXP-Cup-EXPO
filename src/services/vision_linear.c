/*
  vision_linear.c
  ===============
  PURE logic implementation for the linear camera "vision".

  It detects black regions in the 128 pixel scan, estimates:
   - left edge
   - right edge
   - lane center
   - error from center
   - confidence
   - intersection heuristic

  NO hardware calls here.
*/

#include "vision_linear.h"

/* IMPORTANT:
   Your compiler does NOT have include paths for "app/...".
   So we include config using a relative path.
*/
#include "../app/car_config.h"

/* Small helpers (pure math) */
static sint32 abs_s32(sint32 x) { return (x < 0) ? -x : x; }

static sint32 clamp_s32(sint32 v, sint32 lo, sint32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* ---------------------------------------------------------
   find_black_regions()
   ---------------------------------------------------------
   We scan 128 pixels left->right.

   "Black pixel" means:
       value < threshold

   We group continuous black pixels into regions.
   Each region becomes one center value.
--------------------------------------------------------- */
static uint8 find_black_regions(const uint8 *vals,
                                uint8 thr,
                                sint16 *centers,
                                uint8 maxCenters,
                                uint16 *blackCountOut)
{
    uint8 inBlack = 0u;
    uint16 start = 0u;
    uint8 count = 0u;
    uint16 blackCount = 0u;

    for (uint16 i = 0u; i < CAM_N_PIXELS; i++)
    {
        boolean isBlack = (vals[i] < thr) ? TRUE : FALSE;
        if (isBlack) { blackCount++; }

        /* entering a black region */
        if (isBlack && (inBlack == 0u))
        {
            inBlack = 1u;
            start = i;
        }
        /* leaving a black region */
        else if ((!isBlack) && (inBlack != 0u))
        {
            uint16 end = (i > 0u) ? (i - 1u) : 0u;

            if (count < maxCenters)
            {
                centers[count] = (sint16)((start + end) / 2u);
                count++;
            }
            inBlack = 0u;
        }
    }

    /* if we ended while still inside black region */
    if (inBlack != 0u)
    {
        uint16 end = (CAM_N_PIXELS - 1u);
        if (count < maxCenters)
        {
            centers[count] = (sint16)((start + end) / 2u);
            count++;
        }
    }

    if (blackCountOut != NULL_PTR)
    {
        *blackCountOut = blackCount;
    }

    return count;
}

/* ---------------------------------------------------------
   VisionLinear_Process()
--------------------------------------------------------- */
LinearVision_t VisionLinear_Process(const LinearCameraFrame *frame,
                                   uint8 thr,
                                   sint16 lastLaneCenter)
{
    LinearVision_t out;

    /* Default values (safe) */
    out.valid = FALSE;
    out.left_edge_px = -1;
    out.right_edge_px = -1;

    out.lane_center_px = lastLaneCenter;
    out.error_px = (sint16)(lastLaneCenter - (sint16)CAM_CENTER_PX);

    out.region_count = 0u;
    out.black_ratio_pct = 0u;
    out.confidence = 0u;
    out.intersection_likely = FALSE;

    sint16 centers[8];
    uint16 blackCount = 0u;

    /* Step 1: find black regions */
    uint8 n = find_black_regions(frame->Values, thr, centers, 8u, &blackCount);

    out.region_count = n;
    out.black_ratio_pct = (uint8)((blackCount * 100u) / CAM_N_PIXELS);

    /* Step 2: interpret regions */
    if (n == 0u)
    {
        /* No black at all -> line lost */
        return out;
    }

    if (n == 1u)
    {
        /* Only one edge visible:
           guess the other edge using EXPECTED_TRACK_WIDTH_PX */
        sint16 e = centers[0];

        if (e < (sint16)CAM_CENTER_PX)
        {
            out.left_edge_px = e;
            out.right_edge_px = (sint16)(e + EXPECTED_TRACK_WIDTH_PX);
            if (out.right_edge_px > 127) out.right_edge_px = 127;
        }
        else
        {
            out.right_edge_px = e;
            out.left_edge_px = (sint16)(e - EXPECTED_TRACK_WIDTH_PX);
            if (out.left_edge_px < 0) out.left_edge_px = 0;
        }

        out.lane_center_px = (sint16)((out.left_edge_px + out.right_edge_px) / 2);
        out.confidence = 40u; /* moderate confidence */
        out.valid = TRUE;
    }
    else
    {
        /* Two or more regions:
           use left-most and right-most as lane edges */
        out.left_edge_px = centers[0];
        out.right_edge_px = centers[n - 1u];
        out.lane_center_px = (sint16)((out.left_edge_px + out.right_edge_px) / 2);

        /* confidence based on width and extra regions */
        sint32 width = (sint32)out.right_edge_px - (sint32)out.left_edge_px;
        sint32 widthErr = abs_s32(width - (sint32)EXPECTED_TRACK_WIDTH_PX);

        sint32 conf = 100;
        conf -= (widthErr * 2);              /* penalize wrong width */
        conf -= ((sint32)(n - 2u) * 10);     /* penalize extra regions */
        conf = clamp_s32(conf, 0, 100);

        out.confidence = (uint8)conf;
        out.valid = (out.confidence > 10u) ? TRUE : FALSE;
    }

    /* Step 3: compute error from center */
    out.error_px = (sint16)(out.lane_center_px - (sint16)CAM_CENTER_PX);

    /* Step 4: intersection heuristic */
    if ((out.black_ratio_pct >= INTERSECTION_BLACK_RATIO_PCT) || (n >= 3u))
    {
        out.intersection_likely = TRUE;
    }

    return out;
}
