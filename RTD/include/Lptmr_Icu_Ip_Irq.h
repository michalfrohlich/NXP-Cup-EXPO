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

#ifndef LPTMR_ICU_IP_IRQ_H
#define LPTMR_ICU_IP_IRQ_H

/**
 *   @file       Interrupt functions description.
 *   @details    This file contains the signature of interrups functions that can be used on the 
 *               LPTMR module.
 *   @addtogroup lptmr_icu_ip LPTMR IPL
 *   @{
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
#include "OsIf.h"
#include "Lptmr_Icu_Ip_Cfg.h"
#include "Lptmr_Icu_Ip_Types.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define LPTMR_ICU_IP_IRQ_VENDOR_ID                   43
#define LPTMR_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION    4
#define LPTMR_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION    7
#define LPTMR_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION 0
#define LPTMR_ICU_IP_IRQ_SW_MAJOR_VERSION            2
#define LPTMR_ICU_IP_IRQ_SW_MINOR_VERSION            0
#define LPTMR_ICU_IP_IRQ_SW_PATCH_VERSION            0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if header file and OsIf.h file are of the same Autosar version */
    #if ((LPTMR_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION != OSIF_AR_RELEASE_MAJOR_VERSION) || \
         (LPTMR_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION != OSIF_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Lptmr_Icu_Ip_Irq.h and OsIf.h are different"
    #endif
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (LPTMR_ICU_IP_IRQ_VENDOR_ID != LPTMR_ICU_IP_CFG_VENDOR_ID)
    #error "Lptmr_Icu_Ip_Irq.h and Lptmr_Icu_Ip_Cfg.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((LPTMR_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION    != LPTMR_ICU_IP_CFG_AR_RELEASE_MAJOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION    != LPTMR_ICU_IP_CFG_AR_RELEASE_MINOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION != LPTMR_ICU_IP_CFG_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Lptmr_Icu_Ip_Irq.h and Lptmr_Icu_Ip_Cfg.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((LPTMR_ICU_IP_IRQ_SW_MAJOR_VERSION != LPTMR_ICU_IP_CFG_SW_MAJOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_SW_MINOR_VERSION != LPTMR_ICU_IP_CFG_SW_MINOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_SW_PATCH_VERSION != LPTMR_ICU_IP_CFG_SW_PATCH_VERSION))
#error "Software Version Numbers of Lptmr_Icu_Ip_Irq.h and Lptmr_Icu_Ip_Cfg.h are different"
#endif

/* Check if header file and Lptmr_Icu_Ip_Types.h header file are of the same vendor */
#if (LPTMR_ICU_IP_IRQ_VENDOR_ID !=  LPTMR_ICU_IP_TYPES_VENDOR_ID)
    #error "Lptmr_Icu_Ip_Irq.h and Lptmr_Icu_Ip_Types.h have different vendor IDs"
#endif
/* Check if header file and Lptmr_Icu_Ip_Types.h header file are of the same AutoSar version */
#if ((LPTMR_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION    != LPTMR_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION    != LPTMR_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION != LPTMR_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Lptmr_Icu_Ip_Irq.h and LPtmr_Icu_Ip_Types.h are different"
#endif
/* Check if header file and Lptmr_Icu_Ip_Types.h header file file are of the same Software version */
#if ((LPTMR_ICU_IP_IRQ_SW_MAJOR_VERSION != LPTMR_ICU_IP_TYPES_SW_MAJOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_SW_MINOR_VERSION != LPTMR_ICU_IP_TYPES_SW_MINOR_VERSION) || \
     (LPTMR_ICU_IP_IRQ_SW_PATCH_VERSION != LPTMR_ICU_IP_TYPES_SW_PATCH_VERSION))
    #error "Software Version Numbers of Lptmr_Icu_Ip_Irq.h and Lptmr_Icu_Ip_Types.h are different"
#endif

/*==================================================================================================
*                                        LOCAL CONSTANTS
==================================================================================================*/
#define ICU_START_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"
/* Table of base addresses for LPTMR instances. */
extern Lptmr_Icu_Ip_BaseType * const s_lptmrBase[LPTMR_ICU_IP_NUM_OF_MODULES];

#define ICU_STOP_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"

/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/
#define ICU_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"

extern Lptmr_Icu_Ip_ChStateType Lptmr_Icu_aChConfig[LPTMR_ICU_IP_NUM_OF_MODULES];

#define ICU_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"

/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"
/**
 * @brief LPTRM_0 Channel 0 independent ISR declarations
*/
#if (defined LPTMR_ICU_0_CH_0_ISR_USED)
ISR(LPTMR_0_CH_0_ISR);
#endif

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"


#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* LPTMR_ICU_IP_IRQ_H */
