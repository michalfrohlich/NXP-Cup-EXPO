
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

#ifndef ICU_IPW_IRQ_H
#define ICU_IPW_IRQ_H

/**
 *     @file
 *     @internal
 *     @addtogroup icu_ipw Icu_Ipw Driver
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
#include "Ftm_Icu_Ip_Irq.h"
#include "Lpit_Icu_Ip_Irq.h"
#include "Lptmr_Icu_Ip_Irq.h"
#include "Port_Ci_Icu_Ip_Irq.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define ICU_IPW_IRQ_VENDOR_ID                   43
#define ICU_IPW_IRQ_AR_RELEASE_MAJOR_VERSION    4
#define ICU_IPW_IRQ_AR_RELEASE_MINOR_VERSION    7
#define ICU_IPW_IRQ_AR_RELEASE_REVISION_VERSION 0
#define ICU_IPW_IRQ_SW_MAJOR_VERSION            2
#define ICU_IPW_IRQ_SW_MINOR_VERSION            0
#define ICU_IPW_IRQ_SW_PATCH_VERSION            0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if source file and ICU header file are of the same vendor */
#if (ICU_IPW_IRQ_VENDOR_ID != FTM_ICU_IP_IRQ_VENDOR_ID)
    #error "Icu_Ipw_Irq.h and Ftm_Icu_Ip_Irq.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((ICU_IPW_IRQ_AR_RELEASE_MAJOR_VERSION    != FTM_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION) || \
     (ICU_IPW_IRQ_AR_RELEASE_MINOR_VERSION    != FTM_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION) || \
     (ICU_IPW_IRQ_AR_RELEASE_REVISION_VERSION != FTM_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Icu_Ipw_Irq.h and Ftm_Icu_Ip_Irq.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((ICU_IPW_IRQ_SW_MAJOR_VERSION != FTM_ICU_IP_IRQ_SW_MAJOR_VERSION) || \
     (ICU_IPW_IRQ_SW_MINOR_VERSION != FTM_ICU_IP_IRQ_SW_MINOR_VERSION) || \
     (ICU_IPW_IRQ_SW_PATCH_VERSION != FTM_ICU_IP_IRQ_SW_PATCH_VERSION))
    #error "Software Version Numbers of Icu_Ipw_Irq.h and Ftm_Icu_Ip_Irq.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (ICU_IPW_IRQ_VENDOR_ID != LPIT_ICU_IP_IRQ_VENDOR_ID)
    #error "Icu_Ipw_Irq.h and Lpit_Icu_Ip_Irq.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((ICU_IPW_IRQ_AR_RELEASE_MAJOR_VERSION    != LPIT_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION) || \
     (ICU_IPW_IRQ_AR_RELEASE_MINOR_VERSION    != LPIT_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION) || \
     (ICU_IPW_IRQ_AR_RELEASE_REVISION_VERSION != LPIT_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Icu_Ipw_Irq.h and Lpit_Icu_Ip_Irq.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((ICU_IPW_IRQ_SW_MAJOR_VERSION != LPIT_ICU_IP_IRQ_SW_MAJOR_VERSION) || \
     (ICU_IPW_IRQ_SW_MINOR_VERSION != LPIT_ICU_IP_IRQ_SW_MINOR_VERSION) || \
     (ICU_IPW_IRQ_SW_PATCH_VERSION != LPIT_ICU_IP_IRQ_SW_PATCH_VERSION))
    #error "Software Version Numbers of Icu_Ipw_Irq.h and Lpit_Icu_Ip_Irq.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (ICU_IPW_IRQ_VENDOR_ID != LPTMR_ICU_IP_IRQ_VENDOR_ID)
    #error "Icu_Ipw_Irq.h and Lptmr_Icu_Ip_Irq.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((ICU_IPW_IRQ_AR_RELEASE_MAJOR_VERSION  != LPTMR_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION) || \
     (ICU_IPW_IRQ_AR_RELEASE_MINOR_VERSION  != LPTMR_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION) || \
     (ICU_IPW_IRQ_AR_RELEASE_REVISION_VERSION   != LPTMR_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Icu_Ipw_Irq.h and Lptmr_Icu_Ip_Irq.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((ICU_IPW_IRQ_SW_MAJOR_VERSION  != LPTMR_ICU_IP_IRQ_SW_MAJOR_VERSION) || \
     (ICU_IPW_IRQ_SW_MINOR_VERSION  != LPTMR_ICU_IP_IRQ_SW_MINOR_VERSION) || \
     (ICU_IPW_IRQ_SW_PATCH_VERSION  != LPTMR_ICU_IP_IRQ_SW_PATCH_VERSION))
    #error "Software Version Numbers of Icu_Ipw_Irq.h and Lptmr_Icu_Ip_Irq.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (ICU_IPW_IRQ_VENDOR_ID != PORT_CI_ICU_IP_IRQ_VENDOR_ID)
    #error "Icu_Ipw_Irq.h and Port_Ci_Icu_Ip_Irq.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((ICU_IPW_IRQ_AR_RELEASE_MAJOR_VERSION    != PORT_CI_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION) || \
     (ICU_IPW_IRQ_AR_RELEASE_MINOR_VERSION    != PORT_CI_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION) || \
     (ICU_IPW_IRQ_AR_RELEASE_REVISION_VERSION != PORT_CI_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Icu_Ipw_Irq.h and Port_Ci_Icu_Ip_Irq.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((ICU_IPW_IRQ_SW_MAJOR_VERSION != PORT_CI_ICU_IP_IRQ_SW_MAJOR_VERSION) || \
     (ICU_IPW_IRQ_SW_MINOR_VERSION != PORT_CI_ICU_IP_IRQ_SW_MINOR_VERSION) || \
     (ICU_IPW_IRQ_SW_PATCH_VERSION != PORT_CI_ICU_IP_IRQ_SW_PATCH_VERSION))
    #error "Software Version Numbers of Icu_Ipw_Irq.h and Port_Ci_Icu_Ip_Irq.h are different"
#endif

/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

/*==================================================================================================
*                                              ENUMS
==================================================================================================*/

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/

#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"


#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* ICU_IPW_IRQ_H */
