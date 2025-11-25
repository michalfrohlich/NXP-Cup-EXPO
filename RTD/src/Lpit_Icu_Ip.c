/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           : Ftm Lpit Lptmr Port_Ci LpCmp
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 2.0.0
*   Build Version        : S32K1_RTD_2_0_0_D2308_ASR_REL_4_7_REV_0000_20230804
*
*   Copyright 2020-2023 NXP Semiconductors
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

/**
 *     @file       Lpit_Icu_Ip.c
 *     @details    This source file contains the code for all driver functions which are using LPIT
 *                 module.
 *     @addtogroup lpit_icu_ip LPIT IPL
 *     @{
 */

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                          INCLUDE FILES
*  1) system and project includes
*  2) needed interfaces from external units
*  3) internal and external interfaces from this unit
==================================================================================================*/
#include "Lpit_Icu_Ip.h"

#if(LPIT_ICU_IP_DEV_ERROR_DETECT == STD_ON)
#include "Devassert.h"
#endif
/*==================================================================================================
*                                  SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define LPIT_ICU_IP_VENDOR_ID_C                    43
#define LPIT_ICU_IP_AR_RELEASE_MAJOR_VERSION_C     4
#define LPIT_ICU_IP_AR_RELEASE_MINOR_VERSION_C     7
#define LPIT_ICU_IP_AR_RELEASE_REVISION_VERSION_C  0
#define LPIT_ICU_IP_SW_MAJOR_VERSION_C             2
#define LPIT_ICU_IP_SW_MINOR_VERSION_C             0
#define LPIT_ICU_IP_SW_PATCH_VERSION_C             0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if source file and Lpit_Icu_Ip.h header file are of the same vendor */
#if (LPIT_ICU_IP_VENDOR_ID_C != LPIT_ICU_IP_VENDOR_ID)
    #error "Lpit_Icu_Ip.c and Lpit_Icu_Ip.h have different vendor IDs"
#endif

/* Check if source file and Lpit_Icu_Ip.h header file are of the same AUTOSAR version */
#if ((LPIT_ICU_IP_AR_RELEASE_MAJOR_VERSION_C    != LPIT_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (LPIT_ICU_IP_AR_RELEASE_MINOR_VERSION_C    != LPIT_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (LPIT_ICU_IP_AR_RELEASE_REVISION_VERSION_C != LPIT_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Lpit_Icu_Ip.c and Lpit_Icu_Ip.h are different"
#endif

/* Check if source file and Lpit_Icu_Ip.h header file are of the same software version */
#if ((LPIT_ICU_IP_SW_MAJOR_VERSION_C != LPIT_ICU_IP_SW_MAJOR_VERSION) || \
     (LPIT_ICU_IP_SW_MINOR_VERSION_C != LPIT_ICU_IP_SW_MINOR_VERSION) || \
     (LPIT_ICU_IP_SW_PATCH_VERSION_C != LPIT_ICU_IP_SW_PATCH_VERSION))
#error "Software Version Numbers of Lpit_Icu_Ip.c and Lpit_Icu_Ip.h are different"
#endif

#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    #if(LPIT_ICU_IP_DEV_ERROR_DETECT == STD_ON)
        /* Check if this header file and Devassert.h file are of the same Autosar version */
        #if ((LPIT_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != DEVASSERT_AR_RELEASE_MAJOR_VERSION) || \
             (LPIT_ICU_IP_AR_RELEASE_MINOR_VERSION_C != DEVASSERT_AR_RELEASE_MINOR_VERSION))
            #error "AutoSar Version Numbers of Lpit_Icu_Ip.c and Devassert.h are different"
        #endif
    #endif
#endif

/*==================================================================================================
*                                        LOCAL CONSTANTS
==================================================================================================*/
#define ICU_START_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"
/* Table of base addresses for LPIT instances. */
LPIT_Type* const lpitBase[] = IP_LPIT_BASE_PTRS;

#define ICU_STOP_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"

/*==================================================================================================
*                                        GLOBAL VARIABLES
==================================================================================================*/
#define ICU_START_SEC_VAR_CLEARED_UNSPECIFIED_NO_CACHEABLE
#include "Icu_MemMap.h"

#if (STD_ON == LPIT_ICU_TIMESTAMP_API)
Lpit_Icu_Ip_TimestampType timestampState[LPIT_INSTANCE_COUNT][LPIT_TMR_COUNT];
#endif

