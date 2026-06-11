/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           : PORT_CI
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 2.0.0
*   Build Version        : S32K1_RTD_2_0_0_D2308_ASR_REL_4_7_REV_0000_20230804
*
*   (c) Copyright 2020-2023 NXP Semiconductors
*   All Rights Reserved.
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
*   @file      Port_Ci_Port_Ip_Cfg.c
*
*   @addtogroup Port_CFG
*   @{
*/

#ifdef __cplusplus
extern "C"{
#endif


/*==================================================================================================
                                         INCLUDE FILES
 1) system and project includes
 2) needed interfaces from external units
 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Port_Ci_Port_Ip_Cfg.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define PORT_CI_PORT_IP_VENDOR_ID_CFG_C                     43
#define PORT_CI_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_C      4
#define PORT_CI_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_C      7
#define PORT_CI_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_C   0
#define PORT_CI_PORT_IP_SW_MAJOR_VERSION_CFG_C              2
#define PORT_CI_PORT_IP_SW_MINOR_VERSION_CFG_C              0
#define PORT_CI_PORT_IP_SW_PATCH_VERSION_CFG_C              0

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/
/* Check if Port_Ci_Port_Ip_Cfg.c and Port_Ci_Port_Ip_Cfg.h are of the same vendor */
#if (PORT_CI_PORT_IP_VENDOR_ID_CFG_C != PORT_CI_PORT_IP_VENDOR_ID_CFG_H)
    #error "Port_Ci_Port_Ip_Cfg.c and Port_Ci_Port_Ip_Cfg.h have different vendor ids"
#endif
/* Check if Port_Ci_Port_Ip_Cfg.c and Port_Ci_Port_Ip_Cfg.h are of the same Autosar version */
#if ((PORT_CI_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_C    != PORT_CI_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_H)    || \
     (PORT_CI_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_C    != PORT_CI_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_H)    || \
     (PORT_CI_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_C != PORT_CI_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_H)    \
    )
    #error "AutoSar Version Numbers of Port_Ci_Port_Ip_Cfg.c and Port_Ci_Port_Ip_Cfg.h are different"
#endif
/* Check if Port_Ci_Port_Ip_Cfg.c and Port_Ci_Port_Ip_Cfg.h are of the same Software version */
#if ((PORT_CI_PORT_IP_SW_MAJOR_VERSION_CFG_C != PORT_CI_PORT_IP_SW_MAJOR_VERSION_CFG_H) || \
     (PORT_CI_PORT_IP_SW_MINOR_VERSION_CFG_C != PORT_CI_PORT_IP_SW_MINOR_VERSION_CFG_H) || \
     (PORT_CI_PORT_IP_SW_PATCH_VERSION_CFG_C != PORT_CI_PORT_IP_SW_PATCH_VERSION_CFG_H)    \
    )
    #error "Software Version Numbers of Port_Ci_Port_Ip_Cfg.c and Port_Ci_Port_Ip_Cfg.h are different"
#endif

/*==================================================================================================
                             LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                             LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                                            LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                           GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           GLOBAL VARIABLES
==================================================================================================*/


/*==================================================================================================
                                      LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                                           LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
                                           GLOBAL FUNCTIONS
==================================================================================================*/


#ifdef __cplusplus
}
#endif

/** @} */


