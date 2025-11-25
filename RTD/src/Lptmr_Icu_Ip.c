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
 *     @file       File with source code used to implement ICU driver functionality on LPTMR module.
 *     @details    This file contains the source code for all functions which are using LPTMR module.
 *     @addtogroup lptmr_icu_ip LPTMR IPL
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
#include "Lptmr_Icu_Ip.h"
#include "SchM_Icu.h"
#include "OsIf.h"

#if(LPTMR_ICU_IP_DEV_ERROR_DETECT == STD_ON)
    #include "Devassert.h"
#endif

#if (STD_ON == LPTMR_ICU_ENABLE_USER_MODE_SUPPORT)
    #define USER_MODE_REG_PROT_ENABLED  (LPTMR_ICU_ENABLE_USER_MODE_SUPPORT)
    #include "RegLockMacros.h"
#endif

/*==================================================================================================
*                                  SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define LPTMR_ICU_IP_VENDOR_ID_C                     43
#define LPTMR_ICU_IP_AR_RELEASE_MAJOR_VERSION_C      4
#define LPTMR_ICU_IP_AR_RELEASE_MINOR_VERSION_C      7
#define LPTMR_ICU_IP_AR_RELEASE_REVISION_VERSION_C   0
#define LPTMR_ICU_IP_SW_MAJOR_VERSION_C              2
#define LPTMR_ICU_IP_SW_MINOR_VERSION_C              0
#define LPTMR_ICU_IP_SW_PATCH_VERSION_C              0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if source file and OsIf.h file are of the same Autosar version */
    #if ((LPTMR_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != OSIF_AR_RELEASE_MAJOR_VERSION) || \
         (LPTMR_ICU_IP_AR_RELEASE_MINOR_VERSION_C != OSIF_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers Lptmr_Icu_Ip.c and OsIf.h are different"
    #endif
#endif

#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if header file and SchM_Icu.h file are of the same Autosar version */
    #if ((LPTMR_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != SCHM_ICU_AR_RELEASE_MAJOR_VERSION) || \
         (LPTMR_ICU_IP_AR_RELEASE_MINOR_VERSION_C != SCHM_ICU_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Lptmr_Icu_Ip.c and SchM_Icu.h are different"
    #endif

    #if (STD_ON == LPTMR_ICU_ENABLE_USER_MODE_SUPPORT)
        /* Check if header file and RegLockMacros.h file are of the same Autosar version */
        #if ((LPTMR_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != REGLOCKMACROS_AR_RELEASE_MAJOR_VERSION) || \
             (LPTMR_ICU_IP_AR_RELEASE_MINOR_VERSION_C != REGLOCKMACROS_AR_RELEASE_MINOR_VERSION))
            #error "AutoSar Version Numbers of Lptmr_Icu_Ip.c and RegLockMacros.h are different"
        #endif
    #endif
#endif /* DISABLE_MCAL_INTERMODULE_ASR_CHECK */

#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    #if(LPTMR_ICU_IP_DEV_ERROR_DETECT == STD_ON)
        /* Check if this header file and Devassert.h file are of the same Autosar version */
        #if ((LPTMR_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != DEVASSERT_AR_RELEASE_MAJOR_VERSION) || \
             (LPTMR_ICU_IP_AR_RELEASE_MINOR_VERSION_C != DEVASSERT_AR_RELEASE_MINOR_VERSION))
            #error "AutoSar Version Numbers of Lptmr_Icu_Ip.c and Devassert.h are different"
        #endif
    #endif
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (LPTMR_ICU_IP_VENDOR_ID_C != LPTMR_ICU_IP_VENDOR_ID)
    #error "Lptmr_Icu_Ip.c and Lptmr_Icu_Ip.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((LPTMR_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != LPTMR_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (LPTMR_ICU_IP_AR_RELEASE_MINOR_VERSION_C != LPTMR_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (LPTMR_ICU_IP_AR_RELEASE_REVISION_VERSION_C != LPTMR_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Lptmr_Icu_Ip.c and Lptmr_Icu_Ip.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((LPTMR_ICU_IP_SW_MAJOR_VERSION_C != LPTMR_ICU_IP_SW_MAJOR_VERSION) || \
     (LPTMR_ICU_IP_SW_MINOR_VERSION_C != LPTMR_ICU_IP_SW_MINOR_VERSION) || \
     (LPTMR_ICU_IP_SW_PATCH_VERSION_C != LPTMR_ICU_IP_SW_PATCH_VERSION))
