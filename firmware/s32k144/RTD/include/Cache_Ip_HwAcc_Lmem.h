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
#ifndef CACHE_IP_HWACC_LMEM_H_
#define CACHE_IP_HWACC_LMEM_H_

/**
*   @file    Cache_Ip_HwAcc_Lmem.h
*
*   @version 2.0.0
*
*   @brief   AUTOSAR Mcl - Cache Ip driver header file.
*   @details
*
*   @addtogroup CACHE_IP_DRIVER CACHE IP Driver
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
#include "Cache_Ip_Cfg_Defines.h"
#include "Cache_Ip_Cfg_DeviceRegisters.h"
#include "Cache_Ip_Devassert.h"
/* LMEM requires timeout for command execution. */
#include "OsIf.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define CACHE_IP_HWACC_LMEM_VENDOR_ID                       43
#define CACHE_IP_HWACC_LMEM_AR_RELEASE_MAJOR_VERSION        4
#define CACHE_IP_HWACC_LMEM_AR_RELEASE_MINOR_VERSION        7
#define CACHE_IP_HWACC_LMEM_AR_RELEASE_REVISION_VERSION     0
#define CACHE_IP_HWACC_LMEM_SW_MAJOR_VERSION                2
#define CACHE_IP_HWACC_LMEM_SW_MINOR_VERSION                0
#define CACHE_IP_HWACC_LMEM_SW_PATCH_VERSION                0

/*==================================================================================================
                                      FILE VERSION CHECKS
==================================================================================================*/
/* Check if header file and Cache_Ip.h file are of the same vendor */
#if (CACHE_IP_HWACC_LMEM_VENDOR_ID != CACHE_IP_CFG_DEFINES_VENDOR_ID)
    #error "Cache_Ip_HwAcc_Lmem.h and Cache_Ip_Cfg_Defines.h have different vendor ids"
#endif

/* Check if header file and Cache_Ip_Cfg_Defines.h file are of the same Autosar version */
#if ((CACHE_IP_HWACC_LMEM_AR_RELEASE_MAJOR_VERSION != CACHE_IP_CFG_DEFINES_AR_RELEASE_MAJOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_AR_RELEASE_MINOR_VERSION != CACHE_IP_CFG_DEFINES_AR_RELEASE_MINOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_AR_RELEASE_REVISION_VERSION != CACHE_IP_CFG_DEFINES_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of Cache_Ip_HwAcc_Lmem.h and Cache_Ip_Cfg_Defines.h are different"
#endif

/* Check if header file and Cache_Ip_Cfg_Defines.h file are of the same Software version */
#if ((CACHE_IP_HWACC_LMEM_SW_MAJOR_VERSION != CACHE_IP_CFG_DEFINES_SW_MAJOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_SW_MINOR_VERSION != CACHE_IP_CFG_DEFINES_SW_MINOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_SW_PATCH_VERSION != CACHE_IP_CFG_DEFINES_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of Cache_Ip_HwAcc_Lmem.h and Cache_Ip_Cfg_Defines.h are different"
#endif


/* Check if header file and Cache_Ip_Cfg_DeviceRegisters.h file are of the same vendor */
#if (CACHE_IP_HWACC_LMEM_VENDOR_ID != CACHE_IP_CFG_DEVICEREGISTERS_VENDOR_ID)
    #error "Cache_Ip_HwAcc_Lmem.h and Cache_Ip_Cfg_DeviceRegisters.h have different vendor ids"
#endif

/* Check if header file and Cache_Ip_Cfg_DeviceRegisters.h file are of the same Autosar version */
#if ((CACHE_IP_HWACC_LMEM_AR_RELEASE_MAJOR_VERSION != CACHE_IP_CFG_DEVICEREGISTERS_AR_RELEASE_MAJOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_AR_RELEASE_MINOR_VERSION != CACHE_IP_CFG_DEVICEREGISTERS_AR_RELEASE_MINOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_AR_RELEASE_REVISION_VERSION != CACHE_IP_CFG_DEVICEREGISTERS_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of Cache_Ip_HwAcc_Lmem.h and Cache_Ip_Cfg_DeviceRegisters.h are different"
#endif

/* Check if header file and Cache_Ip_Cfg_DeviceRegisters.h file are of the same Software version */
#if ((CACHE_IP_HWACC_LMEM_SW_MAJOR_VERSION != CACHE_IP_CFG_DEVICEREGISTERS_SW_MAJOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_SW_MINOR_VERSION != CACHE_IP_CFG_DEVICEREGISTERS_SW_MINOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_SW_PATCH_VERSION != CACHE_IP_CFG_DEVICEREGISTERS_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of Cache_Ip_HwAcc_Lmem.h and Cache_Ip_Cfg_DeviceRegisters.h are different"
#endif


/* Check if header file and Cache_Ip_Devassert.h file are of the same vendor */
#if (CACHE_IP_HWACC_LMEM_VENDOR_ID != CACHE_IP_DEVASSERT_VENDOR_ID)
    #error "Cache_Ip_HwAcc_Lmem.h and Cache_Ip_Devassert.h have different vendor ids"
#endif

/* Check if header file and Cache_Ip_Devassert.h file are of the same Autosar version */
#if ((CACHE_IP_HWACC_LMEM_AR_RELEASE_MAJOR_VERSION != CACHE_IP_DEVASSERT_AR_RELEASE_MAJOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_AR_RELEASE_MINOR_VERSION != CACHE_IP_DEVASSERT_AR_RELEASE_MINOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_AR_RELEASE_REVISION_VERSION != CACHE_IP_DEVASSERT_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of Cache_Ip_HwAcc_Lmem.h and Cache_Ip_Devassert.h are different"
#endif

/* Check if header file and Cache_Ip_Devassert.h file are of the same Software version */
#if ((CACHE_IP_HWACC_LMEM_SW_MAJOR_VERSION != CACHE_IP_DEVASSERT_SW_MAJOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_SW_MINOR_VERSION != CACHE_IP_DEVASSERT_SW_MINOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_SW_PATCH_VERSION != CACHE_IP_DEVASSERT_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of Cache_Ip_HwAcc_Lmem.h and Cache_Ip_Devassert.h are different"
#endif


/* Check if header file and OsIf.h file are of the same vendor */
#if (CACHE_IP_HWACC_LMEM_VENDOR_ID != OSIF_VENDOR_ID)
    #error "Cache_Ip_HwAcc_Lmem.h and OsIf.h have different vendor ids"
#endif

/* Check if header file and OsIf.h file are of the same Autosar version */
#if ((CACHE_IP_HWACC_LMEM_AR_RELEASE_MAJOR_VERSION != OSIF_AR_RELEASE_MAJOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_AR_RELEASE_MINOR_VERSION != OSIF_AR_RELEASE_MINOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_AR_RELEASE_REVISION_VERSION != OSIF_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of Cache_Ip_HwAcc_Lmem.h and OsIf.h are different"
#endif

/* Check if header file and OsIf.h file are of the same Software version */
#if ((CACHE_IP_HWACC_LMEM_SW_MAJOR_VERSION != OSIF_SW_MAJOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_SW_MINOR_VERSION != OSIF_SW_MINOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_SW_PATCH_VERSION != OSIF_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of Cache_Ip_HwAcc_Lmem.h and OsIf.h are different"
#endif

/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                       DEFINES AND MACROS
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
*                                       FUNCTION PROTOTYPES
==================================================================================================*/

#if (STD_ON == CACHE_IP_IS_AVAILABLE)
#if (STD_ON == CACHE_IP_LMEM_IS_AVAILABLE)
#if (STD_ON == CACHE_IP_ALL_IS_AVAILABLE)

#if (STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE)
Std_ReturnType hwAcc_Lmem_ProcessorCodeEnable(void);
Std_ReturnType hwAcc_Lmem_ProcessorCodeDisable(void);
Std_ReturnType hwAcc_Lmem_ProcessorCodeInvalidate(void);
Std_ReturnType hwAcc_Lmem_ProcessorCodeClean(const boolean enInvalidate);
Std_ReturnType hwAcc_Lmem_ProcessorCodeInvalidateByAddr(const uint32 addr, const uint32 length);
Std_ReturnType hwAcc_Lmem_ProcessorCodeCleanByAddr(const boolean enInvalidate, const uint32 addr, const uint32 length);
#endif /* CACHE_IP_LMEM_PC_IS_AVAILABLE */

#if (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE)
Std_ReturnType hwAcc_Lmem_ProcessorSystemEnable(void);
Std_ReturnType hwAcc_Lmem_ProcessorSystemDisable(void);
Std_ReturnType hwAcc_Lmem_ProcessorSystemInvalidate(void);
Std_ReturnType hwAcc_Lmem_ProcessorSystemClean(const boolean enInvalidate);
Std_ReturnType hwAcc_Lmem_ProcessorSystemInvalidateByAddr(const uint32 addr, const uint32 length);
Std_ReturnType hwAcc_Lmem_ProcessorSystemCleanByAddr(const boolean enInvalidate, const uint32 addr, const uint32 length);
#endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */

Std_ReturnType hwAcc_Lmem_Enable(const uint8 BusType);
Std_ReturnType hwAcc_Lmem_Disable(const uint8 BusType);
Std_ReturnType hwAcc_Lmem_Invalidate(const uint8 BusType);
Std_ReturnType hwAcc_Lmem_Clean(const uint8 BusType, const boolean enInvalidate);
Std_ReturnType hwAcc_Lmem_InvalidateByAddr(const uint8 BusType, const uint32 addr, const uint32 length);
Std_ReturnType hwAcc_Lmem_CleanByAddr(const uint8 BusType, const boolean enInvalidate, const uint32 addr, const uint32 length);


#endif /* CACHE_IP_ALL_IS_AVAILABLE */
#endif /* CACHE_IP_LMEM_IS_AVAILABLE) */
#endif /* CACHE_IP_IS_AVAILABLE) */

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* #ifndef CACHE_IP_HWACC_LMEM_H_ */
