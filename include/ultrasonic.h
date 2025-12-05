#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "Std_Types.h"
#include "Dio.h"
#include "Icu.h"

/* Replace these with the actual generated names */
#define ULTRA_TRIG_CH   DioConf_DioChannel_PTE15_UltraTrig
#define ULTRA_ECHO_CH   IcuConf_IcuChannel_Ultrasonic_Echo

void Ultrasonic_Init(void);

/* Send a measurement pulse; result will be ready later */
void Ultrasonic_Trigger(void);

/* Returns TRUE if new distance is available and writes it in cm */
boolean Ultrasonic_GetDistanceCm(float32 *distanceCm);

#endif /* ULTRASONIC_H */
