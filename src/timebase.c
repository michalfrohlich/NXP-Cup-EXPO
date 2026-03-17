#include "timebase.h"
#include "Gpt.h"

/* ===================== Millisecond timebase (EmuTimer, GPT channel 2) ===================== */

/* Free-running millisecond tick */
volatile uint32 g_SystemMs = 0u;

/* GPT channel ID used for the ms tick (matches your Gpt config) */
#define MS_TIMER_CHANNEL   ((Gpt_ChannelType)2u)

/* Initialize the GPT timer that drives the millisecond timebase.
 * GPT channel 2 is configured in ConfigTools at 8 MHz tick frequency,
 * and we start it with a period of 8000 ticks → 1 ms.
 */
void Timebase_Init(void)
{
    /* 8 MHz -> 8000 ticks per ms */
    Gpt_StartTimer(MS_TIMER_CHANNEL, 8000u);
    Gpt_EnableNotification(MS_TIMER_CHANNEL);
}


/*
 * GPT notification for the emulator timer.
 * This function name MUST match the one configured in Gpt (EmuTimer_Notification).
 */
void EmuTimer_Notification(void)
{
    g_SystemMs++;   /* increments on every GPT tick (configured for 1 ms period) */
}

/* Return current ms tick */
uint32 Timebase_GetMs(void)
{
    return g_SystemMs;
}

/* Busy-wait delay in milliseconds using the ms tick */
void Timebase_DelayMs(uint32 delayMs)
{
    uint32 start = g_SystemMs;

    /* Wrap-safe because of unsigned arithmetic */
    while ((uint32)(g_SystemMs - start) < delayMs)
    {
        /* spin */
    }
}


/* ===================== Microsecond one-shot timer (UsTimer, GPT channel 3) ===================== */

/* GPT channel ID used for the microsecond one-shot LPIT timer.
 * In ConfigTools:
 *  - Name:          UsTimer
 *  - GptChannelId:  3
 *  - HW IP:         LPIT_0 / CH_2 (UsTimerChannel)
 *  - Mode:          GPT_CH_MODE_ONESHOT
 *  - Tick freq:     8 MHz (LPIT0 clock from generated config)
 */
#define US_TIMER_CHANNEL   ((Gpt_ChannelType)3u)

/* Flag set to TRUE when UsTimer elapsed (notification fired) */
volatile boolean g_UsTimerElapsed = FALSE;


/* Initialise UsTimer flag. Does NOT touch GPT hardware state.
 * Board_InitDrivers() has already called Gpt_Init() using the generated config.
 */
void UsTimer_Init(void)
{
    g_UsTimerElapsed = FALSE;
}


/* Start a one-shot UsTimer delay in microseconds.
 *
 * Pattern intentionally mirrors the camera shutter usage:
 *   - compute period in ticks
 *   - clear software flag
 *   - enable notification for this channel
 *   - start GPT in ONE_SHOT mode
 *
 * Stopping the timer and disabling the notification is handled by the
 * GPT driver / configuration (one-shot semantics) plus our notification,
 * similar to how the camera shutter GPT channel is used.
 */
void UsTimer_Start(uint32 delayUs)
{
    const uint32 ticksPerUs  = 8u;          /* 8 MHz -> 8 ticks per us */
    const uint32 periodTicks = delayUs * ticksPerUs;

    /* Prepare state */
    g_UsTimerElapsed = FALSE;

    /* Enable notification and start one-shot timer.
     * No explicit StopTimer/DisableNotification beforehand, mirroring
     * the LinearCamera shutter pattern.
     */
    Gpt_EnableNotification(US_TIMER_CHANNEL);
    Gpt_StartTimer(US_TIMER_CHANNEL, periodTicks);
}


/* GPT notification callback for UsTimer.
 * Configure this exact name in the Gpt channel (UsTimer_Notification).
 *
 * This is called once when the one-shot period expires.
 */
void UsTimer_Notification(void)
{
    /* Mark elapsed so application code can react (e.g., switch GPIO state) */
    g_UsTimerElapsed = TRUE;

    /* In ONE_SHOT mode the hardware timer stops automatically.
     * We deliberately do NOT call StopTimer/DisableNotification here,
     * matching the simple pattern used by the camera shutter GPT channel.
     * If you want to be defensive, you can add:
     *
     *   Gpt_DisableNotification(US_TIMER_CHANNEL);
     *
     * but it is not required for correct one-shot operation.
     */
}


/* Poll whether the UsTimer one-shot has elapsed */
boolean UsTimer_HasElapsed(void)
{
    return g_UsTimerElapsed;
}

/* Clear the elapsed flag (after handling) */
void UsTimer_ClearFlag(void)
{
    g_UsTimerElapsed = FALSE;
}
