/*==================================================================================================
* Project :             RTD AUTOSAR 4.7
* Platform :            CORTEXM
* Peripheral :          Ftm_Srtc_Lptmr_LPit
* Dependencies :        none
*
* Autosar Version :     4.7.0
* Autosar Revision :    ASR_REL_4_7_REV_0000
* Autosar Conf.Variant :
* SW Version :          2.0.0
* Build Version :       S32K1_RTD_2_0_0_D2308_ASR_REL_4_7_REV_0000_20230804
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
 *   @file          Ftm_Gpt_Ip_PBcfg.c
 *
 *   @addtogroup    Ftm_Gpt_Ip Ftm_Gpt_Ip IPL
 *
 *   @{
 */

/*==================================================================================================
 *                              GENERATION MACRO DEFINTION
 *================================================================================================*/


/*==================================================================================================*/
#ifdef __cplusplus
extern "C"{
#endif

 /*==================================================================================================
 *                                         INCLUDE FILES
 * 1) system and project includes
 * 2) needed interfaces from external units
 * 3) internal and external interfaces from this unit
 *================================================================================================*/
#include "Ftm_Gpt_Ip_VS_0_PBcfg.h"
#include "Gpt_Irq.h"

/*==================================================================================================
 *                              SOURCE FILE VERSION INFORMATION
 *================================================================================================*/
#define FTM_GPT_IP_VENDOR_ID_VS_0_PBCFG_C                     43
#define FTM_GPT_IP_AR_RELEASE_MAJOR_VERSION_VS_0_PBCFG_C      4
#define FTM_GPT_IP_AR_RELEASE_MINOR_VERSION_VS_0_PBCFG_C      7
#define FTM_GPT_IP_AR_RELEASE_REVISION_VERSION_VS_0_PBCFG_C   0
#define FTM_GPT_IP_SW_MAJOR_VERSION_VS_0_PBCFG_C              2
#define FTM_GPT_IP_SW_MINOR_VERSION_VS_0_PBCFG_C              0
#define FTM_GPT_IP_SW_PATCH_VERSION_VS_0_PBCFG_C              0
/*==================================================================================================
 *                                      FILE VERSION CHECKS
 *================================================================================================*/
#if (FTM_GPT_IP_VENDOR_ID_VS_0_PBCFG_C != FTM_GPT_IP_VENDOR_ID_VS_0_PBCFG_H)
    #error "Ftm_Gpt_Ip_VS_0_PBcfg.c and Ftm_Gpt_Ip_VS_0_PBcfg.h have different vendor ids"
#endif
/* Check if this header file and header file are of the same Autosar version */
#if ((FTM_GPT_IP_AR_RELEASE_MAJOR_VERSION_VS_0_PBCFG_C != FTM_GPT_IP_AR_RELEASE_MAJOR_VERSION_VS_0_PBCFG_H) || \
     (FTM_GPT_IP_AR_RELEASE_MINOR_VERSION_VS_0_PBCFG_C != FTM_GPT_IP_AR_RELEASE_MINOR_VERSION_VS_0_PBCFG_H) || \
     (FTM_GPT_IP_AR_RELEASE_REVISION_VERSION_VS_0_PBCFG_C != FTM_GPT_IP_AR_RELEASE_REVISION_VERSION_VS_0_PBCFG_H) \
    )
    #error "AutoSar Version Numbers of Ftm_Gpt_Ip_VS_0_PBcfg.c and Ftm_Gpt_Ip_VS_0_PBcfg.h are different"
#endif
/* Check if this header file and header file are of the same Software version */
#if ((FTM_GPT_IP_SW_MAJOR_VERSION_VS_0_PBCFG_C != FTM_GPT_IP_SW_MAJOR_VERSION_VS_0_PBCFG_H) || \
     (FTM_GPT_IP_SW_MINOR_VERSION_VS_0_PBCFG_C != FTM_GPT_IP_SW_MINOR_VERSION_VS_0_PBCFG_H) || \
     (FTM_GPT_IP_SW_PATCH_VERSION_VS_0_PBCFG_C != FTM_GPT_IP_SW_PATCH_VERSION_VS_0_PBCFG_H) \
    )
    #error "Software Version Numbers of Ftm_Gpt_Ip_VS_0_PBcfg.c and Ftm_Gpt_Ip_VS_0_PBcfg.h are different"
#endif
#if (FTM_GPT_IP_VENDOR_ID_VS_0_PBCFG_C != GPT_IRQ_VENDOR_ID)
    #error "Ftm_Gpt_Ip_VS_0_PBcfg.c and Gpt_Irq.h have different vendor ids"
#endif
/* Check if this header file and header file are of the same Autosar version */
#if ((FTM_GPT_IP_AR_RELEASE_MAJOR_VERSION_VS_0_PBCFG_C != GPT_IRQ_AR_RELEASE_MAJOR_VERSION) || \
     (FTM_GPT_IP_AR_RELEASE_MINOR_VERSION_VS_0_PBCFG_C != GPT_IRQ_AR_RELEASE_MINOR_VERSION) || \
     (FTM_GPT_IP_AR_RELEASE_REVISION_VERSION_VS_0_PBCFG_C != GPT_IRQ_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of Ftm_Gpt_Ip_VS_0_PBcfg.c and Gpt_Irq.h are different"
#endif
/* Check if this header file and header file are of the same Software version */
#if ((FTM_GPT_IP_SW_MAJOR_VERSION_VS_0_PBCFG_C != GPT_IRQ_SW_MAJOR_VERSION) || \
     (FTM_GPT_IP_SW_MINOR_VERSION_VS_0_PBCFG_C != GPT_IRQ_SW_MINOR_VERSION) || \
     (FTM_GPT_IP_SW_PATCH_VERSION_VS_0_PBCFG_C != GPT_IRQ_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of Ftm_Gpt_Ip_VS_0_PBcfg.c and Gpt_Irq.h are different"
#endif
/*================================================================================================*/

/*==================================================================================================
 *                                       GLOBAL VARIABLES
 *================================================================================================*/

/*==================================================================================================
 *                                       GLOBAL CONSTANTS
 *================================================================================================*/

/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
 *================================================================================================*/
#define GPT_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Gpt_MemMap.h"


const Ftm_Gpt_Ip_InstanceConfigType FTM_1_InitConfig_PB_VS_0 =
{
    /** @brief FTM Freeze Enable */
    (boolean)FALSE,
    /** @brief FTM Clock Source */
    FTM_GPT_IP_CLOCK_SOURCE_EXTERNALCLK,
#if (FTM_GPT_IP_SET_CLOCK_MODE == STD_ON)
    /** @brief FTM Alternate Prescaler Value */
    0U,
#endif
    /** @brief FTM Prescaler Value */
    0U,
    /** @brief FTM Counting Mode */
    FTM_GPT_IP_MODE_UP_TIMER
};


const Ftm_Gpt_Ip_ChannelConfigType FTM_1_ChannelConfig_PB_VS_0[1U] =
{
    {
        /** @brief Ftm Channel Id */
        0U,
        /** @brief Ftm callback name */
        &Gpt_ProcessCommonInterrupt,
        /** @brief Ftm callbackparam */
        (uint8)1U,
        /** @brief Ftm channel mode  */
        FTM_GPT_IP_CH_MODE_ONESHOT,
    }
};
#define GPT_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Gpt_MemMap.h"

/*==================================================================================================
 *                                       LOCAL VARIABLES
 *================================================================================================*/

/*==================================================================================================
 *                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
 *================================================================================================*/

/*==================================================================================================
 *                                       LOCAL MACROS
 *================================================================================================*/

/*==================================================================================================
 *                                      LOCAL CONSTANTS
 *================================================================================================*/

/*==================================================================================================
 *                                       LOCAL FUNCTIONS
 *================================================================================================*/

/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
 *================================================================================================*/

#ifdef __cplusplus
}
#endif  /*FTM_IP_PBCFG_C*/
/** @} */
