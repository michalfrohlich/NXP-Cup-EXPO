#ifndef STEERING_SMOOTHING_H
#define STEERING_SMOOTHING_H

#include "Platform_Types.h"

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
