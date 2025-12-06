#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
 *===============================================================================================*/

#include "Std_Types.h"
#include "Icu.h"
#include "Dio.h"

/*==================================================================================================
 *                                      DEFINES & MACROS
 *===============================================================================================*/

/** Special value used when no valid distance is available. */
#define ULTRASONIC_DISTANCE_INVALID_CM   ((uint16)0xFFFFU)

/*==================================================================================================
 *                                      TYPEDEFS
 *===============================================================================================*/

/**
 * @brief Configuration for one HC-SR04 ultrasonic sensor.
 *
 * EchoChannel : ICU logical channel for ECHO (PTA15 / FTM1_CH2)
 * TrigChannel : Dio channel for TRIG (PTE15)
 * TimerFreqHz : FTM clock frequency used by ICU (ticks per second)
 */
typedef struct
{
    Icu_ChannelType  EchoChannel;
    Dio_ChannelType  TrigChannel;
    uint32           TimerFreqHz;
} Ultrasonic_ConfigType;

/* Global config instance (defined in ultrasonic.c) */
extern const Ultrasonic_ConfigType Ultrasonic_Config;

/*==================================================================================================
 *                                  GLOBAL FUNCTION PROTOTYPES
 *===============================================================================================*/

/**
 * @brief Initialize the ultrasonic driver.
 *
 * Must be called after the MCAL drivers (Icu, Dio) are initialized.
 * Typically from main(), after DriversInit().
 */
void Ultrasonic_Init(const Ultrasonic_ConfigType * ConfigPtr);

/**
 * @brief Start a new distance measurement (non-blocking).
 *
 * Generates a >=10 µs pulse on TRIG and starts the ICU signal
 * measurement on the ECHO channel.
 *
 * The result will be available later and can be queried using:
 *  - Ultrasonic_HasNewMeasurement()
 *  - Ultrasonic_GetLastDistanceCm()
 */
void Ultrasonic_TriggerMeasurement(void);

/**
 * @brief Returns TRUE if a new measurement has completed.
 *
 * This function internally polls the ICU measurement register and
 * updates the internal state if a high-time measurement has finished.
 */
boolean Ultrasonic_HasNewMeasurement(void);

/**
 * @brief Get the last measured distance in centimeters.
 *
 * Reading clears the internal "new measurement" flag.
 *
 * @return Distance in cm, or ULTRASONIC_DISTANCE_INVALID_CM if
 *         the last measurement was invalid or none is available.
 */
uint16 Ultrasonic_GetLastDistanceCm(void);

/**
 * @brief Perform a blocking measurement with timeout.
 *
 * Starts a measurement and polls until it finishes or timeoutMs expires.
 * Intended mainly for test modes (e.g. ULTRASONIC_TEST), not for normal
 * real-time driving logic.
 *
 * @param[out] distanceCm  Pointer where the distance (cm) is stored.
 * @param[in]  timeoutMs   Maximum time to wait in milliseconds.
 *
 * @return E_OK if a measurement completed before timeout, E_NOT_OK otherwise.
 */
Std_ReturnType Ultrasonic_MeasureBlocking(uint16 * distanceCm, uint16 timeoutMs);

#ifdef __cplusplus
}
#endif

#endif /* ULTRASONIC_H */
