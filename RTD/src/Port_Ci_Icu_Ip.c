
/*==================================================================================================
s*   Project              : RTD AUTOSAR 4.7
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
*                                        INCLUDE FILES
==================================================================================================*/
#include "Std_Types.h"
#include "Port_Ci_Icu_Ip.h"
#include "OsIf.h"

#if(PORT_CI_ICU_IP_DEV_ERROR_DETECT == STD_ON)
#include "Devassert.h"
#endif

#if (STD_ON == PORT_CI_ICU_IP_ENABLE_USER_MODE_SUPPORT)
    #define USER_MODE_REG_PROT_ENABLED  (PORT_CI_ICU_IP_ENABLE_USER_MODE_SUPPORT)
    #include "RegLockMacros.h"
#endif

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define PORT_CI_ICU_IP_VENDOR_ID_C                   43
#define PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION_C    4
#define PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION_C    7
#define PORT_CI_ICU_IP_AR_RELEASE_REVISION_VERSION_C 0
#define PORT_CI_ICU_IP_SW_MAJOR_VERSION_C            2
#define PORT_CI_ICU_IP_SW_MINOR_VERSION_C            0
#define PORT_CI_ICU_IP_SW_PATCH_VERSION_C            0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if header file and Std_Types.h file are of the same Autosar version */
    #if ((PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != STD_AR_RELEASE_MAJOR_VERSION) || \
         (PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION_C != STD_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Port_Ci_Icu_Ip.c and Std_Types.h are different"
    #endif

    #if (STD_ON == PORT_CI_ICU_IP_ENABLE_USER_MODE_SUPPORT)
        /* Check if header file and RegLockMacros.h file are of the same Autosar version */
        #if ((PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != REGLOCKMACROS_AR_RELEASE_MAJOR_VERSION) || \
             (PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION_C != REGLOCKMACROS_AR_RELEASE_MINOR_VERSION))
            #error "AutoSar Version Numbers of Port_Ci_Icu_Ip.c and RegLockMacros.h are different"
        #endif
    #endif

    /* Check if this header file and OsIf.h file are of the same Autosar version */
    #if ((PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != OSIF_AR_RELEASE_MAJOR_VERSION) || \
         (PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION_C != OSIF_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Port_Ci_Icu_Ip.c and OsIf.h are different"
    #endif
    #if(PORT_CI_ICU_IP_DEV_ERROR_DETECT == STD_ON)
        /* Check if this header file and Devassert.h file are of the same Autosar version */
        #if ((PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != DEVASSERT_AR_RELEASE_MAJOR_VERSION) || \
             (PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION_C != DEVASSERT_AR_RELEASE_MINOR_VERSION))
            #error "AutoSar Version Numbers of Port_Ci_Icu_Ip.c and Devassert.h are different"
        #endif
    #endif
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (PORT_CI_ICU_IP_VENDOR_ID_C != PORT_CI_ICU_IP_VENDOR_ID)
    #error "Port_Ci_Icu_Ip.c and Port_Ci_Icu_Ip.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION_C  != PORT_CI_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION_C  != PORT_CI_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_AR_RELEASE_REVISION_VERSION_C   != PORT_CI_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Port_Ci_Icu_Ip.c and Port_Ci_Icu_Ip.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((PORT_CI_ICU_IP_SW_MAJOR_VERSION_C != PORT_CI_ICU_IP_SW_MAJOR_VERSION) || \
     (PORT_CI_ICU_IP_SW_MINOR_VERSION_C != PORT_CI_ICU_IP_SW_MINOR_VERSION) || \
     (PORT_CI_ICU_IP_SW_PATCH_VERSION_C != PORT_CI_ICU_IP_SW_PATCH_VERSION))
    #error "Software Version Numbers of Port_Ci_Icu_Ip.c and Port_Ci_Icu_Ip.h are different"
#endif

/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                       GLOBAL CONSTANTS
==================================================================================================*/
#define ICU_START_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"
/* Table of base addresses for SUIL2 instances. */
PORT_Type * const s_Port_Ci_Base[] = IP_PORT_BASE_PTRS;
#define ICU_STOP_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"

/*==================================================================================================
*                                  LOCAL VARIABLE DECLARATIONS
==================================================================================================*/
#define ICU_START_SEC_VAR_CLEARED_8
#include "Icu_MemMap.h"
#if ((PORT_CI_ICU_IP_SET_MODE_API == STD_ON) || (PORT_CI_ICU_IP_EDGE_DETECT_API == STD_ON))
static uint8 Port_Ci_Icu_Ip_aChActivationMode[PORT_PCR_COUNT];
#endif
#define ICU_STOP_SEC_VAR_CLEARED_8
#include "Icu_MemMap.h"

