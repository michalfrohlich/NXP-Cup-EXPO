#include "services/imu_motion.h"

#define DEG_TO_RAD_F  (0.0174532925f)
#define GRAVITY_MPS2  (9.80665f)

void ImuMotion_FromPacket(const TeensyImuPacket_t *packet, ImuMotionSample_t *out)
{
    if ((packet == NULL_PTR) || (out == NULL_PTR))
    {
        return;
    }

    out->axG = (float)packet->axMg * 0.001f;
    out->ayG = (float)packet->ayMg * 0.001f;
    out->azG = (float)packet->azMg * 0.001f;
    out->gxDps = (float)packet->gxDps10 * 0.1f;
    out->gyDps = (float)packet->gyDps10 * 0.1f;
    out->gzDps = (float)packet->gzDps10 * 0.1f;
    out->rollDeg = (float)packet->rollCdeg * 0.01f;
    out->pitchDeg = (float)packet->pitchCdeg * 0.01f;
    out->yawDeg = (float)packet->yawCdeg * 0.01f;
    out->accelNormG = (float)packet->accelNormMg * 0.001f;
    out->lateralG = (float)packet->lateralMg * 0.001f;
    out->tempC = (float)packet->tempC10 * 0.1f;
}

float ImuMotion_EstimateSlipG(const ImuMotionSample_t *sample, float speedMps)
{
    float yawRateRadS;
    float expectedLatMps2;
    float measuredLatMps2;

    if (sample == NULL_PTR)
    {
        return 0.0f;
    }

    yawRateRadS = sample->gzDps * DEG_TO_RAD_F;
    expectedLatMps2 = speedMps * yawRateRadS;
    measuredLatMps2 = sample->lateralG * GRAVITY_MPS2;

    return (measuredLatMps2 - expectedLatMps2) / GRAVITY_MPS2;
}