#error "Software Version Numbers of Lptmr_Icu_Ip.c and Lptmr_Icu_Ip.h are different"
#endif

/*==================================================================================================
*                           LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                         LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                        LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        LOCAL VARIABLES
==================================================================================================*/
#define ICU_START_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"

/* Table of base addresses for LPTMR instances. */
Lptmr_Icu_Ip_BaseType * const s_lptmrBase[LPTMR_ICU_IP_NUM_OF_MODULES] = IP_LPTMR_BASE_PTRS;

#define ICU_STOP_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"

#define ICU_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"

Lptmr_Icu_Ip_ChStateType Lptmr_Icu_aChConfig[LPTMR_ICU_IP_NUM_OF_MODULES];

#define ICU_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"

/*==================================================================================================
*                                    LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static inline void Lptmr_Icu_Ip_SetPrescaler(uint8 instance, Lptmr_Icu_PrescalerType prescalerValue, boolean enable);
static inline void Lptmr_Icu_Ip_ClearCompareFlag(uint8 instance);
static inline void Lptmr_Icu_Ip_TimerEnable(uint8 instance, boolean enable);
static inline void Lptmr_Icu_Ip_TimerModeSelect(uint8 instance, uint32 mode);
static inline void Lptmr_Icu_Ip_PinSelect(uint8 instance, Lptmr_Icu_Ip_PinSelectType Pin);
static inline void Lptmr_Icu_Ip_SetClockSelect(uint8 instance, Lptmr_Icu_Ip_ClockSourceType clocksource);
#if ((STD_ON == LPTMR_ICU_EDGE_COUNT_API) || (STD_ON == LPTMR_ICU_EDGE_DETECT_API))
static inline void Lptmr_Icu_Ip_SetMeasurementMode(uint8 instance,Lptmr_Icu_MeasurementModeType mode);
#endif
#if (STD_ON == LPTMR_ICU_GET_INPUT_STATE_API)
static inline boolean Lptmr_Icu_Ip_GetCmpFlagState(uint8 instance);
static inline boolean Lptmr_Icu_Ip_GetInterruptBit(uint8 instance);
#endif
#if (STD_ON == LPTMR_ICU_STANDBY_WAKEUP_SUPPORT)
static inline boolean Lptmr_Icu_Ip_CheckInterruptAndFlag(uint8 instance);
#endif

/*==================================================================================================
*                                        LOCAL FUNCTIONS
==================================================================================================*/
/*!
 * @brief Enable/Disable the LPTMR
 *
 * Enable/Disable the LPTMR. Starts/Stop the timer/counter.
 *
 * @param[in] base   - LPTMR base pointer
 * @param[in] enable - The new state for LPTMR timer
 *          - true:  enable timer
 *          - false: disable timer and internal logic is reset
 *
 */
static inline void Lptmr_Icu_Ip_TimerEnable(uint8 instance, boolean enable)
{
    if (TRUE == enable)
    {
        s_lptmrBase[instance]->CSR |=  LPTMR_CSR_TEN_MASK;
    }
    else
    {
        s_lptmrBase[instance]->CSR &= ~(LPTMR_CSR_TEN_MASK);
    }
}

/*!
 * @brief Clear the Compare Flag
 *
 * This function clears the Compare Flag/Interrupt Pending state.
 *
 * @param[in] base - LPTMR base pointer
 */
static inline void Lptmr_Icu_Ip_ClearCompareFlag(uint8 instance)
{
    s_lptmrBase[instance]->CSR |= LPTMR_CSR_TCF_MASK;
}

/*!
 * @brief Configure the Work Mode
 *
 * This function configures the LPTMR to either Timer Mode or Pulse Counter
 * Mode. This feature can be configured only when the LPTMR is disabled.
 *
 * @param[in] base - LPTMR base pointer
 * @param[in] mode - New Work Mode
 *          - LPTMR_ICU_IP_TM_MODE: LPTMR set to Pulse Counter Mode
 */