#define ICU_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"
Port_Ci_Icu_Ip_State Port_Ci_Icu_Ip_u32ChState[PORT_INSTANCE_COUNT][PORT_PCR_COUNT];
#define ICU_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"
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
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL FUNCTIONS
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"
#if ((PORT_CI_ICU_IP_SET_MODE_API == STD_ON) ||(PORT_CI_ICU_IP_EDGE_DETECT_API == STD_ON))
/** @implements  Port_Ci_Icu_Ip_EnableInterrupt_Activity */
void Port_Ci_Icu_Ip_EnableInterrupt(uint8 instance, uint8 hwChannel)
{
    uint32 u32pinMask = (uint32)((uint32)1U << (uint32)hwChannel);

     /* Enable interrupt and set mode*/
    if (Port_Ci_Icu_Ip_aChActivationMode[hwChannel] == ICU_PORT_CI_CH_FALLING_EDGE_U8)
    {
        s_Port_Ci_Base[instance]->PCR[hwChannel] &= ~ICU_PORT_CI_PCR_IREE_U32;
        s_Port_Ci_Base[instance]->PCR[hwChannel] |= ICU_PORT_CI_PCR_IFEE_U32;
    }
    else if (Port_Ci_Icu_Ip_aChActivationMode[hwChannel] == ICU_PORT_CI_CH_RISING_EDGE_U8)
    {
        s_Port_Ci_Base[instance]->PCR[hwChannel] &= ~ICU_PORT_CI_PCR_IFEE_U32;
        s_Port_Ci_Base[instance]->PCR[hwChannel] |= ICU_PORT_CI_PCR_IREE_U32;
    }
    else
    {
        s_Port_Ci_Base[instance]->PCR[hwChannel] |= (ICU_PORT_CI_PCR_IREE_U32 | ICU_PORT_CI_PCR_IFEE_U32);
    }

    /* Clear pending flag */
    s_Port_Ci_Base[instance]->ISFR |= u32pinMask;
}
#endif

/** @implements  Port_Ci_Icu_Ip_DisableInterrupt_Activity */
void Port_Ci_Icu_Ip_DisableInterrupt(uint8 instance, uint8 hwChannel)
{
    uint32 u32pinMask = (uint32)((uint32)1U << (uint32)hwChannel);

    /* Disable interrupt */
    s_Port_Ci_Base[instance]->PCR[hwChannel] &= ~PORT_PCR_IRQC_MASK;

    /* Clear pending flag */
    s_Port_Ci_Base[instance]->ISFR |= u32pinMask;
}

/**
* @brief      Driver function that initializes Port_Ci hardware channel.
* @details    This function:
*              - Disables interrupt.
*              - Sets Interrupt Filter Enable Register
*              - Sets Activation Condition
* 
* @param[in]  userConfig IRQ HW Channel
* 
* @return Port_Ci_Icu_Ip_StatusType
* 
* @implements  Port_Ci_Icu_Ip_Init_Activity
* */
Port_Ci_Icu_Ip_StatusType Port_Ci_Icu_Ip_Init (uint8 instance,  const Port_Ci_Icu_Ip_ConfigType * userConfig)
{
    /* logical channel */
    uint8                        nCounter;
    uint8                        hwChannel;

    for (nCounter=0U; nCounter < userConfig->nNumChannels; nCounter++)
    {
        /* only handle Port_Ci channels */
        hwChannel = (uint8)((*userConfig->pChannelsConfig)[nCounter].pinId);

        /* Disable IRQ Interrupt */
        Port_Ci_Icu_Ip_DisableInterrupt(instance, hwChannel);

        /* Set Activation Condition */
        Port_Ci_Icu_Ip_SetActivationCondition 
        (
            instance, hwChannel, 
            (Port_Ci_Icu_Ip_EdgeType)((*userConfig->pChannelsConfig)[nCounter].nDefaultStartEdge)
        );
        Port_Ci_Icu_Ip_u32ChState[instance][hwChannel].chInit = TRUE;
        Port_Ci_Icu_Ip_u32ChState[instance][hwChannel].callback = (*userConfig->pChannelsConfig)[nCounter].callback;
        Port_Ci_Icu_Ip_u32ChState[instance][hwChannel].callbackParam = (*userConfig->pChannelsConfig)[nCounter].callbackParam;
        Port_Ci_Icu_Ip_u32ChState[instance][hwChannel].PortCiChannelNotification = (*userConfig->pChannelsConfig)[nCounter].PortCiChannelNotification;
    }
    return PORT_CI_IP_STATUS_SUCCESS;
}

