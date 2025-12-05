#include "ultrasonic.h"

/* Globals accessed in callback + main */
static volatile uint32 g_ultraRiseTicks = 0U;
static volatile uint32 g_ultraFallTicks = 0U;
static volatile boolean g_ultraNewMeasurement = FALSE;
static volatile boolean g_ultraWaitingForRising = TRUE;

/* --- Small local delay for the 10 us pulse ---
   You can later replace this with a proper GPT-based delay if you want. */
static void Ultra_DelayUs(uint32 us)
{
    volatile uint32 i;
    /* This constant depends on CPU freq; adjust if needed.
       For 80 MHz, ~8–10 NOPs per microsecond is in the right ballpark. */
    for (i = 0U; i < (us * 10U); i++)
    {
        __asm("NOP");
    }
}

/* Call this from your init sequence after Icu_Init & Dio_Init */
void Ultrasonic_Init(void)
{
    g_ultraNewMeasurement = FALSE;
    g_ultraWaitingForRising = TRUE;

    /* Start ICU on rising edge first */
    Icu_SetActivationCondition(ULTRA_ECHO_CH, ICU_RISING_EDGE);
    Icu_EnableEdgeDetection(ULTRA_ECHO_CH);
}

/* One-shot trigger pulse of >= 10 us on TRIG pin */
void Ultrasonic_Trigger(void)
{
    g_ultraNewMeasurement = FALSE;
    g_ultraWaitingForRising = TRUE;

    Icu_SetActivationCondition(ULTRA_ECHO_CH, ICU_RISING_EDGE);

    Dio_WriteChannel(ULTRA_TRIG_CH, STD_LOW);
    Ultra_DelayUs(2U);

    Dio_WriteChannel(ULTRA_TRIG_CH, STD_HIGH);
    Ultra_DelayUs(12U);        /* >= 10 us */
    Dio_WriteChannel(ULTRA_TRIG_CH, STD_LOW);
}

/* This is the notification you configured in the Icu channel: Ultrasonic_IcuNotification */
void Ultrasonic_IcuNotification(void)
{
    Icu_ValueType ticks = Icu_GetInputState(ULTRA_ECHO_CH);
    /* Depending on RTD version you might need Icu_GetCaptureRegisterValue(ULTRA_ECHO_CH) instead. */

    if (g_ultraWaitingForRising == TRUE)
    {
        /* Rising edge -> start of echo pulse */
        g_ultraRiseTicks = ticks;
        g_ultraWaitingForRising = FALSE;
        Icu_SetActivationCondition(ULTRA_ECHO_CH, ICU_FALLING_EDGE);
    }
    else
    {
        /* Falling edge -> end of echo pulse */
        g_ultraFallTicks = ticks;
        g_ultraNewMeasurement = TRUE;
        g_ultraWaitingForRising = TRUE;
        Icu_SetActivationCondition(ULTRA_ECHO_CH, ICU_RISING_EDGE);
    }
}

/* Convert tick difference to distance in cm.
   You must set ULTRA_TICK_TIME_US according to your FTM2 clock + prescaler. */
boolean Ultrasonic_GetDistanceCm(float32 *distanceCm)
{
    if ((distanceCm == NULL_PTR) || (g_ultraNewMeasurement == FALSE))
    {
        return FALSE;
    }

    uint32 diffTicks;

    if (g_ultraFallTicks >= g_ultraRiseTicks)
    {
        diffTicks = g_ultraFallTicks - g_ultraRiseTicks;
    }
    else
    {
        /* Handle 16/24-bit counter wrap; conservative 32-bit wrap here */
        diffTicks = (0xFFFFFFFFUL - g_ultraRiseTicks) + g_ultraFallTicks + 1UL;
    }

    g_ultraNewMeasurement = FALSE;

    /* === IMPORTANT: set this based on FTM2 config ===
       If you configure FTM2 for 1 MHz in the Icu instance, then:
       ULTRA_TICK_TIME_US = 1.0f; (1 tick = 1 microsecond) */
    const float32 ULTRA_TICK_TIME_US = 1.0f; /* adjust once you know FTM2 tick time */

    float32 timeUs = (float32)diffTicks * ULTRA_TICK_TIME_US;

    /* HC-SR04: distance (cm) = time (us) * 0.0343 / 2 */
    *distanceCm = (timeUs * 0.0343f) * 0.5f;

    return TRUE;
}
