/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           : DMA,CACHE,TRGMUX,FLEXIO
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
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

/* Prevention from multiple including the same header */
#ifndef CDD_MCL_IPW_H_
#define CDD_MCL_IPW_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
*   @file    CDD_Mcl_Ipw.h
*
*   @version 2.0.0
*
*   @brief   AUTOSAR Mcl - CDD MCL Ipw header file.
*   @details
*
*   @addtogroup MCL_DRIVER MCL IPW Driver
*   @{
*/

/*==================================================================================================
                                         INCLUDE FILES
 1) system and project includes
 2) needed interfaces from external units
 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Ftm_Mcl_Ip.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define CDD_MCL_IPW_VENDOR_ID                       43
#define CDD_MCL_IPW_AR_RELEASE_MAJOR_VERSION        4
#define CDD_MCL_IPW_AR_RELEASE_MINOR_VERSION        7
#define CDD_MCL_IPW_AR_RELEASE_REVISION_VERSION     0
#define CDD_MCL_IPW_SW_MAJOR_VERSION                2
#define CDD_MCL_IPW_SW_MINOR_VERSION                0
#define CDD_MCL_IPW_SW_PATCH_VERSION                0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if header file and Ftm_Mcl_Ip.h file are of the same vendor */
#if (CDD_MCL_IPW_VENDOR_ID != FTM_MCL_IP_VENDOR_ID)
    #error "CDD_Mcl_Ipw.h and Ftm_Mcl_Ip.h have different vendor ids"
#endif

/* Check if header file and Ftm_Mcl_Ip.h file are of the same Autosar version */
#if ((CDD_MCL_IPW_AR_RELEASE_MAJOR_VERSION != FTM_MCL_IP_AR_RELEASE_MAJOR_VERSION) || \
     (CDD_MCL_IPW_AR_RELEASE_MINOR_VERSION != FTM_MCL_IP_AR_RELEASE_MINOR_VERSION) || \
     (CDD_MCL_IPW_AR_RELEASE_REVISION_VERSION != FTM_MCL_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of CDD_Mcl_Ipw.h and Ftm_Mcl_Ip.h are different"
#endif

/* Check if header file and Ftm_Mcl_Ip.h file are of the same Software version */
#if ((CDD_MCL_IPW_SW_MAJOR_VERSION != FTM_MCL_IP_SW_MAJOR_VERSION) || \
     (CDD_MCL_IPW_SW_MINOR_VERSION != FTM_MCL_IP_SW_MINOR_VERSION) || \
     (CDD_MCL_IPW_SW_PATCH_VERSION != FTM_MCL_IP_SW_PATCH_VERSION))
    #error "Software Version Numbers of CDD_Mcl_Ipw.h and Ftm_Mcl_Ip.h are different"
#endif

/*===============================================================================================
                                           ENUMS
===============================================================================================*/

/*===============================================================================================
                                           STRUCTS
===============================================================================================*/

/*==================================================================================================
*                                   GLOBAL FUNCTION PROTOTYPES
==================================================================================================*/

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* #ifndef CDD_MCL_IPW_H_ */

/*==================================================================================================
 *                                        END OF FILE
==================================================================================================*/