#if (STD_ON == PORT_CI_ICU_IP_DEINIT_API)
/**
* @brief      Driver function that de-initializes Icu Port.
* @details    This function:
*              - Disables interrupt.
*              - Clears edge event enable registers
*              - Clears Interrupt Filter Enable Register
* 
* @param[in]  pPort_CiIpConfig  - IRQ HW Channel
* 
* @return Port_Ci_Icu_Ip_StatusType
*
* @implements  Port_Ci_Icu_Ip_DeInit_Activity
* */
Port_Ci_Icu_Ip_StatusType Port_Ci_Icu_Ip_DeInit(uint8 instance)
{
    /* logical channel */
    uint8   hwChannel;
    uint32  u32pinMask;

    for (hwChannel=0U; hwChannel < PORT_PCR_COUNT; hwChannel++)
    {
        /* only handle Port_Ci channels */
        u32pinMask = (uint32)((uint32)1U << (uint32)hwChannel);

        /* Disable IRQ Interrupt */
        Port_Ci_Icu_Ip_DisableInterrupt(instance, hwChannel);

        /* Clear edge event enable registers */
        s_Port_Ci_Base[instance]->PCR[hwChannel] &= ~PORT_PCR_IRQC_MASK;
            
        /* Clear Interrupt Filter Enable Register */
        s_Port_Ci_Base[instance]->DFER &= ~u32pinMask;

        /* Record the deinit */
        Port_Ci_Icu_Ip_u32ChState[instance][hwChannel].chInit = FALSE;
        Port_Ci_Icu_Ip_u32ChState[instance][hwChannel].callback = NULL_PTR;
        Port_Ci_Icu_Ip_u32ChState[instance][hwChannel].callbackParam = 0U;
        Port_Ci_Icu_Ip_u32ChState[instance][hwChannel].PortCiChannelNotification = NULL_PTR;
#if ((PORT_CI_ICU_IP_SET_MODE_API == STD_ON) || (PORT_CI_ICU_IP_EDGE_DETECT_API == STD_ON))
        Port_Ci_Icu_Ip_aChActivationMode[hwChannel] = ICU_PORT_CI_CH_NEITHER_EDGE_U8;
#endif
    }
    return PORT_CI_IP_STATUS_SUCCESS;
}
#endif

#if (PORT_CI_ICU_IP_SET_MODE_API == STD_ON)
/**
* @brief      Driver function that sets Port_Ci hardware channel into SLEEP mode.
* @details    This function enables the interrupt if wakeup is enabled for corresponding 
*             Port_Ci channel
* 
* @param[in]  hwChannel       - IRQ HW Channel
* 
* @return void
* 
* */

void Port_Ci_Icu_Ip_SetSleepMode(uint8 instance, uint8 hwChannel)
{
    /* Disable IRQ Interrupt */
    Port_Ci_Icu_Ip_DisableInterrupt(instance, hwChannel);
}

/**
* @brief      Driver function that sets the Port_Ci hardware channel into NORMAL mode.
* @details    This function enables the interrupt if Notification is enabled for corresponding
*             Port_Ci channel
* 
* @param[in]  hwChannel - IRQ HW Channel
* 
* @return void
* 
* */

void Port_Ci_Icu_Ip_SetNormalMode(uint8 instance, uint8 hwChannel)
{
    /* Enable IRQ Interrupt */
    Port_Ci_Icu_Ip_EnableInterrupt(instance, hwChannel);
}
#endif  /* PORT_CI_ICU_IP_SET_MODE_API */

/**
* @brief      Driver function that sets activation condition of Port_Ci channel
* @details    This function enables the requested activation condition(rising, falling or both
*             edges) for corresponding Port_Ci channels.
* 
* @param[in]  hwChannel -  IRQ HW Channel
* @param[in]  activation - Activation condition
* 
* @return void
*
* @implements  Port_Ci_Icu_Ip_SetActivationCondition_Activity
* */
void Port_Ci_Icu_Ip_SetActivationCondition(uint8 instance, uint8 hwChannel, Port_Ci_Icu_Ip_EdgeType activation)
{
    switch (activation)
    {
        case PORT_CI_ICU_RISING_EDGE:
        {
            s_Port_Ci_Base[instance]->PCR[hwChannel] &= ~ICU_PORT_CI_PCR_IFEE_U32;
            s_Port_Ci_Base[instance]->PCR[hwChannel] |= ICU_PORT_CI_PCR_IREE_U32;
#if ((PORT_CI_ICU_IP_SET_MODE_API == STD_ON) || (PORT_CI_ICU_IP_EDGE_DETECT_API == STD_ON))
            Port_Ci_Icu_Ip_aChActivationMode[hwChannel] = ICU_PORT_CI_CH_RISING_EDGE_U8;
#endif
        }
        break;

        case PORT_CI_ICU_FALLING_EDGE:
        {
            s_Port_Ci_Base[instance]->PCR[hwChannel] &= ~ICU_PORT_CI_PCR_IREE_U32;
            s_Port_Ci_Base[instance]->PCR[hwChannel] |= ICU_PORT_CI_PCR_IFEE_U32;
#if ((PORT_CI_ICU_IP_SET_MODE_API == STD_ON) || (PORT_CI_ICU_IP_EDGE_DETECT_API == STD_ON))
            Port_Ci_Icu_Ip_aChActivationMode[hwChannel] = ICU_PORT_CI_CH_FALLING_EDGE_U8;
#endif            
        }
        break;

        case PORT_CI_ICU_BOTH_EDGES:
        {
            s_Port_Ci_Base[instance]->PCR[hwChannel] |= ICU_PORT_CI_PCR_IEEE_U32;
#if ((PORT_CI_ICU_IP_SET_MODE_API == STD_ON) || (PORT_CI_ICU_IP_EDGE_DETECT_API == STD_ON))
            Port_Ci_Icu_Ip_aChActivationMode[hwChannel] = ICU_PORT_CI_CH_EITHER_EDGE_U8;
#endif
        }
        break;
        
        default:
        {
            s_Port_Ci_Base[instance]->PCR[hwChannel] &= ~(ICU_PORT_CI_PCR_IREE_U32|ICU_PORT_CI_PCR_IFEE_U32);
#if ((PORT_CI_ICU_IP_SET_MODE_API == STD_ON) || (PORT_CI_ICU_IP_EDGE_DETECT_API == STD_ON))
            Port_Ci_Icu_Ip_aChActivationMode[hwChannel] = ICU_PORT_CI_CH_NEITHER_EDGE_U8;
#endif
        }
        break;
        
    }
}

#if (PORT_CI_ICU_IP_EDGE_DETECT_API == STD_ON)
/**
* @brief      Port_Ci_Icu_Ip_EnableEdgeDetection
* @details    This function enables the interrupt corresponding to the Port_Ci channel.
* 
* @param[in]  hwChannel - IRQ HW Channel
* 
* @return void
*
* @implements  Port_Ci_Icu_Ip_EnableEdgeDetection_Activity
* */
void Port_Ci_Icu_Ip_EnableEdgeDetection(uint8 instance, uint8 hwChannel)
{
    /* Enable IRQ Interrupt */
    Port_Ci_Icu_Ip_EnableInterrupt(instance, hwChannel);
}

/**
* @brief      Port_Ci_Icu_Ip_DisableDetection
* @details    This function disables the interrupt corresponding to the Port_Ci channel.
* 
* @param[in]  hwChannel - IRQ HW Channel
* 
* @return void
*
* @implements  Port_Ci_Icu_Ip_DisableDetection_Activity
* */
void Port_Ci_Icu_Ip_DisableDetection(uint8 instance, uint8 hwChannel)
{   
    /* Disable IRQ Interrupt */
    Port_Ci_Icu_Ip_DisableInterrupt(instance, hwChannel);
}
#endif /* PORT_CI_ICU_IP_EDGE_DETECT_API */

#if (PORT_CI_ICU_IP_GET_INPUT_STATE_API == STD_ON)
/**
* @brief      Port_Ci_Icu_Ip_GetInputState
* @details    This function returns the input state of the corresponding Port_Ci channel.
*             The API is usable only for the case when IRQ is not serviced - application does not enable core interrupt. 
* @param[in]  instance - Port index 
* @param[in]  hwChannel  - Index of the Channel from the selected port
* 
* @return void
*
* @implements  Port_Ci_Icu_Ip_GetInputState_Activity
* */
boolean Port_Ci_Icu_Ip_GetInputState(uint8 instance, uint8 hwChannel)
{
    boolean bStatus = FALSE;
    uint32 u32pinMask = (uint32)((uint32)1U << (uint32)hwChannel);

    /* The flag is generated only if an interrupt is set in PCR registers */
    /* the checking is only done for the flag considering that IRQ is not serviced */
    if (0x0U !=  (s_Port_Ci_Base[instance]->ISFR & u32pinMask) )
    {
        /* Clear IRQ Flag */
        s_Port_Ci_Base[instance]->ISFR |= u32pinMask;
        
        bStatus = TRUE;
    }

    return bStatus;
}
#endif  /* PORT_CI_ICU_IP_GET_INPUT_STATE_API */

/**
 * @brief      Driver function Enable Notification for timestamp.
 * @implements Port_Ci_Icu_Ip_EnableNotification_Activity
 */
void Port_Ci_Icu_Ip_EnableNotification(uint8 instance, uint8 hwChannel)
{
    Port_Ci_Icu_Ip_u32ChState[instance][hwChannel].notificationEnable = TRUE;
}

/**
 * @brief      Driver function Disable Notification for timestamp.
 * @implements Port_Ci_Icu_Ip_DisableNotification_Activity
 */
void Port_Ci_Icu_Ip_DisableNotification(uint8 instance, uint8 hwChannel)
{
    Port_Ci_Icu_Ip_u32ChState[instance][hwChannel].notificationEnable = FALSE;
}

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */
