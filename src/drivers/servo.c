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
static volatile boolean ServoInitialized = FALSE;
static volatile uint16 ServoAppliedDutyCycle = 0U;
static volatile uint16 ServoPendingDutyCycle = 0U;
static volatile boolean ServoPendingUpdate = FALSE;
static volatile uint32 ServoCommandRequestCount = 0U;
static volatile uint32 ServoPeriodCallbackCount = 0U;
static volatile uint32 ServoAppliedUpdateCount = 0U;
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

    if (ServoDutyCycle == ServoAppliedDutyCycle)
    {
        ServoPendingUpdate = FALSE;
    }
    else
    {
        ServoPendingDutyCycle = ServoDutyCycle;
        ServoPendingUpdate = TRUE;
    }

    SchM_Exit_Pwm_PWM_EXCLUSIVE_AREA_00();
}

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

void ServoInit(Pwm_ChannelType ServoPwmChannel, uint16 MaxDutyCycle, uint16 MinDutyCycle, uint16 MedDutyCycle ){
    ServoInstance.ServoPwmChannel = ServoPwmChannel;
    ServoInstance.MaxDutyCycle = MaxDutyCycle;
    ServoInstance.MinDutyCycle = MinDutyCycle;
    ServoInstance.MedDutyCycle = MedDutyCycle;
    ServoAppliedDutyCycle = ServoInstance.MedDutyCycle;
    ServoPendingDutyCycle = ServoInstance.MedDutyCycle;
    ServoPendingUpdate = FALSE;
    ServoCommandRequestCount = 0U;
    ServoPeriodCallbackCount = 0U;
    ServoAppliedUpdateCount = 0U;
    ServoInitialized = TRUE;
    Pwm_SetDutyCycle(ServoInstance.ServoPwmChannel, ServoAppliedDutyCycle);
    Pwm_EnableNotification(ServoInstance.ServoPwmChannel, PWM_FALLING_EDGE);
}

void Steer(int Direction){
    uint16 ServoDutyCycle;

    if (ServoInitialized != TRUE)
    {
        return;
    }

    ServoCommandRequestCount++;
    ServoDutyCycle = Servo_CalcDuty(Direction);
    Servo_PublishPendingDuty(ServoDutyCycle);
}

void SteerLeft(void){
    if (ServoInitialized != TRUE)
    {
        return;
    }

    ServoCommandRequestCount++;
    Servo_PublishPendingDuty(ServoInstance.MinDutyCycle);
}

void SteerRight(void){
    if (ServoInitialized != TRUE)
    {
        return;
    }

    ServoCommandRequestCount++;
    Servo_PublishPendingDuty(ServoInstance.MaxDutyCycle);
}

void SteerStraight(void){
    if (ServoInitialized != TRUE)
    {
        return;
    }

    ServoCommandRequestCount++;
    Servo_PublishPendingDuty(ServoInstance.MedDutyCycle);
}

void Servo_Period_Finished(void)
{
    if (ServoInitialized != TRUE)
    {
        return;
    }

    ServoPeriodCallbackCount++;

    if (ServoPendingUpdate != TRUE)
    {
        return;
    }

    Pwm_SetDutyCycle(ServoInstance.ServoPwmChannel, ServoPendingDutyCycle);
    ServoAppliedDutyCycle = ServoPendingDutyCycle;
    ServoPendingUpdate = FALSE;
    ServoAppliedUpdateCount++;
}

void ServoGetDebugSnapshot(ServoDebugSnapshot *Snapshot)
{
    if (Snapshot == NULL_PTR)
    {
        return;
    }

    SchM_Enter_Pwm_PWM_EXCLUSIVE_AREA_00();
    Snapshot->Initialized = ServoInitialized;
    Snapshot->AppliedDutyCycle = ServoAppliedDutyCycle;
    Snapshot->PendingDutyCycle = ServoPendingDutyCycle;
    Snapshot->PendingUpdate = ServoPendingUpdate;
    Snapshot->CommandRequestCount = ServoCommandRequestCount;
    Snapshot->PeriodCallbackCount = ServoPeriodCallbackCount;
    Snapshot->AppliedUpdateCount = ServoAppliedUpdateCount;
    SchM_Exit_Pwm_PWM_EXCLUSIVE_AREA_00();
}

#ifdef __cplusplus
}
#endif

/** @} */
