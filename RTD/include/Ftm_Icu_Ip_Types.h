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

#ifndef FTM_ICU_IP_TYPES_H
#define FTM_ICU_IP_TYPES_H

/**
 *   @file       Ftm_Icu_Ip_[!IF "var:defined('postBuildVariant')"!][!"$postBuildVariant"!]_[!ENDIF!]PBcfg.c
 *   @version    2.0.0
 *
 *   @brief      AUTOSAR Icu - contains the data exported by the Icu module
 *   @details    Contains the information that will be exported by the module, as requested by Autosar.
 *
 *   @addtogroup ftm_icu_ip FTM IPL
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
#include "Ftm_Icu_Ip_Defines.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define FTM_ICU_IP_TYPES_VENDOR_ID                     43
#define FTM_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION      4
#define FTM_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION      7
#define FTM_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION   0
#define FTM_ICU_IP_TYPES_SW_MAJOR_VERSION              2
#define FTM_ICU_IP_TYPES_SW_MINOR_VERSION              0
#define FTM_ICU_IP_TYPES_SW_PATCH_VERSION              0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if header file and Std_Types.h file are of the same Autosar version */
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    #if ((FTM_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION != STD_AR_RELEASE_MAJOR_VERSION) || \
         (FTM_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION != STD_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Ftm_Icu_Ip_Types.h and Std_Types.h are different"
    #endif
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (FTM_ICU_IP_TYPES_VENDOR_ID != FTM_ICU_IP_DEFINES_VENDOR_ID)
    #error "Ftm_Icu_Ip_Types.h and Ftm_Icu_Ip_Defines.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((FTM_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION != FTM_ICU_IP_DEFINES_AR_RELEASE_MAJOR_VERSION) || \
     (FTM_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION != FTM_ICU_IP_DEFINES_AR_RELEASE_MINOR_VERSION) || \
     (FTM_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION != FTM_ICU_IP_DEFINES_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Ftm_Icu_Ip_Types.h and Ftm_Icu_Ip_Defines.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((FTM_ICU_IP_TYPES_SW_MAJOR_VERSION != FTM_ICU_IP_DEFINES_SW_MAJOR_VERSION) || \
     (FTM_ICU_IP_TYPES_SW_MINOR_VERSION != FTM_ICU_IP_DEFINES_SW_MINOR_VERSION) || \
     (FTM_ICU_IP_TYPES_SW_PATCH_VERSION != FTM_ICU_IP_DEFINES_SW_PATCH_VERSION))
#error "Software Version Numbers of Ftm_Icu_Ip_Types.h and Ftm_Icu_Ip_Defines.h are different"
#endif

/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
/** @brief Channel number for CHAN0 */
#define CHAN0_IDX (0U)
/** @brief Channel number for CHAN1 */
#define CHAN1_IDX (1U)
/** @brief Channel number for CHAN2 */
#define CHAN2_IDX (2U)
/** @brief Channel number for CHAN3 */
#define CHAN3_IDX (3U)
/** @brief Channel number for CHAN4 */
#define CHAN4_IDX (4U)

/** @brief Maximum value of FTM counter. */
#define FTM_MAX_VAL_COUNTER                 (0xFFFFUL)


/** @brief Set flag COMINEx for specified pair: 0, 1, 2. */
#define FTM_COMBINE_COMBINEx_SHIFT(u8ChannelIdx)        ((uint32)(((((uint32)u8ChannelIdx) >> (uint32)1U) * (uint32)8U) + (uint32)0U))
#define FTM_COMBINE_COMBINEx_MASK_U32(u8ChannelIdx)     ((uint32)((uint32)1U << FTM_COMBINE_COMBINEx_SHIFT(u8ChannelIdx)))

/*==================================================================================================
*                                              ENUMS
==================================================================================================*/

/** @brief FTM clock source selection. */
typedef enum
{
    /** @brief No clock selected. This in effect disables the FTM counter. */
    FTM_NO_CLOCK_SELECTED     = 0U,
    /** @brief FTM input clock. */
    FTM_SYSTEM_CLOCK          = 1U,
    /** @brief Fixed frequency clock. */
    FTM_FIXED_FREQUENCY_CLOCK = 2U,
    /** @brief External clock. */
    FTM_EXTERNAL_CLOCK        = 3U
} Ftm_Icu_Ip_ClockSourceType;

/** @brief FTM debug modes of operation. */
typedef enum
{
    /** @brief FTM counter - stopped. */ 
    MODE_0 = 0U,
    /** @brief FTM counter - stopped. */ 
    MODE_1 = 1U,
    /** @brief FTM counter - stopped. */ 
    MODE_2 = 2U,
    /** @brief FTM counter - functional mode. */ 
    MODE_3 = 3U
} Ftm_Icu_Ip_DebugModeType;

/** @brief Activation condition for the measurement - selecting edge type. */
typedef enum
{
    /** @brief No trigger. */
    FTM_ICU_NO_PIN_CONTROL          = 0x00U,
    /** @brief Rising edge trigger. */
    FTM_ICU_RISING_EDGE             = 0x01U,
    /** @brief Falling edge trigger. */
    FTM_ICU_FALLING_EDGE            = 0x02U,
    /** @brief Rising and falling edge trigger */
    FTM_ICU_BOTH_EDGES              = 0x03U
} Ftm_Icu_Ip_EdgeType; 

/** @brief Operation mode for ICU driver. */
typedef enum
{
    /** @brief No measurement mode. */
    FTM_ICU_MODE_NO_MEASUREMENT      = 0x0U,
    /** @brief Signal edge detect measurement mode. */
    FTM_ICU_MODE_SIGNAL_EDGE_DETECT  = 0x1U,
    /** @brief Signal measurement mode.*/
    FTM_ICU_MODE_SIGNAL_MEASUREMENT  = 0x2U,
    /** @brief Timestamp measurement mode.*/
    FTM_ICU_MODE_TIMESTAMP           = 0x4U,
    /** @brief Edge counter measurement mode.*/
    FTM_ICU_MODE_EDGE_COUNTER        = 0x8U
} Ftm_Icu_Ip_ModeType; 

/** @brief Enable/disable DMA support for timestamp. */
typedef enum
{
    /* Disable DMA support. */
    FTM_ICU_MODE_WITHOUT_DMA = 0U,
    /* Enable DMA support. */
    FTM_ICU_MODE_WITH_DMA    = 1U
} Ftm_Icu_Ip_SubModeType;

/** @brief Type of operation for signal measurement. */
typedef enum
{
    /** @brief No measurement. */
    FTM_ICU_NO_MEASUREMENT = 0U,
    /** @brief The time measurement for OFF period. */ 
    FTM_ICU_LOW_TIME       = 1U,
    /** @brief The time measurement for ON period. */ 
    FTM_ICU_HIGH_TIME      = 2U,
    /** @brief Period measurement between two consecutive falling/raising edges. */
    FTM_ICU_PERIOD_TIME    = 4U,
    /** @brief The fraction of active period. */
    FTM_ICU_DUTY_CYCLE     = 8U
} Ftm_Icu_Ip_MeasType; 


#if (STD_ON == FTM_ICU_GET_INPUT_LEVEL_API)
/** @brief Enumeration used for returning the level of input pin. */
typedef enum
{
    /** @brief Low level state.  */
    FTM_ICU_LEVEL_LOW   = 0x0U,
    /** @brief High level state. */
    FTM_ICU_LEVEL_HIGH  = 0x1U
} Ftm_Icu_Ip_LevelType;
#endif

/* #if (defined(ICU_DUAL_CLOCK_MODE_API) && (ICU_DUAL_CLOCK_MODE_API == STD_ON)) */
/** @brief Definition of prescaler type (Normal or Alternate) */
typedef enum
{
    FTM_ICU_NORMAL_CLK        = 0x0U,  /**< @brief Normal prescaler         */
    FTM_ICU_ALTERNATE_CLK     = 0x1U   /**< @brief Alternate prescaler      */
} Ftm_Icu_Ip_ClockModeType;
/* #endif */

/** @brief Generic error codes. */
typedef enum
{
    /** @brief Generic operation success status. */
    FTM_IP_STATUS_SUCCESS                         = 0x00U,
    /** @brief Generic operation failure status. */
    FTM_IP_STATUS_ERROR                           = 0x01U
} Ftm_Icu_Ip_StatusType;


#if (FTM_ICU_TIMESTAMP_API == STD_ON)
/** @brief Type of operation for timestamp. */
typedef enum
{
    /** @brief No timestamp. */
    FTM_ICU_NO_TIMESTAMP = 0U,
    /** @brief The timestamp with circular buffer . */ 
    FTM_ICU_CIRCULAR_BUFFER       = 1U,
    /** @brief The timestamp with linear buffer . */ 
    FTM_ICU_LINEAR_BUFFER      = 2U
} Ftm_Icu_Ip_TimestampBufferType; 
#endif

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
/** @brief The notification functions shall have no parameters and no return value.*/
typedef void                             (*Ftm_Icu_Ip_NotifyType)(void);

/** @brief Structure that contains ICU Duty cycle parameters. It contains the values needed for
 *         calculating duty cycles i.e Period time value and active time value.
 */
typedef struct
{
    uint16 ActiveTime;         /**< @brief Low or High time value. */
    uint16 PeriodTime;         /**< @brief Period time value. */
} Ftm_Icu_Ip_DutyCycleType;

/** @brief Callback type for each channel. */
typedef void (*Ftm_Icu_Ip_CallbackType)(uint16 callbackParam1, boolean callbackParam2);

/** @brief Callback type for each channel. */
typedef void (*Ftm_Icu_Ip_LogicChState)(uint16 logicChannel, uint8 mask, boolean set);

/** @brief FlexTimer driver Input capture parameters for each channel. */
typedef struct
{
    uint8                          hwChannel;                  /*!< Physical hardware channel ID*/
    Ftm_Icu_Ip_ModeType            chMode;                     /*!< FlexTimer module mode of operation  */
    Ftm_Icu_Ip_SubModeType         chSubMode;                  /*!< FlexTimer specific name of operation to execute  */
    Ftm_Icu_Ip_MeasType            measurementMode;            /*!< Measurement Mode for signal measurement*/
    Ftm_Icu_Ip_EdgeType            edgeAlignement;             /*!< Edge alignment Mode for signal measurement*/
    boolean                        continuouseEn;              /*!< Continuous measurement state */
    uint8                          filterValue;                /*!< Filter Value */
    Ftm_Icu_Ip_CallbackType        callback;                   /*!< The callback function for channels edge detect events */
    uint8                          callbackParams;             /*!< The parameters of callback functions for channels events */
#if (STD_ON == FTM_ICU_TIMESTAMP_API)
    Ftm_Icu_Ip_TimestampBufferType timestampBufferType;        /*!< Timestamp buffer type for timestamp mode*/
#endif
    Ftm_Icu_Ip_LogicChState        logicChStateCallback;       /*!< Store address of function used to change the logic state of the channel in HLD. */
    Ftm_Icu_Ip_NotifyType          ftmChannelNotification;     /*!< The notification functions shall have no parameters and no return value.*/
    Ftm_Icu_Ip_NotifyType          ftmOverflowNotification;    /*!< The overflow notification functions shall have no parameters and no return value.*/
} Ftm_Icu_Ip_ChannelConfigType;

/** @brief FTM IP layer module configuration. */
typedef struct
{
    /** @brief Type of clock source used. */
    Ftm_Icu_Ip_ClockSourceType cfgClkSrc;
    /** @brief Prescaler value. */
    uint8                      cfgPrescaler;
#if (FTM_ICU_DUAL_CLOCK_MODE_API == STD_ON)
    /** @brief Alternant prescaler value. */
    uint8                      cfgAltPrescaler;
#endif
    /** @brief Debug mode. */
    Ftm_Icu_Ip_DebugModeType   debugMode;
    /** @brief Maximum counter value. Minimum value is 0. */
    uint16                     maxCountValue;
} Ftm_Icu_Ip_InstanceConfigType;

/** @brief FTM driver input capture parameters. */
typedef struct
{
    /** @brief Number of input capture channel used. */
    uint8                                   nNumChannels;
    /** @brief Input capture instance configuration. */
    const Ftm_Icu_Ip_InstanceConfigType     *pInstanceConfig;
    /** @brief Input capture channels configuration. */
    const Ftm_Icu_Ip_ChannelConfigType      (*pChannelsConfig)[];
} Ftm_Icu_Ip_ConfigType;


/** @brief This structure is used by the IPL driver for internal logic. */
typedef struct
{
#if (defined(FTM_ICU_SIGNAL_MEASUREMENT_API) && (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API))
    /** @brief Store the status of the first measurement. */
    boolean                                     firstEdge;
    /** @brief Store the first edge come to measurement in BOTH_EDGE mode. */
    boolean                                     firstEdgePolarity;
    /** @brief Signal measurement mode. */
    Ftm_Icu_Ip_MeasType                         measurement;
    /** @brief Variable for saving the period. */
    uint16                                      ftm_Icu_Ip_aPeriod;
    /** @brief Variable for saving the pulse width of active time. */
    uint16                                      ftm_Icu_Ip_aActivePulseWidth;
#endif
    /** @brief The notification functions for TIME_STAMP or SIGNAL_EDGE_DETECT mode. */
    Ftm_Icu_Ip_NotifyType                       ftmChannelNotification;
    /** @brief The overflow notification functions. */
    Ftm_Icu_Ip_NotifyType                       ftmOverflowNotification;
#if (defined(FTM_ICU_EDGE_COUNT_API) && (STD_ON == FTM_ICU_EDGE_COUNT_API))
    /** @brief Logic variable to count edges. */
    uint16                                      edgeNumbers;
#endif
    /** @brief FTM channel mode. */
    Ftm_Icu_Ip_ModeType                         channelMode;
    /** @brief Type of edge used for activation. */
    Ftm_Icu_Ip_EdgeType                         edgeTrigger;
    /** @brief Support DMA or not. */
    Ftm_Icu_Ip_SubModeType                      dmaMode;
#if (STD_ON == FTM_ICU_TIMESTAMP_API)
    /** @brief Pointer to the buffer-array where the timestamp values shall be placed. */
    Ftm_Icu_ValueType*                          ftm_Icu_Ip_aBuffer;
    /** @brief Variable for saving the size of the external buffer (number of entries). */
    uint16                                      ftm_Icu_Ip_aBufferSize;
    /** @brief Variable for saving Notification interval (number of events). */
    uint16                                      ftm_Icu_Ip_aBufferNotify;
    /** @brief Variable for saving the number of notify counts. */
    uint16                                      ftm_Icu_Ip_aNotifyCount;
    /** @brief Variable for saving the time stamp index. */
    uint16                                      ftm_Icu_Ip_aBufferIndex;
    /** @brief Timestamp buffer type for timestamp mode. */
    Ftm_Icu_Ip_TimestampBufferType              timestampBufferType;
#endif /* STD_ON == FTM_ICU_TIMESTAMP_API */
    Ftm_Icu_Ip_LogicChState                     logicChStateCallback;       /*!< Store address of function used to change the logic state of the channel in HLD. */
    /** @brief Calback for other types of measurement.*/
    Ftm_Icu_Ip_CallbackType                     callback;
    /** @brief Logic channel for which callback is executed. */
    uint16                                      callbackParam;
    /** @brief Store the state determines whether Notifications are enabled or not. */
    boolean                                     notificationEnable;
} Ftm_Icu_Ip_ChStateType;

/** @brief  This structure is used by the IPL driver for internal logic. */
typedef struct
{
    /** @brief Module initialization state. */
    boolean    instInit;
    /** @brief Module prescaler value. */
    uint8      prescaler;
    /** @brief Module alternate prescaler value. */
    uint8      prescalerAlternate;
    uint8   spuriousMask;     /* Mask with channels not verified for spurious interrupt.*/  
} Ftm_Icu_Ip_InstStateType;

/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* FTM_ICU_IP_TYPES_H */
