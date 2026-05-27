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

#ifndef PORT_CI_ICU_IP_TYPES
#define PORT_CI_ICU_IP_TYPES

/**
 *     @file
 * 
 *     @addtogroup port_ci_icu_ip PORT_CI IPL
 *     @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
==================================================================================================*/
#include "Std_Types.h"
#include "Port_Ci_Icu_Ip_Defines.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define PORT_CI_ICU_IP_TYPES_VENDOR_ID                   43
#define PORT_CI_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION    4
#define PORT_CI_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION    7
#define PORT_CI_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION 0
#define PORT_CI_ICU_IP_TYPES_SW_MAJOR_VERSION            2
#define PORT_CI_ICU_IP_TYPES_SW_MINOR_VERSION            0
#define PORT_CI_ICU_IP_TYPES_SW_PATCH_VERSION            0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if header file and Std_Types.h file are of the same Autosar version */
    #if ((PORT_CI_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION != STD_AR_RELEASE_MAJOR_VERSION) || \
         (PORT_CI_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION != STD_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Port_Ci_Icu_Ip_Types.h and Std_Types.h are different"
    #endif
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (PORT_CI_ICU_IP_TYPES_VENDOR_ID != PORT_CI_ICU_IP_DEFINES_VENDOR_ID)
    #error "Port_Ci_Icu_Ip_Types.h and Port_Ci_Icu_Ip_Defines.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((PORT_CI_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION    != PORT_CI_ICU_IP_DEFINES_AR_RELEASE_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION    != PORT_CI_ICU_IP_DEFINES_AR_RELEASE_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION != PORT_CI_ICU_IP_DEFINES_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Port_Ci_Icu_Ip_Types.h and Port_Ci_Icu_Ip_Defines.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((PORT_CI_ICU_IP_TYPES_SW_MAJOR_VERSION != PORT_CI_ICU_IP_DEFINES_SW_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_TYPES_SW_MINOR_VERSION != PORT_CI_ICU_IP_DEFINES_SW_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_TYPES_SW_PATCH_VERSION != PORT_CI_ICU_IP_DEFINES_SW_PATCH_VERSION))
    #error "Software Version Numbers of Port_Ci_Icu_Ip_Types.h and Port_Ci_Icu_Ip_Defines.h are different"
#endif

/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
#define ICU_PORT_CI_CH_NEITHER_EDGE_U8       ((uint8)(0U))
#define ICU_PORT_CI_CH_RISING_EDGE_U8        ((uint8)(1U))
#define ICU_PORT_CI_CH_FALLING_EDGE_U8       ((uint8)(2U))
#define ICU_PORT_CI_CH_EITHER_EDGE_U8        ((uint8)(3U))
/*==================================================================================================
*                                              ENUMS
==================================================================================================*/
/**
* @brief         Port_Ci_Icu_Ip_EdgeType
* @details       This indicates the activation type Port_Ci channel (Rising, Falling or Both)
*
* */
typedef enum
{
    PORT_CI_ICU_NO_PIN_CONTROL = 0x00, /** @brief No trigger. */

    PORT_CI_ICU_FALLING_EDGE = 0x1U,  /**< @brief  An appropriate action shall be executed when a
                                 * falling edge occurs on the Port_Ci input signal. */

    PORT_CI_ICU_RISING_EDGE  = 0x2U,  /**< @brief  An appropriate action shall be executed when a
                                 * rising edge occurs on the Port_Ci input signal. */

    PORT_CI_ICU_BOTH_EDGES   = 0x3U  /**< @brief  An appropriate action shall be executed when a rising
                                 * edge or falling edge occurs on the Port_Ci input signal. */
} Port_Ci_Icu_Ip_EdgeType;

/** @brief Generic error codes. */
typedef enum
{
    /** @brief Generic operation success status. */
    PORT_CI_IP_STATUS_SUCCESS                         = 0x00U,
    /** @brief Generic operation failure status. */
    PORT_CI_IP_STATUS_ERROR                           = 0x01U
} Port_Ci_Icu_Ip_StatusType;
/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
/** @brief The notification functions shall have no parameters and no return value.*/
typedef void                             (*Port_Ci_Icu_Ip_NotifyType)(void);
/** @brief Callback signature used in each channel with an active interrupt. */
typedef void (*Port_Ci_Icu_Ip_CallbackType)(uint16 callbackParam1, boolean callbackParam2);

typedef struct
{
    uint8                           pinId;                      /** @brief Assigned Port_Ci channel id*/
    Port_Ci_Icu_Ip_EdgeType   nDefaultStartEdge;          /** @brief Port_Ci Default Start Edge */
    Port_Ci_Icu_Ip_CallbackType     callback;                   /**< @brief Pointer to the callback function.            */
    Port_Ci_Icu_Ip_NotifyType       PortCiChannelNotification;  /**< @brief The notification functions shall have no parameters and no return value.*/
    uint8                           callbackParam;              /**< @brief The logic channel for which callback is set. */
} Port_Ci_Icu_Ip_ChannelConfigType;

/**
* @brief      Port_Ci IP specific configuration structure type
*/
typedef struct
{
    uint8                               nNumChannels;           /** @brief Number of Port_Ci channels in the Icu configuration */
    const Port_Ci_Icu_Ip_ChannelConfigType    (*pChannelsConfig)[];   /** @brief Pointer to the configured channels for Port_Ci */
} Port_Ci_Icu_Ip_ConfigType;

/**
 * @brief    PORT CI IP state structure.
 * @details  This structure is used by the IPL driver for internal logic.
 *           The content is populated at initialization time.
 */
typedef struct
{
    boolean                         chInit;                         /** @brief Initialization state. */
    Port_Ci_Icu_Ip_CallbackType     callback;                       /** @brief Pointer to the callback function. */
    Port_Ci_Icu_Ip_NotifyType       PortCiChannelNotification;      /** @brief The notification functions for SIGNAL_EDGE_DETECT mode. */
    uint16                          callbackParam;                  /** @brief The logic channel for which callback is set. */
    boolean                         notificationEnable;             /** @brief Store the initialization state that determines whether Notifications are enabled, it is always TRUE with standalone IPL and FALSE with AUTOSAR mode. */         
} Port_Ci_Icu_Ip_State;
/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* PORT_CI_ICU_IP_TYPES */
