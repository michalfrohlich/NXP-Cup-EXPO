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
#include "SchM_Icu.h"
#include "Std_Types.h"
#include "Port_Ci_Icu_Ip.h"
#include "Port_Ci_Icu_Ip_Irq.h"
#include "Mcal.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define PORT_CI_ICU_IRQ_VENDOR_ID_C                      43
#define PORT_CI_ICU_IRQ_AR_RELEASE_MAJOR_VERSION_C       4
#define PORT_CI_ICU_IRQ_AR_RELEASE_MINOR_VERSION_C       7
#define PORT_CI_ICU_IRQ_AR_RELEASE_REVISION_VERSION_C    0
#define PORT_CI_ICU_IRQ_SW_MAJOR_VERSION_C               2
#define PORT_CI_ICU_IRQ_SW_MINOR_VERSION_C               0
#define PORT_CI_ICU_IRQ_SW_PATCH_VERSION_C               0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if header file and Std_Types.h file are of the same Autosar version */
    #if ((PORT_CI_ICU_IRQ_AR_RELEASE_MAJOR_VERSION_C != STD_AR_RELEASE_MAJOR_VERSION) || \
         (PORT_CI_ICU_IRQ_AR_RELEASE_MINOR_VERSION_C != STD_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Port_Ci_Icu_Ip_Irq.c and Std_Types.h are different"
    #endif

    /* Check if header file and SchM_Icu.h file are of the same Autosar version */
    #if ((PORT_CI_ICU_IRQ_AR_RELEASE_MAJOR_VERSION_C != SCHM_ICU_AR_RELEASE_MAJOR_VERSION) || \
         (PORT_CI_ICU_IRQ_AR_RELEASE_MINOR_VERSION_C != SCHM_ICU_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Port_Ci_Icu_Ip_Irq.c and SchM_Icu.h are different"
    #endif
    
    /* Check if this header file and Mcal.h file are of the same Autosar version */
    #if ((PORT_CI_ICU_IRQ_AR_RELEASE_MAJOR_VERSION_C != MCAL_AR_RELEASE_MAJOR_VERSION) || \
        (PORT_CI_ICU_IRQ_AR_RELEASE_MINOR_VERSION_C != MCAL_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Port_Ci_Icu_Ip_Irq.c and Mcal.h are different"
    #endif
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (PORT_CI_ICU_IRQ_VENDOR_ID_C != PORT_CI_ICU_IP_VENDOR_ID)
    #error "Port_Ci_Icu_Ip_Irq.c and Port_Ci_Icu_Ip.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((PORT_CI_ICU_IRQ_AR_RELEASE_MAJOR_VERSION_C  != PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (PORT_CI_ICU_IRQ_AR_RELEASE_MINOR_VERSION_C  != PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (PORT_CI_ICU_IRQ_AR_RELEASE_REVISION_VERSION_C   != PORT_CI_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Port_Ci_Icu_Ip_Irq.c and Port_Ci_Icu_Ip.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((PORT_CI_ICU_IRQ_SW_MAJOR_VERSION_C  != PORT_CI_ICU_IP_SW_MAJOR_VERSION) || \
     (PORT_CI_ICU_IRQ_SW_MINOR_VERSION_C  != PORT_CI_ICU_IP_SW_MINOR_VERSION) || \
     (PORT_CI_ICU_IRQ_SW_PATCH_VERSION_C  != PORT_CI_ICU_IP_SW_PATCH_VERSION))
    #error "Software Version Numbers of Port_Ci_Icu_Ip_Irq.c and Port_Ci_Icu_Ip.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (PORT_CI_ICU_IRQ_VENDOR_ID_C != PORT_CI_ICU_IP_IRQ_VENDOR_ID)
    #error "Port_Ci_Icu_Ip_Irq.c and Port_Ci_Icu_Ip_Irq.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((PORT_CI_ICU_IRQ_AR_RELEASE_MAJOR_VERSION_C  != PORT_CI_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION) || \
     (PORT_CI_ICU_IRQ_AR_RELEASE_MINOR_VERSION_C  != PORT_CI_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION) || \
     (PORT_CI_ICU_IRQ_AR_RELEASE_REVISION_VERSION_C   != PORT_CI_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Port_Ci_Icu_Ip_Irq.c and Port_Ci_Icu_Ip_Irq.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((PORT_CI_ICU_IRQ_SW_MAJOR_VERSION_C  != PORT_CI_ICU_IP_IRQ_SW_MAJOR_VERSION) || \
     (PORT_CI_ICU_IRQ_SW_MINOR_VERSION_C  != PORT_CI_ICU_IP_IRQ_SW_MINOR_VERSION) || \
     (PORT_CI_ICU_IRQ_SW_PATCH_VERSION_C  != PORT_CI_ICU_IP_IRQ_SW_PATCH_VERSION))
    #error "Software Version Numbers of Port_Ci_Icu_Ip_Irq.c and Port_Ci_Icu_Ip_Irq.h are different"
#endif

/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                    LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"

#if (defined PORT_CI_ICU_IP_SINGLE_INTERRUPT)

/**
* @brief      Port_Ci_Icu_Ip_ProcessSingleInterrupt
* @details
*
* @return void
* */
static inline void Port_Ci_Icu_Ip_ProcessSingleInterrupt(void);
#endif

#if ((defined PORT_CI_ICU_IP_PORT_0_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_1_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_2_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_3_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_4_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_5_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_6_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_7_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_9_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_10_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_11_ISR_USED))

/**
* @brief      Port_Ci_Icu_Ip_ProcessInterrupt
* @details
* 
* @param[in]      portId - first IRQ HW Channel from the HW group
* 
* @return void
*
*
* */
static inline void Port_Ci_Icu_Ip_ProcessInterrupt(uint8 portId);
#endif

#if ((defined PORT_CI_ICU_IP_PORT_0_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_1_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_2_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_3_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_4_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_5_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_6_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_7_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_9_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_10_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_11_ISR_USED)|| (defined PORT_CI_ICU_IP_SINGLE_INTERRUPT))
static inline void Port_Ci_Icu_Ip_ReportEvents(uint8 portId, uint8 pinId);
#endif
#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

/*==================================================================================================
*                                        LOCAL FUNCTIONS
==================================================================================================*/
/** @brief Include Memory mapping specification. **/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"

#if (defined PORT_CI_ICU_IP_SINGLE_INTERRUPT)

/**
* @brief      Port_Ci_Icu_Ip_ProcessSingleInterrupt
* @details
*
* @return void
* @implements Port_Ci_Icu_Ip_ProcessSingleInterrupt_Activity
* */
static inline void Port_Ci_Icu_Ip_ProcessSingleInterrupt(void)
{
    uint8 portId = 0U;
    uint8 pinId;
    uint32 u32Port_CiPCR;
    uint32 pinIdMask;
    uint32 u32RegPort_CiISFR;

    for (portId = 0; portId < ICU_NUM_PORT_CI_HW_MODULE_U8; portId++)
    {
        u32RegPort_CiISFR = s_Port_Ci_Base[portId]->ISFR;
        pinIdMask = (uint32)1U;
        
        for (pinId=0U; pinId<32U; pinId++)
        {
            if (0x0U != (u32RegPort_CiISFR & pinIdMask))
            {
                if (FALSE == Port_Ci_Icu_Ip_u32ChState[portId][pinId].chInit)
                {
                    /* Clear pending interrupt serviced */
                    s_Port_Ci_Base[portId]->PCR[pinId] |= ((uint32)0x1U << PORT_PCR_ISF_SHIFT);
                }
                else
                {
                    u32Port_CiPCR = s_Port_Ci_Base[portId]->PCR[pinId] & PORT_PCR_IRQC_MASK;
                    if ((ICU_PORT_CI_PCR_IREE_U32 == u32Port_CiPCR) || (ICU_PORT_CI_PCR_IFEE_U32 == u32Port_CiPCR) \
                            || (ICU_PORT_CI_PCR_IEEE_U32 == u32Port_CiPCR))
                    {
                        /* Clear pending interrupt serviced */
                        s_Port_Ci_Base[portId]->PCR[pinId] |= ((uint32)0x1U << PORT_PCR_ISF_SHIFT);
                        Port_Ci_Icu_Ip_ReportEvents(portId, pinId);
                    }
                    
                }
            }
            pinIdMask <<= (uint32)1U;
        }
    }
}
#endif /* PORT_CI_ICU_IP_SINGLE_INTERRUPT */

#if ((defined PORT_CI_ICU_IP_PORT_0_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_1_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_2_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_3_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_4_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_5_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_6_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_7_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_9_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_10_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_11_ISR_USED))

/**
* @brief      Port_Ci_Icu_Ip_ProcessInterrupt
* @details
* 
* @param[in]      portId - first IRQ HW Channel from the HW group
* 
* @return void
*
*
* */
static inline void Port_Ci_Icu_Ip_ProcessInterrupt(uint8 portId)
{
    uint8 pinId;
    uint32 u32Port_CiPCR;
    uint32 pinIdMask = (uint32)1U;
    uint32 u32RegPort_CiISFR = s_Port_Ci_Base[portId]->ISFR;

    for (pinId=0U; pinId<32U; pinId++)
    {
        if (0x0U != (pinIdMask & u32RegPort_CiISFR))
        {
            if (FALSE == Port_Ci_Icu_Ip_u32ChState[portId][pinId].chInit)
            {
                /* Clear pending interrupt serviced */
                s_Port_Ci_Base[portId]->PCR[pinId] |= ((uint32)0x1U << PORT_PCR_ISF_SHIFT);
            }
            else
            {
                /* Get IRQ's activated with the enabled flag set */
                u32Port_CiPCR = (s_Port_Ci_Base[portId]->PCR[pinId] & PORT_PCR_IRQC_MASK);
                if ((ICU_PORT_CI_PCR_IREE_U32 == u32Port_CiPCR) || (ICU_PORT_CI_PCR_IFEE_U32 == u32Port_CiPCR) \
                    || (ICU_PORT_CI_PCR_IEEE_U32 == u32Port_CiPCR))
                {
                    /* Clear pending interrupt serviced */
                    s_Port_Ci_Base[portId]->PCR[pinId] |= ((uint32)0x1U << PORT_PCR_ISF_SHIFT);
                    Port_Ci_Icu_Ip_ReportEvents(portId, pinId);
                }
            }
        }
        pinIdMask <<= (uint32)1U;
    }
}
#endif

/*==================================================================================================
*                                        GLOBAL FUNCTIONS
==================================================================================================*/

#if (defined PORT_CI_ICU_IP_SINGLE_INTERRUPT)
/**
* @brief     Interrupt handler for Port
* @details   Process the interrupt of Port of Port_Ci IP
* 
* @isr
* @note   This will be defined only if any channel on Port is configured
*/
ISR(PORT_CI_ICU_IP_EXT_IRQ_SINGLE_INTERRUPT)
{
    /*calling process Single interrupt function*/
    Port_Ci_Icu_Ip_ProcessSingleInterrupt();
    EXIT_INTERRUPT();
}
#endif /* PORT_CI_ICU_IP_SINGLE_INTERRUPT */
    
#if (defined PORT_CI_ICU_IP_PORT_0_ISR_USED)

/**
* @brief     Interrupt handler for Port A
* @details   Process the interrupt of Port A of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port A is configured
*/
ISR(PORT_CI_ICU_IP_A_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port A*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)0U);
    EXIT_INTERRUPT();
}
#endif  /* IRQ PORTA */

#if (defined PORT_CI_ICU_IP_PORT_1_ISR_USED)

/**
* @brief     Interrupt handler for Port B
* @details   Process the interrupt of Port B of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port B is configured
*/
ISR(PORT_CI_ICU_IP_B_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port B*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)1U);
    EXIT_INTERRUPT();
}
#endif  /* IRQ PORTB */


