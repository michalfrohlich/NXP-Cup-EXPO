#include "braking.h"
#include "timebase.h"
#include "hbridge.h"

/* ---------------- Internal state ---------------- */

static Braking_ConfigType g_cfg;

static volatile Braking_StateType     g_brakeState  = BRAKING_STATE_GO;
static volatile Ultrasonic_StatusType g_ultraStatus = ULTRA_STATUS_IDLE;

static volatile float  g_lastDistanceCm   = 0.0f;
static volatile uint8  g_hasValidDistance = 0u;

static volatile uint32 g_lastTrigMs = 0u;

static volatile uint32 g_ultraIrqCount = 0u;
static volatile uint32 g_ultraTicks    = 0u;

/* Track current brake output to avoid spamming the driver */
static volatile uint8 g_brakeOutputActive = 0u;

/* ---------------- Public API ---------------- */

void Braking_Init(const Braking_ConfigType *cfg)
{
    /* Basic defaults if cfg is NULL */
    g_cfg.triggerPeriodMs    = 100u;
    g_cfg.brakeThresholdCm   = 10.0f;
    g_cfg.releaseThresholdCm = 12.0f;
    g_cfg.cruiseSpeed        = 50u;

    if (cfg != (const Braking_ConfigType*)0)
    {
        g_cfg = *cfg;
    }

    g_brakeState        = BRAKING_STATE_GO;
    g_ultraStatus       = ULTRA_STATUS_IDLE;
    g_lastDistanceCm    = 0.0f;
    g_hasValidDistance  = 0u;

    g_lastTrigMs        = Timebase_GetMs();

    g_ultraIrqCount     = 0u;
    g_ultraTicks        = 0u;

    g_brakeOutputActive = 0u;

    /* Own the ultrasonic module here */
    Ultrasonic_Init();

    /* Ensure brake is released at startup */
    HbridgeSetBrake(0U);
    g_brakeOutputActive = 0u;

    /* Start in GO */
    HbridgeSetSpeed(g_cfg.cruiseSpeed);
}

void Braking_Task(void)
{
    uint32 nowMs = Timebase_GetMs();

    /* 1) Periodic trigger (don’t interrupt BUSY) */
    if ((uint32)(nowMs - g_lastTrigMs) >= g_cfg.triggerPeriodMs)
    {
        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            g_lastTrigMs = nowMs;
            Ultrasonic_StartMeasurement();
        }
    }

    /* 2) Service ultrasonic driver (completion + timeout) */
    Ultrasonic_Task();

    /* 3) Snapshot status + diagnostics for UI */
    g_ultraStatus   = Ultrasonic_GetStatus();
    g_ultraIrqCount = Ultrasonic_GetIrqCount();
    g_ultraTicks    = Ultrasonic_GetLastHighTicks();

    /* 4) Consume new distance (only when available) */
    float d;
    if (Ultrasonic_GetDistanceCm(&d) == TRUE)
    {
        g_lastDistanceCm   = d;
        g_hasValidDistance = 1u;
    }

    /* 5) Braking decision (with hysteresis) */
    if (g_hasValidDistance != 0u)
    {
        if (g_brakeState == BRAKING_STATE_GO)
        {
            if ((g_lastDistanceCm > 0.0f) &&
                (g_lastDistanceCm <= g_cfg.brakeThresholdCm))
            {
                g_brakeState = BRAKING_STATE_BRAKE;
            }
        }
        else /* BRAKING_STATE_BRAKE */
        {
            if (g_lastDistanceCm >= g_cfg.releaseThresholdCm)
            {
                g_brakeState = BRAKING_STATE_GO;
            }
        }
    }

    /* 6) Actuate motors (IMPORTANT: use HbridgeSetBrake for real stop) */
    if (g_brakeState == BRAKING_STATE_BRAKE)
    {
        if (g_brakeOutputActive == 0u)
        {
            HbridgeSetBrake(1U);          /* true brake mode */
            g_brakeOutputActive = 1u;
        }
        /* No HbridgeSetSpeed() here: speed is locked out while brake is active */
    }
    else
    {
        if (g_brakeOutputActive != 0u)
        {
            HbridgeSetBrake(0U);          /* release brake */
            g_brakeOutputActive = 0u;
        }

        HbridgeSetSpeed(g_cfg.cruiseSpeed);
    }
}

/* ---------------- Accessors ---------------- */

Braking_StateType Braking_GetState(void)
{
    return g_brakeState;
}

Ultrasonic_StatusType Braking_GetUltraStatus(void)
{
    return g_ultraStatus;
}

float Braking_GetLastDistanceCm(void)
{
    return (float)g_lastDistanceCm;
}

uint8 Braking_HasValidDistance(void)
{
    return (uint8)g_hasValidDistance;
}

uint32 Braking_GetUltraIrqCount(void)
{
    return (uint32)g_ultraIrqCount;
}

uint32 Braking_GetUltraTicks(void)
{
    return (uint32)g_ultraTicks;
}
