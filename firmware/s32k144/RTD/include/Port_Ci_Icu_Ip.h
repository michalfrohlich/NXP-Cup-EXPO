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

#ifndef PORT_CI_ICU_IP_H
#define PORT_CI_ICU_IP_H

/**
 *     @file
 *
 *     @addtogroup port_ci_icu_ip PORT_CI IPL
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
#include "Port_Ci_Icu_Ip_Types.h"
#include "Port_Ci_Icu_Ip_Cfg.h"

#if (PORT_CI_ICU_IP_ENABLE_USER_MODE_SUPPORT == STD_ON)
#include "Reg_eSys.h"
#endif

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define PORT_CI_ICU_IP_VENDOR_ID                    43
#define PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION     4
#define PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION     7
#define PORT_CI_ICU_IP_AR_RELEASE_REVISION_VERSION  0
#define PORT_CI_ICU_IP_SW_MAJOR_VERSION             2
#define PORT_CI_ICU_IP_SW_MINOR_VERSION             0
#define PORT_CI_ICU_IP_SW_PATCH_VERSION             0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if source file and ICU header file are of the same vendor */
#if (PORT_CI_ICU_IP_VENDOR_ID != PORT_CI_ICU_IP_TYPES_VENDOR_ID)
    #error "Port_Ci_Icu_Ip.h and Port_Ci_Icu_Ip_Types.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION    != PORT_CI_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION    != PORT_CI_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_AR_RELEASE_REVISION_VERSION != PORT_CI_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Port_Ci_Icu_Ip.h and Port_Ci_Icu_Ip_Types.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((PORT_CI_ICU_IP_SW_MAJOR_VERSION != PORT_CI_ICU_IP_TYPES_SW_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_SW_MINOR_VERSION != PORT_CI_ICU_IP_TYPES_SW_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_SW_PATCH_VERSION != PORT_CI_ICU_IP_TYPES_SW_PATCH_VERSION))
    #error "Software Version Numbers of Port_Ci_Icu_Ip.h and Port_Ci_Icu_Ip_Types.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (PORT_CI_ICU_IP_VENDOR_ID != PORT_CI_ICU_IP_CFG_VENDOR_ID)
    #error "Port_Ci_Icu_Ip.h and Port_Ci_Icu_Ip_Cfg.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION    != PORT_CI_ICU_IP_CFG_AR_RELEASE_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION    != PORT_CI_ICU_IP_CFG_AR_RELEASE_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_AR_RELEASE_REVISION_VERSION != PORT_CI_ICU_IP_CFG_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Port_Ci_Icu_Ip.h and Port_Ci_Icu_Ip_Cfg.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((PORT_CI_ICU_IP_SW_MAJOR_VERSION != PORT_CI_ICU_IP_CFG_SW_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_SW_MINOR_VERSION != PORT_CI_ICU_IP_CFG_SW_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_SW_PATCH_VERSION != PORT_CI_ICU_IP_CFG_SW_PATCH_VERSION))
    #error "Software Version Numbers of Port_Ci_Icu_Ip.h and Port_Ci_Icu_Ip_Cfg.h are different"
#endif

#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    #if (PORT_CI_ICU_IP_ENABLE_USER_MODE_SUPPORT == STD_ON)
        /* Check if header file and Reg_eSys.h file are of the same Autosar version */
        #if ((PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION != REG_ESYS_AR_RELEASE_MAJOR_VERSION) || \
             (PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION != REG_ESYS_AR_RELEASE_MINOR_VERSION))
            #error "AutoSar Version Numbers of Port_Ci_Icu_Ip.h and Reg_eSys.h are different"
        #endif
    #endif
#endif

/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
#if (defined PORT_CI_ICU_CONFIG_EXT)
#define ICU_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Icu_MemMap.h"

/* Macro used to import PORT_CI PB generated configurations. */
PORT_CI_ICU_CONFIG_EXT

#define ICU_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Icu_MemMap.h"
#endif

/* @brief DMA/Interrupt Request rising/falling/either edge masks */
#define ICU_PORT_CI_PCR_IREE_U32                            (0x90000u)
#define ICU_PORT_CI_PCR_IFEE_U32                            (0xA0000u)
#define ICU_PORT_CI_PCR_IEEE_U32                            (0xB0000u)
/*==================================================================================================
*                                              ENUMS
==================================================================================================*/

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/
#define ICU_START_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"
extern PORT_Type * const s_Port_Ci_Base[];
#define ICU_STOP_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"

/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"
#if ((PORT_CI_ICU_IP_SET_MODE_API == STD_ON) ||(PORT_CI_ICU_IP_EDGE_DETECT_API == STD_ON))
void Port_Ci_Icu_Ip_EnableInterrupt(uint8 instance, uint8 hwChannel);
#endif

void Port_Ci_Icu_Ip_DisableInterrupt(uint8 instance, uint8 hwChannel);

Port_Ci_Icu_Ip_StatusType Port_Ci_Icu_Ip_Init (uint8 instance,  const Port_Ci_Icu_Ip_ConfigType * userConfig);

#if (STD_ON == PORT_CI_ICU_IP_DEINIT_API)
Port_Ci_Icu_Ip_StatusType Port_Ci_Icu_Ip_DeInit(uint8 instance);
#endif

#if (PORT_CI_ICU_IP_SET_MODE_API == STD_ON)
void Port_Ci_Icu_Ip_SetSleepMode(uint8 instance, uint8 hwChannel);
void Port_Ci_Icu_Ip_SetNormalMode(uint8 instance, uint8 hwChannel);
#endif  /* PORT_CI_ICU_IP_SET_MODE_API  */

void Port_Ci_Icu_Ip_SetActivationCondition(uint8 instance, uint8 hwChannel, Port_Ci_Icu_Ip_EdgeType activation);

#if (PORT_CI_ICU_IP_EDGE_DETECT_API == STD_ON)
void Port_Ci_Icu_Ip_EnableEdgeDetection(uint8 instance, uint8 hwChannel);
void Port_Ci_Icu_Ip_DisableDetection(uint8 instance, uint8 hwChannel);
#endif /* PORT_CI_ICU_IP_EDGE_DETECT_API */

#if (PORT_CI_ICU_IP_GET_INPUT_STATE_API == STD_ON)
boolean Port_Ci_Icu_Ip_GetInputState(uint8 instance, uint8 hwChannel);
#endif  /* PORT_CI_ICU_IP_GET_INPUT_STATE_API */

/**
 * @brief      Driver function Enable Notification for timestamp.
 */
void Port_Ci_Icu_Ip_EnableNotification(uint8 instance, uint8 hwChannel);

/**
 * @brief      Driver function Disable Notification for timestamp.
 */
void Port_Ci_Icu_Ip_DisableNotification(uint8 instance, uint8 hwChannel);

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* PORT_CI_ICU_IP_H */
