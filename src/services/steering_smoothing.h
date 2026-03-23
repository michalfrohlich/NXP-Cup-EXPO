#ifndef STEERING_SMOOTHING_H
#define STEERING_SMOOTHING_H

#include "Platform_Types.h"

/* Steering smoothing approach used for the camera path:
   1) dead-zone around center to ignore tiny camera jitter
   2) command shaping so small corrections are softer than large ones
   3) slew-rate limiting so the servo output cannot jump too fast

   Reference ideas:
   - MathWorks Simulink "Dead Zone" block
   - MathWorks Simulink "Rate Limiter" block
   - WPILib "SlewRateLimiter" docs

   Practical tuning notes live in src/app/steering_tuning_notes.md. */

sint16 SteeringSmooth_ClampS16(sint16 x, sint16 lo, sint16 hi);
sint16 SteeringSmooth_IirS16(sint16 yPrev, sint16 x, float alpha);
sint16 SteeringSmooth_ShapeS16(sint16 cmd, sint16 clamp, sint16 blendPctCfg);
sint16 SteeringSmooth_DeadzoneS16(sint16 cmd, sint16 deadband, sint16 clamp);
sint16 SteeringSmooth_RateLimitS16(sint16 current,
                                   sint16 target,
                                   sint16 baseRateMax,
                                   sint16 clamp,
                                   sint16 rateBoostDiv,
                                   sint16 rateBoostMax);

#endif
