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

#ifndef INTCTRL_IP_CFG_H_
#define INTCTRL_IP_CFG_H_

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "IntCtrl_Ip_TypesDef.h"


/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define CDD_PLATFORM_INTCTRL_IP_CFG_VENDOR_ID                          43
#define CDD_PLATFORM_INTCTRL_IP_CFG_SW_MAJOR_VERSION                   2
#define CDD_PLATFORM_INTCTRL_IP_CFG_SW_MINOR_VERSION                   0
#define CDD_PLATFORM_INTCTRL_IP_CFG_SW_PATCH_VERSION                   0
#define CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_MAJOR_VERSION           4
#define CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_MINOR_VERSION           7
#define CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_REVISION_VERSION        0
/*==================================================================================================
                                      FILE VERSION CHECKS
==================================================================================================*/
/* Check if current file and IntCtrl_Ip_TypesDef header file are of the same vendor */
#if (CDD_PLATFORM_INTCTRL_IP_CFG_VENDOR_ID != CDD_PLATFORM_INTCTRL_IP_TYPESDEF_VENDOR_ID)
    #error "IntCtrl_Ip_Cfg.h and IntCtrl_Ip_TypesDef.h have different vendor ids"
#endif
/* Check if current file and IntCtrl_Ip_TypesDef header file are of the same Autosar version */
#if ((CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_MAJOR_VERSION    != CDD_PLATFORM_INTCTRL_IP_TYPESDEF_AR_RELEASE_MAJOR_VERSION) || \
     (CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_MINOR_VERSION    != CDD_PLATFORM_INTCTRL_IP_TYPESDEF_AR_RELEASE_MINOR_VERSION) || \
     (CDD_PLATFORM_INTCTRL_IP_CFG_AR_RELEASE_REVISION_VERSION != CDD_PLATFORM_INTCTRL_IP_TYPESDEF_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of IntCtrl_Ip_Cfg.H and IntCtrl_Ip_TypesDef.h are different"
#endif
/* Check if current file and Fls header file are of the same Software version */
#if ((CDD_PLATFORM_INTCTRL_IP_CFG_SW_MAJOR_VERSION != CDD_PLATFORM_INTCTRL_IP_TYPESDEF_SW_MAJOR_VERSION) || \
     (CDD_PLATFORM_INTCTRL_IP_CFG_SW_MINOR_VERSION != CDD_PLATFORM_INTCTRL_IP_TYPESDEF_SW_MINOR_VERSION) || \
     (CDD_PLATFORM_INTCTRL_IP_CFG_SW_PATCH_VERSION != CDD_PLATFORM_INTCTRL_IP_TYPESDEF_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of IntCtrl_Ip_Cfg.h and IntCtrl_Ip_TypesDef.h are different"
#endif
/*==================================================================================================
                                       GLOBAL VARIABLES
==================================================================================================*/
#define PLATFORM_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Platform_MemMap.h"

/* Declaration of the configuration structure for Interrupt Controller (core related) */
extern const IntCtrl_Ip_CtrlConfigType intCtrlConfig;
/* Declaration of the ISR Handlers */
extern void FTM_3_CH_6_CH_7_ISR(void);
extern void FTM_2_CH_0_CH_1_ISR(void);
extern void MCL_FLEXIO_ISR(void);
extern void Adc_0_Isr(void);
extern void PORT_CI_ICU_IP_E_EXT_IRQ_ISR(void);
extern void LPTMR_0_CH_0_ISR(void);
extern void FTM_3_OVF_RELOAD_ISR(void);
extern void FTM_0_OVF_RELOAD_ISR(void);
extern void FTM_1_CH_0_CH_1_ISR(void);
extern void FTM_2_OVF_RELOAD_ISR(void);
extern void LPI2C0_Master_IRQHandler(void);
extern void FTM_1_CH_2_CH_3_ISR(void);

#define PLATFORM_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Platform_MemMap.h"

#endif /* INTCTRL_IP_CFG_H_ */

