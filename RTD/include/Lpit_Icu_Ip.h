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

#ifndef LPIT_ICU_IP_H
#define LPIT_ICU_IP_H

/**
 *     @file       Lpit_Icu_Ip.h 
 *     @brief      Header file of LPIT module.
 *     @details    This file contains signatures for all the functions which are related to LPIT 
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
#include "Lpit_Icu_Ip_Types.h"
#include "Lpit_Icu_Ip_Cfg.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define LPIT_ICU_IP_VENDOR_ID                    43
#define LPIT_ICU_IP_AR_RELEASE_MAJOR_VERSION     4
#define LPIT_ICU_IP_AR_RELEASE_MINOR_VERSION     7
#define LPIT_ICU_IP_AR_RELEASE_REVISION_VERSION  0
#define LPIT_ICU_IP_SW_MAJOR_VERSION             2
#define LPIT_ICU_IP_SW_MINOR_VERSION             0
#define LPIT_ICU_IP_SW_PATCH_VERSION             0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if header file and Lpit_Icu_Ip_Types.h header file are of the same vendor */
#if (LPIT_ICU_IP_VENDOR_ID != LPIT_ICU_IP_TYPES_VENDOR_ID)
    #error "Lpit_Icu_Ip.h and Lpit_Icu_Ip_Types.h have different vendor IDs"
#endif
/* Check if header file and Lpit_Icu_Ip_Types.h header file are of the same AUTOSAR version */
#if ((LPIT_ICU_IP_AR_RELEASE_MAJOR_VERSION    != LPIT_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION) || \
     (LPIT_ICU_IP_AR_RELEASE_MINOR_VERSION    != LPIT_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION) || \
     (LPIT_ICU_IP_AR_RELEASE_REVISION_VERSION != LPIT_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Lpit_Icu_Ip.h and Lpit_Icu_Ip_Types.h are different"
#endif
/* Check if header file and Lpit_Icu_Ip_Types.h header file are of the same software version */
#if ((LPIT_ICU_IP_SW_MAJOR_VERSION != LPIT_ICU_IP_TYPES_SW_MAJOR_VERSION) || \
     (LPIT_ICU_IP_SW_MINOR_VERSION != LPIT_ICU_IP_TYPES_SW_MINOR_VERSION) || \
     (LPIT_ICU_IP_SW_PATCH_VERSION != LPIT_ICU_IP_TYPES_SW_PATCH_VERSION))
#error "Software Version Numbers of Lpit_Icu_Ip.h and Lpit_Icu_Ip_Types.h are different"
#endif

/* Check if header file and ICU Lpit_Icu_Ip_Cfg.h header file are of the same vendor */
#if (LPIT_ICU_IP_VENDOR_ID != LPIT_ICU_IP_CFG_VENDOR_ID)
    #error "Lpit_Icu_Ip.h and Lpit_Icu_Ip_Cfg.h have different vendor IDs"
#endif
/* Check if header file and ICU Lpit_Icu_Ip_Cfg.h header file are of the same AutoSar version */
#if ((LPIT_ICU_IP_AR_RELEASE_MAJOR_VERSION    != LPIT_ICU_IP_CFG_AR_RELEASE_MAJOR_VERSION) || \
     (LPIT_ICU_IP_AR_RELEASE_MINOR_VERSION    != LPIT_ICU_IP_CFG_AR_RELEASE_MINOR_VERSION) || \
     (LPIT_ICU_IP_AR_RELEASE_REVISION_VERSION != LPIT_ICU_IP_CFG_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Lpit_Icu_Ip.h and Lpit_Icu_Ip_Cfg.h are different"
#endif
/* Check if header file and ICU Lpit_Icu_Ip_Cfg.h header file are of the same Software version */
#if ((LPIT_ICU_IP_SW_MAJOR_VERSION != LPIT_ICU_IP_CFG_SW_MAJOR_VERSION) || \
     (LPIT_ICU_IP_SW_MINOR_VERSION != LPIT_ICU_IP_CFG_SW_MINOR_VERSION) || \
     (LPIT_ICU_IP_SW_PATCH_VERSION != LPIT_ICU_IP_CFG_SW_PATCH_VERSION))
#error "Software Version Numbers of Lpit_Icu_Ip.h and Lpit_Icu_Ip_Cfg.h are different"
#endif

/*==================================================================================================
*                                        GLOBAL VARIABLES
==================================================================================================*/
#if (defined LPIT_ICU_CONFIG_EXT)
#define ICU_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Icu_MemMap.h"

/* Macro used to import LPIT generated configurations. */
LPIT_ICU_CONFIG_EXT

#define ICU_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Icu_MemMap.h"
#endif /* LPIT_ICU_CONFIG_EXT */

#define ICU_START_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"
/* Table of base addresses for LPIT instances. */
extern LPIT_Type* const lpitBase[];

#define ICU_STOP_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"
/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"
/**
 * @brief   LPIT driver initialization function for LPIT module.
 * @details This function is called separately for each LPIT instace and will do the following:
 *          - enables the LPIT module
 *          - configures the debug mode (enabled or disabled)
 *          - disable the IRQ correpsonding to the LPIT channel
 *          - clears the (pending) interrupt flag corresponding to LPIT channel
 *          - enable channel interrupts
 *          - Set Trigger Input Capture Mode
 *
 * @param[in] instance     - hardware instance to be configured
 * @param[in] userConfig - configuration of the instance that will be intialized
 * @return Lpit_Icu_Ip_StatusType
 */
