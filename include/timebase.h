#ifndef TIMEBASE_H
#define TIMEBASE_H

#include "Std_Types.h"

/* ===================== Millisecond timebase (EmuTimer, GPT channel 2) ===================== */

/* Global millisecond counter, incremented by GPT callback */
extern volatile uint32 g_SystemMs;

/* Flag used by the camera emulator: set every 1000 ms */
extern volatile boolean g_EmuNewFrameFlag;

/* Initialize GPT-based ms timebase (start timer and enable notification) */
void Timebase_Init(void);

/* GPT notification callback for the ms timer
 * (name must match configuration in Gpt: EmuTimer_Notification).
 */
void EmuTimer_Notification(void);

/* Simple API to read the current ms tick */
uint32 Timebase_GetMs(void);

/* Coarse blocking delay using the ms tick */
void Timebase_DelayMs(uint32 delayMs);


/* ===================== Microsecond one-shot timer (UsTimer, GPT channel 3) ===================== */

/* One-shot UsTimer API:
 *  - Call UsTimer_Init() once after DriversInit() to reset internal flag.
 *  - Call UsTimer_Start(delayUs) to start a one-shot delay.
 *  - When the delay expires, UsTimer_Notification() is called by GPT
 *    (configured in Gpt as the channel's notification).
 *  - Application code can poll UsTimer_HasElapsed() and clear the flag
 *    using UsTimer_ClearFlag() after handling the event.
 */

/* Flag that becomes TRUE when the UsTimer one-shot has elapsed. */
extern volatile boolean g_UsTimerElapsed;

/* Initialise UsTimer flag (does not touch hardware state). */
void UsTimer_Init(void);

/* Start a one-shot UsTimer delay in microseconds. */
void UsTimer_Start(uint32 delayUs);

/* GPT notification callback for UsTimer (configured in Gpt as UsTimer_Notification). */
void UsTimer_Notification(void);

/* Poll whether the UsTimer one-shot has elapsed (flag set by notification). */
boolean UsTimer_HasElapsed(void);

/* Clear the elapsed flag (after your code has reacted to the event). */
void UsTimer_ClearFlag(void);

#endif /* TIMEBASE_H */
