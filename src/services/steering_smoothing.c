#include "steering_smoothing.h"

static sint16 abs_s16(sint16 x)
{
    return (x < 0) ? (sint16)(-x) : x;
}

sint16 SteeringSmooth_ClampS16(sint16 x, sint16 lo, sint16 hi)
{
    if (x < lo) { return lo; }
    if (x > hi) { return hi; }
    return x;
}

sint16 SteeringSmooth_IirS16(sint16 yPrev, sint16 x, float alpha)
{
    float y;

    if (alpha < 0.0f) { alpha = 0.0f; }
    if (alpha > 1.0f) { alpha = 1.0f; }

    y = (float)yPrev + alpha * ((float)x - (float)yPrev);

    if (y > 32767.0f)  { y = 32767.0f; }
    if (y < -32768.0f) { y = -32768.0f; }

    return (sint16)y;
}

sint16 SteeringSmooth_ShapeS16(sint16 cmd, sint16 clamp, sint16 blendPctCfg)
{
    sint32 absCmd;
    sint32 quad;
    sint32 shaped;
    sint32 blendPct = (sint32)blendPctCfg;

    if (clamp <= 0)
    {
        return 0;
    }

    if (blendPct < 0)
    {
        blendPct = 0;
    }
    if (blendPct > 100)
    {
        blendPct = 100;
    }

    absCmd = (sint32)abs_s16(cmd);
    if (absCmd > (sint32)clamp)
    {
        absCmd = (sint32)clamp;
    }

    quad = (absCmd * absCmd) / (sint32)clamp;
    shaped = (((100L - blendPct) * absCmd) + (blendPct * quad)) / 100L;

    if (shaped > absCmd)
    {
        shaped = absCmd;
    }

    return (cmd < 0) ? (sint16)(-shaped) : (sint16)shaped;
}

sint16 SteeringSmooth_DeadzoneS16(sint16 cmd, sint16 deadband, sint16 clamp)
{
    sint32 absCmd;
    sint32 scaled;

    if (clamp <= 0)
    {
        return 0;
    }

    if (deadband <= 0)
    {
        return SteeringSmooth_ClampS16(cmd, (sint16)(-clamp), clamp);
    }

    if (deadband >= clamp)
    {
        return 0;
    }

    absCmd = (sint32)abs_s16(cmd);
    if (absCmd <= (sint32)deadband)
    {
        return 0;
    }

    if (absCmd > (sint32)clamp)
    {
        absCmd = (sint32)clamp;
    }

    /* Real dead-zone with rescaling: keeps a wider zero region near center
       without causing a sharp jump right after the threshold. This keeps the
       servo smooth near straight while still letting larger commands build. */
    scaled = ((absCmd - (sint32)deadband) * (sint32)clamp) /
             ((sint32)clamp - (sint32)deadband);

    if (scaled > (sint32)clamp)
    {
        scaled = (sint32)clamp;
    }

    return (cmd < 0) ? (sint16)(-scaled) : (sint16)scaled;
}

sint16 SteeringSmooth_RateLimitS16(sint16 current,
                                   sint16 target,
                                   sint16 baseRateMax,
                                   sint16 clamp,
                                   sint16 rateBoostDiv,
                                   sint16 rateBoostMax)
{
    sint16 boost;
    sint16 rateMax;
    sint16 delta;

    if (baseRateMax < 0)
    {
        baseRateMax = 0;
    }

    if ((rateBoostDiv <= 0) || (rateBoostMax <= 0))
    {
        boost = 0;
    }
    else
    {
        boost = (sint16)(abs_s16(target) / rateBoostDiv);
        if (boost > rateBoostMax)
        {
            boost = rateBoostMax;
        }
    }

    rateMax = (sint16)(baseRateMax + boost);
    if (rateMax > clamp)
    {
        rateMax = clamp;
    }

    delta = (sint16)(target - current);
    delta = SteeringSmooth_ClampS16(delta, (sint16)(-rateMax), rateMax);

    return (sint16)(current + delta);
}
