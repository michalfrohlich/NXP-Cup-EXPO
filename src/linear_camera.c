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
#include "linear_camera.h"
#include "Adc_Types.h"
#include "pixy2.h"
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
static volatile LinearCamera LinearCameraInstance;

/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/
static Adc_ValueGroupType AdcResultBuffer;

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/
void NewCameraFrame(void){
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0x4000);
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
    Pwm_EnableNotification(LinearCameraInstance.ClkPwmChannel, PWM_FALLING_EDGE);
}

void CameraClock(void){
    if(LinearCameraInstance.CurrentIndex < 128U){
        Adc_StartGroupConversion(LinearCameraInstance.InputAdcGroup);
    }
    else{
        Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    }
}

void CameraAdcFinished(void){
    LinearCameraInstance.BufferReference->Values[LinearCameraInstance.CurrentIndex] = AdcResultBuffer*25U/64U;
    LinearCameraInstance.CurrentIndex++;
}
/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

void LinearCameraInit(Pwm_ChannelType ClkPwmChannel, Gpt_ChannelType ShutterGptChannel, Adc_GroupType InputAdcGroup, Dio_ChannelType ShutterDioChannel){
    LinearCameraInstance.ClkPwmChannel = ClkPwmChannel;
    LinearCameraInstance.ShutterGptChannel = ShutterGptChannel;
    LinearCameraInstance.InputAdcGroup = InputAdcGroup;
    LinearCameraInstance.ShutterDioChannel = ShutterDioChannel;
    LinearCameraInstance.CurrentIndex = 0U;
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
    Adc_SetupResultBuffer(LinearCameraInstance.InputAdcGroup , &AdcResultBuffer);
    Adc_EnableGroupNotification(LinearCameraInstance.InputAdcGroup);
}

void LinearCameraGetFrame(LinearCameraFrame *Frame){
    LinearCameraInstance.BufferReference = Frame;
    LinearCameraInstance.CurrentIndex = 0U;
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_HIGH);
    Gpt_StartTimer(LinearCameraInstance.ShutterGptChannel, 100U);
    Gpt_EnableNotification(LinearCameraInstance.ShutterGptChannel);
    while(LinearCameraInstance.CurrentIndex <= 127U){
        ;
    }
}

#ifdef __cplusplus
}
#endif

/** @} */