static inline void Lptmr_Icu_Ip_TimerModeSelect(uint8 instance, uint32 mode)
{

    s_lptmrBase[instance]->CSR = (s_lptmrBase[instance]->CSR & ~LPTMR_CSR_TMS_MASK) | LPTMR_CSR_TMS(mode);

}

/*!
 * @brief Configure the Prescaler/Glitch Filter divider value
 *
 * This function configures the value for the Prescaler/Glitch Filter. This
 * feature can be configured only when the LPTMR is disabled.
 *
 * @param[in] base  - LPTMR base pointer
 * @param[in] presc - The new Prescaler value
 *          - LPTMR_PRESCALE_2: Timer mode: prescaler 2, Glitch filter mode: invalid
 *          - LPTMR_PRESCALE_4_GLITCHFILTER_2: Timer mode: prescaler 4, Glitch filter mode: 2 clocks
 *          - LPTMR_PRESCALE_8_GLITCHFILTER_4: Timer mode: prescaler 8, Glitch filter mode: 4 clocks
 *          - LPTMR_PRESCALE_16_GLITCHFILTER_8: Timer mode: prescaler 16, Glitch filter mode: 8 clocks
 *          - LPTMR_PRESCALE_32_GLITCHFILTER_16: Timer mode: prescaler 32, Glitch filter mode: 16 clocks
 *          - LPTMR_PRESCALE_64_GLITCHFILTER_32: Timer mode: prescaler 64, Glitch filter mode: 32 clocks
 *          - LPTMR_PRESCALE_128_GLITCHFILTER_64: Timer mode: prescaler 128, Glitch filter mode: 64 clocks
 *          - LPTMR_PRESCALE_256_GLITCHFILTER_128: Timer mode: prescaler 256, Glitch filter mode: 128 clocks
 *          - LPTMR_PRESCALE_512_GLITCHFILTER_256: Timer mode: prescaler 512, Glitch filter mode: 256 clocks
 *          - LPTMR_PRESCALE_1024_GLITCHFILTER_512: Timer mode: prescaler 1024, Glitch filter mode: 512 clocks
 *          - LPTMR_PRESCALE_2048_GLITCHFILTER_1024: Timer mode: prescaler 2048, Glitch filter mode: 1024 clocks
 *          - LPTMR_PRESCALE_4096_GLITCHFILTER_2048: Timer mode: prescaler 4096, Glitch filter mode: 2048 clocks
 *          - LPTMR_PRESCALE_8192_GLITCHFILTER_4096: Timer mode: prescaler 8192, Glitch filter mode: 4096 clocks
 *          - LPTMR_PRESCALE_16384_GLITCHFILTER_8192: Timer mode: prescaler 16384, Glitch filter mode: 8192 clocks
 *          - LPTMR_PRESCALE_32768_GLITCHFILTER_16384: Timer mode: prescaler 32768, Glitch filter mode: 16384 clocks
 *          - LPTMR_PRESCALE_65536_GLITCHFILTER_32768: Timer mode: prescaler 65536, Glitch filter mode: 32768 clocks
 */
static inline void Lptmr_Icu_Ip_SetPrescaler(uint8 instance, Lptmr_Icu_PrescalerType prescalerValue, boolean enable)
{
    if(TRUE == enable)
    {
        s_lptmrBase[instance]->PSR &= ~(LPTMR_PSR_PBYP_MASK);
        s_lptmrBase[instance]->PSR = (s_lptmrBase[instance]->PSR & ~LPTMR_PSR_PRESCALE_MASK) | LPTMR_PSR_PRESCALE(prescalerValue);
    }else
    {
        s_lptmrBase[instance]->PSR |= (LPTMR_PSR_PBYP_MASK);
    }
}

/*!
 * @brief Configure the Pin selection for Pulse Counter Mode
 *
 * This function configures the input Pin selection for Pulse Counter Mode.
 * This feature can be configured only when the LPTMR is disabled.
 *
 * @param[in] base   - LPTMR base pointer
 * @param[in] pinsel - Pin selection
 *          - LPTMR_PINSELECT_TRGMUX: count pulses from TRGMUX output
 *          - LPTMR_PINSELECT_ALT1: count pulses from pin alt 1
 *          - LPTMR_PINSELECT_ALT2: count pulses from pin alt 2
 *          - LPTMR_PINSELECT_ALT3: count pulses from pin alt 3
 */
static inline void Lptmr_Icu_Ip_PinSelect(uint8 instance, Lptmr_Icu_Ip_PinSelectType Pin)
{
    s_lptmrBase[instance]->CSR = (s_lptmrBase[instance]->CSR & ~LPTMR_CSR_TPS_MASK) | LPTMR_CSR_TPS(Pin);
}

/*!
 * @brief Configure the LPTMR input Clock selection
 *
 * This function configures a clock source for the LPTMR. This feature can be
 * configured only when the LPTMR is disabled.
 *
 * @param[in] base - LPTMR base pointer
 * @param[in] clocksel - New Clock Source
 *          - LPTMR_CLOCKSOURCE_SIRCDIV2: clock from SIRC DIV2
 *          - LPTMR_CLOCKSOURCE_1KHZ_LPO: clock from 1kHz LPO
 *          - LPTMR_CLOCKSOURCE_RTC: clock from RTC
 *          - LPTMR_CLOCKSOURCE_PCC: clock from PCC
 */
static inline void Lptmr_Icu_Ip_SetClockSelect(uint8 instance, Lptmr_Icu_Ip_ClockSourceType clocksource)
{
    s_lptmrBase[instance]->PSR = (s_lptmrBase[instance]->PSR & ~LPTMR_PSR_PCS_MASK) | LPTMR_PSR_PCS(clocksource);
}

#if ((STD_ON == LPTMR_ICU_EDGE_COUNT_API) || (STD_ON == LPTMR_ICU_EDGE_DETECT_API))
/*!
 * @brief Configure the Measurement Mode state
 *
 * This function configures the Free Running feature of the LPTMR. This feature
 * can be configured only when the LPTMR is disabled.
 *
 * @param[in] base   - LPTMR base pointer
 *            mode
 *
 */
static inline void Lptmr_Icu_Ip_SetMeasurementMode(uint8 instance, Lptmr_Icu_MeasurementModeType mode)
{
#if (STD_ON == LPTMR_ICU_EDGE_COUNT_API)
    if (LPTMR_ICU_MODE_EDGE_COUNTER == mode)
    {
        /* Clear flag*/
        Lptmr_Icu_Ip_ClearCompareFlag(instance);
        /* CNR is reset on overflow */
        s_lptmrBase[instance]->CSR = (s_lptmrBase[instance]->CSR & ~LPTMR_CSR_TFC_MASK) | LPTMR_CSR_TFC_MASK;
    }
#endif
#if (STD_ON == LPTMR_ICU_EDGE_DETECT_API)
    if (LPTMR_ICU_MODE_SIGNAL_EDGE_DETECT == mode)
    {
        /* Clear flag*/
        Lptmr_Icu_Ip_ClearCompareFlag(instance);
        /* CNR is reset whenever TCF is set. */
        s_lptmrBase[instance]->CSR &= ~(LPTMR_CSR_TFC_MASK);
        /* set the hardware trigger asserts until the next time the CNR increments */
        s_lptmrBase[instance]->CMR = (s_lptmrBase[instance]->CMR & ~LPTMR_CMR_COMPARE_MASK) | LPTMR_CMR_COMPARE(1U);
    }
#endif
}
#endif

#if (STD_ON == LPTMR_ICU_GET_INPUT_STATE_API)
/**
* @brief         Lptmr_Icu_Ip_GetCmpFlagState Get the Compare Flag state
* @details       This function checks whether a Compare Match event has occurred or if there is an Interrupt Pending.
* @param[in]     instance     LPtimer hw instance number
*
* @return       The Compare Flag state
*               - true: Compare Match/Interrupt Pending asserted
*               - false: Compare Match/Interrupt Pending not asserted
* @pre           The driver needs to be initialized.
*
*/
static inline boolean Lptmr_Icu_Ip_GetCmpFlagState(uint8 instance)
{
    uint32 CmpFlagState;
    CmpFlagState = (s_lptmrBase[instance]->CSR & LPTMR_CSR_TCF_MASK) >> LPTMR_CSR_TCF_SHIFT;
    return ((CmpFlagState == 1u) ? TRUE : FALSE);
}

