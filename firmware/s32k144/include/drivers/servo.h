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
#ifndef SERVO_H
#define SERVO_H
/*These functions assume the relevant drivers are already initialised*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/

#include "Pwm.h"
/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

typedef enum
{
    SERVO_UPDATE_ON_PWM_CALLBACK = 0,
    SERVO_UPDATE_PHASED_FOREGROUND
} ServoUpdatePolicy_t;

typedef struct
{
    boolean Initialized;
    ServoUpdatePolicy_t UpdatePolicy;
    uint16 AppliedDutyCycle;
    uint16 PendingDutyCycle;
    boolean PendingUpdate;
    uint32 CommandRequestCount;
    uint32 PeriodSequence;
    uint32 PeriodStartMs;
    uint32 CommitCount;
    uint32 MissedCommitCount;
    uint32 DuplicateServiceCount;
} ServoDebugSnapshot_t;
/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/


/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

void Servo_Init(Pwm_ChannelType ServoPwmChannel, uint16 MaxDutyCycle, uint16 MinDutyCycle, uint16 MedDutyCycle);
void Servo_SetSteer(int Direction);    /*takes a value between -100 (left) and 100 (right) and translates it into a duty cycle*/
void Servo_SetFullLeft(void);    /*sets the duty cycle to the configured maximum*/
void Servo_SetFullRight(void);/*sets the duty cycle to the configured minimum*/
void Servo_SetStraight(void);    /*sets the duty cycle to the configured middle*/
void Servo_SetUpdatePolicy(ServoUpdatePolicy_t Policy);
void Servo_Service(uint32 nowMs);
void Servo_Period_Finished(void);
void Servo_GetDebugSnapshot(ServoDebugSnapshot_t *Snapshot);

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/




#ifdef __cplusplus
}
#endif

#endif
/** @} */
