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
 *     @file       File with source code used to implement ICU driver interrupts on LPIT module.
 *     @details    This file contains the source code for all interrupt functions which are using 
 *                 LPIT module.
 *     @addtogroup lpit_icu_ip LPIT IPL
 *     @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
*                                          INCLUDE FILES
*  1) system and project includes
*  2) needed interfaces from external units
*  3) internal and external interfaces from this unit
==================================================================================================*/
#include "Lpit_Icu_Ip.h"
#include "Lpit_Icu_Ip_Irq.h"

#if(LPIT_ICU_IP_DEV_ERROR_DETECT == STD_ON)
#include "Devassert.h"
#endif
#include "Mcal.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define LPIT_ICU_IP_IRQ_VENDOR_ID_C                   43
#define LPIT_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C    4
#define LPIT_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C    7
#define LPIT_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION_C 0
#define LPIT_ICU_IP_IRQ_SW_MAJOR_VERSION_C            2
#define LPIT_ICU_IP_IRQ_SW_MINOR_VERSION_C            0
#define LPIT_ICU_IP_IRQ_SW_PATCH_VERSION_C            0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if source file and Lpit_Icu_Ip.h header file are of the same vendor */
#if (LPIT_ICU_IP_IRQ_VENDOR_ID_C != LPIT_ICU_IP_VENDOR_ID)
    #error "Lpit_Icu_Ip_Irq.c and Lpit_Icu_Ip.h have different vendor IDs"
#endif
/* Check if source file and Lpit_Icu_Ip.h header file are of the same AutoSar version */
#if ((LPIT_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C    != LPIT_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (LPIT_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C    != LPIT_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (LPIT_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION_C != LPIT_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Lpit_Icu_Ip_Irq.c and Lpit_Icu_Ip.h are different"
#endif
/* Check if source file and Lpit_Icu_Ip.h header file are of the same Software version */
#if ((LPIT_ICU_IP_IRQ_SW_MAJOR_VERSION_C != LPIT_ICU_IP_SW_MAJOR_VERSION) || \
     (LPIT_ICU_IP_IRQ_SW_MINOR_VERSION_C != LPIT_ICU_IP_SW_MINOR_VERSION) || \
     (LPIT_ICU_IP_IRQ_SW_PATCH_VERSION_C != LPIT_ICU_IP_SW_PATCH_VERSION))
    #error "Software Version Numbers of Lpit_Icu_Ip_Irq.c and Lpit_Icu_Ip.h are different"
#endif

/* Check if source file and Lpit_Icu_Ip_Irq.h header file are of the same vendor */
#if (LPIT_ICU_IP_IRQ_VENDOR_ID_C != LPIT_ICU_IP_IRQ_VENDOR_ID)
    #error "Lpit_Icu_Ip_Irq.c and Lpit_Icu_Ip_Irq.h have different vendor IDs"
#endif
/* Check if source file and Lpit_Icu_Ip_Irq.h header file are of the same AutoSar version */
#if ((LPIT_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C    != LPIT_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION) || \
     (LPIT_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C    != LPIT_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION) || \
     (LPIT_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION_C != LPIT_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Lpit_Icu_Ip_Irq.c and Lpit_Icu_Ip_Irq.h are different"
#endif
/* Check if source file and Lpit_Icu_Ip_Irq.h header file are of the same Software version */
#if ((LPIT_ICU_IP_IRQ_SW_MAJOR_VERSION_C != LPIT_ICU_IP_IRQ_SW_MAJOR_VERSION) || \
     (LPIT_ICU_IP_IRQ_SW_MINOR_VERSION_C != LPIT_ICU_IP_IRQ_SW_MINOR_VERSION) || \
     (LPIT_ICU_IP_IRQ_SW_PATCH_VERSION_C != LPIT_ICU_IP_IRQ_SW_PATCH_VERSION))
    #error "Software Version Numbers of Lpit_Icu_Ip_Irq.c and Lpit_Icu_Ip_Irq.h are different"
#endif

