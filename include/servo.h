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

typedef struct{
    Pwm_ChannelType ServoPwmChannel;
    uint16 MinDutyCycle;    /*maximum right steer (the lower the duty cycle, the more it steers right)*/
    uint16 MaxDutyCycle;    /*maximum left steer  (the higher the duty cycle, the more it steers left)*/
    uint16 MedDutyCycle;    /*centered    (needed if left and right steering angles are not equal physically for any reason)*/
}Servo;
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

void ServoInit(Pwm_ChannelType ServoPwmChannel, uint16 MaxDutyCycle, uint16 MinDutyCycle, uint16 MedDutyCycle);
void Steer(int Direction);    /*takes a value between -100 (left) and 100 (right) and translates it into a duty cycle*/
void SteerLeft(void);    /*sets the duty cycle to the configured maximum*/
void SteerRight(void);/*sets the duty cycle to the configured minimum*/
void SteerStraight(void);    /*sets the duty cycle to the configured middle*/

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
