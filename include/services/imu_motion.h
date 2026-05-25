#ifndef IMU_MOTION_H
#define IMU_MOTION_H

#include "Std_Types.h"
#include "drivers/teensy_imu.h"

typedef struct
{
    float axG;
    float ayG;
    float azG;
    float gxDps;
    float gyDps;
    float gzDps;
    float rollDeg;
    float pitchDeg;
    float yawDeg;
    float accelNormG;
    float lateralG;
    float tempC;
} ImuMotionSample_t;

/* Convert packet fixed-point units into normal engineering units. */
void ImuMotion_FromPacket(const TeensyImuPacket_t *packet, ImuMotionSample_t *out);

/* First slip metric for MATLAB/control work: measured lateral minus expected. */
float ImuMotion_EstimateSlipG(const ImuMotionSample_t *sample, float speedMps);

#endif /* IMU_MOTION_H */
