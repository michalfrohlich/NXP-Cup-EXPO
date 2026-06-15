/*==================================================================================================
*    Copyright 2021-2024 NXP
*
*    NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be
*    used strictly in accordance with the applicable license terms. By expressly
*    accepting such terms or by downloading, installing, activating and/or otherwise
*    using the software, you are agreeing that you have read, and that you agree to
*    comply with and are bound by, such license terms. If you do not agree to be
*    bound by the applicable license terms, then you may not retain, install,
*    activate or otherwise use the software.
==================================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "drivers/servo.h"
#include "drivers/timebase.h"
#include "SchM_Pwm.h"

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/
static Servo ServoInstance;
static volatile boolean Servo_Initialized = FALSE;
static volatile ServoUpdatePolicy Servo_UpdatePolicy = SERVO_UPDATE_ON_PWM_CALLBACK;
static volatile uint16 ServoAppliedDutyCycle = 0U;
static volatile uint16 ServoPendingDutyCycle = 0U;
static volatile boolean ServoPendingUpdate = FALSE;
static volatile uint32 ServoCommandRequestCount = 0U;
static volatile uint32 ServoPeriodSequence = 0U;
static volatile uint32 ServoPeriodStartMs = 0U;
static volatile uint32 ServoLastCommitPeriod = 0U;
static volatile uint32 ServoCommitCount = 0U;
static volatile uint32 ServoMissedCommitCount = 0U;
static volatile uint32 ServoDuplicateServiceCount = 0U;

#define SERVO_COMMIT_PHASE_START_MS  (17U)
#define SERVO_COMMIT_PHASE_END_MS    (19U)
/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/
static uint16 Servo_CalcDuty(int Direction)
{
    uint16 ServoDutyCycle;

    if(Direction>(int)0)
    {
        if(Direction > 100)
        {
            Direction = 100;
        }
        ServoDutyCycle = ServoInstance.MedDutyCycle + Direction*(int)(ServoInstance.MaxDutyCycle-ServoInstance.MedDutyCycle)/100;
    }
    else
    {
        if(Direction < -100)
        {
            Direction = -100;
        }
        ServoDutyCycle = ServoInstance.MedDutyCycle - Direction*(int)(ServoInstance.MinDutyCycle-ServoInstance.MedDutyCycle)/100;
    }

    return ServoDutyCycle;
}

static void Servo_PublishPendingDuty(uint16 ServoDutyCycle)
{
    SchM_Enter_Pwm_PWM_EXCLUSIVE_AREA_00();

    ServoPendingDutyCycle = ServoDutyCycle;
    ServoPendingUpdate = TRUE;

    SchM_Exit_Pwm_PWM_EXCLUSIVE_AREA_00();
}

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

void Servo_Init(Pwm_ChannelType ServoPwmChannel, uint16 MaxDutyCycle, uint16 MinDutyCycle, uint16 MedDutyCycle ){
    ServoInstance.ServoPwmChannel = ServoPwmChannel;
    ServoInstance.MaxDutyCycle = MaxDutyCycle;
    ServoInstance.MinDutyCycle = MinDutyCycle;
    ServoInstance.MedDutyCycle = MedDutyCycle;
    Servo_UpdatePolicy = SERVO_UPDATE_ON_PWM_CALLBACK;
    ServoAppliedDutyCycle = ServoInstance.MedDutyCycle;
    ServoPendingDutyCycle = ServoInstance.MedDutyCycle;
    ServoPendingUpdate = FALSE;
    ServoCommandRequestCount = 0U;
    ServoPeriodSequence = 0U;
    ServoPeriodStartMs = Timebase_GetMs();
    ServoLastCommitPeriod = 0U;
    ServoCommitCount = 0U;
    ServoMissedCommitCount = 0U;
    ServoDuplicateServiceCount = 0U;
    Servo_Initialized = TRUE;
    Pwm_SetDutyCycle(ServoInstance.ServoPwmChannel, ServoAppliedDutyCycle);
    Pwm_EnableNotification(ServoInstance.ServoPwmChannel, PWM_FALLING_EDGE);
}

void Servo_SetSteer(int Direction){
    uint16 ServoDutyCycle;

    if (Servo_Initialized != TRUE)
    {
        return;
    }

    ServoCommandRequestCount++;
    ServoDutyCycle = Servo_CalcDuty(Direction);
    Servo_PublishPendingDuty(ServoDutyCycle);
}

void SteerLeft(void){
    if (Servo_Initialized != TRUE)
    {
        return;
    }

    ServoCommandRequestCount++;
    Servo_PublishPendingDuty(ServoInstance.MinDutyCycle);
}

void SteerRight(void){
    if (Servo_Initialized != TRUE)
    {
        return;
    }

    ServoCommandRequestCount++;
    Servo_PublishPendingDuty(ServoInstance.MaxDutyCycle);
}

void SteerStraight(void){
    if (Servo_Initialized != TRUE)
    {
        return;
    }

    ServoCommandRequestCount++;
    Servo_PublishPendingDuty(ServoInstance.MedDutyCycle);
}

void Servo_SetUpdatePolicy(ServoUpdatePolicy Policy)
{
    if (Servo_Initialized != TRUE)
    {
        return;
    }

    if ((Policy != SERVO_UPDATE_ON_PWM_CALLBACK) &&
        (Policy != SERVO_UPDATE_PHASED_FOREGROUND))
    {
        return;
    }

    Pwm_DisableNotification(ServoInstance.ServoPwmChannel);

    SchM_Enter_Pwm_PWM_EXCLUSIVE_AREA_00();
    Servo_UpdatePolicy = Policy;
    ServoPendingDutyCycle = ServoAppliedDutyCycle;
    ServoPendingUpdate = FALSE;
    ServoPeriodSequence = 0U;
    ServoPeriodStartMs = Timebase_GetMs();
    ServoLastCommitPeriod = 0U;
    SchM_Exit_Pwm_PWM_EXCLUSIVE_AREA_00();

    Pwm_EnableNotification(
        ServoInstance.ServoPwmChannel,
        (Policy == SERVO_UPDATE_PHASED_FOREGROUND) ? PWM_RISING_EDGE : PWM_FALLING_EDGE);
}

void Servo_Service(uint32 nowMs)
{
    uint32 periodSequence;
    uint32 periodStartMs;
    uint32 phaseMs;
    uint16 pendingDutyCycle;

    if ((Servo_Initialized != TRUE) ||
        (Servo_UpdatePolicy != SERVO_UPDATE_PHASED_FOREGROUND))
    {
        return;
    }

    SchM_Enter_Pwm_PWM_EXCLUSIVE_AREA_00();
    periodSequence = ServoPeriodSequence;
    periodStartMs = ServoPeriodStartMs;

    if ((periodSequence == 0U) || (ServoPendingUpdate != TRUE))
    {
        SchM_Exit_Pwm_PWM_EXCLUSIVE_AREA_00();
        return;
    }

    phaseMs = (uint32)(nowMs - periodStartMs);
    if ((phaseMs < SERVO_COMMIT_PHASE_START_MS) ||
        (phaseMs >= SERVO_COMMIT_PHASE_END_MS))
    {
        SchM_Exit_Pwm_PWM_EXCLUSIVE_AREA_00();
        return;
    }

    if (ServoLastCommitPeriod == periodSequence)
    {
        ServoDuplicateServiceCount++;
        SchM_Exit_Pwm_PWM_EXCLUSIVE_AREA_00();
        return;
    }

    pendingDutyCycle = ServoPendingDutyCycle;
    ServoPendingUpdate = FALSE;
    ServoLastCommitPeriod = periodSequence;
    SchM_Exit_Pwm_PWM_EXCLUSIVE_AREA_00();

    Pwm_SetDutyCycle(ServoInstance.ServoPwmChannel, pendingDutyCycle);

    SchM_Enter_Pwm_PWM_EXCLUSIVE_AREA_00();
    ServoAppliedDutyCycle = pendingDutyCycle;
    ServoCommitCount++;
    SchM_Exit_Pwm_PWM_EXCLUSIVE_AREA_00();
}

void Servo_Period_Finished(void)
{
    uint16 pendingDutyCycle;

    if (Servo_Initialized != TRUE)
    {
        return;
    }

    if (Servo_UpdatePolicy == SERVO_UPDATE_ON_PWM_CALLBACK)
    {
        ServoPeriodSequence++;
        ServoPeriodStartMs = Timebase_GetMs();

        if (ServoPendingUpdate != TRUE)
        {
            return;
        }

        pendingDutyCycle = ServoPendingDutyCycle;
        ServoPendingUpdate = FALSE;
        Pwm_SetDutyCycle(ServoInstance.ServoPwmChannel, pendingDutyCycle);
        ServoAppliedDutyCycle = pendingDutyCycle;
        ServoCommitCount++;
        return;
    }

    if ((ServoPeriodSequence != 0U) &&
        (ServoPendingUpdate == TRUE))
    {
        ServoMissedCommitCount++;
        ServoPendingUpdate = FALSE;
    }

    ServoPeriodSequence++;
    ServoPeriodStartMs = Timebase_GetMs();
}

void Servo_GetDebugSnapshot(ServoDebugSnapshot *Snapshot)
{
    if (Snapshot == NULL_PTR)
    {
        return;
    }

    SchM_Enter_Pwm_PWM_EXCLUSIVE_AREA_00();
    Snapshot->Initialized = Servo_Initialized;
    Snapshot->UpdatePolicy = Servo_UpdatePolicy;
    Snapshot->AppliedDutyCycle = ServoAppliedDutyCycle;
    Snapshot->PendingDutyCycle = ServoPendingDutyCycle;
    Snapshot->PendingUpdate = ServoPendingUpdate;
    Snapshot->CommandRequestCount = ServoCommandRequestCount;
    Snapshot->PeriodSequence = ServoPeriodSequence;
    Snapshot->PeriodStartMs = ServoPeriodStartMs;
    Snapshot->CommitCount = ServoCommitCount;
    Snapshot->MissedCommitCount = ServoMissedCommitCount;
    Snapshot->DuplicateServiceCount = ServoDuplicateServiceCount;
    SchM_Exit_Pwm_PWM_EXCLUSIVE_AREA_00();
}

#ifdef __cplusplus
}
#endif

/** @} */
