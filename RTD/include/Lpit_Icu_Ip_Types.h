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

#ifndef LPIT_ICU_TYPES
#define LPIT_ICU_TYPES

/**
 * @file       Lpit_Icu_Ip_Types.h
 * @details    This header contains all the types used by ICU driver for CMP IP.
 *
 * @addtogroup lpit_icu_ip LPIT IPL
 * @{
 */

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                         INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Lpit_Icu_Ip_Defines.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define LPIT_ICU_IP_TYPES_VENDOR_ID                    43
#define LPIT_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION     4
#define LPIT_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION     7
#define LPIT_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION  0
#define LPIT_ICU_IP_TYPES_SW_MAJOR_VERSION             2
#define LPIT_ICU_IP_TYPES_SW_MINOR_VERSION             0
#define LPIT_ICU_IP_TYPES_SW_PATCH_VERSION             0

/*==================================================================================================
*                                       FILE VERSION CHECKS
===================================================================================================*/
/* Check if header file and Lpit_Icu_Ip_Defines.h header file are of the same vendor */
#if (LPIT_ICU_IP_TYPES_VENDOR_ID != LPIT_ICU_IP_DEFINES_VENDOR_ID)
    #error "Vendor IDs of Lpit_Icu_Ip_Types.h and Lpit_Icu_Ip_Defines.h are different."
#endif

/* Check if header file and Lpit_Icu_Ip_Defines.h header file are of the same AUTOSAR version */
#if ((LPIT_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION    != LPIT_ICU_IP_DEFINES_AR_RELEASE_MAJOR_VERSION) || \
     (LPIT_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION    != LPIT_ICU_IP_DEFINES_AR_RELEASE_MINOR_VERSION) || \
     (LPIT_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION != LPIT_ICU_IP_DEFINES_AR_RELEASE_REVISION_VERSION))
    #error "AUTOSAR version numbers of Lpit_Icu_Ip_Types.h and Lpit_Icu_Ip_Defines.h are different."
#endif

/* Check if header file and Lpit_Icu_Ip_Defines.h header file are of the same software version */
#if ((LPIT_ICU_IP_TYPES_SW_MAJOR_VERSION != LPIT_ICU_IP_DEFINES_SW_MAJOR_VERSION) || \
     (LPIT_ICU_IP_TYPES_SW_MINOR_VERSION != LPIT_ICU_IP_DEFINES_SW_MINOR_VERSION) || \
     (LPIT_ICU_IP_TYPES_SW_PATCH_VERSION != LPIT_ICU_IP_DEFINES_SW_PATCH_VERSION))
    #error "Software version numbers of Lpit_Icu_Ip_Types.h and Lpit_Icu_Ip_Defines.h are different."
#endif

/*==================================================================================================
*                                       DEFINES AND MACROS
==================================================================================================*/

/*==================================================================================================
*                                              ENUMS
==================================================================================================*/
/** @brief Generic error codes. */
typedef enum
{
    /** @brief Generic operation success status. */
    LPIT_IP_STATUS_SUCCESS                         = 0x00U,
    /** @brief Generic operation failure status. */
    LPIT_IP_STATUS_ERROR                           = 0x01U
} Lpit_Icu_Ip_StatusType;
/*==================================================================================================
*                                STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
/** @brief The notification functions shall have no parameters and no return value.*/
typedef void                             (*Lpit_Icu_Ip_NotifyType)(void);

/** @brief Callback type for each channel. */
typedef void (*Lpit_Icu_Ip_CallbackType)(uint16 callbackParam1, boolean callbackParam2);

/** @brief Callback type for each channel. */
typedef void (*Lpit_Icu_Ip_LogicChState)(uint16 logicChannel, uint8 mask, boolean set);

#if (STD_ON == LPIT_ICU_TIMESTAMP_API)
/** @brief Type of operation for timestamp. */
typedef enum
{
    LPIT_ICU_IP_NO_BUFFER       = 0U, /**< @brief No timestamp. */
    LPIT_ICU_IP_CIRCULAR_BUFFER = 1U, /**< @brief The timestamp with circular buffer . */
    LPIT_ICU_IP_LINEAR_BUFFER   = 2U  /**< @brief The timestamp with linear buffer . */
} Lpit_Icu_Ip_TimestampBufferType; 

typedef struct
{
    uint32*                         bufferPtr;
    uint16                          bufferSize;
    uint16                          notifyInterval;
    uint16                          notifyCount;
    uint16                          bufferIndex;
    Lpit_Icu_Ip_TimestampBufferType timestampBuffer;
} Lpit_Icu_Ip_TimestampType;
#endif /* STD_ON == LPIT_ICU_TIMESTAMP_API */

/** @brief LPIT channel configuration structure. */
typedef struct
{
    const uint8                           hwChannel;               /**< @brief Channel hardware index. */
    const uint8                           triggerSelect;           /**< @brief Trigger to use for starting and/or reloading the LPIT timer. */
    const uint8                           triggerSource;           /**< @brief Select source of trigger. */
#if (STD_ON == LPIT_ICU_TIMESTAMP_API)
    const Lpit_Icu_Ip_TimestampBufferType timestampBuffer;
#endif
    Lpit_Icu_Ip_CallbackType              callback;                /*!< @brief The callback function for channels edge detect events */
    uint8                                 callbackParams;          /*!< @brief The parameters of callback functions for channels events */
    Lpit_Icu_Ip_LogicChState              logicChStateCallback;    /*!< @brief Store address of function used to change the logic state of the channel in HLD. */
    const Lpit_Icu_Ip_NotifyType          lpitChannelNotify;       /*!< @brief The notification functions shall have no parameters and no return value.*/
} Lpit_Icu_Ip_ChannelConfigType;

/** @brief LPIT IP specific configuration structure type. */
typedef struct
{
    const uint8                         instance;             /**< @brief Instance hardware index. */
    const boolean                       debugState;           /**< @brief Debug(freeze) mode. */
    const boolean                       dozeMode;             /**< @brief enable/disable Doze mode. */
    const uint8                         numberOfChannels;     /**< @brief Number of LPIT channels on the current instance. */ 
    const Lpit_Icu_Ip_ChannelConfigType (*pChannelsConfig)[]; /**< @brief Pointer to the array of configured channels. */
} Lpit_Icu_Ip_ConfigType;

/** @brief LPIT channel measurement mode supported. */
typedef enum
{
    LPIT_ICU_MODE_NO_MEASUREMENT     = 0x0U, /**< @brief No measurement mode. */
    LPIT_ICU_MODE_SIGNAL_EDGE_DETECT = 0x1U, /**< @brief Signal edge detect measurement mode. */
    LPIT_ICU_MODE_TIMESTAMP          = 0x2U  /**< @brief Timestamp measurement mode.*/
} Lpit_Icu_Ip_MeasurementMode;

/** @brief LPIT channels state. */
typedef struct
{
    boolean                     initState;               /**< @brief Initialization status. */
    Lpit_Icu_Ip_MeasurementMode measurementMode;         /**< @brief Measurement mode for current channel. */
    Lpit_Icu_Ip_CallbackType    callback;                /*!< @brief The callback function for channels edge detect events */
    uint8                       callbackParams;          /*!< @brief The parameters of callback functions for channels events */
    Lpit_Icu_Ip_LogicChState    logicChStateCallback;    /*!< @brief Store address of function used to change the logic state of the channel in HLD. */
    Lpit_Icu_Ip_NotifyType      lpitChannelNotify;       /*!< @brief The notification functions shall have no parameters and no return value.*/
    boolean                     notificationEnable;      /*!< @brief Store the initialization state that determines whether Notifications are enabled, it is always TRUE with standalone IPL and FALSE with AUTOSAR mode. */         
} Lpit_Icu_Ip_ChannelsStateType;


#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* LPIT_ICU_TYPES */