Lpit_Icu_Ip_StatusType Lpit_Icu_Ip_Init (uint8 instance, const Lpit_Icu_Ip_ConfigType * userConfig);

#if (LPIT_ICU_DEINIT_API == STD_ON)
/**
 * @brief   LPIT driver deinitialization function for LPIT module.
 * @details This function is called separately for each LPIT instace and will do the following:
 *          - disables the LPIT channel
 *          - disables the debug mode
 *          - disables IRQ corresponding to LPit channel
 *          - clears the (pending) interrupt flag corresponding to LPIT channel
 *
 * @param[in] instance  - hardware instance of the module
 * @return Lpit_Icu_Ip_StatusType
 */
Lpit_Icu_Ip_StatusType Lpit_Icu_Ip_Deinit(uint8 instance);
#endif /* LPIT_ICU_DEINIT_API */

#if (LPIT_ICU_IP_SET_MODE_API == STD_ON)
void Lpit_Icu_Ip_SetSleepMode(uint8 instance, uint8 hwChannel);
void Lpit_Icu_Ip_SetNormalMode(uint8 instance, uint8 hwChannel);
#endif  /* LPIT_ICU_IP_SET_MODE_API  */

/**
 * @brief   LPIT driver function that enables interrupts on a LPIT channel.
 * @details This function enables LPIT channel interrupt.
 * 
 * @param[in] instance  - hardware instance of the module
 * @param[in] hwChannel - channel instance of the module
 */
void Lpit_Icu_Ip_EnableInterrupt(uint8 instance, uint8 hwChannel);

/**
 * @brief   LPIT driver function that disables interrupts on a LPIT channel.
 * @details This function disables LPIT channel interrupt.
 * 
 * @param[in] instance  - hardware instance of the module
 * @param[in] hwChannel - channel instance of the module
 */
void Lpit_Icu_Ip_DisableInterrupt(uint8 instance, uint8 hwChannel);

#if (LPIT_ICU_EDGE_DETECT_API == STD_ON)
/**
 * @brief LPIT IP layer function that enables edge detection measure mode for a given module and channel.
 * 
 * @param[in] instance  - hardware instance of the module
 * @param[in] hwChannel - channel instance of the module
 */
void Lpit_Icu_Ip_EnableEdgeDetection(uint8 instance, uint8 hwChannel);
#endif /* LPIT_ICU_EDGE_DETECT_API */

#if ((LPIT_ICU_EDGE_DETECT_API == STD_ON) || (LPIT_ICU_TIMESTAMP_API == STD_ON))
/**
 * @brief LPIT IP layer function that disable all measure modes for a given module and channel.
 * 
 * @param[in] instance  - hardware instance of the module
 * @param[in] hwChannel - channel instance of the module
 */
void Lpit_Icu_Ip_DisableDetectionMode(uint8 instance, uint8 hwChannel);
#endif /* (LPIT_ICU_EDGE_DETECT_API == STD_ON) || (LPIT_ICU_TIMESTAMP_API == STD_ON) */

#if (LPIT_ICU_GET_INPUT_STATE_API == STD_ON)
/**
 * @brief 
 * 
 * @param instance        module instance number
 * @param hwChannel       channel number
 * @return boolean 
 */
boolean Lpit_Icu_Ip_GetInputState(uint8 instance, uint8 hwChannel);
#endif /* LPIT_ICU_GET_INPUT_STATE_API */

#if (STD_ON == LPIT_ICU_TIMESTAMP_API)
/**
 * @brief 
 * 
 * @param instance  LPIT module on which the current channel is located.
 * @param hwChannel   LPIT hardware channel used.
 * @param bufferPtr 
 * @param bufferSize 
 * @param notifyInterval 
 */
void Lpit_Icu_Ip_StartTimestamp(uint8 instance,
                                uint8 hwChannel,
                                uint32 *bufferPtr,
                                uint16 bufferSize,
                                uint16 notifyInterval);
#endif /* STD_ON == LPIT_ICU_TIMESTAMP_API */

/**
 * @brief      Driver function Enable Notification for timestamp.
 *
 * @param[in]  instance  Hardware instance of FTM used. 
 * @param[in]  hwChannel Hardware channel of FTM used.
 * @return     void
 */
void Lpit_Icu_Ip_EnableNotification(uint8 instance, uint8 hwChannel);

/**
 * @brief      Driver function Disable Notification for timestamp.
 *
 * @param[in]  instance  Hardware instance of FTM used. 
 * @param[in]  hwChannel Hardware channel of FTM used.
 * @return     void
 */
void Lpit_Icu_Ip_DisableNotification(uint8 instance, uint8 hwChannel);

/**
 * @brief      Get timestamp index for timestamp mode.
 *
 * @param[in]  instance  Hardware instance of FTM used. 
 * @param[in]  hwChannel Hardware channel of FTM used.
 * @return     uint16
 */
uint16 Lpit_Icu_Ip_GetTimestampIndex(uint8 instance, uint8 channel);

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"


#if defined(__cplusplus)
}
#endif

/** @} */

#endif /* LPIT_ICU_IP_H */
