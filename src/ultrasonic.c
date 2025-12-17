#include "ultrasonic.h"

static volatile Ultrasonic_StatusType g_ultraStatus         = ULTRA_STATUS_IDLE;
static volatile float                 g_ultraLastDistanceCm = 0.0f;

static volatile uint32                g_ultraIrqCount        = 0u;
static volatile uint32                g_ultraLastTicks       = 0u;
static volatile uint32                g_ultraStartTimeMs     = 0u;

static volatile Icu_ValueType         g_ultraTsBuf[ULTRA_TS_BUF_SIZE];

static void Ultra_BusyDelayUs(uint32 microseconds)
{
    if (microseconds == 0u) { return; }

    UsTimer_Start(microseconds);
    while (UsTimer_HasElapsed() == FALSE) { }
    UsTimer_ClearFlag();
}

static void Ultrasonic_SendTriggerPulse(void)
{
    (void)Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_LOW);
    Ultra_BusyDelayUs(5u);

    (void)Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_HIGH);
    Ultra_BusyDelayUs(20u);

    (void)Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_LOW);
}

static void Ultrasonic_TryCompleteFromBuffer(void)
{
    Icu_IndexType idx = Icu_GetTimestampIndex(ULTRA_ICU_ECHO_CHANNEL);

    if (idx >= 2u)
    {
        uint32 t0 = (uint32)g_ultraTsBuf[0];
        uint32 t1 = (uint32)g_ultraTsBuf[1];

        uint32 ticks = (uint32)(t1 - t0); /* wrap-safe unsigned subtraction */
        g_ultraLastTicks = ticks;

        Icu_StopTimestamp(ULTRA_ICU_ECHO_CHANNEL);

        float distanceCm = ((float)ticks) * ULTRA_CM_PER_TICK;
        g_ultraLastDistanceCm = distanceCm;

        if ((distanceCm < ULTRA_MIN_DISTANCE_CM) || (distanceCm > ULTRA_MAX_DISTANCE_CM))
        {
            g_ultraStatus = ULTRA_STATUS_ERROR;
        }
        else
        {
            g_ultraStatus = ULTRA_STATUS_NEW_DATA;
        }
    }
}

void Ultrasonic_Init(void)
{
    g_ultraStatus          = ULTRA_STATUS_IDLE;
    g_ultraLastDistanceCm  = 0.0f;
    g_ultraIrqCount         = 0u;
    g_ultraLastTicks        = 0u;
    g_ultraStartTimeMs      = 0u;

    (void)Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_LOW);

    UsTimer_Init();

    /* Enable notifications for the channel */
    Icu_EnableNotification(ULTRA_ICU_ECHO_CHANNEL);
}

void Ultrasonic_StartMeasurement(void)
{
    if (g_ultraStatus == ULTRA_STATUS_BUSY)
    {
        return;
    }

    g_ultraStatus      = ULTRA_STATUS_BUSY;
    g_ultraLastTicks   = 0u;
    g_ultraStartTimeMs = Timebase_GetMs();

    for (uint32 i = 0u; i < ULTRA_TS_BUF_SIZE; i++)
    {
        g_ultraTsBuf[i] = 0u;
    }

    /* Force edge config at runtime (removes UI ambiguity) */
    Icu_SetActivationCondition(ULTRA_ICU_ECHO_CHANNEL, ICU_BOTH_EDGES);


    Icu_StartTimestamp(
        ULTRA_ICU_ECHO_CHANNEL,
        (Icu_ValueType*)g_ultraTsBuf,
        (uint16)ULTRA_TS_BUF_SIZE,
        (uint16)ULTRA_TS_NOTIFY_INTERVAL
    );

    Ultrasonic_SendTriggerPulse();
}

void Ultrasonic_Task(void)
{
    if (g_ultraStatus == ULTRA_STATUS_BUSY)
    {
        /* If timestamps are filling but callback isn’t firing, still complete */
        Ultrasonic_TryCompleteFromBuffer();

        uint32 elapsedMs = Timebase_GetMs() - g_ultraStartTimeMs;
        if (elapsedMs >= ULTRA_TIMEOUT_MS)
        {
            Icu_StopTimestamp(ULTRA_ICU_ECHO_CHANNEL);
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
        g_ultraStatus = ULTRA_STATUS_IDLE;
        return TRUE;
    }

    return FALSE;
}

uint32 Ultrasonic_GetLastHighTicks(void)
{
    return (uint32)g_ultraLastTicks;
}

uint32 Ultrasonic_GetIrqCount(void)
{
    return (uint32)g_ultraIrqCount;
}

uint16 Ultrasonic_GetTsIndex(void)
{
    return (uint16)Icu_GetTimestampIndex(ULTRA_ICU_ECHO_CHANNEL);
}

/* IMPORTANT: this function name MUST match the one referenced in generated config */
void Icu_TimestampUltrasonicNotification(void)
{
    g_ultraIrqCount++;

    if (g_ultraStatus != ULTRA_STATUS_BUSY)
    {
        return;
    }

    Ultrasonic_TryCompleteFromBuffer();
}
