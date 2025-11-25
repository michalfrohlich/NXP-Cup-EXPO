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
#include "hbridge.h"
#include "Pwm.h"
#include "Dio.h"
/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/
static Hbridge HbridgeInstance;
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

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

void HbridgeInit(Pwm_ChannelType Motor1_Speed, Pwm_ChannelType Motor2_Speed, Dio_ChannelType Motor1_Forward,
    Dio_ChannelType Motor1_Backward, Dio_ChannelType Motor2_Forward, Dio_ChannelType Motor2_Backward){
    HbridgeInstance.Motor1_Speed = Motor1_Speed;
    HbridgeInstance.Motor2_Speed = Motor2_Speed;
    HbridgeInstance.Motor1_Forward = Motor1_Forward;
    HbridgeInstance.Motor1_Backward = Motor1_Backward;
    HbridgeInstance.Motor2_Forward = Motor2_Forward;
    HbridgeInstance.Motor2_Backward = Motor2_Backward;
}

void HbridgeSetSpeed(int Speed){
    uint16 SpeedDutyCycle;
    HbridgeInstance.Speed = Speed;
    if(HbridgeInstance.Brake == 0U){
        if(Speed > 100){
            Speed = 100;
        }
        else if(Speed < -100){
            Speed = -100;
        }
        if(Speed >= 0){
            SpeedDutyCycle = (uint32)Speed*8192UL/25UL;
            Dio_WriteChannel(HbridgeInstance.Motor1_Forward, (Dio_LevelType)STD_HIGH);
            Dio_WriteChannel(HbridgeInstance.Motor1_Backward, (Dio_LevelType)STD_LOW);
            Dio_WriteChannel(HbridgeInstance.Motor2_Forward, (Dio_LevelType)STD_HIGH);
            Dio_WriteChannel(HbridgeInstance.Motor2_Backward, (Dio_LevelType)STD_LOW);
        }
        else{
            SpeedDutyCycle = (uint32)(Speed*-1)*8192UL/25UL;
            Dio_WriteChannel(HbridgeInstance.Motor1_Forward, (Dio_LevelType)STD_LOW);
            Dio_WriteChannel(HbridgeInstance.Motor1_Backward, (Dio_LevelType)STD_HIGH);
            Dio_WriteChannel(HbridgeInstance.Motor2_Forward, (Dio_LevelType)STD_LOW);
            Dio_WriteChannel(HbridgeInstance.Motor2_Backward, (Dio_LevelType)STD_HIGH);
        }
        Pwm_SetDutyCycle(HbridgeInstance.Motor1_Speed, SpeedDutyCycle);
        Pwm_SetDutyCycle(HbridgeInstance.Motor2_Speed, SpeedDutyCycle);
    }
}

void HbridgeSetBrake(uint8 Brake){
    if(Brake != 0U){
        HbridgeInstance.Brake = 1U;
        Pwm_SetDutyCycle(HbridgeInstance.Motor1_Speed, 0x8000);
        Pwm_SetDutyCycle(HbridgeInstance.Motor2_Speed, 0x8000);
        Dio_WriteChannel(HbridgeInstance.Motor1_Forward, (Dio_LevelType)STD_LOW);
        Dio_WriteChannel(HbridgeInstance.Motor1_Backward, (Dio_LevelType)STD_LOW);
        Dio_WriteChannel(HbridgeInstance.Motor2_Forward, (Dio_LevelType)STD_LOW);
        Dio_WriteChannel(HbridgeInstance.Motor2_Backward, (Dio_LevelType)STD_LOW);
    }
    else{
        HbridgeInstance.Brake = 0U;
        HbridgeSetSpeed(HbridgeInstance.Speed);
    }
}

#ifdef __cplusplus
}
#endif

/** @} */


