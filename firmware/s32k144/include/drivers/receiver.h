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
#ifndef RECEIVER_H
#define RECEIVER_H
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
#include "Icu_Types.h"
#include "Gpt.h"
/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
enum ReceiverStates{
    UnSynced, /*this State indicates the program is not synced with the Receiver. Either there is
    * no signal from Receiver, or the program is unable to synchronise with it
    * (just started program/bad signal/connection lost with remote)*/
    Channel1, /*right joystick left/right movement*/
    Channel2, /*left joystick up/down movement*/
    Channel3, /*right joystick up/down movement*/
    Channel4, /*left joystick left/right movement*/
    Channel5, /*SWA (leftmost switch with 2 positions)*/
    Channel6, /*SWB (left middle switch with 3 positions)*/
    Channel7, /*SWC (right middle switch with 3 positions)*/
    Channel8, /*SWD (rightmost switch with 2 positions)*/
    BetweenChannels /*this State represents the time between two transmissions*/
};

typedef struct{
    Icu_ChannelType IcuChannel;/*icu channel configured for the Receiver*/
    Gpt_ChannelType GptChannel;/*Gpt channel for detecting synchronisation errors*/
    uint16 ChannelMinTicks;/*minimum value in ticks a channel can have*/
    uint16 ChannelMedTicks;/*channel middle/idle value in ticks*/
    uint16 ChannelMaxTicks;/*maximum value a channel can have in ticks*/
    uint16 OutsideChannelTicks;/*minimum value that represents the area between the ppm signals*/
    Icu_ValueType Channels[8U];/*stores values received for all Channels*/
    enum ReceiverStates State;/*variable that keeps track of the State machine*/
}Receiver;
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
void ReceiverInit(Icu_ChannelType ReceiverIcuChannel, Gpt_ChannelType ReceiverGptChannel, uint16 ChannelMinTicks, uint16 ChannelMedTicks, uint16 ChannelMaxTicks, uint16 OutsideChannelTicks);
int GetReceiverChannel(uint8 Channel);
void ReceiverStateUpdate(Gpt_ValueType ReceiverPeriod);
enum ReceiverStates GetReceiverState(void);
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
