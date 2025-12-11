#include "ultrasonic.h"
#include "timebase.h"


/* Internal state */
static volatile Ultrasonic_StatusType g_ultraStatus         = ULTRA_STATUS_IDLE;
static volatile float                 g_ultraLastDistanceCm = 0.0f;

/* Debug helpers if you want to watch them in the debugger */
static volatile uint32                g_ultraIrqCount       = 0u;
static volatile uint32                g_ultraLastTicks      = 0u;

/* For timeout handling */
static volatile uint32                g_ultraStartTimeMs    = 0u;

/* =========================================================================
 * Local helpers
 * ========================================================================= */
/* Very simple crude delay in microseconds.
 * Assumes core clock is high enough (e.g. 80 MHz); exact timing is not critical
 * for a 10 µs TRIG pulse.
 */
void Ultra_BusyDelayUs(uint32 microseconds)
{
    volatile uint32 inner;
    while (microseconds > 0u)
    {
        /* Tune this constant if you want:
         * - Larger value = longer delay per µs
         * Start with 10–20 and adjust based on oscilloscope if needed.
         */
        inner = 20u;
        while (inner > 0u)
        {
            __asm("NOP");
            inner--;
        }
        microseconds--;
    }
}

static void Ultrasonic_SendTriggerPulse(void)
{
    /* Make sure TRIG is low first */
    (void)Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_LOW);
    ULTRA_DELAY_US(2u);

    /* 10 µs high pulse */
    (void)Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_HIGH);
    ULTRA_DELAY_US(10u);
    (void)Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_LOW);
}

/* =========================================================================
 * Public API
 * ========================================================================= */
void Ultrasonic_Init(void)
{
    g_ultraStatus         = ULTRA_STATUS_IDLE;
    g_ultraLastDistanceCm = 0.0f;
    g_ultraIrqCount       = 0u;
    g_ultraLastTicks      = 0u;
    g_ultraStartTimeMs    = 0u;

    /* Ensure TRIG is low at startup */
    (void)Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_LOW);

    /* Defensive: ensure ICU is in NORMAL mode */
    //Icu_SetMode(ICU_MODE_NORMAL); - this was not recognized

    /* Runtime enable of notification for the echo channel.
     * ConfigTools must also:
     *   - select Signal Measurement for this channel
     *   - select HIGH_TIME as property
     *   - set Notification = Icu_UltrasonicNotification
     *   - enable IcuIsrEnable for that channel
     */
    Icu_EnableNotification(ULTRA_ICU_ECHO_CHANNEL);
}

void Ultrasonic_StartMeasurement(void)
{
    /* Reset state */
    g_ultraStatus      = ULTRA_STATUS_BUSY;
    g_ultraLastTicks   = 0u;
    g_ultraStartTimeMs = Timebase_GetMs();

    /* Arm ICU for HIGH_TIME measurement on echo channel.
     * Mode/property come from ConfigTools.
     */
    Icu_StartSignalMeasurement(ULTRA_ICU_ECHO_CHANNEL);

    /* Fire TRIG pulse (sensor sends burst and then returns echo) */
    Ultrasonic_SendTriggerPulse();
}

/* Call this regularly from your main loop (e.g. every 1–5 ms).
 * It checks for timeout while BUSY and converts it to ERROR.
 */
void Ultrasonic_Task(void)
{
    if (g_ultraStatus == ULTRA_STATUS_BUSY)
    {
        uint32 nowMs     = Timebase_GetMs();
        uint32 elapsedMs = nowMs - g_ultraStartTimeMs;  /* wrap-safe for uint32 */

        if (elapsedMs >= ULTRA_TIMEOUT_MS)
        {
            /* No echo within timeout -> stop measurement and flag ERROR */
            Icu_StopSignalMeasurement(ULTRA_ICU_ECHO_CHANNEL);
            g_ultraStatus = ULTRA_STATUS_ERROR;
        }
    }
}

Ultrasonic_StatusType Ultrasonic_GetStatus(void)
{
    return g_ultraStatus;
}

boolean Ultrasonic_GetDistanceCm(float *distanceCm)
{
    if (distanceCm == NULL_PTR)
    {
        return FALSE;
    }

    if (g_ultraStatus == ULTRA_STATUS_NEW_DATA)
    {
        *distanceCm = g_ultraLastDistanceCm;
        g_ultraStatus = ULTRA_STATUS_IDLE; /* consume result */
        return TRUE;
    }
    else if (g_ultraStatus == ULTRA_STATUS_ERROR)
    {
        g_ultraStatus = ULTRA_STATUS_IDLE; /* clear error after reporting */
        return FALSE;
    }
    else
    {
        /* IDLE or BUSY: nothing to return yet */
        return FALSE;
    }
}

/* =========================================================================
 * ICU Notification callback
 * =========================================================================
 * - Called by the ICU driver when the HIGH_TIME measurement is complete.
 * - Uses Icu_GetTimeElapsed() (correct API for ICU_HIGH_TIME).
 * - Converts ticks to cm using ULTRA_CM_PER_TICK.
 */
void Icu_UltrasonicNotification(void)
{
    Icu_ValueType ticks;
    float         distanceCm;

    g_ultraIrqCount++;

    /* For ICU_HIGH_TIME, AUTOSAR SWS specifies Icu_GetTimeElapsed().
     * ConfigTools must have set Measurement Property = High Time.
     */
    ticks = Icu_GetTimeElapsed(ULTRA_ICU_ECHO_CHANNEL);
    g_ultraLastTicks = ticks;

    /* Stop measurement until next trigger */
    Icu_StopSignalMeasurement(ULTRA_ICU_ECHO_CHANNEL);

    /* Convert ticks -> distance [cm] using the configured FTM tick frequency */
    distanceCm = ((float)ticks) * ULTRA_CM_PER_TICK;
    g_ultraLastDistanceCm = distanceCm;

    /* Range check (5–200 cm). Values outside this are flagged as ERROR. */
    if ((distanceCm < ULTRA_MIN_DISTANCE_CM) || (distanceCm > ULTRA_MAX_DISTANCE_CM))
    {
        g_ultraStatus = ULTRA_STATUS_ERROR;
    }
    else
    {
        g_ultraStatus = ULTRA_STATUS_NEW_DATA;
    }
}
