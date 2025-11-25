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
#ifndef LINEAR_CAMERA_H
#define LINEAR_CAMERA_H
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
#include "Adc.h"
#include "Gpt.h"
#include "Dio.h"
/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
typedef struct{
    uint8 Values[128];
}LinearCameraFrame;

typedef struct{
    Pwm_ChannelType ClkPwmChannel;
    Gpt_ChannelType ShutterGptChannel;
    Adc_GroupType InputAdcGroup;
    Dio_ChannelType ShutterDioChannel;
    uint16 CurrentIndex;
    LinearCameraFrame *BufferReference;
}LinearCamera;


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
void LinearCameraInit(Pwm_ChannelType ClkPwmChannel, Gpt_ChannelType ShutterGptChannel, Adc_GroupType InputAdcGroup, Dio_ChannelType ShutterDioChannel);
void LinearCameraGetFrame(LinearCameraFrame *Frame);

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
