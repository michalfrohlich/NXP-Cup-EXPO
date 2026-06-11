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
#ifndef ESC_H
#define ESC_H
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
/*if your chosen ESC has braking capabilities, set this on STD_ON. If not, set it on STD_OFF.
 * Using the wrong configuration can make the car go full speed backward on braking!*/
#define ESC_HAS_BRAKE     STD_ON

enum EscStates{
    Forward,
    Braking,
    Neutral,
    Reverse
};

typedef struct{
    boolean Initialized;
    enum EscStates State;
    Pwm_ChannelType Channel;
    uint16 MinDutyCycle;
    uint16 MaxDutyCycle;
    uint16 MedDutyCycle;
    int Speed;    /*values between -100 and 100: val >= 0 means Forward, val < 0 means Reverse*/
    uint8 Brake; /*value is zero or non zero: val == 0 means no brake, val != 0 means brake*/
}Esc;

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
void Esc_Init(Pwm_ChannelType PrimaryEscPwmChannel,
             Pwm_ChannelType SecondaryEscPwmChannel,
             uint16 MinDutyCycle,
             uint16 MedDutyCycle,
             uint16 MaxDutyCycle);
void Esc_SetSpeed(int PrimarySpeed, int SecondarySpeed);
void Esc_SetBrake(uint8 PrimaryBrake, uint8 SecondaryBrake);

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
