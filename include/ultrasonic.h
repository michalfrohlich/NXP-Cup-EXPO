#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "Std_Types.h"
#include "Dio.h"
#include "Icu.h"
#include "timebase.h"

/* Channels */
#define ULTRA_DIO_TRIG_CHANNEL    (DioConf_DioChannel_PTE15_UltraTrig)
#define ULTRA_ICU_ECHO_CHANNEL    (IcuConf_IcuChannel_Ultrasonic_Echo)

/* Timestamp */
#define ULTRA_TS_BUF_SIZE         (2u)
#define ULTRA_TS_NOTIFY_INTERVAL  (1u)

/* Tick frequency (still needs calibration later if distance scaling is wrong) */
#define ULTRA_FTM_TICK_HZ         (2000000u) /* Tick frequency: SIRC (8MHz) / Prescaler (4) = 2MHz */
#define ULTRA_CM_PER_TICK         (34300.0f / (2.0f * (float)ULTRA_FTM_TICK_HZ))

#define ULTRA_MIN_DISTANCE_CM     (3.0f) //if ticks convert to LESS than this then trigger ERROR state
#define ULTRA_MAX_DISTANCE_CM     (400.0f) //if ticks convert to MORE than this then trigger ERROR state

/* 25ms timeout: (25ms * 34300 cm/s) / 2 = 428cm max detection.
   At 2MHz, this is 50,000 ticks (fits in 16-bit FTM). */
#define ULTRA_TIMEOUT_MS  (25u)

typedef enum
{
    ULTRA_STATUS_IDLE = 0,
    ULTRA_STATUS_BUSY,
    ULTRA_STATUS_NEW_DATA,
    ULTRA_STATUS_ERROR
} Ultrasonic_StatusType;

void Ultrasonic_Init(void);
void Ultrasonic_StartMeasurement(void);
void Ultrasonic_Task(void);
Ultrasonic_StatusType Ultrasonic_GetStatus(void);
boolean Ultrasonic_GetDistanceCm(float *distanceCm);

/* Debug getters */
uint32 Ultrasonic_GetLastHighTicks(void);
uint32 Ultrasonic_GetIrqCount(void);
uint16 Ultrasonic_GetTsIndex(void);   /* NEW: shows whether timestamps are filling */

/* Implement EXACTLY the callback name selected in ConfigTools.
 * If ConfigTools expects Icu_TimestampUltrasonicNotification, implement that symbol here.
 */
void Icu_TimestampUltrasonicNotification(void);

#endif /* ULTRASONIC_H */
