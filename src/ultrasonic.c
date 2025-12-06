#include "ultrasonic.h"
#include "Icu.h"
#include "Dio.h"
#include "Dio_Cfg.h"      /* for DioConf_DioChannel_PTE15_UltraTrig */
#include "Icu_Cfg.h"      /* for IcuConf_IcuChannel_Ultrasonic_Echo */

/*==================================================================================================
 *                                      LOCAL MACROS
 *===============================================================================================*/

/* Based on your GPT comment: 8000 ticks -> 1 ms => timer = 8 MHz.
 * Adjust this if your FTM1 ICU clock is different.
 */
#define ULTRASONIC_TIMER_FREQ_HZ          (8000000UL)

/* HC-SR04 trigger pulse width: >=10 µs; we use a small margin. */
#define ULTRASONIC_TRIG_PULSE_US          (15U)

/* Max distance we care about (for clamping, in cm) */
#define ULTRASONIC_MAX_DISTANCE_CM        (500U)

/* Polling step in the blocking helper (µs). Rough, doesn’t need to be exact. */
#define ULTRASONIC_BLOCKING_STEP_US       (1000U)   /* 1 ms */

/*==================================================================================================
 *                                  LOCAL VARIABLES
 *===============================================================================================*/

static const Ultrasonic_ConfigType * Ultrasonic_pConfig = NULL_PTR;

/* last measured echo pulse width (timer ticks) */
static volatile uint32  Ultrasonic_LastPulseTicks = 0UL;

/* last measured distance in cm */
static volatile uint16  Ultrasonic_LastDistanceCm = ULTRASONIC_DISTANCE_INVALID_CM;

/* new-value flag */
static volatile boolean Ultrasonic_NewMeasurementFlag = FALSE;

/* busy flag (between Trigger and measurement completion) */
static volatile boolean Ultrasonic_BusyFlag = FALSE;

/*==================================================================================================
 *                             LOCAL FUNCTION PROTOTYPES
 *===============================================================================================*/

static uint16 Ultrasonic_ConvertTicksToDistanceCm(uint32 ticks);
static void   Ultrasonic_UpdateMeasurement(void);
static void   Ultrasonic_DelayUs(uint32 us);

/*==================================================================================================
 *                                   GLOBAL CONFIG INSTANCE
 *===============================================================================================*/

/* Global config instance for your board.
 *  - EchoChannel : ICU logical channel for PTA15 / FTM1_CH2 (Ultrasonic_Echo)
 *  - TrigChannel : DIO channel for PTE15 (PTE15_UltraTrig)
 *  - TimerFreqHz : FTM1 clock frequency (8 MHz)
 *
 * If your generated names differ, adjust the two lines below.
 */
const Ultrasonic_ConfigType Ultrasonic_Config =
{
    /* EchoChannel */ IcuConf_IcuChannel_Ultrasonic_Echo,
    /* TrigChannel */ DioConf_DioChannel_PTE15_UltraTrig,
    /* TimerFreqHz */ ULTRASONIC_TIMER_FREQ_HZ
};

/*==================================================================================================
 *                                  LOCAL FUNCTIONS
 *===============================================================================================*/

/* Simple busy-wait delay in microseconds.
 * Used only for:
 *  - 15 µs TRIG pulse
 *  - ~1 ms steps in the blocking test helper
 */
static void Ultrasonic_DelayUs(uint32 us)
{
    volatile uint32 i;
    /* The constant (20U) is approximate; you can tune it if needed. */
    for (i = 0U; i < (us * 20U); i++)
    {
        __asm("NOP");
    }
}

/* Convert pulse width (ticks) to distance in cm. */
static uint16 Ultrasonic_ConvertTicksToDistanceCm(uint32 ticks)
{
    uint32 timeUs;
    uint32 freqHz;
    uint32 distanceCm;

    if ((ticks == 0UL) || (Ultrasonic_pConfig == NULL_PTR))
    {
        return ULTRASONIC_DISTANCE_INVALID_CM;
    }

    freqHz = Ultrasonic_pConfig->TimerFreqHz;
    if (freqHz == 0UL)
    {
        return ULTRASONIC_DISTANCE_INVALID_CM;
    }

    /* us = ticks * 1e6 / freq */
    timeUs = (ticks * 1000000UL) / freqHz;

    /* distance(cm) ≈ timeUs * 0.0343 / 2  ->  (timeUs * 343) / 20000 */
    distanceCm = (timeUs * 343UL + 20000UL / 2UL) / 20000UL;

    if (distanceCm > ULTRASONIC_MAX_DISTANCE_CM)
    {
        distanceCm = ULTRASONIC_MAX_DISTANCE_CM;
    }

    return (uint16)distanceCm;
}

/* Poll the ICU channel to see if the measurement has finished.
 * When done, fill LastPulseTicks/LastDistance and set NewMeasurementFlag.
 */
