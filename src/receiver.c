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
#include "Icu.h"
#include "Gpt.h"

#include"receiver.h"
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
volatile Receiver ReceiverInstance;

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

/**
 * @brief        Receiver State machine update function
 * @details         The function gets the measured signal period value
 * from the ICU Channel assigned to the receiver and updates the
 * State machine accordingly. It is called automatically with an interrupt
 */
void Icu_SignalNotification(void){
    Gpt_StopTimer(ReceiverInstance.GptChannel);
    Gpt_ValueType ReceivedPeriod = Gpt_GetTimeElapsed(ReceiverInstance.GptChannel);
    Gpt_StartTimer(ReceiverInstance.GptChannel, ReceiverInstance.OutsideChannelTicks);
    ReceiverStateUpdate(ReceivedPeriod);
}

void Gpt_Notification(void){
    /*Timer should only end after decoding the entire PPM signal, thus the state being between channels.
    Otherwise, the program is not synced with the receiver*/
    if(ReceiverInstance.State != BetweenChannels){
        ReceiverInstance.State = UnSynced;
    }
}

/**
 * @brief        Receiver initialisation function
 * @details      The function initialises the State machine for decoding
 * the ppm signal and starts the ICU signal measurement on the given Channel
 */
void ReceiverInit(Icu_ChannelType ReceiverIcuChannel, Gpt_ChannelType ReceiverGptChannel, uint16 ChannelMinTicks, uint16 ChannelMedTicks, uint16 ChannelMaxTicks, uint16 OutsideChannelTicks){
    ReceiverInstance.IcuChannel = ReceiverIcuChannel;
    ReceiverInstance.GptChannel = ReceiverGptChannel;
    ReceiverInstance.ChannelMinTicks = ChannelMinTicks;
    ReceiverInstance.ChannelMedTicks = ChannelMedTicks;
    ReceiverInstance.ChannelMaxTicks = ChannelMaxTicks;
    ReceiverInstance.OutsideChannelTicks = OutsideChannelTicks;
    Icu_EnableEdgeDetection(ReceiverInstance.IcuChannel);
    Icu_EnableNotification(ReceiverInstance.IcuChannel);
    Gpt_EnableNotification(ReceiverInstance.GptChannel);
    Gpt_StartTimer(ReceiverInstance.GptChannel, ReceiverInstance.OutsideChannelTicks);
    ReceiverInstance.State = UnSynced;
}
/*returns the Channel value in range -100 to 100, or -200 for errors at receiver*/
int GetReceiverChannel(uint8 Channel){
    int ChannelValue;
    if(ReceiverInstance.State != UnSynced){/*the program is synced and working normally*/
        if(Channel < 8U){/*the Channel number is valid*/
            ChannelValue = ((int)ReceiverInstance.Channels[Channel]-ReceiverInstance.ChannelMedTicks)/60;
        }
        else{/*the Channel number is invalid*/
            ChannelValue = 0;
        }
        /*repair errors that can be introduced by software decoding*/
        if(ChannelValue > 100){
            ChannelValue = 100;
        }
        else if(ChannelValue < -100){
            ChannelValue = -100;
        }
    }
    else{
        ChannelValue = 0;/*there program is not sync'd with the receiver, it is not safe!*/
    }
    return ChannelValue;
}

void ReceiverStateUpdate(Gpt_ValueType ReceivedPeriod){
    if(ReceivedPeriod != 0){
        switch(ReceiverInstance.State){
        case UnSynced:
            if(ReceivedPeriod>=ReceiverInstance.OutsideChannelTicks){
                ReceiverInstance.State = Channel1;
            }
            break;
        case BetweenChannels:
            if(ReceivedPeriod<ReceiverInstance.OutsideChannelTicks){
                ReceiverInstance.State = UnSynced;
            }
            else{
                ReceiverInstance.State = Channel1;
            }
            break;
        case Channel1:
            if(ReceivedPeriod>=ReceiverInstance.OutsideChannelTicks){
                ReceiverInstance.State = UnSynced;
            }
            else{
                ReceiverInstance.Channels[0] = ReceivedPeriod;
                ReceiverInstance.State = Channel2;
            }
            break;
        case Channel2:
            if(ReceivedPeriod>=ReceiverInstance.OutsideChannelTicks){
                ReceiverInstance.State = UnSynced;
            }
            else{
                ReceiverInstance.Channels[1] = ReceivedPeriod;
                ReceiverInstance.State = Channel3;
            }
            break;
        case Channel3:
            if(ReceivedPeriod>=ReceiverInstance.OutsideChannelTicks){
                ReceiverInstance.State = UnSynced;
            }
            else{
                ReceiverInstance.Channels[2] = ReceivedPeriod;
                ReceiverInstance.State = Channel4;
            }
            break;
        case Channel4:
            if(ReceivedPeriod>=ReceiverInstance.OutsideChannelTicks){
                ReceiverInstance.State = UnSynced;
            }
            else{
                ReceiverInstance.Channels[3] = ReceivedPeriod;
                ReceiverInstance.State = Channel5;
            }
            break;
        case Channel5:
            if(ReceivedPeriod>=ReceiverInstance.OutsideChannelTicks){
                ReceiverInstance.State = UnSynced;
            }
            else{
                ReceiverInstance.Channels[4] = ReceivedPeriod;
                ReceiverInstance.State = Channel6;
            }
            break;
        case Channel6:
            if(ReceivedPeriod>=ReceiverInstance.OutsideChannelTicks){
                ReceiverInstance.State = UnSynced;
            }
            else{
                ReceiverInstance.Channels[5] = ReceivedPeriod;
                ReceiverInstance.State = Channel7;
            }
            break;
        case Channel7:
            if(ReceivedPeriod>=ReceiverInstance.OutsideChannelTicks){
                ReceiverInstance.State = UnSynced;
            }
            else{
                ReceiverInstance.Channels[6] = ReceivedPeriod;
                ReceiverInstance.State = Channel8;
            }
            break;
        case Channel8:
            if(ReceivedPeriod>=ReceiverInstance.OutsideChannelTicks){
                ReceiverInstance.State = UnSynced;
            }
            else{
                ReceiverInstance.Channels[7] = ReceivedPeriod;
                ReceiverInstance.State = BetweenChannels;
            }
            break;
        }
    }
}

enum ReceiverStates GetReceiverState(){
    return ReceiverInstance.State;
}


#ifdef __cplusplus
}
#endif

/** @} */
