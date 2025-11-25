/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           : LPI2C
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

#ifndef CDD_I2C_CFGDEFINES_H
#define CDD_I2C_CFGDEFINES_H

/**
*   @file
*
*   @addtogroup i2c_driver_config I2c Driver Configuration
*   @{
*/

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                          INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define I2C_VENDOR_ID_CFGDEFINES                    43
#define I2C_AR_RELEASE_MAJOR_VERSION_CFGDEFINES     4
#define I2C_AR_RELEASE_MINOR_VERSION_CFGDEFINES     7
#define I2C_AR_RELEASE_REVISION_VERSION_CFGDEFINES  0
#define I2C_SW_MAJOR_VERSION_CFGDEFINES             2
#define I2C_SW_MINOR_VERSION_CFGDEFINES             0
#define I2C_SW_PATCH_VERSION_CFGDEFINES             0

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                       DEFINES AND MACROS
==================================================================================================*/



/**
* @brief          Precompile Support On.
* @details        VARIANT-PRE-COMPILE: Only parameters with "Pre-compile time"
*                 configuration are allowed in this variant.
*/
#define I2C_PRECOMPILE_SUPPORT   (STD_ON)
/**
* @brief   Total number of I2c channel configured.
*/
#define I2C_HW_MAX_CONFIG        (1U)

/**
* @brief   Total number of available hardware I2C channels.
*/
#define I2C_HW_MAX_MODULES       (3U)

/**
* @brief   Total number of available hardware Lpi2c channels.
*/
#define I2C_LPI2C_MAX_MODULES      (1U)

/**
* @brief            This is the ID of the first FLEXIO channel.
*/
#define I2C_FLEXIO_FIRST_CHANNEL_U8   (I2C_LPI2C_MAX_MODULES)

/**
* @brief            TIMEOUT for sync transmissions
*/
#define I2C_TIMEOUT (65535U)

/**
* @brief   Switches the Development Error Detection and Notification ON or OFF.
*/
#define I2C_DEV_ERROR_DETECT      (STD_ON) /* Enable Development Error Detection */
/**
* @brief          Support for version info API.
*/
#define I2C_VERSION_INFO_API      (STD_OFF) /* Disable API I2c_GetVersionInfo      */
/**
* @brief          Link I2C channels symbolic names with I2C channel IDs.
*/
#define I2C_LPI2C_0  (0U)
/**
* @brief          I2C channels symbolic names with I2C channel IDs.
*/
#define I2C_MASTER_0  (0U)
/**
* @brief          spurious coreid.
*/
#define I2C_SPURIOUS_CORE_ID     (0U)

/**
* @brief          Symbolic names for configured channels.
*/
#define I2cConf_I2cChannel_Display_Channel  (0U)

/**
* @brief            FlexIO is used (STD_ON/STD_OFF)
*/
#define I2C_FLEXIO_USED   (STD_OFF)

/**
* @brief            Enable/Disable the API for reporting the Dem Error.
*/
#define I2C_DISABLE_DEM_REPORT_ERROR_STATUS   (STD_ON)
/**
* @brief            Enable/Disable Multi Core Support.
*/
#define I2C_MULTICORE_SUPPORT    (STD_OFF)
/**
* @brief            DMA is used for at least one channel (STD_ON/STD_OFF)
*/
#define I2C_DMA_USED   (STD_OFF)

#if (I2C_MULTICORE_SUPPORT == STD_ON)
    #define I2c_GetCoreID()            OsIf_GetCoreID()
#else
    #define I2c_GetCoreID()            ((uint32)0UL)
#endif /* (I2C_MULTICORE_SUPPORT == STD_ON) */


/**
* @brief        Variable storing number of maximum partitions using in configuration.
*/
#define I2C_MAX_CORE_ID     (1U)

/**
* @brief    The value initialization un init for each core.
*/
#define I2C_UNINIT_CORE \
{ I2C_UNINIT }
/**
* @brief    The value initialization null pointer for each core.
*/
#define I2C_NULL_PTR \
{ NULL_PTR }
/**
* @brief    The value initialization Hardware Map.
*/
#define I2C_HW_MAP_INIT \
{ -1, -1, -1 }
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

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* CDD_I2C_CFGDEFINES_H */