/**
* @brief        : Lptmr_Icu_Ip_GetInterruptBit get the bit CSR_TIE (CSR)
* @details       This function checks whether Timer Interrupt enable or not
* @param[in]     instance       LPtimer hardware instance number
*
* @return        status
*
*/
static inline boolean Lptmr_Icu_Ip_GetInterruptBit(uint8 instance)
{ 
    uint32 status;
    status = (s_lptmrBase[instance]->CSR & LPTMR_CSR_TIE_MASK) >> LPTMR_CSR_TIE_SHIFT;
    return ((status == 1u) ? TRUE : FALSE);
}
#endif

#if (STD_ON == LPTMR_ICU_STANDBY_WAKEUP_SUPPORT)
/**
* @brief        : Lptmr_Icu_Ip_CheckInterruptAndFlag get the bit CSR_TIE and CSR_TCF
* @details       This function checks if Timer Interrupt enable and Compare Match event has occurred
* @param[in]     instance       LPtimer hardware instance number
*
* @return        status
* 
*/
static inline boolean Lptmr_Icu_Ip_CheckInterruptAndFlag(uint8 instance)
{ 
    uint32 status;
    uint32 CmpFlagState;
    status = (s_lptmrBase[instance]->CSR & LPTMR_CSR_TIE_MASK) >> LPTMR_CSR_TIE_SHIFT;
    CmpFlagState = (s_lptmrBase[instance]->CSR & LPTMR_CSR_TCF_MASK) >> LPTMR_CSR_TCF_SHIFT;
    return (((status == 1u) && (CmpFlagState == 1u)) ? TRUE : FALSE);
}
#endif

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"
/** @implements  Lptmr_Icu_Ip_Init_Activity */
Lptmr_Icu_Ip_StatusType Lptmr_Icu_Ip_Init(uint8 instance, const Lptmr_Icu_Ip_ConfigType *userConfig)
{
    const Lptmr_Icu_Ip_ChannelConfigType *pLptmrChannelConfig = &(*userConfig->pChannelsConfig)[0];
    Lptmr_Icu_Ip_StatusType   retStatus = LPTMR_IP_STATUS_SUCCESS;

    if(FALSE == Lptmr_Icu_aChConfig[instance].chInit)
    {
#if (STD_ON == LPTMR_ICU_STANDBY_WAKEUP_SUPPORT)
        if(TRUE == Lptmr_Icu_Ip_CheckInterruptAndFlag(instance))
#endif
        {
            /*Stop timer */
            Lptmr_Icu_Ip_TimerEnable(instance, FALSE);

            /* Disable interrupts*/
            Lptmr_Icu_Ip_DisableInterrupt(instance);

            /*Clear Flag*/
            Lptmr_Icu_Ip_ClearCompareFlag(instance);

            /*Set Input source */
            Lptmr_Icu_Ip_PinSelect(instance, userConfig->PinSelect);

            /* Configures the mode of LPTMR is Time Counter */
            Lptmr_Icu_Ip_TimerModeSelect(instance, LPTMR_ICU_IP_TM_MODE);
        }

        /* Set Clock source*/
        Lptmr_Icu_Ip_SetClockSelect(instance, userConfig->ClockSource);

        /* Set Prescale*/
        Lptmr_Icu_Ip_SetPrescaler(instance, userConfig->Prescaler, userConfig->PrescalerEnable);

        /*Set Activation Condition*/
        Lptmr_Icu_Ip_SetActivationCondition(instance, pLptmrChannelConfig->DefaultStartEdge);

#if ((STD_ON == LPTMR_ICU_EDGE_COUNT_API) || (STD_ON == LPTMR_ICU_EDGE_DETECT_API))
        /*Set Measurement Mode*/
        Lptmr_Icu_Ip_SetMeasurementMode(instance, pLptmrChannelConfig->MeasurementModeType);
#endif

        /* Start timer. */
        Lptmr_Icu_Ip_TimerEnable(instance, TRUE);

        /* Save instance info. */
        Lptmr_Icu_aChConfig[instance].chInit = TRUE;
        Lptmr_Icu_aChConfig[instance].measMode = pLptmrChannelConfig->MeasurementModeType;
        Lptmr_Icu_aChConfig[instance].callback = pLptmrChannelConfig->callback;
        Lptmr_Icu_aChConfig[instance].callbackParam = pLptmrChannelConfig->callbackParam;
        Lptmr_Icu_aChConfig[instance].lptmrChannelNotification = pLptmrChannelConfig->lptmrChannelNotification;
    }
    else
    {
        /* instance already initialized - use deinitialize first */
        retStatus = LPTMR_IP_STATUS_ERROR;
    }
    return retStatus;
}

