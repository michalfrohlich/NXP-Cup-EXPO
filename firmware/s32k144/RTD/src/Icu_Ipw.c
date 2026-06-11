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

/**
 *     @file
 *     @internal
 *     @addtogroup icu_ipw Icu Driver
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
#include "Icu_Ipw.h"
#include "Ftm_Icu_Ip.h"
#include "Lpit_Icu_Ip.h"
#include "Lptmr_Icu_Ip.h"
#include "Port_Ci_Icu_Ip.h"
#include "Cmp_Ip.h"

/*==================================================================================================
*                                    SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define ICU_IPW_VENDOR_ID_C                     43
#define ICU_IPW_AR_RELEASE_MAJOR_VERSION_C      4
#define ICU_IPW_AR_RELEASE_MINOR_VERSION_C      7
#define ICU_IPW_AR_RELEASE_REVISION_VERSION_C   0
#define ICU_IPW_SW_MAJOR_VERSION_C              2
#define ICU_IPW_SW_MINOR_VERSION_C              0
#define ICU_IPW_SW_PATCH_VERSION_C              0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if source file and ICU header file are of the same vendor */
#if (ICU_IPW_VENDOR_ID_C != ICU_IPW_VENDOR_ID)
    #error "Icu_Ipw.c and Icu_Ipw.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((ICU_IPW_AR_RELEASE_MAJOR_VERSION_C  != ICU_IPW_AR_RELEASE_MAJOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_MINOR_VERSION_C  != ICU_IPW_AR_RELEASE_MINOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_REVISION_VERSION_C   != ICU_IPW_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Icu_Ipw.c and Icu_Ipw.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((ICU_IPW_SW_MAJOR_VERSION_C  != ICU_IPW_SW_MAJOR_VERSION) || \
     (ICU_IPW_SW_MINOR_VERSION_C  != ICU_IPW_SW_MINOR_VERSION) || \
     (ICU_IPW_SW_PATCH_VERSION_C  != ICU_IPW_SW_PATCH_VERSION))
    #error "Software Version Numbers of Icu_Ipw.c and Icu_Ipw.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (ICU_IPW_VENDOR_ID_C != PORT_CI_ICU_IP_VENDOR_ID)
    #error "Icu_Ipw.c and Port_Ci_Icu_Ip.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((ICU_IPW_AR_RELEASE_MAJOR_VERSION_C    != PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_MINOR_VERSION_C    != PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_REVISION_VERSION_C != PORT_CI_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Icu_Ipw.c and Port_Ci_Icu_Ip.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((ICU_IPW_SW_MAJOR_VERSION_C  != PORT_CI_ICU_IP_SW_MAJOR_VERSION) || \
     (ICU_IPW_SW_MINOR_VERSION_C  != PORT_CI_ICU_IP_SW_MINOR_VERSION) || \
     (ICU_IPW_SW_PATCH_VERSION_C  != PORT_CI_ICU_IP_SW_PATCH_VERSION))
    #error "Software Version Numbers of Icu_Ipw.c and Port_Ci_Icu_Ip.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (ICU_IPW_VENDOR_ID_C != FTM_ICU_IP_VENDOR_ID)
    #error "Icu_Ipw.c and Ftm_Icu_Ip.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((ICU_IPW_AR_RELEASE_MAJOR_VERSION_C    != FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_MINOR_VERSION_C    != FTM_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_REVISION_VERSION_C != FTM_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Icu_Ipw.c and Ftm_Icu_Ip.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((ICU_IPW_SW_MAJOR_VERSION_C != FTM_ICU_IP_SW_MAJOR_VERSION) || \
     (ICU_IPW_SW_MINOR_VERSION_C != FTM_ICU_IP_SW_MINOR_VERSION) || \
     (ICU_IPW_SW_PATCH_VERSION_C != FTM_ICU_IP_SW_PATCH_VERSION))
    #error "Software Version Numbers of Icu_Ipw.c and Ftm_Icu_Ip.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (ICU_IPW_VENDOR_ID_C != LPTMR_ICU_IP_VENDOR_ID)
    #error "Icu_Ipw.c and Lptmr_Icu_Ip.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((ICU_IPW_AR_RELEASE_MAJOR_VERSION_C    != LPTMR_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_MINOR_VERSION_C    != LPTMR_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_REVISION_VERSION_C != LPTMR_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Icu_Ipw.c and Lptmr_Icu_Ip.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((ICU_IPW_SW_MAJOR_VERSION_C != LPTMR_ICU_IP_SW_MAJOR_VERSION) || \
     (ICU_IPW_SW_MINOR_VERSION_C != LPTMR_ICU_IP_SW_MINOR_VERSION) || \
     (ICU_IPW_SW_PATCH_VERSION_C != LPTMR_ICU_IP_SW_PATCH_VERSION))
    #error "Software Version Numbers of Icu_Ipw.c and Lptmr_Icu_Ip.h are different"
#endif

/* Check if source file and ICU Lpit_Icu_Ip.h header file are of the same vendor */
#if (ICU_IPW_VENDOR_ID_C != LPIT_ICU_IP_VENDOR_ID)
    #error "Icu_Ipw.c and Lpit_Icu_Ip.h have different vendor IDs!"
#endif
/* Check if source file and ICU Lpit_Icu_Ip.h header file are of the same AutoSar version */
#if ((ICU_IPW_AR_RELEASE_MAJOR_VERSION_C    != LPIT_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_MINOR_VERSION_C    != LPIT_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_REVISION_VERSION_C != LPIT_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Icu_Ipw.c and Lpit_Icu_Ip.h are different!"
#endif
/* Check if source file and ICU Lpit_Icu_Ip.h header file are of the same Software version */
#if ((ICU_IPW_SW_MAJOR_VERSION_C != LPIT_ICU_IP_SW_MAJOR_VERSION) || \
     (ICU_IPW_SW_MINOR_VERSION_C != LPIT_ICU_IP_SW_MINOR_VERSION) || \
     (ICU_IPW_SW_PATCH_VERSION_C != LPIT_ICU_IP_SW_PATCH_VERSION))
    #error "Software Version Numbers of Icu_Ipw.c and Lpit_Icu_Ip.h are different!"
#endif

/* Check if source file and ICU Cmp_Ip.h header file are of the same vendor */
#if (ICU_IPW_VENDOR_ID_C != CMP_IP_VENDOR_ID)
    #error "Icu_Ipw.c and Cmp_Ip.h have different vendor IDs!"
#endif
/* Check if source file and ICU Cmp_Ip.h header file are of the same AutoSar version */
#if ((ICU_IPW_AR_RELEASE_MAJOR_VERSION_C    != CMP_IP_AR_RELEASE_MAJOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_MINOR_VERSION_C    != CMP_IP_AR_RELEASE_MINOR_VERSION) || \
     (ICU_IPW_AR_RELEASE_REVISION_VERSION_C != CMP_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Icu_Ipw.c and Cmp_Ip.h are different!"
#endif
/* Check if source file and ICU Cmp_Ip.h header file are of the same Software version */
#if ((ICU_IPW_SW_MAJOR_VERSION_C != CMP_IP_SW_MAJOR_VERSION) || \
     (ICU_IPW_SW_MINOR_VERSION_C != CMP_IP_SW_MINOR_VERSION) || \
     (ICU_IPW_SW_PATCH_VERSION_C != CMP_IP_SW_PATCH_VERSION))
    #error "Software Version Numbers of Icu_Ipw.c and Cmp_Ip.h are different!"
#endif

/*==================================================================================================
*                           LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                        LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                    LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                        LOCAL FUNCTIONS
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"

/*==================================================================================================
*                                        GLOBAL FUNCTIONS
==================================================================================================*/
/**
* @brief                Icu_Ipw_Init
* @details              Initialize a hardware Icu IP Instance
*
* @param[in]            count       - Number of instances to be configured
* @param[in]            pIpConfig   - Pointer that contains IP specific configuration data for the Icu driver
*
* @return void
*
* @pre                  Icu_DeInit must be called before.
*
*/
void Icu_Ipw_Init(uint8 count, const Icu_Ipw_IpConfigType (*pIpConfig)[])
{
    uint8 index;

    for(index = 0; index < count; index++)
    {
        if (ICU_FTM_MODULE == (*pIpConfig)[index].instanceIp)
        {
            (void)Ftm_Icu_Ip_Init((*pIpConfig)[index].instanceNo, \
                                              (*pIpConfig)[index].pFtmHwIpConfig);
        }
        else if (ICU_LPIT_MODULE == (*pIpConfig)[index].instanceIp)
        {
            (void)Lpit_Icu_Ip_Init((*pIpConfig)[index].instanceNo, \
                                   (*pIpConfig)[index].pLpitHwInstanceConfig);
        }
        else if (ICU_LPTMR_MODULE == (*pIpConfig)[index].instanceIp)
        {
            (void)Lptmr_Icu_Ip_Init((*pIpConfig)[index].instanceNo, \
                                              (*pIpConfig)[index].pLptmrHwIpConfig);
        }
        else if (ICU_PORT_CI_MODULE == (*pIpConfig)[index].instanceIp)
        {
            (void)Port_Ci_Icu_Ip_Init((*pIpConfig)[index].instanceNo, \
                                              (*pIpConfig)[index].pPortCiHwIpConfig);
        }
#if (STD_ON == CMP_IP_USED)
        else if (ICU_CMP_MODULE == (*pIpConfig)[index].instanceIp)
        {
            (void)Cmp_Ip_Init((*pIpConfig)[index].instanceNo, \
                            (*pIpConfig)[index].pCmpHwIpConfig);
        }
#endif  /* CMP_IP_USED */
        else
        {
            /* Default do nothing. */
        }
    }
}

