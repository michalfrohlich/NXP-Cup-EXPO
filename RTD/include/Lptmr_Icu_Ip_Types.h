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

#ifndef LPTMR_ICU_IP_TYPES_H
#define LPTMR_ICU_IP_TYPES_H

/**
 *   @file       Lptmr_Icu_Ip_Types.h
 *   @version    2.0.0
 *
 *   @brief      AUTOSAR Icu - LPTMR driver header file.
 *   @details    LPTMR driver defines which need to be exported to external application
 *
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
#include "Lptmr_Icu_Ip_Defines.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define LPTMR_ICU_IP_TYPES_VENDOR_ID                     43
#define LPTMR_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION      4
#define LPTMR_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION      7
#define LPTMR_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION   0
#define LPTMR_ICU_IP_TYPES_SW_MAJOR_VERSION              2
#define LPTMR_ICU_IP_TYPES_SW_MINOR_VERSION              0
#define LPTMR_ICU_IP_TYPES_SW_PATCH_VERSION              0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if source file and ICU header file are of the same vendor */
#if (LPTMR_ICU_IP_TYPES_VENDOR_ID != LPTMR_ICU_IP_DEFINES_VENDOR_ID)
    #error "Lptmr_Icu_Ip_Types.h and Lptmr_Icu_Ip_Defines.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((LPTMR_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION != LPTMR_ICU_IP_DEFINES_AR_RELEASE_MAJOR_VERSION) || \
     (LPTMR_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION != LPTMR_ICU_IP_DEFINES_AR_RELEASE_MINOR_VERSION) || \
     (LPTMR_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION != LPTMR_ICU_IP_DEFINES_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Lptmr_Icu_Ip_Types.h and Lptmr_Icu_Ip_Defines.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((LPTMR_ICU_IP_TYPES_SW_MAJOR_VERSION != LPTMR_ICU_IP_DEFINES_SW_MAJOR_VERSION) || \
     (LPTMR_ICU_IP_TYPES_SW_MINOR_VERSION != LPTMR_ICU_IP_DEFINES_SW_MINOR_VERSION) || \
     (LPTMR_ICU_IP_TYPES_SW_PATCH_VERSION != LPTMR_ICU_IP_DEFINES_SW_PATCH_VERSION))
    #error "Software Version Numbers of Lptmr_Icu_Ip_Types.h and Lptmr_Icu_Ip_Defines.h are different"
#endif

/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
/* Mode Type */
#define    LPTMR_ICU_IP_TM_MODE                 ((uint8)(0x01))      /**< @brief Pulse Time Counter mode */

/*==================================================================================================
*                                              ENUMS
==================================================================================================*/
/** 
 * @brief Prescaler Selection
 */
typedef enum {
    LPTMR_ICU_GLITCH_FILTER_2                   = 0x00U, /*!< Timer mode: prescaler 2, Glitch filter mode: invalid */
    LPTMR_ICU_GLITCH_FILTER_4                   = 0x01U, /*!< Timer mode: prescaler 4, Glitch filter mode: 2 clocks */
    LPTMR_ICU_GLITCH_FILTER_8                   = 0x02U, /*!< Timer mode: prescaler 8, Glitch filter mode: 4 clocks */
    LPTMR_ICU_GLITCH_FILTER_16                  = 0x03U, /*!< Timer mode: prescaler 16, Glitch filter mode: 8 clocks */
    LPTMR_ICU_GLITCH_FILTER_32                  = 0x04U, /*!< Timer mode: prescaler 32, Glitch filter mode: 16 clocks */
    LPTMR_ICU_GLITCH_FILTER_64                  = 0x05U, /*!< Timer mode: prescaler 64, Glitch filter mode: 32 clocks */
    LPTMR_ICU_GLITCH_FILTER_128                 = 0x06U, /*!< Timer mode: prescaler 128, Glitch filter mode: 64 clocks */
    LPTMR_ICU_GLITCH_FILTER_256                 = 0x07U, /*!< Timer mode: prescaler 256, Glitch filter mode: 128 clocks */
    LPTMR_ICU_GLITCH_FILTER_512                 = 0x08U, /*!< Timer mode: prescaler 512, Glitch filter mode: 256 clocks */
    LPTMR_ICU_GLITCH_FILTER_1024                = 0x09U, /*!< Timer mode: prescaler 1024, Glitch filter mode: 512 clocks */
    LPTMR_ICU_GLITCH_FILTER_2048                = 0x0AU, /*!< Timer mode: prescaler 2048, Glitch filter mode: 1024 clocks */
    LPTMR_ICU_GLITCH_FILTER_4096                = 0x0BU, /*!< Timer mode: prescaler 4096, Glitch filter mode: 2048 clocks */
    LPTMR_ICU_GLITCH_FILTER_8192                = 0x0CU, /*!< Timer mode: prescaler 8192, Glitch filter mode: 4096 clocks */
    LPTMR_ICU_GLITCH_FILTER_16384               = 0x0DU, /*!< Timer mode: prescaler 16384, Glitch filter mode: 8192 clocks */
    LPTMR_ICU_GLITCH_FILTER_32768               = 0x0EU, /*!< Timer mode: prescaler 32768, Glitch filter mode: 16384 clocks */
    LPTMR_ICU_GLITCH_FILTER_65536               = 0x0FU  /*!< Timer mode: prescaler 65536, Glitch filter mode: 32768 clocks */
} Lptmr_Icu_PrescalerType;

/**
* @brief            Lptmr_Icu_ChannelMeasurementModeType
* @details          Type that indicates the channel mode type(capture mode, edge counter).
*
* */
typedef enum
{
    LPTMR_ICU_MODE_NO_MEASUREMENT        = 0x00U, /*< @brief No measurement mode. */
    LPTMR_ICU_MODE_SIGNAL_EDGE_DETECT    = 0x01U, /*< @brief Mode for detecting edges.  */
    LPTMR_ICU_MODE_EDGE_COUNTER          = 0x08U  /*< @brief Mode for counting edges on */
} Lptmr_Icu_MeasurementModeType;


/**
* @brief    Definition of input pin type.
*/
typedef enum
{
    /** @brief TRGMUX output */
    TRGMUX_OUTPUT = 0x0U,
    /** @brief LPTMR_ALT1 pin */
    ALT1          = 0x1U,
    /** @brief LPTMR_ALT2 pin */
    ALT2          = 0x2U,
    /** @brief LPTMR_ALT3 pin */
    ALT3          = 0x3U
} Lptmr_Icu_Ip_PinSelectType;

/** @brief LPTMR clock source selection. */
typedef enum
{
    LPTMR_ICU_SIRCDIV2_CLK = 0x0U,    /*!< @brief Select SIRCDIV2_CLK. */
    LPTMR_ICU_LPO1K_CLK    = 0x1U,    /*!< @brief Select LPO1K_CLK. */
    LPTMR_ICU_RTC_CLK      = 0x2U,    /*!< @brief Select RTC_CLK. */
    LPTMR_ICU_PCC_LPTMR0   = 0x3U     /*!< @brief Select PCC_LPTMR0. */
} Lptmr_Icu_Ip_ClockSourceType;

/**
* @brief     Activation condition for the measurement - selecting edge type.
*/
typedef enum
{
    /** @brief Rising edge trigger. */
    LPTMR_ICU_RISING_EDGE       = 0x0U,
    /** @brief Rising edge trigger. */
    LPTMR_ICU_FALLING_EDGE      = 0x1U,
} Lptmr_Icu_Ip_EdgeType;

/** @brief Generic error codes. */
typedef enum
{
    /** @brief Generic operation success status. */
    LPTMR_IP_STATUS_SUCCESS                         = 0x00U,
    /** @brief Generic operation failure status. */
    LPTMR_IP_STATUS_ERROR                           = 0x01U
} Lptmr_Icu_Ip_StatusType;
/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
/** @brief The notification functions shall have no parameters and no return value.*/
typedef void (*Lptmr_Icu_Ip_NotifyType)(void);

/** @brief Callback type for each channel. */
typedef void (*Lptmr_Icu_Ip_CallbackType)(uint16 logicChannel, boolean overflow);

/**
* @brief
*/
typedef struct
{
    boolean                                 chInit;                     /**< chInit state                                      */
    Lptmr_Icu_MeasurementModeType           measMode;                   /**< measurement mode                                  */
    Lptmr_Icu_Ip_NotifyType                 lptmrChannelNotification;   /*!< notification function for LPTMR IPL.              */
    Lptmr_Icu_Ip_CallbackType               callback;                   /*!< interrupt callback function.                      */
    uint16                                  callbackParam;              /*!< Logic channel for which callback is executed.     */
    boolean                                 notificationEnable;         /*!< Store the initialization state that determines whether Notifications are enabled, it is always TRUE with standalone IPL and FALSE with AUTOSAR mode. */         
}Lptmr_Icu_Ip_ChStateType;

/**
* @brief      Lptmr Channel specific configuration structure type
*/
typedef struct
{
    uint8                                   HwChannel;                  /*!< Physical hardware channel ID                                            */
    const Lptmr_Icu_Ip_EdgeType       DefaultStartEdge;           /*!< Lptmr Default Start Edge                                                */
    const Lptmr_Icu_MeasurementModeType     MeasurementModeType;        /*!< Lptmr MeasurementMode                                                   */
    Lptmr_Icu_Ip_NotifyType                 lptmrChannelNotification;   /*!< The notification functions shall have no parameters and no return value.*/
    Lptmr_Icu_Ip_CallbackType               callback;                   /*!< interrupt callback function.                                            */
    uint16                                  callbackParam;              /*!< The parameters of callback functions for channels events                */
} Lptmr_Icu_Ip_ChannelConfigType;

/**
* @brief      Lptmr IP specific configuration structure type
*/
typedef struct
{
    uint8                                   nNumChannels;               /*!< Number of Lptmr channels in the Icu configuration  */
    const Lptmr_Icu_Ip_PinSelectType        PinSelect;                  /*!< Lptmr channel parameters                           */
    const Lptmr_Icu_Ip_ClockSourceType      ClockSource;                /*!< Lptmr clock source                                 */
    const Lptmr_Icu_PrescalerType           Prescaler;                  /*!< The Lptmr Prescaler values                         */
    boolean                                 PrescalerEnable;            /*!< The Lptmr Prescaler Bypass                         */
    const Lptmr_Icu_Ip_ChannelConfigType    (*pChannelsConfig)[];       /*!< Pointer to the configured channels for Lptmr       */
} Lptmr_Icu_Ip_ConfigType;

/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* LPTMR_ICU_IP_TYPES_H */