static void Ultrasonic_UpdateMeasurement(void)
{
    Icu_ValueType pulseTicks;

    if ((Ultrasonic_pConfig == NULL_PTR) || (Ultrasonic_BusyFlag == FALSE))
    {
        return;
    }

    /* For ICU_MODE_SIGNAL_MEASUREMENT + ICU_HIGH_TIME,
     * Icu_GetTimeElapsed() returns 0 until a high pulse was measured.
     */
    pulseTicks = Icu_GetTimeElapsed(Ultrasonic_pConfig->EchoChannel);

    if (pulseTicks == 0U)
    {
        /* Still measuring, nothing to do. */
        return;
    }

    Ultrasonic_LastPulseTicks = (uint32)pulseTicks;
    Ultrasonic_LastDistanceCm = Ultrasonic_ConvertTicksToDistanceCm(Ultrasonic_LastPulseTicks);

    Ultrasonic_NewMeasurementFlag = TRUE;
    Ultrasonic_BusyFlag           = FALSE;

    /* Optional: stop measurement until next trigger */
    Icu_StopSignalMeasurement(Ultrasonic_pConfig->EchoChannel);
}

/*==================================================================================================
 *                                  GLOBAL FUNCTIONS
 *===============================================================================================*/

void Ultrasonic_Init(const Ultrasonic_ConfigType * ConfigPtr)
{
    Ultrasonic_pConfig = ConfigPtr;

    Ultrasonic_LastPulseTicks     = 0UL;
    Ultrasonic_LastDistanceCm     = ULTRASONIC_DISTANCE_INVALID_CM;
    Ultrasonic_NewMeasurementFlag = FALSE;
    Ultrasonic_BusyFlag           = FALSE;

    if (Ultrasonic_pConfig != NULL_PTR)
    {
        /* Ensure ICU channel is stopped/idle */
        Icu_StopSignalMeasurement(Ultrasonic_pConfig->EchoChannel);

        /* Make sure TRIG starts low */
        Dio_WriteChannel(Ultrasonic_pConfig->TrigChannel, STD_LOW);
    }
}

void Ultrasonic_TriggerMeasurement(void)
{
    if (Ultrasonic_pConfig == NULL_PTR)
    {
        return;
    }

    if (Ultrasonic_BusyFlag == TRUE)
    {
        /* Previous measurement not finished yet */
        return;
    }

    Ultrasonic_NewMeasurementFlag = FALSE;
    Ultrasonic_LastPulseTicks     = 0UL;
    Ultrasonic_LastDistanceCm     = ULTRASONIC_DISTANCE_INVALID_CM;
    Ultrasonic_BusyFlag           = TRUE;

    /* Start hardware high-time measurement on the ECHO pin */
    Icu_StopSignalMeasurement(Ultrasonic_pConfig->EchoChannel);
    Icu_StartSignalMeasurement(Ultrasonic_pConfig->EchoChannel);

    /* Send TRIG pulse: HIGH for ~15 us */
    Dio_WriteChannel(Ultrasonic_pConfig->TrigChannel, STD_LOW);
    Ultrasonic_DelayUs(2U);                              /* small settle, optional */
    Dio_WriteChannel(Ultrasonic_pConfig->TrigChannel, STD_HIGH);
    Ultrasonic_DelayUs(ULTRASONIC_TRIG_PULSE_US);        /* ~15 µs */
    Dio_WriteChannel(Ultrasonic_pConfig->TrigChannel, STD_LOW);
}

boolean Ultrasonic_HasNewMeasurement(void)
{
    /* Poll hardware first, then return flag */
    Ultrasonic_UpdateMeasurement();
    return Ultrasonic_NewMeasurementFlag;
}

uint16 Ultrasonic_GetLastDistanceCm(void)
{
    /* Reading clears the "new" flag (simple one-consumer model) */
    Ultrasonic_NewMeasurementFlag = FALSE;
    return Ultrasonic_LastDistanceCm;
}

Std_ReturnType Ultrasonic_MeasureBlocking(uint16 * distanceCm, uint16 timeoutMs)
{
    uint32 waitedUs = 0UL;

    if ((distanceCm == NULL_PTR) || (Ultrasonic_pConfig == NULL_PTR))
    {
        return E_NOT_OK;
    }

    Ultrasonic_TriggerMeasurement();

    while (waitedUs < ((uint32)timeoutMs * 1000UL))
    {
        Ultrasonic_UpdateMeasurement();

        if (Ultrasonic_NewMeasurementFlag == TRUE)
        {
            *distanceCm = Ultrasonic_GetLastDistanceCm();
            return E_OK;
        }

        Ultrasonic_DelayUs(ULTRASONIC_BLOCKING_STEP_US);
        waitedUs += ULTRASONIC_BLOCKING_STEP_US;
    }

    /* timeout */
    return E_NOT_OK;
}
