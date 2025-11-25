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
 *     @file       File with source code used to implement ICU driver interrupts on LPTMR module.
 *     @details    This file contains the source code for all interrupt functions which are using 
 *                 LPTMR module.
 *     @addtogroup lptmr_icu_ip LPTMR IPL
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
#include "Lptmr_Icu_Ip_Irq.h"
#include "Mcal.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define LPTMR_ICU_IP_IRQ_VENDOR_ID_C                   43
#define LPTMR_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C    4
#define LPTMR_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C    7
#define LPTMR_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION_C 0
#define LPTMR_ICU_IP_IRQ_SW_MAJOR_VERSION_C            2
#define LPTMR_ICU_IP_IRQ_SW_MINOR_VERSION_C            0
#define LPTMR_ICU_IP_IRQ_SW_PATCH_VERSION_C            0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if source file and Lptmr_Icu_Ip_Irq.h header file are of the same vendor */
#if (LPTMR_ICU_IP_IRQ_VENDOR_ID_C != LPTMR_ICU_IP_IRQ_VENDOR_ID)
    #error "Lptmr_Icu_Ip_Irq.c and Lptmr_Icu_Ip_Irq.h have different vendor IDs"
#endif
/* Check if source file and Lptmr_Icu_Ip_Irq.h header file are of the same AutoSar version */
#if ((LPTMR_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C    != LPTMR_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C    != LPTMR_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION_C != LPTMR_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Lptmr_Icu_Ip_Irq.c and Lptmr_Icu_Ip_Irq.h are different"
#endif
/* Check if source file and Lptmr_Icu_Ip_Irq.h header file are of the same Software version */
#if ((LPTMR_ICU_IP_IRQ_SW_MAJOR_VERSION_C != LPTMR_ICU_IP_IRQ_SW_MAJOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_SW_MINOR_VERSION_C != LPTMR_ICU_IP_IRQ_SW_MINOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_SW_PATCH_VERSION_C != LPTMR_ICU_IP_IRQ_SW_PATCH_VERSION))
    #error "Software Version Numbers of Lptmr_Icu_Ip_Irq.c and Lptmr_Icu_Ip_Irq.h are different"
#endif

