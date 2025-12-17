#if 0
#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "Std_Types.h"
#include "Dio.h"
#include "Icu.h"
#include "OsIf.h"

/* =========================================================================
 * Pin / channel mappings (from your ConfigTools setup)
 * =========================================================================
 * If your generated symbols are named differently (e.g. DioConf_DioChannel_…),
 * just change these two defines.
 */
#define ULTRA_DIO_TRIG_CHANNEL    (DioConf_DioChannel_PTE15_UltraTrig)         /* TRIG: PTE15 */
#define ULTRA_ICU_ECHO_CHANNEL    (IcuConf_IcuChannel_Ultrasonic_Echo) /* ECHO: PTA15 → FTM1_CH2 */

/* =========================================================================
 * Timer frequency and conversion
 *
 * ULTRA_FTM_TICK_HZ = FTM timer clock (Hz) used by ICU for this channel.
 * You said it's 8 MHz.
 *
 * distance_cm = ticks * (speed_of_sound_cm_per_s / (2 * FTM_tick_Hz))
 *             = ticks * (34300 / (2 * ULTRA_FTM_TICK_HZ))
 * ========================================================================= */
#define ULTRA_FTM_TICK_HZ         (8000000u)
#define ULTRA_CM_PER_TICK         (34300.0f / (2.0f * (float)ULTRA_FTM_TICK_HZ))

/* =========================================================================
 * Distance range and timeout
 * ========================================================================= */
#define ULTRA_MIN_DISTANCE_CM     (5.0f)
#define ULTRA_MAX_DISTANCE_CM     (200.0f)

/* HC-SR04-ish: ~58 µs/cm → 200 cm ≈ 11.6 ms. Use 12 ms timeout. */
#define ULTRA_TIMEOUT_MS          (12u)

typedef enum
{
    ULTRA_STATUS_IDLE = 0,   /* No measurement in progress, no new data */
    ULTRA_STATUS_BUSY,       /* Waiting for echo / ICU measurement */
    ULTRA_STATUS_NEW_DATA,   /* New valid distance available */
    ULTRA_STATUS_ERROR       /* Timeout or out-of-range (>200 cm or <5 cm) */
} Ultrasonic_StatusType;

void Ultra_BusyDelayUs(uint32 microseconds);
#define ULTRA_DELAY_US(_us_)   Ultra_BusyDelayUs((_us_))

/* Initialize internal state + enable ICU notification for echo channel.
 * Assumes Dio_Init() and Icu_Init() were already called.
 */
void Ultrasonic_Init(void);

/* Start a new measurement:
 * - Arms ICU on ULTRA_ICU_ECHO_CHANNEL (Signal Measurement, HIGH_TIME)
 * - Sends 10 µs TRIG pulse on ULTRA_DIO_TRIG_CHANNEL
 * Non-blocking: returns immediately.
 */
void Ultrasonic_StartMeasurement(void);

/* Must be called regularly from your main loop.
 * - Implements timeout: if BUSY for more than ULTRA_TIMEOUT_MS, it stops
 *   the ICU measurement and sets status = ULTRA_STATUS_ERROR.
 */
void Ultrasonic_Task(void);

/* Get current status (IDLE/BUSY/NEW_DATA/ERROR). */
Ultrasonic_StatusType Ultrasonic_GetStatus(void);

/* If status == NEW_DATA, returns TRUE and writes distance into *distanceCm.
 * On ERROR or if not ready, returns FALSE.
 * Calling this will clear NEW_DATA/ERROR back to IDLE.
 */
boolean Ultrasonic_GetDistanceCm(float *distanceCm);

/* ICU notification callback.
 * Configure this function name as the Notification for ULTRA_ICU_ECHO_CHANNEL
 * in S32 Design Studio ConfigTools (IcuIsrEnable: Icu_UltrasonicNotification).
 */
void Icu_UltrasonicNotification(void);

#endif /* ULTRASONIC_H */
#endif //commented older version

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "Std_Types.h"
#include "Dio.h"
#include "Icu.h"
#include "OsIf.h"