Lpit_Icu_Ip_ChannelsStateType channelsState[LPIT_INSTANCE_COUNT][LPIT_TMR_COUNT];

#define ICU_STOP_SEC_VAR_CLEARED_UNSPECIFIED_NO_CACHEABLE
#include "Icu_MemMap.h"

/*==================================================================================================
*                                        GLOBAL FUNCTIONS
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"

/** @implements Lpit_Icu_Ip_Init_Activity */
Lpit_Icu_Ip_StatusType Lpit_Icu_Ip_Init (uint8 instance, const Lpit_Icu_Ip_ConfigType * userConfig)
{
    const Lpit_Icu_Ip_ChannelConfigType* lpitChConfig;
    uint8  counter;

    /* Enable hardware module clock. This shall be be executed before any other setup is made. */
    lpitBase[instance]->MCR |= LPIT_MCR_M_CEN_MASK;

    /* Enable LPIT in DOZE mode. */
    lpitBase[instance]->MCR |= (uint32)((uint32)(userConfig->dozeMode?1U:0U) << LPIT_MCR_DOZE_EN_SHIFT);

    /* Enable/Disable Debug(freeze) mode. */
    lpitBase[instance]->MCR |= (uint32)((uint32)(userConfig->debugState?1U:0U) << LPIT_MCR_DBG_EN_SHIFT);
        
    for (counter = 0U; counter < userConfig->numberOfChannels; counter++)
    {
        lpitChConfig = &(*userConfig->pChannelsConfig)[counter];

        /* Enables the LPIT timer channel. */
        lpitBase[instance]->TMR[lpitChConfig->hwChannel].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;

        /* Disable channel interrupts. */
        lpitBase[instance]->MIER &= ~(uint32)(LPIT_MIER_TIE_START_MASK << (uint32)lpitChConfig->hwChannel);

        /* Clear pending interrupts. */
        lpitBase[instance]->MSR |= (uint32)(LPIT_MSR_TIF_START_MASK << (uint32)lpitChConfig->hwChannel);


        /* Select source of trigger as internal/external trigger. */
        lpitBase[instance]->TMR[lpitChConfig->hwChannel].TCTRL |= (uint32)((uint32)lpitChConfig->triggerSource << LPIT_TMR_TCTRL_TRG_SRC_SHIFT);

        /* Selects the trigger to use for starting and/or reloading the LPIT timer. */
        lpitBase[instance]->TMR[lpitChConfig->hwChannel].TCTRL |= (uint32)((uint32)lpitChConfig->triggerSelect << LPIT_TMR_TCTRL_TRG_SEL_SHIFT);

        /* Enter Trigger Input Capture Mode */
        lpitBase[instance]->TMR[lpitChConfig->hwChannel].TCTRL |= LPIT_TMR_TCTRL_MODE_MASK;

        channelsState[instance][lpitChConfig->hwChannel].initState = TRUE;
        channelsState[instance][lpitChConfig->hwChannel].callback = lpitChConfig->callback;
        channelsState[instance][lpitChConfig->hwChannel].callbackParams = lpitChConfig->callbackParams;
        channelsState[instance][lpitChConfig->hwChannel].lpitChannelNotify = lpitChConfig->lpitChannelNotify;
        channelsState[instance][lpitChConfig->hwChannel].logicChStateCallback = lpitChConfig->logicChStateCallback;

#if (STD_ON == LPIT_ICU_TIMESTAMP_API)
        /* Save state for the timestamp APIs. */
        timestampState[instance][lpitChConfig->hwChannel].timestampBuffer     = lpitChConfig->timestampBuffer;
#endif
    }
    return LPIT_IP_STATUS_SUCCESS;
}