#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    #if(LPIT_ICU_IP_DEV_ERROR_DETECT == STD_ON)
        /* Check if source file and Devassert.h file are of the same Autosar version */
        #if ((LPIT_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C != DEVASSERT_AR_RELEASE_MAJOR_VERSION) || \
             (LPIT_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C != DEVASSERT_AR_RELEASE_MINOR_VERSION))
            #error "AutoSar Version Numbers of Lpit_Icu_Ip.h and Devassert.h are different"
        #endif
    #endif
    /* Check if this header file and Mcal.h file are of the same Autosar version */
    #if ((LPIT_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C != MCAL_AR_RELEASE_MAJOR_VERSION) || \
        (LPIT_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C != MCAL_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Lpit_Icu_Ip.c and Mcal.h are different"
    #endif
#endif

/*==================================================================================================
*                                    LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"

#if ((defined LPIT_ICU_0_CH_0_ISR_USED) || (defined LPIT_ICU_0_CH_1_ISR_USED) || (defined LPIT_ICU_0_CH_2_ISR_USED) || (defined LPIT_ICU_0_CH_3_ISR_USED) || (defined LPIT_ICU_0_ISR_USED))
#if (STD_ON == LPIT_ICU_TIMESTAMP_API)
/**
 * @brief
 * 
 * @param channel
 * @param instance
 * @internal
 */
static void Lpit_Icu_Ip_TimestampHandler(uint8 instance, uint8 channel);
#endif /* STD_ON == LPIT_ICU_TIMESTAMP_API */

/**
 * @brief   LPIT IP function that handles the interrupt of a LPIT channel for ICU driver.
 * @details This function:
 *           - Reads the status register
 *           - Clears the pending interrupt
 *           - Processes interrupt for corresponding LPIT channel
 *
 * @param[in]  instance LPIT hardware module
 * @param[in]  channel  LPIT hardware channel
 */
static inline void Lpit_Icu_Ip_IrqHandler(uint8 instance, uint8 channel);

#if (STD_ON == LPIT_ICU_EDGE_DETECT_API)
/**
 * @brief 
 * 
 * @param instance 
 * @param hwChannel 
 */
static inline void Lpit_Icu_Ip_ReportEvents(uint8 instance, uint8 hwChannel);
#endif /* STD_ON == LPIT_ICU_EDGE_DETECT_API */
#endif /* ISRs USED defines */

/*==================================================================================================
*                                        LOCAL FUNCTIONS
==================================================================================================*/
#if ((defined LPIT_ICU_0_CH_0_ISR_USED) || (defined LPIT_ICU_0_CH_1_ISR_USED) || (defined LPIT_ICU_0_CH_2_ISR_USED) || (defined LPIT_ICU_0_CH_3_ISR_USED) || (defined LPIT_ICU_0_ISR_USED))
#if (STD_ON == LPIT_ICU_TIMESTAMP_API)
static void Lpit_Icu_Ip_TimestampHandler(uint8 instance,
                                         uint8 channel)
{
    uint16 chBufferSize;
    uint16 chBufferNotify;

    chBufferSize = timestampState[instance][channel].bufferSize;
    chBufferNotify = timestampState[instance][channel].notifyInterval;

    timestampState[instance][channel].bufferPtr[timestampState[instance][channel].bufferIndex] = (uint32)lpitBase[instance]->TMR[channel].CVAL;
    timestampState[instance][channel].bufferIndex++;

    if (timestampState[instance][channel].bufferIndex >= chBufferSize)
    {
        /* If circular buffer, loop; if linear buffer, terminate. */
        if (LPIT_ICU_IP_CIRCULAR_BUFFER == timestampState[instance][channel].timestampBuffer)
        {
            timestampState[instance][channel].bufferIndex = 0U;
        }
        else
        {
            /* Linear buffer is full, stop the channel */
            Lpit_Icu_Ip_DisableDetectionMode(instance, channel);
            if (NULL_PTR != channelsState[instance][channel].logicChStateCallback)
            {
                channelsState[instance][channel].logicChStateCallback(channelsState[instance][channel].callbackParams, (1U << 3), FALSE);
            }
        }
    }

    if (0U != chBufferNotify)
    {
        timestampState[instance][channel].notifyCount++;
        if (timestampState[instance][channel].notifyCount >= chBufferNotify)
        {
            timestampState[instance][channel].notifyCount = 0U;
            /* Call User Notification Function */
            if ( (NULL_PTR != channelsState[instance][channel].lpitChannelNotify) && \
                 ((boolean)TRUE == channelsState[instance][channel].notificationEnable))
            {
                channelsState[instance][channel].lpitChannelNotify();
            }
        }
    }
}
#endif /* STD_ON == LPIT_ICU_TIMESTAMP_API */

/* @implements Lpit_Icu_Ip_IrqHandler_Activity */
static inline void Lpit_Icu_Ip_IrqHandler(uint8 instance, uint8 channel)
{
#if (STD_ON == LPIT_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(instance < LPIT_INSTANCE_COUNT);
    DevAssert(channel < LPIT_TMR_COUNT);
#endif

    boolean channelStatusFlags = FALSE;
    Lpit_Icu_Ip_MeasurementMode measMode = channelsState[instance][channel].measurementMode;

    /* Interrupt status: enable/disable. */
    channelStatusFlags = ((((lpitBase[instance]->MIER & ((uint32)(LPIT_MIER_TIE_START_MASK << (uint32)channel))) != 0U) && 
                         ((lpitBase[instance]->MSR & ((uint32)(LPIT_MSR_TIF_START_MASK << (uint32)channel))) != 0U)) ?
                          TRUE : FALSE);
    /* ISR shall check whether its respective driver is initialized */
    if (FALSE == channelsState[instance][channel].initState)
    {
        /*  If the driver is not initialized, the ISR shall only clear interrupt status flag  and return immediately. */
        lpitBase[instance]->MSR &= (uint32)(LPIT_MSR_TIF_START_MASK << (uint32)channel);
    }
    else
    {
        /* ISR shall check whether its respective interrupt status flag and interrupt enable flag are set. */
        if (TRUE == channelStatusFlags)
        {
            /* Clear pending interrupt serviced. */
            lpitBase[instance]->MSR &= (uint32)(LPIT_MSR_TIF_START_MASK << (uint32)channel);

            if (LPIT_ICU_MODE_NO_MEASUREMENT != measMode)
            {
                switch (measMode)
                {
    #if (STD_ON == LPIT_ICU_TIMESTAMP_API)
                    case LPIT_ICU_MODE_TIMESTAMP:
                    {
                        /* Copy the Counter value in the Timestamp Buffer*/
                        /* TODO: lpit has a counter on 32bits but we are treating in the ICU driver values on 16 bits only. */
                        /* Make the call to handler function. */
                        Lpit_Icu_Ip_TimestampHandler(instance, channel);
                        break;
                    }
    #endif /* STD_ON == LPIT_ICU_TIMESTAMP_API */
    #if (STD_ON == LPIT_ICU_EDGE_DETECT_API)
                    case LPIT_ICU_MODE_SIGNAL_EDGE_DETECT:
                    {
                        /* Call ReportEvents. */
                        Lpit_Icu_Ip_ReportEvents(instance, channel);
                    }
                    break;
    #endif
                    default:
                    {
                        /* Do nothing. */
                        break;
                    }
                }
            }
        }
    }
}

#if (STD_ON == LPIT_ICU_EDGE_DETECT_API)
static inline void Lpit_Icu_Ip_ReportEvents(uint8 instance, uint8 hwChannel)
{
    /* Calling HLD Report Events for the logical channel */
    if(channelsState[instance][hwChannel].callback != NULL_PTR)
    {
        channelsState[instance][hwChannel].callback(channelsState[instance][hwChannel].callbackParams, FALSE);
    }
    else
    {
        /* Calling Notification for the IPL channel */
        if ( (NULL_PTR != channelsState[instance][hwChannel].lpitChannelNotify) && \
             ((boolean)TRUE == channelsState[instance][hwChannel].notificationEnable))
        {
            channelsState[instance][hwChannel].lpitChannelNotify();
        }
    }
}
#endif /* STD_ON == LPIT_ICU_EDGE_DETECT_AP */
#endif /* ISRs USED defines */
/*==================================================================================================
*                                        GLOBAL FUNCTIONS
==================================================================================================*/
#if (defined LPIT_ICU_0_CH_0_ISR_USED)
/**
 * @brief   LPIT IP function that handles the interrupt of a LPIT 0 channel 0 for ICU driver.
 */
ISR(LPIT_0_CH_0_ISR)
{
    uint8 u8LPitchannel = 0U;
    uint8 u8ModuleIdx = 0U;
    uint32 u32IsrStatus = lpitBase[u8ModuleIdx]->MSR;
    /* Read MSR value and verify if it was an interrupt. */
    if ((uint8)((uint8)u32IsrStatus & (uint8)(1U << u8LPitchannel)) == (uint8)(1U << u8LPitchannel))
    {
        Lpit_Icu_Ip_IrqHandler(u8ModuleIdx, u8LPitchannel);
    }
    EXIT_INTERRUPT();
}
#endif

#if (defined LPIT_ICU_0_CH_1_ISR_USED)
/**
 * @brief   LPIT IP function that handles the interrupt of a LPIT 0 channel 1 for ICU driver.
 */
ISR(LPIT_0_CH_1_ISR)
{
    uint8 u8LPitchannel = 1U;
    uint8 u8ModuleIdx = 0U;
    uint32 u32IsrStatus = lpitBase[u8ModuleIdx]->MSR;
    /* Read MSR value and verify if it was an interrupt. */
    if ((uint8)((uint8)u32IsrStatus & (uint8)(1U << u8LPitchannel)) == (uint8)(1U << u8LPitchannel))
    {
        Lpit_Icu_Ip_IrqHandler(u8ModuleIdx, u8LPitchannel);
    }
    EXIT_INTERRUPT();
}
#endif

#if (defined LPIT_ICU_0_CH_2_ISR_USED)
/**
 * @brief   LPIT IP function that handles the interrupt of a LPIT 0 channel 2 for ICU driver.
 */
ISR(LPIT_0_CH_2_ISR)
{
    uint8 u8LPitchannel = 2U;
    uint8 u8ModuleIdx = 0U;
    uint32 u32IsrStatus = lpitBase[u8ModuleIdx]->MSR;
    /* Read MSR value and verify if it was an interrupt. */
    if ((uint8)((uint8)u32IsrStatus & (uint8)(1U << u8LPitchannel)) == (uint8)(1U << u8LPitchannel))
    {
        Lpit_Icu_Ip_IrqHandler(u8ModuleIdx, u8LPitchannel);
    }
    EXIT_INTERRUPT();
}
#endif

#if (defined LPIT_ICU_0_CH_3_ISR_USED)
/**
 * @brief   LPIT IP function that handles the interrupt of a LPIT 0 channel 3 for ICU driver.
 */
ISR(LPIT_0_CH_3_ISR)
{
    uint8 u8LPitchannel = 3U;
    uint8 u8ModuleIdx = 0U;
    uint32 u32IsrStatus = lpitBase[u8ModuleIdx]->MSR;
    /* Read MSR value and verify if it was an interrupt. */
    if ((uint8)((uint8)u32IsrStatus & (uint8)(1U << u8LPitchannel)) == (uint8)(1U << u8LPitchannel))
    {
        Lpit_Icu_Ip_IrqHandler(u8ModuleIdx, u8LPitchannel);
    }
    EXIT_INTERRUPT();
}
#endif

#if (defined LPIT_ICU_0_ISR_USED)
/**
 * @brief   LPIT IP function that handles the interrupt of a LPIT 0.
 */
ISR(LPIT_0_ISR)
{
    uint8 u8LPitchannel = 0U;
    uint8 u8ModuleIdx = 0U;
    uint32 u32IsrStatus = lpitBase[u8ModuleIdx]->MSR;
    /* Read MSR value and verify if it was an interrupt. */
    for (u8LPitchannel = 0U; u8LPitchannel < LPIT_TMR_COUNT; u8LPitchannel++)
    {
        if ((uint8)((uint8)u32IsrStatus & (uint8)(1U << u8LPitchannel)) == (uint8)(1U << u8LPitchannel))
        {
            Lpit_Icu_Ip_IrqHandler(u8ModuleIdx, u8LPitchannel);
        }
    }
    EXIT_INTERRUPT();
}
#endif

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */
