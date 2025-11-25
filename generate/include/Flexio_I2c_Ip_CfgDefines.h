/*==================================================================================================
* Project : RTD AUTOSAR 4.7
* Platform : CORTEXM
* Peripheral : LPI2C
* Dependencies : none
*
* Autosar Version : 4.7.0
* Autosar Revision : ASR_REL_4_7_REV_0000
* Autosar Conf.Variant :
* SW Version : 2.0.0
* Build Version : S32K1_RTD_2_0_0_D2308_ASR_REL_4_7_REV_0000_20230804
*
* Copyright 2020-2023 NXP Semiconductors
*
* NXP Confidential. This software is owned or controlled by NXP and may only be
* used strictly in accordance with the applicable license terms. By expressly
* accepting such terms or by downloading, installing, activating and/or otherwise
* using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms. If you do not agree to be
* bound by the applicable license terms, then you may not retain, install,
* activate or otherwise use the software.
==================================================================================================*/

#ifndef FLEXIO_I2C_IP_CFGDEFINES_H
#define FLEXIO_I2C_IP_CFGDEFINES_H

/**
*   @file
*
*   @addtogroup FLEXIO_I2C_DRIVER_CONFIGURATION Flexio_I2c Driver Configurations
*   @{
*/

#ifdef __cplusplus
extern "C"
{
#endif


/*==================================================================================================
*                                          INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "S32K144_FLEXIO.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define FLEXIO_I2C_IP_CFGDEFINES_VENDOR_ID                     43
#define FLEXIO_I2C_IP_CFGDEFINES_MODULE_ID                     255
#define FLEXIO_I2C_IP_CFGDEFINES_AR_RELEASE_MAJOR_VERSION      4
#define FLEXIO_I2C_IP_CFGDEFINES_AR_RELEASE_MINOR_VERSION      7
#define FLEXIO_I2C_IP_CFGDEFINES_AR_RELEASE_REVISION_VERSION   0
#define FLEXIO_I2C_IP_CFGDEFINES_SW_MAJOR_VERSION              2
#define FLEXIO_I2C_IP_CFGDEFINES_SW_MINOR_VERSION              0
#define FLEXIO_I2C_IP_CFGDEFINES_SW_PATCH_VERSION              0
/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
*                         LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/
/**
* @brief            Dma optimize of the i2c module enable/disabled
*/
#define FLEXIO_I2C_IP_DMA_OPTIMIZE_ENABLE   (STD_OFF)
/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/

#define I2C_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "I2c_MemMap.h"

#define I2C_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "I2c_MemMap.h"

/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                  LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                      GLOBAL FUNCTIONS
==================================================================================================*/


#ifdef __cplusplus
}
#endif

/** @} */

#endif /* FLEXIO_I2C_IP_CFGDEFINES_H */

