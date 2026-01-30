#ifndef BRAKING_H
#define BRAKING_H

#include "Platform_Types.h"
#include "ultrasonic.h"

/* Braking state for display + control */
typedef enum
{
    BRAKING_STATE_GO = 0,
    BRAKING_STATE_BRAKE
} Braking_StateType;

/* Configuration for the combined ultrasonic+braking module */
typedef struct
{
    uint32 triggerPeriodMs;     /* e.g. 60..100ms recommended, or 500ms for demo */
    float  brakeThresholdCm;    /* e.g. 10.0 */
    float  releaseThresholdCm;  /* e.g. 12.0 (hysteresis) */
    uint8  cruiseSpeed;         /* speed when GO */
} Braking_ConfigType;

/* Initialize module (also calls Ultrasonic_Init internally) */
void Braking_Init(const Braking_ConfigType *cfg);

/* Call periodically in the main loop (non-blocking) */
void Braking_Task(void);

/* ---- Accessors for UI/debug ---- */
Braking_StateType     Braking_GetState(void);
Ultrasonic_StatusType Braking_GetUltraStatus(void);

float  Braking_GetLastDistanceCm(void);
uint8  Braking_HasValidDistance(void);

uint32 Braking_GetUltraIrqCount(void);
uint32 Braking_GetUltraTicks(void);

#endif /* BRAKING_H */
