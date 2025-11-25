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

#ifndef FTM_ICU_IP_TRUSTEDFUNCTIONS_H
#define FTM_ICU_IP_TRUSTEDFUNCTIONS_H

/**
 *     @file       Ftm_Ip_TrustedFunctions.h 
 *     @brief      Header file of Flextimer module.
 *     @details    This file contains some functions so that OS Application can call them  
 *     @addtogroup ftm_icu_ip FTM IPL 
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

#include "Ftm_Icu_Ip_Defines.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define FTM_ICU_IP_TRUSTEDFUNCTIONS_VENDOR_ID                    43
#define FTM_ICU_IP_TRUSTEDFUNCTIONS_AR_RELEASE_MAJOR_VERSION     4
#define FTM_ICU_IP_TRUSTEDFUNCTIONS_AR_RELEASE_MINOR_VERSION     7
#define FTM_ICU_IP_TRUSTEDFUNCTIONS_AR_RELEASE_REVISION_VERSION  0
#define FTM_ICU_IP_TRUSTEDFUNCTIONS_SW_MAJOR_VERSION             2
#define FTM_ICU_IP_TRUSTEDFUNCTIONS_SW_MINOR_VERSION             0
#define FTM_ICU_IP_TRUSTEDFUNCTIONS_SW_PATCH_VERSION             0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if this header file and Define header file are of the same vendor */
#if (FTM_ICU_IP_TRUSTEDFUNCTIONS_VENDOR_ID != FTM_ICU_IP_DEFINES_VENDOR_ID)
    #error "Ftm_Icu_Ip_TrustedFunctions.h and Ftm_Icu_Ip_Defines.h have different vendor IDs"
#endif
/* Check if this header  file and Define header file are of the same AutoSar version */
#if ((FTM_ICU_IP_TRUSTEDFUNCTIONS_AR_RELEASE_MAJOR_VERSION  != FTM_ICU_IP_DEFINES_AR_RELEASE_MAJOR_VERSION) || \
     (FTM_ICU_IP_TRUSTEDFUNCTIONS_AR_RELEASE_MINOR_VERSION  != FTM_ICU_IP_DEFINES_AR_RELEASE_MINOR_VERSION) || \
     (FTM_ICU_IP_TRUSTEDFUNCTIONS_AR_RELEASE_REVISION_VERSION   != FTM_ICU_IP_DEFINES_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Ftm_Icu_Ip_TrustedFunctions.h and Ftm_Icu_Ip_Defines.h are different"
#endif
/* Check if source file and Define header file are of the same Software version */
#if ((FTM_ICU_IP_TRUSTEDFUNCTIONS_SW_MAJOR_VERSION  != FTM_ICU_IP_DEFINES_SW_MAJOR_VERSION) || \
     (FTM_ICU_IP_TRUSTEDFUNCTIONS_SW_MINOR_VERSION  != FTM_ICU_IP_DEFINES_SW_MINOR_VERSION) || \
     (FTM_ICU_IP_TRUSTEDFUNCTIONS_SW_PATCH_VERSION  != FTM_ICU_IP_DEFINES_SW_PATCH_VERSION))
    #error "Software Version Numbers of Ftm_Icu_Ip_TrustedFunctions.h and Ftm_Icu_Ip_Defines.h are different"
#endif
/*===============================================================================================
*                                       DEFINES AND MACROS
===============================================================================================*/

/*==================================================================================================
*                                        GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/

#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"
/**
 * @brief      Ftm_Icu_SetUserAccessAllowed
 * @details    This function is called externally by OS Application
 * @param[in]  FtmBaseAddr - The base address of Ftm module.
 */
#if (STD_ON == FTM_ICU_ENABLE_USER_MODE_SUPPORT)
    extern void Ftm_Icu_SetUserAccessAllowed(uint32 FtmBaseAddr);
#endif 


#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#if defined(__cplusplus)
}
#endif

/** @} */

#endif /* FTM_ICU_IP_TRUSTEDFUNCTIONS_H */