#if (STD_ON == ICU_DE_INIT_API)
/**
* @brief              Icu_Ipw_DeInit
* @details            De-initialize ICU hardware channel
*
* @param[in]          pIpConfig - Pointer to ICU top configuration structure
*
* @return void
*
* @pre                Icu_Init must be called before.
*
*/
void Icu_Ipw_DeInit(uint8 count, const Icu_Ipw_IpConfigType (*pIpConfig)[])
{
    uint8 index;

    for(index = 0; index < count; index++)
    {
        if (ICU_FTM_MODULE == (*pIpConfig)[index].instanceIp)
        {
            (void)Ftm_Icu_Ip_DeInit((*pIpConfig)[index].instanceNo);
        }
        else if (ICU_LPIT_MODULE == (*pIpConfig)[index].instanceIp)
        {
            (void)Lpit_Icu_Ip_Deinit((*pIpConfig)[index].instanceNo);
        }
        else if (ICU_LPTMR_MODULE == (*pIpConfig)[index].instanceIp)
        {
            (void)Lptmr_Icu_Ip_Deinit((*pIpConfig)[index].instanceNo);
        }
        else if (ICU_PORT_CI_MODULE == (*pIpConfig)[index].instanceIp)
        {
            (void)Port_Ci_Icu_Ip_DeInit((*pIpConfig)[index].instanceNo);
        }
#if (STD_ON == CMP_IP_USED)
        else if (ICU_CMP_MODULE == (*pIpConfig)[index].instanceIp)
        {
            (void)Cmp_Ip_Deinit((*pIpConfig)[index].instanceNo);
        }
#endif  /* CMP_IP_USED */
        else
        {
            /* Default do nothing. */
        }
    }
}
#endif /* ICU_DE_INIT_API == STD_ON */

#if (ICU_SET_MODE_API == STD_ON)
/**
 * @brief Put the channel in a reduce power state.
 * @details    Set sleep mode
 * 
 * @param ChannelConfig - configuration of the channel
 *
 * @return void
 * @internal
 */
void Icu_Ipw_SetSleepMode(const Icu_Ipw_ChannelConfigType* ChannelConfig)
{
    uint8 channel;
    uint8 module = ChannelConfig->instanceNo;
    Icu_Ipw_ModuleType ipType = ChannelConfig->channelIp;

    /* Select IP type case. */
    switch(ipType)
    {
        case ICU_FTM_MODULE:
        {
            channel = (ChannelConfig->pFtmHwChannelConfig)->hwChannel;
            Ftm_Icu_Ip_SetSleepMode(module, channel);
            break;
        }
        case ICU_LPTMR_MODULE:
        {
            Lptmr_Icu_Ip_SetSleepMode(module);
            break;
        }
        case ICU_PORT_CI_MODULE:
        {
            channel = (ChannelConfig->pPortCiHwChannelConfig)->pinId;
            Port_Ci_Icu_Ip_SetSleepMode(module, channel);
            break;
        }
        case ICU_LPIT_MODULE:
        {
            channel = (ChannelConfig->pLpitHwChannelConfig)->hwChannel;
            Lpit_Icu_Ip_SetSleepMode(module, channel);
            break;
        }
#if (STD_ON == CMP_IP_USED)
        case ICU_CMP_MODULE:
        {
            Cmp_Ip_SetSleepMode(module);
            break;
        }
#endif
        default:
        {
            /* Do nothing. */
        }
        break;
    }
}

/**
 * @brief      Icu_Ipw_SetNormalMode
 * @details    Set normal mode
 *
 * @param[in]  ChannelConfig - The index of ICU channel for current configuration structure
 *
 * @return void
 * @internal
 */
void Icu_Ipw_SetNormalMode (const Icu_Ipw_ChannelConfigType* ChannelConfig)
{
    uint8 channel;
    uint8 module = ChannelConfig->instanceNo;
    Icu_Ipw_ModuleType ipType = ChannelConfig->channelIp;

    /* Select IP type case. */
    switch(ipType)
    {
        case ICU_FTM_MODULE:
        {
            channel = (ChannelConfig->pFtmHwChannelConfig)->hwChannel;
            Ftm_Icu_Ip_SetNormalMode(module, channel);
            break;
        }
        case ICU_LPTMR_MODULE:
        {
            Lptmr_Icu_Ip_SetNormalMode(module);
            break;
        }
        case ICU_PORT_CI_MODULE:
        {
            channel = (ChannelConfig->pPortCiHwChannelConfig)->pinId;
            Port_Ci_Icu_Ip_SetNormalMode(module, channel);
            break;
        }
        case ICU_LPIT_MODULE:
        {
            channel = (ChannelConfig->pLpitHwChannelConfig)->hwChannel;
            Lpit_Icu_Ip_SetNormalMode(module, channel);
            break;
        }
#if (STD_ON == CMP_IP_USED)
        case ICU_CMP_MODULE:
        {
            Cmp_Ip_SetNormalMode(module);
            break;
        }
#endif
        default:
            {
                /* Do nothing. */
            }
            break;
    }
}
#endif  /* ICU_SET_MODE_API */

