#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "Std_Types.h"
#include "Dio.h"
#include "Icu.h"
#include "drivers/timebase.h"
#include "config/board_config.h"
#include "config/sensor_config.h"

/* Timestamp */
#define ULTRA_TS_BUF_SIZE         (2u)
#define ULTRA_TS_NOTIFY_INTERVAL  (1u)

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