/* =========================================================================
 * Pin / channel mappings (from your ConfigTools setup)
 * =========================================================================
 * If your generated symbols are named differently (e.g. DioConf_DioChannel_…),
 * just change these two defines.
 */
#define ULTRA_DIO_TRIG_CHANNEL    (DioConf_DioChannel_PTE15_UltraTrig)         /* TRIG: PTE15 */
#define ULTRA_ICU_ECHO_CHANNEL    (IcuConf_IcuChannel_Ultrasonic_Echo)         /* ECHO: PTA15 → FTM1_CH2 */

/* =========================================================================
 * Timer frequency and conversion
 *
 * ULTRA_FTM_TICK_HZ = FTM timer clock (Hz) used by ICU for this channel.
 * You said it's 8 MHz.
 *
 * distance_cm = ticks * (speed_of_sound_cm_per_s / (2 * FTM_tick_Hz))
 *             = ticks * (34300 / (2 * ULTRA_FTM_TICK_HZ))
 * ========================================================================= */
#define ULTRA_FTM_TICK_HZ         (8000000u)
#define ULTRA_CM_PER_TICK         (34300.0f / (2.0f * (float)ULTRA_FTM_TICK_HZ))

/* =========================================================================
 * Distance range and timeout
 * ========================================================================= */
#define ULTRA_MIN_DISTANCE_CM     (5.0f)
#define ULTRA_MAX_DISTANCE_CM     (200.0f)

/* HC-SR04-ish: ~58 µs/cm → 200 cm ≈ 11.6 ms. Use 12 ms timeout. */
#define ULTRA_TIMEOUT_MS          (12u)

typedef enum
{
    ULTRA_STATUS_IDLE = 0,   /* No measurement in progress, no new data */
    ULTRA_STATUS_BUSY,       /* Waiting for echo / ICU measurement */
    ULTRA_STATUS_NEW_DATA,   /* New valid distance available */
    ULTRA_STATUS_ERROR       /* Timeout or out-of-range (>200 cm or <5 cm) */
} Ultrasonic_StatusType;

/* Busy delay in microseconds implemented using the UsTimer one-shot.
 * Requires:
 *  - DriversInit() (so Gpt_Init is done)
 *  - UsTimer_Init() to have been called at least once.
 */
void Ultra_BusyDelayUs(uint32 microseconds);

/* Convenience macro so existing code can keep using ULTRA_DELAY_US() */
#define ULTRA_DELAY_US(_us_)   Ultra_BusyDelayUs((_us_))

/* Initialize internal state + enable ICU notification for echo channel.
 * Assumes Dio_Init() and Icu_Init() were already called.
 * Also calls UsTimer_Init() to prepare the microsecond delay timer.
 */
void Ultrasonic_Init(void);

/* Start a new measurement:
 * - Arms ICU on ULTRA_ICU_ECHO_CHANNEL (Signal Measurement, HIGH_TIME)
 * - Sends 10 µs TRIG pulse on ULTRA_DIO_TRIG_CHANNEL
 * Non-blocking w.r.t. echo: returns immediately after firing TRIG.
 */
void Ultrasonic_StartMeasurement(void);

/* Must be called regularly from your main loop.
 * - Implements timeout: if BUSY for more than ULTRA_TIMEOUT_MS, it stops
 *   the ICU measurement and sets status = ULTRA_STATUS_ERROR.
 */
void Ultrasonic_Task(void);

/* Get current status (IDLE/BUSY/NEW_DATA/ERROR). */
Ultrasonic_StatusType Ultrasonic_GetStatus(void);

/* If status == NEW_DATA, returns TRUE and writes distance into *distanceCm.
 * On ERROR or if not ready, returns FALSE.
 * Calling this will clear NEW_DATA/ERROR back to IDLE.
 */
boolean Ultrasonic_GetDistanceCm(float *distanceCm);

/* ICU notification callback.
 * Configure this function name as the Notification for ULTRA_ICU_ECHO_CHANNEL
 * in S32 Design Studio ConfigTools (IcuIsrEnable: Icu_UltrasonicNotification).
 */
void Icu_UltrasonicNotification(void);

#endif /* ULTRASONIC_H */
