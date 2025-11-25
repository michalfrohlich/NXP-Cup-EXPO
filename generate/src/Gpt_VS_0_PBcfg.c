/*==================================================================================================
* Project : RTD AUTOSAR 4.7
* Platform : CORTEXM
* Peripheral : Ftm_Srtc_Lptmr_LPit
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

/**
*   @file           Gpt_PBcfg.c
*
*   @addtogroup     gpt Gpt Driver
*   @{
*/
#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                         INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
===================================================================================================*/
#include "Gpt.h"
#include "Gpt_Ipw_VS_0_PBcfg.h"
/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define GPT_VENDOR_ID_VS_0_PBCFG_C                      43
#define GPT_AR_RELEASE_MAJOR_VERSION_VS_0_PBCFG_C       4
#define GPT_AR_RELEASE_MINOR_VERSION_VS_0_PBCFG_C       7
#define GPT_AR_RELEASE_REVISION_VERSION_VS_0_PBCFG_C    0
#define GPT_SW_MAJOR_VERSION_VS_0_PBCFG_C               2
#define GPT_SW_MINOR_VERSION_VS_0_PBCFG_C               0
#define GPT_SW_PATCH_VERSION_VS_0_PBCFG_C               0

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/

#if (GPT_VENDOR_ID_VS_0_PBCFG_C != GPT_VENDOR_ID)
    #error "Gpt_VS_0_PBcfg.c and Gpt.h have different vendor ids"
#endif
/* Check if the header files are of the same Autosar version */
#if ((GPT_AR_RELEASE_MAJOR_VERSION_VS_0_PBCFG_C != GPT_AR_RELEASE_MAJOR_VERSION) || \
     (GPT_AR_RELEASE_MINOR_VERSION_VS_0_PBCFG_C != GPT_AR_RELEASE_MINOR_VERSION) || \
     (GPT_AR_RELEASE_REVISION_VERSION_VS_0_PBCFG_C != GPT_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of Gpt_VS_0_PBcfg.c and Gpt.h are different"
#endif
/* Check if the header files are of the same Software version */
#if ((GPT_SW_MAJOR_VERSION_VS_0_PBCFG_C != GPT_SW_MAJOR_VERSION) || \
     (GPT_SW_MINOR_VERSION_VS_0_PBCFG_C != GPT_SW_MINOR_VERSION) || \
     (GPT_SW_PATCH_VERSION_VS_0_PBCFG_C != GPT_SW_PATCH_VERSION)    \
    )
    #error "Software Version Numbers of Gpt_VS_0_PBcfg.c and Gpt.h are different"
#endif

#if (GPT_VENDOR_ID_VS_0_PBCFG_C != GPT_IPW_VENDOR_ID_VS_0_PBCFG_H)
    #error "Gpt_VS_0_PBcfg.c and Gpt_Ipw_VS_0_PBcfg.h have different vendor ids"
#endif
/* Check if the header files are of the same Autosar version */
#if ((GPT_AR_RELEASE_MAJOR_VERSION_VS_0_PBCFG_C != GPT_IPW_AR_RELEASE_MAJOR_VERSION_VS_0_PBCFG_H) || \
     (GPT_AR_RELEASE_MINOR_VERSION_VS_0_PBCFG_C != GPT_IPW_AR_RELEASE_MINOR_VERSION_VS_0_PBCFG_H) || \
     (GPT_AR_RELEASE_REVISION_VERSION_VS_0_PBCFG_C != GPT_IPW_AR_RELEASE_REVISION_VERSION_VS_0_PBCFG_H) \
    )
    #error "AutoSar Version Numbers of Gpt_VS_0_PBcfg.c and Gpt_Ipw_VS_0_PBcfg.h are different"
#endif
/* Check if the header files are of the same Software version */
#if ((GPT_SW_MAJOR_VERSION_VS_0_PBCFG_C != GPT_IPW_SW_MAJOR_VERSION_VS_0_PBCFG_H) || \
     (GPT_SW_MINOR_VERSION_VS_0_PBCFG_C != GPT_IPW_SW_MINOR_VERSION_VS_0_PBCFG_H) || \
     (GPT_SW_PATCH_VERSION_VS_0_PBCFG_C != GPT_IPW_SW_PATCH_VERSION_VS_0_PBCFG_H)    \
    )
    #error "Software Version Numbers of Gpt_VS_0_PBcfg.c and Gpt_Ipw_VS_0_PBcfg.h. are different"
#endif

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
 *                                       GLOBAL VARIABLES
===================================================================================================*/
/**
*
* @brief The callback functions defined by the user to be called as channel notifications
*/
extern void Gpt_Notification(void);
extern void NewCameraFrame(void);

/*==================================================================================================
*                                       GLOBAL CONSTANTS
==================================================================================================*/
#define GPT_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Gpt_MemMap.h"
/* Number of channels per variant without partitions */
#define GPT_CONF_CHANNELS_PB_VS_0 2U
#define GPT_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Gpt_MemMap.h"


#define GPT_START_SEC_CONST_UNSPECIFIED
#include "Gpt_MemMap.h"
/*
*   @brief Translation LUT for Logical channel number to Partition Configuration indexed location
*/
static const uint8 u8GptChannelIdToIndexMap_VS_0[GPT_NUM_CONFIG] = 
{
    0        /*Logical Channel Receiver_Timeout*/,
    1        /*Logical Channel LinearCameraShutter*/
};
#define GPT_STOP_SEC_CONST_UNSPECIFIED
#include "Gpt_MemMap.h"


#define GPT_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Gpt_MemMap.h"
/*
*  @brief    Gpt_VS_0 Configuration
*/
static const Gpt_ChannelConfigType Gpt_InitChannelPB_VS_0[GPT_CONF_CHANNELS_PB_VS_0]=
{
    {    /*Receiver_Timeout configuration data*/
        (boolean)FALSE,/* Wakeup capability */
        &Gpt_Notification, /* Channel notification */
#if ((GPT_WAKEUP_FUNCTIONALITY_API == STD_ON) && (GPT_REPORT_WAKEUP_SOURCE == STD_ON))
        (EcuM_WakeupSourceType)0U,/* Wakeup information */
#endif
        (Gpt_ValueType)(65535U),/* Maximum ticks value*/
        (GPT_CH_MODE_ONESHOT),/* Timer mode:continous/one-shot */
        &Gpt_Ipw_ChannelConfig_PB_VS_0[0U]
    }
,
    {    /*LinearCameraShutter configuration data*/
        (boolean)FALSE,/* Wakeup capability */
        &NewCameraFrame, /* Channel notification */
#if ((GPT_WAKEUP_FUNCTIONALITY_API == STD_ON) && (GPT_REPORT_WAKEUP_SOURCE == STD_ON))
        (EcuM_WakeupSourceType)0U,/* Wakeup information */
#endif
        (Gpt_ValueType)(65535U),/* Maximum ticks value*/
        (GPT_CH_MODE_ONESHOT),/* Timer mode:continous/one-shot */
        &Gpt_Ipw_ChannelConfig_PB_VS_0[1U]
    }
};

const Gpt_ConfigType Gpt_Config = 
{
    /** @brief Number of GPT channels (configured in tresos plugin builder)*/
    (Gpt_ChannelType)2U,
    /**@brief Pointer to the GPT channel configuration */
    &Gpt_InitChannelPB_VS_0,
    /** @brief Number of GPT instances (configured in tresos plugin builder)*/
    2U,
    /** @brief Pointer to the GPT instance configuration */
    &Gpt_Ipw_HwInstanceConfig_PB_VS_0,
#if(GPT_PREDEFTIMER_FUNCTIONALITY_API == STD_ON)
    /** @brief Pointer configuration of PredefTimer  */
    Gpt_pInitPredefTimerChannelPB_VS_0,
#endif
    /** @brief Index of channel in each partition map table*/
    &u8GptChannelIdToIndexMap_VS_0
};

#define GPT_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Gpt_MemMap.h"

/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
 *================================================================================================*/

/*==================================================================================================
 *                                       LOCAL FUNCTIONS
 *================================================================================================*/

/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
 *================================================================================================*/

#ifdef __cplusplus
}
#endif