#if (LPIT_ICU_DEINIT_API == STD_ON)
/** @implements Lpit_Icu_Ip_Deinit_Activity */
Lpit_Icu_Ip_StatusType Lpit_Icu_Ip_Deinit(uint8 instance)
{
    uint8  currentCh;

    /* Enable hardware module clock. This shall be be executed before any other setup is made */
    lpitBase[instance]->MCR |= LPIT_MCR_M_CEN_MASK;

    for (currentCh=0U; currentCh < LPIT_TMR_COUNT; currentCh++)
    {
        /* Disable the LPIT timer channel. */
        lpitBase[instance]->TMR[currentCh].TCTRL &= ~LPIT_TMR_TCTRL_T_EN_MASK;

        /* Disable channel interrupts. */
        lpitBase[instance]->MIER &= ~(uint32)(LPIT_MIER_TIE_START_MASK << (uint32)currentCh);

        /* Clear interrupt flag. */
        lpitBase[instance]->MSR |= (uint32)((uint32)LPIT_MSR_TIF0_MASK << (uint32)currentCh);


        /* Update state variable. */
        channelsState[instance][currentCh].initState = FALSE;
    }
    /* Clear debug mode. */
    lpitBase[instance]->MCR &= ~LPIT_MCR_DBG_EN_MASK;
    
    /* Disable DOZE mode. */
    lpitBase[instance]->MCR &= ~LPIT_MCR_DOZE_EN_MASK;
        
    /* Disable hardware module clock */
    lpitBase[instance]->MCR &= ~LPIT_MCR_M_CEN_MASK;
    return LPIT_IP_STATUS_SUCCESS;
}
#endif /* LPIT_ICU_DEINIT_API */

#if (LPIT_ICU_IP_SET_MODE_API == STD_ON)
/**
* @brief      Driver function that sets Lpit hardware channel into SLEEP mode.
* @details    This function enables the interrupt if wakeup is enabled for corresponding 
*             Lpit channel
* 
* @param[in]  hwChannel       - IRQ HW Channel
* 
* @return void
* 
* */

void Lpit_Icu_Ip_SetSleepMode(uint8 instance, uint8 hwChannel)
{
    /* Disable IRQ Interrupt */
    Lpit_Icu_Ip_DisableInterrupt(instance, hwChannel);
}

/**
* @brief      Driver function that sets the Lpit hardware channel into NORMAL mode.
* @details    This function enables the interrupt if Notification is enabled for corresponding
*             Lpit channel
* 
* @param[in]  hwChannel - IRQ HW Channel
* 
* @return void
* 
* */

void Lpit_Icu_Ip_SetNormalMode(uint8 instance, uint8 hwChannel)
{
    /* Enable IRQ Interrupt */
    Lpit_Icu_Ip_EnableInterrupt(instance, hwChannel);
}
#endif  /* LPIT_ICU_IP_SET_MODE_API */

/** @implements Lpit_Icu_Ip_EnableInterrupt_Activity */
void Lpit_Icu_Ip_EnableInterrupt(uint8 instance, uint8 hwChannel)
{
    /* Clear timer interrupt flag if it is enabled. */
    lpitBase[instance]->MSR |= (uint32)(LPIT_MSR_TIF_START_MASK << (uint32)hwChannel);

    /* Enable channel interrupts. */
    lpitBase[instance]->MIER |= (uint32)(LPIT_MIER_TIE_START_MASK << (uint32)hwChannel);
}

/** @implements Lpit_Icu_Ip_DisableInterrupt_Activity */
void Lpit_Icu_Ip_DisableInterrupt(uint8 instance, uint8 hwChannel)
{
    /* Disable channel interrupts. */
    lpitBase[instance]->MIER &= ~(uint32)(LPIT_MIER_TIE_START_MASK << (uint32)hwChannel);
}

#if (LPIT_ICU_EDGE_DETECT_API == STD_ON)
/** @implements Lpit_Icu_Ip_EnableEdgeDetection_Activity */
void Lpit_Icu_Ip_EnableEdgeDetection(uint8 instance, uint8 hwChannel)
{
    /* Enable interrupt for channel. */
    Lpit_Icu_Ip_EnableInterrupt(instance, hwChannel);
    /* Set Edge Detect mode for the Lpit channel in the configuration array */
    channelsState[instance][hwChannel].measurementMode = LPIT_ICU_MODE_SIGNAL_EDGE_DETECT;
}
#endif /* LPIT_ICU_EDGE_DETECT_API */

#if ((LPIT_ICU_EDGE_DETECT_API == STD_ON) || (LPIT_ICU_TIMESTAMP_API == STD_ON))
/** @implements Lpit_Icu_Ip_DisableDetectionMode_Activity */
void Lpit_Icu_Ip_DisableDetectionMode(uint8 instance, uint8 hwChannel)
{
    /* Disable interrupt for channel. */
    Lpit_Icu_Ip_DisableInterrupt(instance, hwChannel);
    /* Clean measurement mode for the LPIT channel in the configuration array. */
    channelsState[instance][hwChannel].measurementMode = LPIT_ICU_MODE_NO_MEASUREMENT;
}
#endif /* (LPIT_ICU_EDGE_DETECT_API == STD_ON) || (LPIT_ICU_TIMESTAMP_API == STD_ON) */

#if (STD_ON == LPIT_ICU_GET_INPUT_STATE_API)
/** @implements Lpit_Icu_Ip_GetInputState_Activity */
boolean Lpit_Icu_Ip_GetInputState(uint8 instance, uint8 hwChannel)
{
    boolean result         = FALSE;
    uint32  statusRegister = lpitBase[instance]->MSR;
    uint32  intEnRegister  = lpitBase[instance]->MIER;

    /* Interrupt not enabled, flag bit was set. */
    if (0x0U == ((uint32)(LPIT_MIER_TIE_START_MASK << hwChannel) & intEnRegister))
    {
        if (0x0U != ((uint32)(LPIT_MSR_TIF_START_MASK << hwChannel) & statusRegister))
        {
            result = TRUE;
            /* Clear interrupt flag. */
            lpitBase[instance]->MSR |= (uint32)(LPIT_MSR_TIF_START_MASK << hwChannel);
        }
    }
    return result;
}
#endif /* STD_ON == LPIT_ICU_GET_INPUT_STATE_API */

#if (STD_ON == LPIT_ICU_TIMESTAMP_API)
/** @implements Lpit_Icu_Ip_StartTimestamp_Activity */
void Lpit_Icu_Ip_StartTimestamp(uint8 instance,
                                uint8 hwChannel,
                                uint32 *bufferPtr,
                                uint16 bufferSize,
                                uint16 notifyInterval)
{
#if (STD_ON == LPIT_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(LPIT_INSTANCE_COUNT > instance);
    DevAssert(LPIT_TMR_COUNT > hwChannel);
    DevAssert(NULL_PTR != lpitBase[instance]);
#endif
    /* Configure the Timestamp mode for the LPIT hwChannel in the state configuration array. */
    channelsState[instance][hwChannel].measurementMode = LPIT_ICU_MODE_TIMESTAMP;

    /* Initiate timestamp state for current channel. */
    timestampState[instance][hwChannel].bufferPtr      = bufferPtr;
    timestampState[instance][hwChannel].bufferSize     = bufferSize;
    timestampState[instance][hwChannel].notifyInterval = notifyInterval;
    timestampState[instance][hwChannel].notifyCount    = (uint16)0U;
    timestampState[instance][hwChannel].bufferIndex    = (uint16)0U;

    /* Clear interrupt flag. */
    lpitBase[instance]->MSR |= (uint32)(LPIT_MSR_TIF_START_MASK << (uint32)hwChannel);

    /* Enable interrupt for channel. */
    Lpit_Icu_Ip_EnableInterrupt(instance, hwChannel);
}

/**
 * @brief      Get timestamp index for timestamp mode.
 * @implements Lpit_Icu_Ip_GetTimestampIndex_Activity
 */
uint16 Lpit_Icu_Ip_GetTimestampIndex(uint8 instance, uint8 channel)
{
#if (STD_ON == LPIT_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(LPIT_INSTANCE_COUNT > instance);
    DevAssert(LPIT_TMR_COUNT > channel);
#endif
    uint16 timestampIndex = 0U;
    if (NULL_PTR != timestampState[instance][channel].bufferPtr)
    {
        timestampIndex = timestampState[instance][channel].bufferIndex;
    }
    return timestampIndex;
}

#endif /* STD_ON == LPIT_ICU_TIMESTAMP_API */

/**
 * @brief      Driver function Enable Notification for timestamp.
 * @implements Lpit_Icu_Ip_EnableNotification_Activity
 */
void Lpit_Icu_Ip_EnableNotification(uint8 instance, uint8 hwChannel)
{
#if (STD_ON == LPIT_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(LPIT_INSTANCE_COUNT > instance);
    DevAssert(LPIT_TMR_COUNT > hwChannel);
#endif

    channelsState[instance][hwChannel].notificationEnable = TRUE;
}

/**
 * @brief      Driver function Disable Notification for timestamp.
 * @implements Lpit_Icu_Ip_DisableNotification_Activity
 */
void Lpit_Icu_Ip_DisableNotification(uint8 instance, uint8 hwChannel)
{
#if (STD_ON == LPIT_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(LPIT_INSTANCE_COUNT > instance);
    DevAssert(LPIT_TMR_COUNT > hwChannel);
#endif

    channelsState[instance][hwChannel].notificationEnable = FALSE;
}

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"


#ifdef __cplusplus
}
#endif

/** @} */