#if (defined PORT_CI_ICU_IP_PORT_2_ISR_USED)

/**
* @brief     Interrupt handler for Port C
* @details   Process the interrupt of Port C of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port C is configured
*/
ISR(PORT_CI_ICU_IP_C_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port C*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)2U);
    EXIT_INTERRUPT();
}
#endif    /* IRQ PORTC */


#if (defined PORT_CI_ICU_IP_PORT_3_ISR_USED)

/**
* @brief     Interrupt handler for Port D
* @details   Process the interrupt of Port D of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port D is configured
*/
ISR(PORT_CI_ICU_IP_D_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port D*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)3U);
    EXIT_INTERRUPT();
}

#endif    /* IRQ PORTD */


#if (defined PORT_CI_ICU_IP_PORT_4_ISR_USED)

/**
* @brief     Interrupt handler for Port E
* @details   Process the interrupt of Port E of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port E is configured
*/
ISR(PORT_CI_ICU_IP_E_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port E*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)4U);
    EXIT_INTERRUPT();
}

#endif    /* IRQ PORTE */


#if (defined PORT_CI_ICU_IP_PORT_5_ISR_USED)

/**
* @brief     Interrupt handler for Port F
* @details   Process the interrupt of Port F of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port F is configured
*/
ISR(PORT_CI_ICU_IP_F_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port F*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)5U);
    EXIT_INTERRUPT();
}
#endif  /* IRQ PORTF */


#if (defined PORT_CI_ICU_IP_PORT_6_ISR_USED)

/**
* @brief     Interrupt handler for Port G
* @details   Process the interrupt of Port G of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port G is configured
*/
ISR(ICU_PORT_CI_G_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port G*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)6U);
    EXIT_INTERRUPT();
}
#endif  /* IRQ PORTG */


#if (defined PORT_CI_ICU_IP_PORT_7_ISR_USED)

/**
* @brief     Interrupt handler for Port H
* @details   Process the interrupt of Port H of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port H is configured
*/
ISR(ICU_PORT_CI_H_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port H*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)7U);
    EXIT_INTERRUPT();
}
#endif  /* IRQ PORTH */


#if (defined PORT_CI_ICU_IP_PORT_9_ISR_USED)

/**
* @brief     Interrupt handler for Port J
* @details   Process the interrupt of Port J of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port J is configured
*/
ISR(ICU_PORT_CI_J_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port J*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)9U);
    EXIT_INTERRUPT();
}
#endif  /* IRQ PORTJ */


#if (defined PORT_CI_ICU_IP_PORT_10_ISR_USED)

/**
* @brief     Interrupt handler for Port K
* @details   Process the interrupt of Port K of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port K is configured
*/
ISR(PORT_CI_ICU_IP_K_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port K*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)10U);
    EXIT_INTERRUPT();
}
#endif  /* IRQ PORTK */


#if (defined PORT_CI_ICU_IP_PORT_11_ISR_USED)
/**
* @brief     Interrupt handler for Port L
* @details   Process the interrupt of Port L of Port_Ci IP
* 
* @isr
* 
* *
* @note   This will be defined only if any channel on Port L is configured
*/
ISR(ICU_PORT_CI_L_EXT_IRQ_ISR)
{
    /*calling process interrupt function for Port L*/
    Port_Ci_Icu_Ip_ProcessInterrupt((uint8)11U);
    EXIT_INTERRUPT();
}
#endif  /* IRQ PORTL */

#if ((defined PORT_CI_ICU_IP_PORT_0_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_1_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_2_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_3_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_4_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_5_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_6_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_7_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_9_ISR_USED) || (defined PORT_CI_ICU_IP_PORT_10_ISR_USED) || \
     (defined PORT_CI_ICU_IP_PORT_11_ISR_USED)|| (defined PORT_CI_ICU_IP_SINGLE_INTERRUPT))
static inline void Port_Ci_Icu_Ip_ReportEvents(uint8 portId, uint8 pinId)
{
    if((TRUE == Port_Ci_Icu_Ip_u32ChState[portId][pinId].chInit) && \
       (NULL_PTR != Port_Ci_Icu_Ip_u32ChState[portId][pinId].callback))
    {
        Port_Ci_Icu_Ip_u32ChState[portId][pinId].callback(Port_Ci_Icu_Ip_u32ChState[portId][pinId].callbackParam, FALSE);
    }
    else
    {
        /* Calling Notification for the IPL channel */
        if ((TRUE == Port_Ci_Icu_Ip_u32ChState[portId][pinId].chInit) && \
            (NULL_PTR != Port_Ci_Icu_Ip_u32ChState[portId][pinId].PortCiChannelNotification) && \
            ((boolean)TRUE == Port_Ci_Icu_Ip_u32ChState[portId][pinId].notificationEnable))
        {
            Port_Ci_Icu_Ip_u32ChState[portId][pinId].PortCiChannelNotification();
        }
    }
}
#endif

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */
