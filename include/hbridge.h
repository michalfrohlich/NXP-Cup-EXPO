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
#ifndef HBRIDGE
#define HBRIDGE
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
#include "Dio_Cfg.h"
/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

typedef struct{
    Pwm_ChannelType Motor1_Speed;
    Pwm_ChannelType Motor2_Speed;
    Dio_ChannelType Motor1_Forward;
    Dio_ChannelType Motor1_Backward;
    Dio_ChannelType Motor2_Forward;
    Dio_ChannelType Motor2_Backward;
    uint8 Brake; /*value is zero or non zero: val == 0 means brake, val != 0 means no brake*/
    int Speed;    /*values between -100 and 100: val >= 0 means Forward, val < 0 means Reverse*/
}Hbridge;


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

void HbridgeInit(Pwm_ChannelType Motor1_Speed, Pwm_ChannelType Motor2_Speed, Dio_ChannelType Motor1_Forward,
        Dio_ChannelType Motor1_Backward, Dio_ChannelType Motor2_Forward, Dio_ChannelType Motor2_Backward);
void HbridgeSetSpeed(int Speed);
void HbridgeSetBrake(uint8 Brake);
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