#if (STD_ON == LPTMR_ICU_DEINIT_API)
/** @implements  Lptmr_Icu_Ip_Deinit_Activity */
Lptmr_Icu_Ip_StatusType Lptmr_Icu_Ip_Deinit(uint8 instance)
{
    Lptmr_Icu_Ip_StatusType   retStatus = LPTMR_IP_STATUS_SUCCESS;

    if (TRUE == Lptmr_Icu_aChConfig[instance].chInit)
    {
        /*Stop timer */
        Lptmr_Icu_Ip_TimerEnable(instance, FALSE);

        /* Disable interrupts*/
        Lptmr_Icu_Ip_DisableInterrupt(instance);

        /*Clear Flag*/
        Lptmr_Icu_Ip_ClearCompareFlag(instance);

        /* Record the deinit */
        Lptmr_Icu_aChConfig[instance].chInit = FALSE;
        Lptmr_Icu_aChConfig[instance].measMode = (Lptmr_Icu_MeasurementModeType)LPTMR_ICU_MODE_NO_MEASUREMENT;
        Lptmr_Icu_aChConfig[instance].callback = NULL_PTR;
        Lptmr_Icu_aChConfig[instance].callbackParam = 0U;
        Lptmr_Icu_aChConfig[instance].lptmrChannelNotification = NULL_PTR;
    }
    else
    {
        /* instance already de-initialize - use initialize first */
        retStatus = LPTMR_IP_STATUS_ERROR;
    }
    return retStatus;
}
#endif

#if (STD_ON == LPTMR_ICU_SET_MODE_API)
/**
* @brief      Driver function that sets Lptmr hardware channel into SLEEP mode.
* @details    This function enables the interrupt if wakeup is enabled for corresponding
*             Lptmr channel
*
* @return void
*
* */
void Lptmr_Icu_Ip_SetSleepMode(uint8 instance)
{
    /* Disable interrupts*/
    Lptmr_Icu_Ip_DisableInterrupt(instance);
}

/**
* @brief      Driver function that sets the Lptmr hardware channel into NORMAL mode.
* @details    This function enables the interrupt if Notification is enabled for corresponding
*             Lptmr channel
*
* @return void
*
* */
void Lptmr_Icu_Ip_SetNormalMode(uint8 instance)
{
    /* Enable interrupts*/
    Lptmr_Icu_Ip_EnableInterrupt(instance);
}
#endif  /* LPTMR_ICU_SET_MODE_API  */

/** @implements  Lptmr_Icu_Ip_SetActivationCondition_Activity */
void Lptmr_Icu_Ip_SetActivationCondition(uint8 instance, Lptmr_Icu_Ip_EdgeType activation)
{
    s_lptmrBase[instance]->CSR = ((s_lptmrBase[instance]->CSR & ~(LPTMR_CSR_TPP_MASK)) | LPTMR_CSR_TPP(activation));
}

#if (STD_ON == LPTMR_ICU_EDGE_DETECT_API)
/** @implements  Lptmr_Icu_Ip_EnableEdgeDetection_Activity */
void Lptmr_Icu_Ip_EnableEdgeDetection(uint8 instance)
{
    Lptmr_Icu_Ip_EnableInterrupt(instance);
}

/** @implements  Lptmr_Icu_Ip_DisableDetection_Activity */
void Lptmr_Icu_Ip_DisableDetection(uint8 instance)
{
    Lptmr_Icu_Ip_DisableInterrupt(instance);
}

#endif /* LPTMR_ICU_EDGE_DETECT_API */

#if (STD_ON == LPTMR_ICU_EDGE_COUNT_API)
/** @implements  Lptmr_Icu_Ip_ResetEdgeCount_Activity */
void Lptmr_Icu_Ip_ResetEdgeCount(uint8 instance)
{
    /*Stop timer */
    Lptmr_Icu_Ip_TimerEnable(instance, FALSE);
    /*Start timer */
    Lptmr_Icu_Ip_TimerEnable(instance, TRUE);
}

/** @implements  Lptmr_Icu_Ip_EnableEdgeCount_Activity */
void Lptmr_Icu_Ip_EnableEdgeCount(uint8 instance)
{
    /*Stop timer */
    Lptmr_Icu_Ip_TimerEnable(instance, FALSE);
    /*Start timer */
    Lptmr_Icu_Ip_TimerEnable(instance, TRUE);
}

/** @implements  Lptmr_Icu_Ip_DisableEdgeCount_Activity */
Lptmr_Icu_Ip_StatusType Lptmr_Icu_Ip_DisableEdgeCount(uint8 instance)
{
    /*Stop timer */
    Lptmr_Icu_Ip_TimerEnable(instance, FALSE);
    return LPTMR_IP_STATUS_SUCCESS;
}
/** @implements  Lptmr_Icu_Ip_GetEdgeNumbers_Activity */
uint16 Lptmr_Icu_Ip_GetEdgeNumbers(uint8 instance)
{
    /*Write random number to read counter value*/
    s_lptmrBase[instance]->CNR &= LPTMR_CNR_COUNTER(1);
    return (uint16)s_lptmrBase[instance]->CNR;
}
#endif  /* LPTMR_ICU_EDGE_COUNT_API */

#if (STD_ON == LPTMR_ICU_GET_INPUT_STATE_API)
/** @implements  Lptmr_Icu_Ip_GetInputState_Activity */
boolean Lptmr_Icu_Ip_GetInputState(uint8 instance)
{
    boolean bStatus = FALSE;
    if((FALSE == Lptmr_Icu_Ip_GetInterruptBit(instance)) && (TRUE == Lptmr_Icu_Ip_GetCmpFlagState(instance)))
    {
        Lptmr_Icu_Ip_ClearCompareFlag(instance);
        bStatus = TRUE;
    }
    return bStatus;
}
#endif  /* LPTMR_ICU_GET_INPUT_STATE_API */

/*!
 * @brief Configure the Interrupt Enable state
 *
 * This function configures the Interrupt Enable state for the LPTMR. If enabled,
 * an interrupt is generated when a Compare Match event occurs.
 *
 * @param[in] base   - LPTMR base pointer
 * @implements  Lptmr_Icu_Ip_EnableInterrupt_Activity
 */
void Lptmr_Icu_Ip_EnableInterrupt(uint8 instance)
{
    s_lptmrBase[instance]->CSR |= LPTMR_CSR_TIE_MASK;
}

/*!
 * @brief Configure the Interrupt disable state
 *
 * This function configures the Interrupt disable state for the LPTMR. If enabled,
 * an interrupt is generated when a Compare Match event occurs.
 *
 * @param[in] base   - LPTMR base pointer
 * @implements  Lptmr_Icu_Ip_DisableInterrupt_Activity
 */
void Lptmr_Icu_Ip_DisableInterrupt(uint8 instance)
{
    s_lptmrBase[instance]->CSR &= ~(LPTMR_CSR_TIE_MASK);
}

/**
 * @brief      Driver function Enable Notification for timestamp.
 * @implements Lptmr_Icu_Ip_EnableNotification_Activity
 */
void Lptmr_Icu_Ip_EnableNotification(uint8 instance)
{
    Lptmr_Icu_aChConfig[instance].notificationEnable = TRUE;
}

/**
 * @brief      Driver function Disable Notification for timestamp.
 * @implements Lptmr_Icu_Ip_DisableNotification_Activity
 */
void Lptmr_Icu_Ip_DisableNotification(uint8 instance)
{
    Lptmr_Icu_aChConfig[instance].notificationEnable = FALSE;
}

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */
