/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           : 
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 2.0.0
*   Build Version        : S32K1_RTD_2_0_0_D2308_ASR_REL_4_7_REV_0000_20230804
*
*   (c) Copyright 2020-2023 NXP Semiconductors
*   All Rights Reserved.
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "IntCtrl_Ip_Cfg.h"
/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define CDD_PLATFORM_INTCTRL_IP_CFG_VENDOR_ID_C                          43
#define CDD_PLATFORM_INTCTRL_IP_CFG_SW_MAJOR_VERSION_C                   2
#define CDD_PLATFORM_INTCTRL_IP_CFG_SW_MINOR_VERSION_C                   0
#define CDD_PLATFORM_INTCTRL_IP_CFG_SW_PATCH_VERSION_C                   0
#define CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_MAJOR_VERSION_C           4
#define CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_MINOR_VERSION_C           7
#define CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_REVISION_VERSION_C        0

/*==================================================================================================
                                      FILE VERSION CHECKS
==================================================================================================*/
/* Check if current file and IntCtrl_Ip_Cfg header file are of the same vendor */
#if (CDD_PLATFORM_INTCTRL_IP_CFG_VENDOR_ID_C != CDD_PLATFORM_INTCTRL_IP_CFG_VENDOR_ID)
    #error "IntCtrl_Ip_Cfg.c and IntCtrl_Ip_Cfg.h have different vendor ids"
#endif
/* Check if current file and IntCtrl_Ip_Cfg header file are of the same Autosar version */
#if ((CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_MAJOR_VERSION_C    != CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_MAJOR_VERSION) || \
     (CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_MINOR_VERSION_C    != CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_MINOR_VERSION) || \
     (CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_REVISION_VERSION_C != CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of IntCtrl_Ip_Cfg.C and IntCtrl_Ip_Cfg.h are different"
#endif
/* Check if current file and IntCtrl_Ip_Cfg header file are of the same Software version */
#if ((CDD_PLATFORM_INTCTRL_IP_CFG_SW_MAJOR_VERSION_C != CDD_PLATFORM_INTCTRL_IP_CFG_SW_MAJOR_VERSION) || \
     (CDD_PLATFORM_INTCTRL_IP_CFG_SW_MINOR_VERSION_C != CDD_PLATFORM_INTCTRL_IP_CFG_SW_MINOR_VERSION) || \
     (CDD_PLATFORM_INTCTRL_IP_CFG_SW_PATCH_VERSION_C != CDD_PLATFORM_INTCTRL_IP_CFG_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of IntCtrl_Ip_Cfg.c and IntCtrl_Ip_Cfg.h are different"
#endif
/*==================================================================================================
                                       GLOBAL VARIABLES
==================================================================================================*/
#define PLATFORM_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Platform_MemMap.h"

/* List of configuration for interrupts  */
static const IntCtrl_Ip_IrqConfigType aIrqConfiguration[] = {
    {FTM3_Ch6_Ch7_IRQn, (boolean)TRUE, 0U, FTM_3_CH_6_CH_7_ISR},
    {FTM2_Ch0_Ch1_IRQn, (boolean)TRUE, 0U, FTM_2_CH_0_CH_1_ISR},
    {FLEXIO_IRQn, (boolean)FALSE, 0U, MCL_FLEXIO_ISR},
    {ADC0_IRQn, (boolean)TRUE, 0U, Adc_0_Isr},
    {PORTE_IRQn, (boolean)TRUE, 0U, PORT_CI_ICU_IP_E_EXT_IRQ_ISR},
    {LPTMR0_IRQn, (boolean)TRUE, 0U, LPTMR_0_CH_0_ISR},
    {FTM3_Ovf_Reload_IRQn, (boolean)TRUE, 0U, FTM_3_OVF_RELOAD_ISR},
    {FTM0_Ovf_Reload_IRQn, (boolean)FALSE, 0U, FTM_0_OVF_RELOAD_ISR},
    {FTM1_Ch0_Ch1_IRQn, (boolean)TRUE, 0U, FTM_1_CH_0_CH_1_ISR},
    {FTM2_Ovf_Reload_IRQn, (boolean)TRUE, 0U, FTM_2_OVF_RELOAD_ISR},
    {LPI2C0_Master_IRQn, (boolean)FALSE, 0U, LPI2C0_Master_IRQHandler},
    {FTM1_Ch2_Ch3_IRQn, (boolean)TRUE, 0U, FTM_1_CH_2_CH_3_ISR},
};
const IntCtrl_Ip_CtrlConfigType intCtrlConfig = {
    12U,
    aIrqConfiguration
};

#define PLATFORM_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Platform_MemMap.h"

#ifdef __cplusplus
}
#endif