/**
* @brief      Icu_Ipw_SetActivationCondition
 * @brief 
 * 
 * @param activation    - the type of activation for the ICU channel.
 * @param ChannelConfig - channel to be configured.
*
* @return void
*
*/
void Icu_Ipw_SetActivationCondition(Icu_ActivationType activation,
                                    const Icu_Ipw_ChannelConfigType* ChannelConfig)
{
    uint8 channel;
    uint8 module = ChannelConfig->instanceNo;
    Icu_Ipw_ModuleType ipType = ChannelConfig->channelIp;

    /* Select IP type case. */
    switch(ipType)
    {
        case ICU_FTM_MODULE:
        {
            channel = (ChannelConfig->pFtmHwChannelConfig)->hwChannel;
            if (ICU_FALLING_EDGE == activation)
            {
                Ftm_Icu_Ip_SetActivationCondition(module, channel, FTM_ICU_FALLING_EDGE);
            }
            else if (ICU_RISING_EDGE == activation)
            {
                Ftm_Icu_Ip_SetActivationCondition(module, channel, FTM_ICU_RISING_EDGE);
            }
            else if (ICU_BOTH_EDGES == activation)
            {
                Ftm_Icu_Ip_SetActivationCondition(module, channel, FTM_ICU_BOTH_EDGES);
            }
            else
            {
                Ftm_Icu_Ip_SetActivationCondition(module, channel, FTM_ICU_NO_PIN_CONTROL);
            }
            break;
        }
        case ICU_PORT_CI_MODULE:
        {
            channel = (ChannelConfig->pPortCiHwChannelConfig)->pinId;
            if (ICU_FALLING_EDGE == activation)
            {
                Port_Ci_Icu_Ip_SetActivationCondition(module, channel, PORT_CI_ICU_FALLING_EDGE);
            }
            else if (ICU_RISING_EDGE == activation)
            {
                Port_Ci_Icu_Ip_SetActivationCondition(module, channel, PORT_CI_ICU_RISING_EDGE);
            }
            else if (ICU_BOTH_EDGES == activation)
            {
                Port_Ci_Icu_Ip_SetActivationCondition(module, channel, PORT_CI_ICU_BOTH_EDGES);
            }
            else
            {
                Port_Ci_Icu_Ip_SetActivationCondition(module, channel, PORT_CI_ICU_NO_PIN_CONTROL);
            }
            break;
        }
        case ICU_LPTMR_MODULE:
        {
            if (ICU_FALLING_EDGE == activation)
            {
                Lptmr_Icu_Ip_SetActivationCondition(module, LPTMR_ICU_FALLING_EDGE);
            }
            else
            {
                Lptmr_Icu_Ip_SetActivationCondition(module, LPTMR_ICU_RISING_EDGE);
            }
            break;
        }
#if (STD_ON == CMP_IP_USED)
        case ICU_CMP_MODULE:
        {
            if (ICU_FALLING_EDGE == activation)
            {
                Cmp_Ip_SetInterruptActivation(module, CMP_IP_INTTRIG_FALLING_EDGE);
            }
            else if (ICU_RISING_EDGE == activation)
            {
                Cmp_Ip_SetInterruptActivation(module, CMP_IP_INTTRIG_RISING_EDGE);
            }
            else if (ICU_BOTH_EDGES == activation)
            {
                Cmp_Ip_SetInterruptActivation(module, CMP_IP_INTTRIG_BOTH_EDGES);
            }
            else
            {
                Cmp_Ip_SetInterruptActivation(module, CMP_IP_INTTRIG_NONE);
            }
            break;
        }
#endif  /* CMP_IP_USED */
        default:
            {
                /* Do nothing. */
            }
            break;
    }
}

#if (ICU_GET_INPUT_STATE_API == STD_ON)
/**
 * @brief Service that returns the state of the ICU driver.
 * 
 * @param ChannelConfig 
 * @return boolean 
 */
boolean Icu_Ipw_GetInputState(const Icu_Ipw_ChannelConfigType *ChannelConfig)
{
    boolean retState = FALSE;

    if (ICU_FTM_MODULE == ChannelConfig->channelIp)
    {
        retState = Ftm_Icu_Ip_GetInputState(ChannelConfig->instanceNo, \
                        (ChannelConfig->pFtmHwChannelConfig)->hwChannel);
    }
    else if (ICU_LPIT_MODULE == ChannelConfig->channelIp)
    {
        retState = Lpit_Icu_Ip_GetInputState(ChannelConfig->instanceNo, \
                        (ChannelConfig->pLpitHwChannelConfig)->hwChannel);
    }
    else if (ICU_LPTMR_MODULE == ChannelConfig->channelIp)
    {
        retState = Lptmr_Icu_Ip_GetInputState(ChannelConfig->instanceNo);
    }
    else if (ICU_PORT_CI_MODULE == ChannelConfig->channelIp)
    {
        retState = Port_Ci_Icu_Ip_GetInputState(ChannelConfig->instanceNo, \
                        (ChannelConfig->pPortCiHwChannelConfig)->pinId);
    }
#if (STD_ON == CMP_IP_USED)
    else if (ICU_CMP_MODULE == ChannelConfig->channelIp)
    {
        retState = Cmp_Ip_GetStatus(ChannelConfig->instanceNo);
    }
#endif  /* CMP_IP_USED */
    else
    {
        retState = FALSE;
    }
    return (retState);
}
#endif /* ICU_GET_INPUT_STATE_API */

#if (STD_ON == ICU_TIMESTAMP_API)
void Icu_Ipw_StartTimestamp(const Icu_Ipw_ChannelConfigType* channelConfig,
                            Icu_ValueType* bufferPtr,
                            uint16  bufferSize,
                            uint16  notifyInterval)
{
    /* Select IP type. */
    switch(channelConfig->channelIp)
    {
        case ICU_FTM_MODULE:
        {
            /* Call FTM start timestamp function. */
            Ftm_Icu_Ip_StartTimestamp(channelConfig->instanceNo,
                                      (channelConfig->pFtmHwChannelConfig)->hwChannel,
                                      bufferPtr,
                                      bufferSize,
                                      notifyInterval);
            break;
        }
        case ICU_LPIT_MODULE:
        {
            Lpit_Icu_Ip_StartTimestamp(channelConfig->instanceNo,
                                       (channelConfig->pLpitHwChannelConfig)->hwChannel,
                                       bufferPtr,
                                       bufferSize,
                                       notifyInterval);
            break;
        }
        default:
        {
            /* Do nothing. */
            break;
        }
    }
}

void Icu_Ipw_StopTimestamp(const Icu_Ipw_ChannelConfigType *channelConfig)
{
    /* Select IP type. */
    switch(channelConfig->channelIp)
    {
        case ICU_FTM_MODULE:
        {
            /* Call FTM stop timestamp function. */
            Ftm_Icu_Ip_StopTimestamp(channelConfig->instanceNo, \
                                     (channelConfig->pFtmHwChannelConfig)->hwChannel);
            break;
        }
        case ICU_LPIT_MODULE:
        {
            Lpit_Icu_Ip_DisableDetectionMode(channelConfig->instanceNo, \
                                      (channelConfig->pLpitHwChannelConfig)->hwChannel);
            break;
        }
        default:
        {
            /* Do nothing. */
            break;
        }
    }
}

uint16 Icu_Ipw_GetTimestampIndex(const Icu_Ipw_ChannelConfigType* channelConfig)
{
    uint16 timestampIndex = 0U;
    /* Select IP type. */
    switch(channelConfig->channelIp)
    {
        case ICU_FTM_MODULE:
        {
            timestampIndex = Ftm_Icu_Ip_GetTimestampIndex(channelConfig->instanceNo, \
                                                          (channelConfig->pFtmHwChannelConfig)->hwChannel);
            break;
        }
        case ICU_LPIT_MODULE:
        {
            timestampIndex = Lpit_Icu_Ip_GetTimestampIndex(channelConfig->instanceNo,
                                                           (channelConfig->pLpitHwChannelConfig)->hwChannel);
            break;
        }
        default:
        {
            /* Do nothing. */
            break;
        }
    }
    return timestampIndex;
}

#if (ICU_TIMESTAMP_USES_DMA == STD_ON)
uint32 Icu_Ipw_GetStartAddress(const Icu_Ipw_ChannelConfigType *ChannelConfig)
{
    uint32 startAddr = 0U;
    if (ICU_FTM_MODULE == ChannelConfig->channelIp)
    {
        /* Get start address when using DMA */
        startAddr = Ftm_Icu_Ip_GetStartAddress(ChannelConfig->instanceNo, \
                               (ChannelConfig->pFtmHwChannelConfig)->hwChannel);
    }
    return startAddr;
}
#endif  /* ICU_TIMESTAMP_USES_DMA == STD_ON */

#endif  /* ICU_TIMESTAMP_API == STD_ON */

#if (STD_ON == ICU_EDGE_COUNT_API)
void Icu_Ipw_ResetEdgeCount(const Icu_Ipw_ChannelConfigType *ChannelConfig)
{
    /* Select IP type case. */
    if(ICU_FTM_MODULE == ChannelConfig->channelIp)
    {
        Ftm_Icu_Ip_ResetEdgeCount(ChannelConfig->instanceNo, \
                    (ChannelConfig->pFtmHwChannelConfig)->hwChannel);
    }
    else if(ICU_LPTMR_MODULE == ChannelConfig->channelIp)
    {
        Lptmr_Icu_Ip_ResetEdgeCount(ChannelConfig->instanceNo);
    }
    else
    {
        /* Default do nothing. */
    }
}

void Icu_Ipw_EnableEdgeCount(const Icu_Ipw_ChannelConfigType *ChannelConfig)
{
    /* Select IP type case. */
    if(ICU_FTM_MODULE == ChannelConfig->channelIp)
    {
        (void)Ftm_Icu_Ip_EnableEdgeCount(ChannelConfig->instanceNo, \
                    (ChannelConfig->pFtmHwChannelConfig)->hwChannel);
    }
    else if(ICU_LPTMR_MODULE == ChannelConfig->channelIp)
    {
        Lptmr_Icu_Ip_EnableEdgeCount(ChannelConfig->instanceNo);
    }
    else
    {
        /* Default do nothing. */
    }
}

void Icu_Ipw_DisableEdgeCount(const Icu_Ipw_ChannelConfigType *ChannelConfig)
{
    /* Select IP type case. */
    if(ICU_FTM_MODULE == ChannelConfig->channelIp)
    {
        (void)Ftm_Icu_Ip_DisableEdgeCount(ChannelConfig->instanceNo, \
                    (ChannelConfig->pFtmHwChannelConfig)->hwChannel);
    }
    else if(ICU_LPTMR_MODULE == ChannelConfig->channelIp)
    {
        (void)Lptmr_Icu_Ip_DisableEdgeCount(ChannelConfig->instanceNo);
    }
    else
    {
        /* Default do nothing. */
    }
}

uint16 Icu_Ipw_GetEdgeNumbers(const Icu_Ipw_ChannelConfigType *ChannelConfig)
{
    uint16 edgeNumber = (uint16)0U;

    /* Select IP type case. */
    if(ICU_FTM_MODULE == ChannelConfig->channelIp)
    {
        edgeNumber = (uint16)Ftm_Icu_Ip_GetEdgeNumbers(ChannelConfig->instanceNo, \
                        (ChannelConfig->pFtmHwChannelConfig)->hwChannel);
    }
    else if(ICU_LPTMR_MODULE == ChannelConfig->channelIp)
    {
        edgeNumber = (uint16)Lptmr_Icu_Ip_GetEdgeNumbers(ChannelConfig->instanceNo);
    }
    else
    {
        edgeNumber = (uint16)0U;
    }

    return edgeNumber;
}
#endif  /* STD_ON == ICU_EDGE_COUNT_API */

#if (STD_ON == ICU_EDGE_DETECT_API)
/**
 * @brief          Retrieve the number of edges
 *
 * @param[in]      nChannelNumber - The index of ICU channel for current configuration structure
 * @return         void
 */
void Icu_Ipw_EnableEdgeDetection(const Icu_Ipw_ChannelConfigType * ChannelConfig)
{
    uint8 channel;
    uint8 module = ChannelConfig->instanceNo;
    Icu_Ipw_ModuleType ipType = ChannelConfig->channelIp;

    /* Select IP type case. */
    switch(ipType)
    {
        case ICU_FTM_MODULE:
        {
            channel = (ChannelConfig->pFtmHwChannelConfig)->hwChannel;
            (void)Ftm_Icu_Ip_EnableEdgeDetection(module, channel);
            break;
        }
        case ICU_LPIT_MODULE:
        {
            channel = (ChannelConfig->pLpitHwChannelConfig)->hwChannel;
            Lpit_Icu_Ip_EnableEdgeDetection(module, channel);
            break;
        }
        case ICU_LPTMR_MODULE:
        {
            Lptmr_Icu_Ip_EnableEdgeDetection(module);
            break;
        }
        case ICU_PORT_CI_MODULE:
        {
            channel = (ChannelConfig->pPortCiHwChannelConfig)->pinId;
            Port_Ci_Icu_Ip_EnableEdgeDetection(module, channel);
            break;
        }
#if (STD_ON == CMP_IP_USED)
        case ICU_CMP_MODULE:
        {
            Cmp_Ip_EnableInterrupt(module);
            Cmp_Ip_EnableNotification(module);
            break;
        }
#endif  /* CMP_IP_USED */
        default:
        {
            /* Do nothing. */
            break;
        }
    }
}
#endif /* ICU_EDGE_DETECT_API */

#if (ICU_EDGE_DETECT_API == STD_ON)
/**
 * @brief 
 * 
 * @param ChannelConfig - channel configuration used.
 */
void Icu_Ipw_DisableEdgeDetection(const Icu_Ipw_ChannelConfigType* ChannelConfig)
{
    uint8 channel;
    uint8 module = ChannelConfig->instanceNo;
    Icu_Ipw_ModuleType ipType = ChannelConfig->channelIp;

    /* Select IP type case. */
    switch(ipType)
    {
        case ICU_FTM_MODULE:
        {
            channel = (ChannelConfig->pFtmHwChannelConfig)->hwChannel;
            (void)Ftm_Icu_Ip_DisableEdgeDetection(module, channel);
            break;
        }
        case ICU_LPIT_MODULE:
        {
            channel = (ChannelConfig->pLpitHwChannelConfig)->hwChannel;
            Lpit_Icu_Ip_DisableDetectionMode(module, channel);
            break;
        }
        case ICU_LPTMR_MODULE:
        {
            Lptmr_Icu_Ip_DisableDetection(module);
            break;
        }
        case ICU_PORT_CI_MODULE:
        {
            channel = (ChannelConfig->pPortCiHwChannelConfig)->pinId;
            Port_Ci_Icu_Ip_DisableDetection(module, channel);
            break;
        }
#if (STD_ON == CMP_IP_USED)
        case ICU_CMP_MODULE:
        {
            Cmp_Ip_DisableInterrupt(module);
            Cmp_Ip_DisableNotification(module);
            break;
        }
#endif  /* CMP_IP_USED */
        default:
        {
            /* Do nothing. */
            break;
        }
    }
}
#endif /* ICU_EDGE_DETECT_API */

#if ((ICU_OVERFLOW_NOTIFICATION_API == STD_OFF) && (ICU_VALIDATE_PARAMS == STD_ON))
#if ((ICU_EDGE_COUNT_API == STD_ON) || (ICU_TIMESTAMP_API == STD_ON) || \
     (ICU_GET_TIME_ELAPSED_API == STD_ON) || (ICU_GET_DUTY_CYCLE_VALUES_API == STD_ON) )
/**
 * @brief The function get the state of the overflow flag
 * @internal
 * @param ChannelConfig      Channel configuration pointer
 * @return      boolean      the state of the overflow flag
 * @retval      TRUE         the overflow flag is set
 * @retval      FALSE        the overflow flag is not set
 */
boolean Icu_Ipw_Get_Overflow(const Icu_Ipw_ChannelConfigType* ChannelConfig)
{
    boolean bOverflow = FALSE;

    if (ICU_FTM_MODULE == ChannelConfig->channelIp)
    {
        /* Check if FTM Module Overflow. */
        bOverflow = (boolean)Ftm_Icu_Ip_GetOverflow(ChannelConfig->instanceNo);
    }
    return bOverflow;
}
#endif /* ICU_EDGE_COUNT_API == STD_ON */
#endif /* (ICU_OVERFLOW_NOTIFICATION_API == STD_OFF) && (ICU_VALIDATE_PARAMS == STD_ON) */

#if (STD_ON == ICU_GET_INPUT_LEVEL_API)
Icu_LevelType Icu_Ipw_GetInputLevel(const Icu_Ipw_ChannelConfigType* ChannelConfig)
{
    Icu_LevelType nInputLevel;

    /* Select IP type case. */
    switch(ChannelConfig->channelIp)
    {
        case ICU_FTM_MODULE:
        {
            nInputLevel = (Ftm_Icu_Ip_GetInputLevel(ChannelConfig->instanceNo, \
                            (ChannelConfig->pFtmHwChannelConfig)->hwChannel) == FTM_ICU_LEVEL_HIGH)?ICU_LEVEL_HIGH:ICU_LEVEL_LOW;
            break;
        }
        default:
        {
            nInputLevel = ICU_LEVEL_LOW;
            break;
        }
    }
    return nInputLevel;
}
#endif /* STD_ON == ICU_GET_INPUT_LEVEL_API */

#if ((ICU_CAPTURERGISTER_API == STD_ON) && ((ICU_SIGNAL_MEASUREMENT_API == STD_ON) || (ICU_TIMESTAMP_API == STD_ON)))/**
 * @brief      Capture the value of counter register for a specified channel.
 * 
 * @details   The API shall return the value stored in capture register.
 *            The API is the equivalent of AUTOSAR API GetCaptureRegisterValue.
 * 
 * @param ChannelConfig      Channel configuration pointer
 * @return     uint32  Value of the register captured.
 */
Icu_ValueType Icu_Ipw_GetCaptureRegisterValue (const Icu_Ipw_ChannelConfigType* ChannelConfig)
{
    Icu_ValueType nRValue;
    if (ICU_FTM_MODULE == ChannelConfig->channelIp)
    {
        /* Get FTM Module Capture Register Value */
        nRValue = (Icu_ValueType)Ftm_Icu_Ip_GetCaptureRegisterValue(ChannelConfig->instanceNo, \
                                                                    (ChannelConfig->pFtmHwChannelConfig)->hwChannel);
    }
    else
    {
        nRValue = (Icu_ValueType)0U;
    }
    return nRValue;
}
#endif

#if (STD_ON == ICU_SIGNAL_MEASUREMENT_API)
void Icu_Ipw_StartSignalMeasurement (const Icu_Ipw_ChannelConfigType* ChannelConfig)
{
    if (ICU_FTM_MODULE == ChannelConfig->channelIp)
    {
        /* Start Signal Measurement. */
        Ftm_Icu_Ip_StartSignalMeasurement(ChannelConfig->instanceNo, \
                    (ChannelConfig->pFtmHwChannelConfig)->hwChannel);
    }
}

void Icu_Ipw_StopSignalMeasurement(const Icu_Ipw_ChannelConfigType * ChannelConfig)
{
    if (ICU_FTM_MODULE == ChannelConfig->channelIp)
    {
        /* Start Signal Measurement. */
        Ftm_Icu_Ip_StopSignalMeasurement(ChannelConfig->instanceNo, \
                    (ChannelConfig->pFtmHwChannelConfig)->hwChannel);
    }
}

#endif  /* STD_ON == ICU_SIGNAL_MEASUREMENT_API */

#if (STD_ON == ICU_GET_DUTY_CYCLE_VALUES_API)
void Icu_Ipw_GetDutyCycleValues(const Icu_Ipw_ChannelConfigType* channelConfig,
                                Icu_DutyCycleType* DutyCycleValues)
{
    /* Select IP type case. */
    if(ICU_FTM_MODULE == channelConfig->channelIp)
    {
        Ftm_Icu_Ip_DutyCycleType ftmDutyCycleValues = {0, 0};
        Ftm_Icu_Ip_GetDutyCycleValues(channelConfig->instanceNo, \
                                      (channelConfig->pFtmHwChannelConfig)->hwChannel, \
                                      &ftmDutyCycleValues);
        DutyCycleValues->ActiveTime = (Icu_ValueType)ftmDutyCycleValues.ActiveTime;
        DutyCycleValues->PeriodTime = (Icu_ValueType)ftmDutyCycleValues.PeriodTime;
    }
}
#endif /* STD_ON == ICU_GET_DUTY_CYCLE_VALUES_API */

#if (STD_ON == ICU_GET_TIME_ELAPSED_API)
Icu_ValueType Icu_Ipw_GetTimeElapsed(const Icu_Ipw_ChannelConfigType* channelConfig)
{
    uint16 timeElapsed;
    /* Select IP type case. */
    switch(channelConfig->channelIp)
    {
        case ICU_FTM_MODULE:
        {
            timeElapsed = Ftm_Icu_Ip_GetTimeElapsed(channelConfig->instanceNo, \
                                                   (channelConfig->pFtmHwChannelConfig)->hwChannel);
            break;
        }
        default:
        {
            timeElapsed = 0U;
            break;
        }
    }
    return (Icu_ValueType)timeElapsed;
}

