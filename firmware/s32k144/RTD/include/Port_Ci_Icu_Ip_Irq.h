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

#ifndef PORT_CI_ICU_IP_IRQ_H
#define PORT_CI_ICU_IP_IRQ_H

/**
 *     @file
 *
 *     @addtogroup port_ci_icu_ip PORT_CI IPL
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
#include "OsIf.h"
#include "Port_Ci_Icu_Ip_Cfg.h"
#include "Port_Ci_Icu_Ip_Types.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define PORT_CI_ICU_IP_IRQ_VENDOR_ID                    43
#define PORT_CI_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION     4
#define PORT_CI_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION     7
#define PORT_CI_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION  0
#define PORT_CI_ICU_IP_IRQ_SW_MAJOR_VERSION             2
#define PORT_CI_ICU_IP_IRQ_SW_MINOR_VERSION             0
#define PORT_CI_ICU_IP_IRQ_SW_PATCH_VERSION             0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if this header file and OsIf.h file are of the same Autosar version */
    #if ((PORT_CI_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION != OSIF_AR_RELEASE_MAJOR_VERSION) || \
         (PORT_CI_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION != OSIF_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Port_Ci_Icu_Ip_Irq.h and OsIf.h are different"
    #endif
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (PORT_CI_ICU_IP_IRQ_VENDOR_ID != PORT_CI_ICU_IP_TYPES_VENDOR_ID)
    #error "Port_Ci_Icu_Ip_Irq.h and Port_Ci_Icu_Ip_Types.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((PORT_CI_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION  != PORT_CI_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION  != PORT_CI_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION   != PORT_CI_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Port_Ci_Icu_Ip_Irq.h and Port_Ci_Icu_Ip_Types.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((PORT_CI_ICU_IP_IRQ_SW_MAJOR_VERSION  != PORT_CI_ICU_IP_TYPES_SW_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_IRQ_SW_MINOR_VERSION  != PORT_CI_ICU_IP_TYPES_SW_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_IRQ_SW_PATCH_VERSION  != PORT_CI_ICU_IP_TYPES_SW_PATCH_VERSION))
    #error "Software Version Numbers of Port_Ci_Icu_Ip_Irq.h and Port_Ci_Icu_Ip_Types.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (PORT_CI_ICU_IP_IRQ_VENDOR_ID != PORT_CI_ICU_IP_CFG_VENDOR_ID)
    #error "Port_Ci_Icu_Ip_Irq.h and Port_Ci_Icu_Ip_Cfg.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((PORT_CI_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION  != PORT_CI_ICU_IP_CFG_AR_RELEASE_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION  != PORT_CI_ICU_IP_CFG_AR_RELEASE_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION   != PORT_CI_ICU_IP_CFG_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Port_Ci_Icu_Ip_Irq.h and Port_Ci_Icu_Ip_Cfg.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((PORT_CI_ICU_IP_IRQ_SW_MAJOR_VERSION  != PORT_CI_ICU_IP_CFG_SW_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_IRQ_SW_MINOR_VERSION  != PORT_CI_ICU_IP_CFG_SW_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_IRQ_SW_PATCH_VERSION  != PORT_CI_ICU_IP_CFG_SW_PATCH_VERSION))
    #error "Software Version Numbers of Port_Ci_Icu_Ip_Irq.h and Port_Ci_Icu_Ip_Cfg.h are different"
#endif

/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

/*==================================================================================================
*                                              ENUMS
==================================================================================================*/

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

#define ICU_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"
extern Port_Ci_Icu_Ip_State Port_Ci_Icu_Ip_u32ChState[PORT_INSTANCE_COUNT][PORT_PCR_COUNT];
#define ICU_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"
/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"

#if (defined PORT_CI_ICU_IP_SINGLE_INTERRUPT)
/**
* @brief     Interrupt handler for Port
* @details   Process the interrupt of Port of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port is configured
*/
ISR(PORT_CI_ICU_IP_EXT_IRQ_SINGLE_INTERRUPT);
#else
#if (defined PORT_CI_ICU_IP_PORT_0_ISR_USED)

/**
* @brief     Interrupt handler for Port A
* @details   Process the interrupt of Port A of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port A is configured
*/
ISR(PORT_CI_ICU_IP_A_EXT_IRQ_ISR);
#endif  /* IRQ PORTA */

#if (defined PORT_CI_ICU_IP_PORT_1_ISR_USED)
/**
* @brief     Interrupt handler for Port B
* @details   Process the interrupt of Port B of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port B is configured
*/
ISR(PORT_CI_ICU_IP_B_EXT_IRQ_ISR);
#endif  /* IRQ PORTB */


#if (defined PORT_CI_ICU_IP_PORT_2_ISR_USED)
/**
* @brief     Interrupt handler for Port C
* @details   Process the interrupt of Port C of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port C is configured
*/
ISR(PORT_CI_ICU_IP_C_EXT_IRQ_ISR);
#endif    /* IRQ PORTC */

#if (defined PORT_CI_ICU_IP_PORT_3_ISR_USED)
/**
* @brief     Interrupt handler for Port D
* @details   Process the interrupt of Port D of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port D is configured
*/
ISR(PORT_CI_ICU_IP_D_EXT_IRQ_ISR);
#endif    /* IRQ PORTD */


#if (defined PORT_CI_ICU_IP_PORT_4_ISR_USED)
/**
* @brief     Interrupt handler for Port E
* @details   Process the interrupt of Port E of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port E is configured
*/
ISR(PORT_CI_ICU_IP_E_EXT_IRQ_ISR);
#endif    /* IRQ PORTE */

#if (defined PORT_CI_ICU_IP_PORT_5_ISR_USED)
/**
* @brief     Interrupt handler for Port F
* @details   Process the interrupt of Port F of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port F is configured
*/
ISR(PORT_CI_ICU_IP_F_EXT_IRQ_ISR);
#endif  /* IRQ PORTF */

#if (defined PORT_CI_ICU_IP_PORT_6_ISR_USED)
/**
* @brief     Interrupt handler for Port G
* @details   Process the interrupt of Port G of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port G is configured
*/
ISR(ICU_PORT_CI_G_EXT_IRQ_ISR);
#endif  /* IRQ PORTG */

#if (defined PORT_CI_ICU_IP_PORT_7_ISR_USED)
/**
* @brief     Interrupt handler for Port H
* @details   Process the interrupt of Port H of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port H is configured
*/
ISR(ICU_PORT_CI_H_EXT_IRQ_ISR);
#endif  /* IRQ PORTH */

#if (defined PORT_CI_ICU_IP_PORT_9_ISR_USED)

/**
* @brief     Interrupt handler for Port J
* @details   Process the interrupt of Port J of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port J is configured
*/
ISR(ICU_PORT_CI_J_EXT_IRQ_ISR);
#endif  /* IRQ PORTJ */

#if (defined PORT_CI_ICU_IP_PORT_10_ISR_USED)

/**
* @brief     Interrupt handler for Port K
* @details   Process the interrupt of Port K of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port K is configured
*/
ISR(PORT_CI_ICU_IP_K_EXT_IRQ_ISR);
#endif  /* IRQ PORTK */

#if (defined PORT_CI_ICU_IP_PORT_11_ISR_USED)
/**
* @brief     Interrupt handler for Port L
* @details   Process the interrupt of Port L of Port_Ci IP
* 
* @note   This will be defined only if any channel on Port L is configured
*/
ISR(ICU_PORT_CI_L_EXT_IRQ_ISR);
#endif  /* IRQ PORTL */
#endif /* PORT_CI_ICU_IP_SINGLE_INTERRUPT */

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#ifdef __cplusplus
}
#endif
/** @} */

#endif  /* PORT_CI_ICU_IP_IRQ_H */