#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if this header file and Mcal.h file are of the same Autosar version */
    #if ((LPTMR_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C != MCAL_AR_RELEASE_MAJOR_VERSION) || \
        (LPTMR_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C != MCAL_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Lptmr_Icu_Ip_Irq.c and Mcal.h are different"
    #endif
#endif
/*==================================================================================================
*                                    LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"

#if (defined LPTMR_ICU_0_CH_0_ISR_USED)
static inline void Lptmr_Icu_Ip_ClearCompareFlagIrq(uint8 instance);
static inline boolean Lptmr_Icu_Ip_GetCmpFlagStateIrq(uint8 instance);
static inline boolean Lptmr_Icu_Ip_GetInterruptBitIrq(uint8 instance);
static inline void Lptmr_Icu_Ip_ProcessInterruptIrq(uint8 instance);

#if (STD_ON == LPTMR_ICU_EDGE_DETECT_API)
/**
 * @brief
 * 
 * @param instance
 * @param overflow
 */
static inline void Lptmr_Icu_Ip_ReportEvents(uint16 instance);
#endif
#endif

/*==================================================================================================
*                                        LOCAL FUNCTIONS
==================================================================================================*/
#if (defined LPTMR_ICU_0_CH_0_ISR_USED)
/*!
 * @brief Clear the Compare Flag
 *
 * This function clears the Compare Flag/Interrupt Pending state.
 *
 * @param[in] base - LPTMR base pointer
 */
static inline void Lptmr_Icu_Ip_ClearCompareFlagIrq(uint8 instance)
{
    s_lptmrBase[instance]->CSR |= LPTMR_CSR_TCF_MASK;
}

/**
* @brief         Lptmr_Icu_Ip_GetCmpFlagStateIrq Get the Compare Flag state
* @details       This function checks whether a Compare Match event has occurred or if there is an Interrupt Pending.
* @param[in]     instance     LPtimer hw instance number
*
* @return       The Compare Flag state
*               - true: Compare Match/Interrupt Pending asserted
*               - false: Compare Match/Interrupt Pending not asserted
* @pre           The driver needs to be initialized.
*
*/
static inline boolean Lptmr_Icu_Ip_GetCmpFlagStateIrq(uint8 instance)
{
    uint32 CmpFlagState;
    CmpFlagState = (s_lptmrBase[instance]->CSR & LPTMR_CSR_TCF_MASK) >> LPTMR_CSR_TCF_SHIFT;
    return ((CmpFlagState == 1u) ? TRUE : FALSE);
}

/**
* @brief        : Lptmr_Icu_Ip_GetInterruptBitIrq get the bit CSR_TIE (CSR)
* @details       This function checks whether Timer Interrupt enable or not
* @param[in]     instance       LPtimer hardware instance number              
*
* @return        status
*
*/
static inline boolean Lptmr_Icu_Ip_GetInterruptBitIrq(uint8 instance)
{ 
    uint32 status;
    status = (s_lptmrBase[instance]->CSR & LPTMR_CSR_TIE_MASK) >> LPTMR_CSR_TIE_SHIFT;
    return ((status == 1u) ? TRUE : FALSE);
}

/**
* @brief   Lptmr_Icu_Ip_ProcessInterruptIrq.
* @details Function used by interrupt service routines to call notification
*          functions if provided and enabled
*
* @param[in]     channel      hardware channel index
* @implements Lptmr_Icu_Ip_ProcessInterruptIrq_Activity
*/
static inline void Lptmr_Icu_Ip_ProcessInterruptIrq(uint8 instance)
{
    /* Call handler */
    if (FALSE == Lptmr_Icu_aChConfig[instance].chInit)
    {
        /* Clear Interrupt Flag */
        Lptmr_Icu_Ip_ClearCompareFlagIrq(instance);
    }
    else
    {
        if (Lptmr_Icu_Ip_GetInterruptBitIrq(instance))
        {
            /* Clear Interrupt Flag */
            Lptmr_Icu_Ip_ClearCompareFlagIrq(instance);
#if (STD_ON == LPTMR_ICU_EDGE_DETECT_API)
            switch (Lptmr_Icu_aChConfig[instance].measMode)
            {
                case LPTMR_ICU_MODE_SIGNAL_EDGE_DETECT:
                {
                    Lptmr_Icu_Ip_ReportEvents(instance);
                }
                break;
                default: /* case LPTMR_ICU_EDGE_COUNT:*/
                {
                    /* Do nothing */
                }
                break;
            }   
#endif
        }
    }
}

#if (STD_ON == LPTMR_ICU_EDGE_DETECT_API)
static inline void Lptmr_Icu_Ip_ReportEvents(uint16 instance)
{
    /* Calling HLD Report Events for the logical channel */
    if(Lptmr_Icu_aChConfig[instance].callback != NULL_PTR)
    {
        Lptmr_Icu_aChConfig[instance].callback(Lptmr_Icu_aChConfig[instance].callbackParam, FALSE);
    }
    else
    {
        /* Calling Notification for the IPL channel */
        if ((NULL_PTR != Lptmr_Icu_aChConfig[instance].lptmrChannelNotification) && \
           ((boolean)TRUE == Lptmr_Icu_aChConfig[instance].chInit) && \
           ((boolean)TRUE == Lptmr_Icu_aChConfig[instance].notificationEnable))
        {
            Lptmr_Icu_aChConfig[instance].lptmrChannelNotification();
        }
    }
}
#endif /* STD_ON == LPTMR_ICU_EDGE_DETECT_API */
#endif /* defined LPTMR_ICU_0_CH_0_ISR_USED */

/*==================================================================================================
*                                        GLOBAL FUNCTIONS
==================================================================================================*/

#if (defined LPTMR_ICU_0_CH_0_ISR_USED)
ISR(LPTMR_0_CH_0_ISR)
{
    uint8 instance = 0U;
    /* Read TCF value and verify if it was an interrupt. */
    if (TRUE == (Lptmr_Icu_Ip_GetCmpFlagStateIrq(instance)))
    {
        Lptmr_Icu_Ip_ProcessInterruptIrq(instance);
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