#endif /* STD_ON == ICU_GET_TIME_ELAPSED_API */

#if (STD_ON == ICU_DUAL_CLOCK_MODE_API)
/**
 * @brief This function sets the module prescalers based on the input mode.
 * 
 * @param moduleConfig 
 * @param selectPrescaler Prescaler type ( Normal or Alternate )
 * @param modulesNumber 
 */
void Icu_Ipw_SetClockMode(const Icu_Ipw_IpConfigType (*moduleConfig)[], Icu_SelectPrescalerType selectPrescaler, uint8 modulesNumber)
{
    uint8              index;
    uint8              module;
    Icu_Ipw_ModuleType ipType;

    for(index = 0; index < modulesNumber; index++)
    {
        ipType = (*moduleConfig)[index].instanceIp;
        module = (*moduleConfig)[index].instanceNo;

        /* Select IP type case. */
        switch(ipType)
        {
            case ICU_FTM_MODULE:
            {
                Ftm_Icu_Ip_SetPrescaler(module, \
                    (selectPrescaler == ICU_ALTERNATE_CLOCK_MODE) ? FTM_ICU_ALTERNATE_CLK : FTM_ICU_NORMAL_CLK);
                break;
            }
            default:
            {
                /* Do nothing. */
                break;
            }

        }
    }
}
#endif /* STD_ON == ICU_DUAL_CLOCK_MODE_API */

#if ((ICU_VALIDATE_PARAMS == STD_ON) && (ICU_GET_INPUT_LEVEL_API == STD_ON))
Icu_Ipw_StatusType Icu_Ipw_ValidateGetInputLevel(const Icu_Ipw_ChannelConfigType * ChannelConfig)
{
    Icu_Ipw_ModuleType  ipType = ChannelConfig->channelIp;
    Icu_Ipw_StatusType result = ICU_IPW_ERROR;

    /* Select IP type case. */
    if(ICU_FTM_MODULE == ipType)
    {
        result = ICU_IPW_SUCCESS;
    }
    return result;
}
#endif /* (ICU_VALIDATE_PARAMS == STD_ON) && (ICU_GET_INPUT_LEVEL_API == STD_ON */

void Icu_Ipw_EnableNotification(const Icu_Ipw_ChannelConfigType* ChannelConfig)
{
    uint8 channel;
    uint8 module = ChannelConfig->instanceNo;
    Icu_Ipw_ModuleType ipType = ChannelConfig->channelIp;

    /* Select IP type case. */
    switch(ipType)
    {
        case ICU_FTM_MODULE:
        {
            channel = (ChannelConfig->pFtmHwChannelConfig)->hwChannel;
            Ftm_Icu_Ip_EnableNotification(module, channel);
            break;
        }
        case ICU_LPIT_MODULE:
        {
            channel = (ChannelConfig->pLpitHwChannelConfig)->hwChannel;
            Lpit_Icu_Ip_EnableNotification(module, channel);
            break;
        }
        case ICU_LPTMR_MODULE:
        {
            Lptmr_Icu_Ip_EnableNotification(module);
            break;
        }
        case ICU_PORT_CI_MODULE:
        {
            channel = (ChannelConfig->pPortCiHwChannelConfig)->pinId;
            Port_Ci_Icu_Ip_EnableNotification(module, channel);
            break;
        }
#if (STD_ON == CMP_IP_USED)
        case ICU_CMP_MODULE:
        {
            Cmp_Ip_EnableNotification(module);
            break;
        }
#endif  /* CMP_IP_USED */
        default:
        {
            /* Do nothing. */
        }
        break;
    }
}

void Icu_Ipw_DisableNotification(const Icu_Ipw_ChannelConfigType* ChannelConfig)
{
    uint8 channel;
    uint8 module = ChannelConfig->instanceNo;
    Icu_Ipw_ModuleType ipType = ChannelConfig->channelIp;

    /* Select IP type case. */
    switch(ipType)
    {
        case ICU_FTM_MODULE:
        {
            channel = (ChannelConfig->pFtmHwChannelConfig)->hwChannel;
            Ftm_Icu_Ip_DisableNotification(module, channel);
            break;
        }
        case ICU_LPIT_MODULE:
        {
            channel = (ChannelConfig->pLpitHwChannelConfig)->hwChannel;
            Lpit_Icu_Ip_DisableNotification(module, channel);
            break;
        }
        case ICU_LPTMR_MODULE:
        {
            Lptmr_Icu_Ip_DisableNotification(module);
            break;
        }
        case ICU_PORT_CI_MODULE:
        {
            channel = (ChannelConfig->pPortCiHwChannelConfig)->pinId;
            Port_Ci_Icu_Ip_DisableNotification(module, channel);
            break;
        }
#if (STD_ON == CMP_IP_USED)
        case ICU_CMP_MODULE:
        {
            Cmp_Ip_DisableNotification(module);
            break;
        }
#endif  /* CMP_IP_USED */
        default:
        {
            /* Do nothing. */
        }
        break;
    }
}

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */
