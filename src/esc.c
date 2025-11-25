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
#include "esc.h"

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/
static volatile Esc EscInstance;
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
static void SetPwm(int EscSpeedCommand) {
    uint16 EscDutyCycle;
    if(EscSpeedCommand >= 0) {
        EscDutyCycle = (uint16)(EscInstance.MedDutyCycle + EscSpeedCommand*(int)(EscInstance.MaxDutyCycle-EscInstance.MedDutyCycle)/100);
        Pwm_SetDutyCycle(EscInstance.Channel, EscDutyCycle);
    }
    else{
        EscDutyCycle = (uint16)(EscInstance.MedDutyCycle + EscSpeedCommand*(int)(EscInstance.MedDutyCycle-EscInstance.MinDutyCycle)/100);
        Pwm_SetDutyCycle(EscInstance.Channel, EscDutyCycle);
    }
}

void Esc_Period_Finished(void){
    int EscSpeedCommand;
    /*if your chosen ESC has braking capabilities, set this on STD_ON. If not, set it on STD_OFF.
     * Using the wrong configuration can make the car go full speed backward on braking!*/
#if (ESC_HAS_BRAKE == STD_ON)
    /* state machine updates here at every PWM signal edge*/
    switch(EscInstance.State){
    case Forward:
        if(EscInstance.Brake != 0U || EscInstance.Speed < 0){
            EscInstance.State = Braking;
        }
        break;
    case Braking:
        if(EscInstance.Brake == 0U && EscInstance.Speed >= 0){
            EscInstance.State = Forward;
        }
        else if(EscInstance.Brake == 0U && EscInstance.Speed < 0){
            EscInstance.State = Neutral;
        }
        break;
#else
    switch(EscInstance.State){
        case Forward:
            if(EscInstance.Brake != 0U){
                EscInstance.State = Braking;
            }
            else if(EscInstance.Speed < 0){
                EscInstance.State = Reverse;
            }
            break;
        case Braking:
            if(EscInstance.Brake != 0U){
                EscInstance.State = Neutral;
            }
            else if(EscInstance.Speed < 0){
                EscInstance.State = Reverse;
            }
            else if(EscInstance.Speed >= 0){
                EscInstance.State = Forward;
            }
            break;
#endif
        case Neutral:
            if(EscInstance.Brake == 0U && EscInstance.Speed < 0){
                EscInstance.State = Reverse;
            }
            else if(EscInstance.Brake == 0U && EscInstance.Speed >= 0){
                EscInstance.State = Forward;
            }
            break;
        case Reverse:
            if(EscInstance.Brake != 0U){
                EscInstance.State = Braking;
            }
            else if(EscInstance.Speed >= 0){
                EscInstance.State = Forward;
            }
            break;
        default:/*invalid case, set safe values for esc*/
            EscInstance.Speed = 0;
    }

    /*update the car's speed*/
    switch(EscInstance.State){
        case Forward:
        case Reverse:
            EscSpeedCommand = EscInstance.Speed;
            break;
        case Braking:
#if (ESC_HAS_BRAKE == STD_ON)
            EscSpeedCommand = -100;
#else
            EscSpeedCommand = -EscInstance.Speed;
#endif
            break;
        case Neutral:
        default:
            EscSpeedCommand = 0;
    }
    SetPwm(EscSpeedCommand);
}
/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

void EscInit(Pwm_ChannelType EscPwmChannel, uint16 MinDutyCycle, uint16 MedDutyCycle, uint16 MaxDutyCycle){
    EscInstance.Channel = EscPwmChannel;
    EscInstance.MinDutyCycle = MinDutyCycle;/*1ms*/
    EscInstance.MedDutyCycle = MedDutyCycle;/*1.5ms*/
    EscInstance.MaxDutyCycle = MaxDutyCycle;/*2ms*/
    EscInstance.State = Neutral;
    EscInstance.Speed = 0;
    EscInstance.Brake = 0U;
    Pwm_SetDutyCycle(EscInstance.Channel, EscInstance.MedDutyCycle);/*sending the 'Neutral' command arms the esc*/
    Pwm_EnableNotification(EscInstance.Channel, PWM_FALLING_EDGE);/*enables interrupt for state machine update at every PWM signal falling edge*/
}

void EscSetSpeed(int Speed){
    if(Speed > 100){
        EscInstance.Speed = 100;
    }
    else if(Speed < -100){
        EscInstance.Speed = -100;
    }
    else{
        EscInstance.Speed = Speed;
    }
}

void EscSetBrake(uint8 Brake){
    if(Brake != 0U){
        EscInstance.Brake = 1U;
    }
    else{
        EscInstance.Brake = 0U;
    }
}

#ifdef __cplusplus
}
#endif

/** @} */

