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
#include "servo.h"

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

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

void ServoInit(Pwm_ChannelType ServoPwmChannel, uint16 MaxDutyCycle, uint16 MinDutyCycle, uint16 MedDutyCycle ){
    ServoInstance.ServoPwmChannel = ServoPwmChannel;
    ServoInstance.MaxDutyCycle = MaxDutyCycle;
    ServoInstance.MinDutyCycle = MinDutyCycle;
    ServoInstance.MedDutyCycle = MedDutyCycle;
    Pwm_SetDutyCycle(ServoInstance.ServoPwmChannel, ServoInstance.MedDutyCycle);
}

void Steer(int Direction){
    uint16 ServoDutyCycle;
    if(Direction>(int)0){
        if(Direction > 100){
            Direction = 100;
        }
        ServoDutyCycle = ServoInstance.MedDutyCycle + Direction*(int)(ServoInstance.MaxDutyCycle-ServoInstance.MedDutyCycle)/100;
    }
    else{
        if(Direction < -100){
            Direction = -100;
        }
        ServoDutyCycle = ServoInstance.MedDutyCycle - Direction*(int)(ServoInstance.MinDutyCycle-ServoInstance.MedDutyCycle)/100;
    }
    Pwm_SetDutyCycle(ServoInstance.ServoPwmChannel, ServoDutyCycle);
}

void SteerLeft(void){
    Pwm_SetDutyCycle(ServoInstance.ServoPwmChannel, ServoInstance.MinDutyCycle);
}

void SteerRight(void){
    Pwm_SetDutyCycle(ServoInstance.ServoPwmChannel, ServoInstance.MaxDutyCycle);
}

void SteerStraight(void){
    Pwm_SetDutyCycle(ServoInstance.ServoPwmChannel, ServoInstance.MedDutyCycle);
}

#ifdef __cplusplus
}
#endif

/** @} */
