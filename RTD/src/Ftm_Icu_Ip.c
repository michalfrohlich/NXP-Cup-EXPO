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
 *     @file       File with source code used to implement ICU driver functionality on FTM module.
 *     @details    This file contains the source code for all functions which are using FTM module.
 *     @addtogroup ftm_icu_ip FTM IPL
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
#include "Ftm_Icu_Ip.h"
#include "SchM_Icu.h"
#include "OsIf.h"

#if(FTM_ICU_IP_DEV_ERROR_DETECT == STD_ON)
#include "Devassert.h"
#endif

#if (STD_ON == FTM_ICU_ENABLE_USER_MODE_SUPPORT)
#define USER_MODE_REG_PROT_ENABLED  (FTM_ICU_ENABLE_USER_MODE_SUPPORT)
#include "RegLockMacros.h"
#endif

/*==================================================================================================
*                                  SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define FTM_ICU_IP_VENDOR_ID_C                     43
#define FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION_C      4
#define FTM_ICU_IP_AR_RELEASE_MINOR_VERSION_C      7
#define FTM_ICU_IP_AR_RELEASE_REVISION_VERSION_C   0
#define FTM_ICU_IP_SW_MAJOR_VERSION_C              2
#define FTM_ICU_IP_SW_MINOR_VERSION_C              0
#define FTM_ICU_IP_SW_PATCH_VERSION_C              0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if source file and OsIf.h file are of the same Autosar version */
    #if ((FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != OSIF_AR_RELEASE_MAJOR_VERSION) || \
         (FTM_ICU_IP_AR_RELEASE_MINOR_VERSION_C != OSIF_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers Ftm_Icu_Ip.c and OsIf.h are different"
    #endif
#endif

#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    #if(FTM_ICU_IP_DEV_ERROR_DETECT == STD_ON)
        /* Check if this header file and Devassert.h file are of the same Autosar version */
        #if ((FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != DEVASSERT_AR_RELEASE_MAJOR_VERSION) || \
            (FTM_ICU_IP_AR_RELEASE_MINOR_VERSION_C != DEVASSERT_AR_RELEASE_MINOR_VERSION))
            #error "AutoSar Version Numbers of Ftm_Icu_Ip.c and Devassert.h are different"
        #endif
    #endif
#endif

#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if header file and SchM_Icu.h file are of the same Autosar version */
    #if ((FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != SCHM_ICU_AR_RELEASE_MAJOR_VERSION) || \
         (FTM_ICU_IP_AR_RELEASE_MINOR_VERSION_C != SCHM_ICU_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Ftm_Icu_Ip.c and SchM_Icu.h are different"
    #endif

    #if (STD_ON == FTM_ICU_ENABLE_USER_MODE_SUPPORT)
        /* Check if header file and RegLockMacros.h file are of the same Autosar version */
        #if ((FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != REGLOCKMACROS_AR_RELEASE_MAJOR_VERSION) || \
             (FTM_ICU_IP_AR_RELEASE_MINOR_VERSION_C != REGLOCKMACROS_AR_RELEASE_MINOR_VERSION))
            #error "AutoSar Version Numbers of Ftm_Icu_Ip.c and RegLockMacros.h are different"
        #endif
    #endif
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (FTM_ICU_IP_VENDOR_ID_C != FTM_ICU_IP_VENDOR_ID)
    #error "Ftm_Icu_Ip.c and Ftm_Icu_Ip.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION_C != FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (FTM_ICU_IP_AR_RELEASE_MINOR_VERSION_C != FTM_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (FTM_ICU_IP_AR_RELEASE_REVISION_VERSION_C != FTM_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Ftm_Icu_Ip.c and Ftm_Icu_Ip.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((FTM_ICU_IP_SW_MAJOR_VERSION_C != FTM_ICU_IP_SW_MAJOR_VERSION) || \
     (FTM_ICU_IP_SW_MINOR_VERSION_C != FTM_ICU_IP_SW_MINOR_VERSION) || \
     (FTM_ICU_IP_SW_PATCH_VERSION_C != FTM_ICU_IP_SW_PATCH_VERSION))
    #error "Software Version Numbers of Ftm_Icu_Ip.c and Ftm_Icu_Ip.h are different"
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
#if (STD_ON == FTM_ICU_TIMESTAMP_API)

#define ICU_START_SEC_VAR_CLEARED_16
#include "Icu_MemMap.h"

uint16 Ftm_Icu_Ip_aBufferPtr[FTM_INSTANCE_COUNT][FTM_ICU_IP_NUM_OF_CHANNELS];

#define ICU_STOP_SEC_VAR_CLEARED_16
#include "Icu_MemMap.h"

#endif

#define ICU_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"

/* State of initialized FTM channels */
Ftm_Icu_Ip_ChStateType Ftm_Icu_Ip_ChState[FTM_INSTANCE_COUNT][FTM_ICU_IP_NUM_OF_CHANNELS];

Ftm_Icu_Ip_InstStateType Ftm_Icu_Ip_InsState[FTM_INSTANCE_COUNT];


#if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)
/* Variables hold Edge Align for SignalMeasurement mode only.
 * This variable is only set when the init driver, and is not
 * affected by the Ftm_Icu_Ip_SetActivationCondition function
 * such as the Ftm_Icu_Ip_ChState[module][channel].edgeTrigger variable.
 */
static Ftm_Icu_Ip_EdgeType SignalMeasurementEdgeAlign[FTM_INSTANCE_COUNT][FTM_ICU_IP_NUM_OF_CHANNELS];
#endif

#define ICU_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"

#define ICU_START_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"
/* Table of base addresses for FTM instances. */
Ftm_Icu_Ip_BaseType * const ftmIcuBase[] = IP_FTM_BASE_PTRS;

#define ICU_STOP_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"

/*==================================================================================================
*                                        GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                    LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"


#if (STD_ON == FTM_ICU_ENABLE_USER_MODE_SUPPORT)
    void Ftm_Icu_SetUserAccessAllowed(uint32 FtmBaseAddr);
#endif

#if (defined (MCAL_FTM_REG_PROT_AVAILABLE) && (STD_ON == FTM_ICU_ENABLE_USER_MODE_SUPPORT))
    #define Call_Ftm_Icu_SetUserAccessAllowed(BaseAddr) OsIf_Trusted_Call1param(Ftm_Icu_SetUserAccessAllowed,(BaseAddr))
#else
    #define Call_Ftm_Icu_SetUserAccessAllowed(BaseAddr)
#endif
#if (STD_ON == FTM_ICU_DEINIT_API)
static void Ftm_Icu_Ip_ClearChState(uint8 instance, uint8 hwChannel);
#endif
/**
 * @brief Sets the FTM peripheral timer channel input capture filter value.
 * 
 * @param ftmBase 
 * @param hwChannel 
 * @param value   - Filer value for a specific channel; Set (value = 0) => filter disable.
 */
static void Ftm_Icu_Ip_SetChnInputCaptureFilter(Ftm_Icu_Ip_BaseType * const ftmBase, uint8 hwChannel, uint8 value);

/*==================================================================================================
*                                        LOCAL FUNCTIONS
==================================================================================================*/

/**
 * @brief Sets the FTM peripheral timer channel input capture filter value.
 * 
 * @param ftmBase 
 * @param hwChannel 
 * @param value   - Filer value for a specific channel; Set (value = 0) => filter disable.
 */
static void Ftm_Icu_Ip_SetChnInputCaptureFilter(Ftm_Icu_Ip_BaseType * const ftmBase, uint8 hwChannel, uint8 value)
{
#if (STD_ON == FTM_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(CHAN4_IDX > hwChannel);
    DevAssert(NULL_PTR != ftmBase);
    DevAssert((uint8)16U > value);
#endif
    /* For the current channel filter value will be set. */
    switch (hwChannel)
    {
        case CHAN0_IDX:
            ftmBase->FILTER &= ~FTM_FILTER_CH0FVAL_MASK;
            ftmBase->FILTER |= FTM_FILTER_CH0FVAL(value);
            break;
        case CHAN1_IDX:
            ftmBase->FILTER &= ~FTM_FILTER_CH1FVAL_MASK;
            ftmBase->FILTER |= FTM_FILTER_CH1FVAL(value);
            break;
        case CHAN2_IDX:
            ftmBase->FILTER &= ~FTM_FILTER_CH2FVAL_MASK;
            ftmBase->FILTER |= FTM_FILTER_CH2FVAL(value);
            break;
        case CHAN3_IDX:
            ftmBase->FILTER &= ~FTM_FILTER_CH3FVAL_MASK;
            ftmBase->FILTER |= FTM_FILTER_CH3FVAL(value);
            break;
        default:
            /* Nothing to do. */
            break;
    }
}

/**
 * @brief Clear status, interrupt and overflow flags.
 * 
 * @param instance   - FTM instance where the current channel is located.
 * @param hwChannel  - FTM hardware channel used.
 */
static inline void Ftm_Icu_Ip_ClearInterruptFlags(uint8 instance, uint8 hwChannel)
{
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_15();
    {
        /* Clear the status and interrupt enable flags for the FTM channel. */
        ftmIcuBase[instance]->CONTROLS[hwChannel].CSC &= (~(FTM_CSC_CHIE_MASK | FTM_CSC_CHF_MASK));
        /* Clear overflow flag. */
        ftmIcuBase[instance]->SC &= ~(FTM_SC_TOF_MASK);
    }
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_15();
}

#if ((STD_ON == FTM_ICU_TIMESTAMP_API) || (STD_ON == FTM_ICU_EDGE_COUNT_API) || (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API))
/**
 * @brief Pin not used for FTM — revert the channel pin to general purpose I/O 
 *        or other peripheral control
 * 
 * @param instance   - FTM instance where the current channel is located.
 * @param hwChannel  - FTM hardware channel used.
 */
static inline void Ftm_Icu_Ip_StopChannel(uint8 instance, uint8 hwChannel)
{
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_16();
    /* Clear bits which controle channel edge. */
    ftmIcuBase[instance]->CONTROLS[hwChannel].CSC &= ~(FTM_CSC_ELSA_MASK | FTM_CSC_ELSB_MASK);
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_16();
}
#endif

/**
 * @brief Sets up the global configuration for the FTM hardware instance.
 * 
 * @param instance 
 * @param maxCountValue 
 */
static void Ftm_Icu_Ip_GlobalConfiguration(uint8 instance, uint16 maxCountValue)
{
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_17();
    /* If write protection is enabled, then disable it. */
    if (0U != (ftmIcuBase[instance]->FMS & FTM_FMS_WPEN_MASK))
    {
       /* Disable write protection */
       ftmIcuBase[instance]->MODE |= FTM_MODE_WPDIS_MASK;
    }
    /* Disable the FTM counter */
    ftmIcuBase[instance]->SC &= (~FTM_SC_CLKS_MASK);
    /* Value used to reset counter register:
    *  after reaching the maximum value the overflow flag will be set. */
    ftmIcuBase[instance]->MOD = maxCountValue;
    /* Initialize with the reset value counter */
    ftmIcuBase[instance]->CNTIN = (uint16) 0x0000U;
    /* Clear TOF by reading SC and writing 0 to TOF */
    ftmIcuBase[instance]->SC &= (~FTM_SC_TOF_MASK);
    /* Enable FTM module */
    ftmIcuBase[instance]->MODE |= FTM_MODE_FTMEN_MASK;
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_17();
}

/**
 * @brief 
 * 
 * @param instance       - FTM instance hardware number.
 * @param clockSource  - Source clock used by the current FTM instance.
 * @param prescaler    - Value which divides the source clock.
 */
static void Ftm_Icu_Ip_SetClock(uint8 instance, Ftm_Icu_Ip_ClockSourceType clockSource, uint8 prescaler)
{
    /* Set Prescaler. */
    uint8 psValue = 0U;
    for (psValue = 0U; psValue < 8U; psValue++)
    {
        if(prescaler == (((uint8)1U) << psValue))
        {
            break;
        }
    }
    ftmIcuBase[instance]->SC &= (~FTM_SC_PS_MASK);
    ftmIcuBase[instance]->SC |= FTM_SC_PS(psValue);
    /* Set Clock Source. */
    ftmIcuBase[instance]->SC = (ftmIcuBase[instance]->SC & (~FTM_SC_CLKS_MASK)) | FTM_SC_CLKS(clockSource);
}

#if (STD_ON == FTM_ICU_DEINIT_API)
/**
 * @brief Reset channel state.
 * 
 * @param instance  - FTM instance where the current channel is located.
 * @param hwChannel - FTM hardware channel used.
 */
static void Ftm_Icu_Ip_ClearChState(uint8 instance, uint8 hwChannel)
{
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_18();
    {
#if (defined(FTM_ICU_SIGNAL_MEASUREMENT_API) && (FTM_ICU_SIGNAL_MEASUREMENT_API == STD_ON))
        Ftm_Icu_Ip_ChState[instance][hwChannel].firstEdge         = FALSE;
        Ftm_Icu_Ip_ChState[instance][hwChannel].measurement       = FTM_ICU_NO_MEASUREMENT;
#endif
#if (defined(FTM_ICU_EDGE_COUNT_API) && (STD_ON == FTM_ICU_EDGE_COUNT_API))
        Ftm_Icu_Ip_ChState[instance][hwChannel].edgeNumbers       = 0U;
#endif
        Ftm_Icu_Ip_ChState[instance][hwChannel].channelMode       = FTM_ICU_MODE_NO_MEASUREMENT;
        Ftm_Icu_Ip_ChState[instance][hwChannel].edgeTrigger       = FTM_ICU_NO_PIN_CONTROL;
        Ftm_Icu_Ip_ChState[instance][hwChannel].dmaMode           = FTM_ICU_MODE_WITHOUT_DMA;
        Ftm_Icu_Ip_ChState[instance][hwChannel].callback          = NULL_PTR;
        Ftm_Icu_Ip_ChState[instance][hwChannel].callbackParam     = 0U;
#if (STD_ON == FTM_ICU_TIMESTAMP_API)
        Ftm_Icu_Ip_ChState[instance][hwChannel].timestampBufferType = FTM_ICU_NO_TIMESTAMP;
#endif

    }
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_18();
}
#endif

/*==================================================================================================
*                                        GLOBAL FUNCTIONS
==================================================================================================*/

#if (STD_ON == FTM_ICU_ENABLE_USER_MODE_SUPPORT)
/**
 * @brief        Enables FTM registers writing in User Mode by configuring REG_PROT
 * @details      Sets the UAA (User Access Allowed) bit of the FTM IP allowing FTM registers writing in User Mode
 *
 * @param[in]    FtmBaseAddr
 *
 * @return       none
 *
 * @pre          Should be executed in supervisor mode
 */
void Ftm_Icu_SetUserAccessAllowed(uint32 FtmBaseAddr)
{
    SET_USER_ACCESS_ALLOWED(FtmBaseAddr, FTM_PROT_MEM_U32);
}
#endif /* STD_ON == FTM_ICU_ENABLE_USER_MODE_SUPPORT */

#if (FTM_ICU_CAPTURERGISTER_API == STD_ON)
/* @implements Ftm_Icu_Ip_GetCaptureRegisterValue_Activity */
uint32 Ftm_Icu_Ip_GetCaptureRegisterValue(uint8 instance, uint8 hwChannel)
{
    return (ftmIcuBase[instance]->CONTROLS[hwChannel].CV);
}
#endif

/**
 * @brief 
 * 
 * @param instance 
 * @param userConfig 
 * @return Ftm_Icu_Ip_StatusType
 * @implements  Ftm_Icu_Ip_Init_Activity
 */
Ftm_Icu_Ip_StatusType Ftm_Icu_Ip_Init (uint8 instance, const  Ftm_Icu_Ip_ConfigType *userConfig)
{
    uint8    index;
    uint8    hwChannel;
    Ftm_Icu_Ip_ModeType     ipMeasurementMode;
    Ftm_Icu_Ip_BaseType*    ftmBase   = ftmIcuBase[instance];
    Ftm_Icu_Ip_StatusType   retStatus = FTM_IP_STATUS_SUCCESS;

#if (STD_ON == FTM_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(instance < FTM_INSTANCE_COUNT);
    DevAssert(userConfig != NULL_PTR);
#endif 

    if (FALSE == Ftm_Icu_Ip_InsState[instance].instInit)
    {
        Call_Ftm_Icu_SetUserAccessAllowed((uint32)ftmIcuBase[instance]);
        
        Ftm_Icu_Ip_GlobalConfiguration(instance, userConfig->pInstanceConfig->maxCountValue);
        Ftm_Icu_Ip_InsState[instance].prescaler = userConfig->pInstanceConfig->cfgPrescaler;
        Ftm_Icu_Ip_InsState[instance].spuriousMask = (uint8)0U;
#if (FTM_ICU_DUAL_CLOCK_MODE_API == STD_ON)
        Ftm_Icu_Ip_InsState[instance].prescalerAlternate = userConfig->pInstanceConfig->cfgAltPrescaler;
#endif
        for (index = 0U; index < userConfig->nNumChannels; index++)
        {
            hwChannel   = (*userConfig->pChannelsConfig)[index].hwChannel;

            Ftm_Icu_Ip_ChState[instance][hwChannel].ftmChannelNotification  = (*userConfig->pChannelsConfig)[index].ftmChannelNotification;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftmOverflowNotification = (*userConfig->pChannelsConfig)[index].ftmOverflowNotification;
            Ftm_Icu_Ip_ChState[instance][hwChannel].callback                = (*userConfig->pChannelsConfig)[index].callback;
            Ftm_Icu_Ip_ChState[instance][hwChannel].callbackParam           = (*userConfig->pChannelsConfig)[index].callbackParams;

            /* Set the event which will generate the interrupt */
            Ftm_Icu_Ip_ChState[instance][hwChannel].edgeTrigger       = (*userConfig->pChannelsConfig)[index].edgeAlignement;
            /* Save suboperation mode. */
            Ftm_Icu_Ip_ChState[instance][hwChannel].dmaMode   = (*userConfig->pChannelsConfig)[index].chSubMode;
#if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)
            SignalMeasurementEdgeAlign[instance][hwChannel] = (*userConfig->pChannelsConfig)[index].edgeAlignement;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aPeriod = (uint16)0U;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aActivePulseWidth = (uint16)0U;
#endif

#if (STD_ON == FTM_ICU_TIMESTAMP_API)
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBuffer = NULL_PTR;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferSize = (uint16)0U;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferNotify = (uint16)0U;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aNotifyCount = (uint16)0U;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferIndex = (uint16)0U;
            Ftm_Icu_Ip_ChState[instance][hwChannel].timestampBufferType    = (*userConfig->pChannelsConfig)[index].timestampBufferType;
#endif /* STD_ON == FTM_ICU_TIMESTAMP_API */

            Ftm_Icu_Ip_ChState[instance][hwChannel].logicChStateCallback = (*userConfig->pChannelsConfig)[index].logicChStateCallback;
            /* Initialize with the reset value the channel status and control register. */
            ftmBase->CONTROLS[hwChannel].CSC  = (uint32) 0x0U;

            ipMeasurementMode = (*userConfig->pChannelsConfig)[index].chMode;
            switch(ipMeasurementMode)
            {
#if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)
                case FTM_ICU_MODE_SIGNAL_MEASUREMENT:
                    {
                        /* Save what type of signal measuremet will start. */
                        Ftm_Icu_Ip_ChState[instance][hwChannel].measurement = (*userConfig->pChannelsConfig)[index].measurementMode;
                        /* Save measurement mode in channel state. */
                        Ftm_Icu_Ip_ChState[instance][hwChannel].channelMode = FTM_ICU_MODE_SIGNAL_MEASUREMENT;
                    }
                    break;
#endif
#if (STD_ON == FTM_ICU_EDGE_DETECT_API)
                case FTM_ICU_MODE_SIGNAL_EDGE_DETECT:
                    {
                        /* Disable the dual edge mode. */
                        ftmBase->COMBINE &= ~(uint32)FTM_COMBINE_COMBINEx_MASK_U32(hwChannel);
                        /* Set non-continuouse mode */
                        ftmBase->CONTROLS[hwChannel].CSC &= ~((uint32)FTM_CSC_MSB_MASK | (uint32)FTM_CSC_MSA_MASK);
                        /* Save measurement mode in channel state. */
                        Ftm_Icu_Ip_ChState[instance][hwChannel].channelMode = FTM_ICU_MODE_SIGNAL_EDGE_DETECT;
                    }
                    break;
#endif
#if (STD_ON == FTM_ICU_TIMESTAMP_API)
                case FTM_ICU_MODE_TIMESTAMP:
                    {
                        /* Verify if channel supports DMA. */
                        /* Save measurement mode in channel state. */
                        Ftm_Icu_Ip_ChState[instance][hwChannel].channelMode = FTM_ICU_MODE_TIMESTAMP;
                    }
                    break;
#endif
#if (STD_ON == FTM_ICU_EDGE_COUNT_API)
                case FTM_ICU_MODE_EDGE_COUNTER:
                    {
                        /* Save measurement mode in channel state. */
                        Ftm_Icu_Ip_ChState[instance][hwChannel].channelMode = FTM_ICU_MODE_EDGE_COUNTER;
                    }
                    break;
#endif
                default:
                    {
                        /* Do nothing. */
                    }
                    break;
            }
            /* Enable filtering for input channels */
            if (hwChannel < CHAN4_IDX)
            {
                Ftm_Icu_Ip_SetChnInputCaptureFilter(ftmBase, (uint8)hwChannel, (*userConfig->pChannelsConfig)[index].filterValue);
    #if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)
                if ((FTM_ICU_PERIOD_TIME != Ftm_Icu_Ip_ChState[instance][hwChannel].measurement)&&\
                   (FTM_ICU_MODE_SIGNAL_MEASUREMENT == (*userConfig->pChannelsConfig)[index].chMode))
                {
                    Ftm_Icu_Ip_SetChnInputCaptureFilter(ftmBase, (uint8)(hwChannel + 1U), (*userConfig->pChannelsConfig)[index].filterValue);
                }
    #endif
            }
#if (STD_ON == FTM_ICU_TIMESTAMP_USES_DMA)
            /* Set DMA property for the channel*/
            ftmBase->CONTROLS[hwChannel].CSC |= FTM_CSC_DMA((uint32)Ftm_Icu_Ip_ChState[instance][hwChannel].dmaMode);
#endif
            Ftm_Icu_Ip_SetActivationCondition(instance, hwChannel, (Ftm_Icu_Ip_EdgeType)Ftm_Icu_Ip_ChState[instance][hwChannel].edgeTrigger);
        }

        /* Set debug mode. */
        ftmBase->CONF |= ((uint32)userConfig->pInstanceConfig->debugMode << FTM_CONF_BDMMODE_SHIFT);
        /* Set clock source to start the counter */
        Ftm_Icu_Ip_SetClock(instance, userConfig->pInstanceConfig->cfgClkSrc, userConfig->pInstanceConfig->cfgPrescaler);
        Ftm_Icu_Ip_InsState[instance].instInit = TRUE;
    }
    else
    {
        /* instance already initialized - use deinitialize first */
        retStatus = FTM_IP_STATUS_ERROR;
    }
    return retStatus;
}


/**
 * @brief FTM IP function that sets up the activation condition.
 * 
 * @param instance 
 * @param hwChannel 
 * @param activation 
 * @implements Ftm_Icu_Ip_SetActivationCondition_Activity
 */
void Ftm_Icu_Ip_SetActivationCondition (uint8 instance, uint8 hwChannel, Ftm_Icu_Ip_EdgeType activation)
{
    Ftm_Icu_Ip_ChState[instance][hwChannel].edgeTrigger = activation;
    /* Set activation condition in the Channel Status and Control Register (CSCR) */
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_19();
    ftmIcuBase[instance]->CONTROLS[hwChannel].CSC &= ~(FTM_CSC_ELSB_MASK | FTM_CSC_ELSA_MASK);
    ftmIcuBase[instance]->CONTROLS[hwChannel].CSC |= ((FTM_CSC_ELSB_MASK | FTM_CSC_ELSA_MASK) & \
                                                    (uint32)((uint32)activation << FTM_CSC_ELSA_SHIFT));
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_19();
}

#if ((FTM_ICU_OVERFLOW_NOTIFICATION_API == STD_OFF) && (FTM_ICU_IP_DEV_ERROR_DETECT == STD_ON))
#if ((FTM_ICU_EDGE_COUNT_API == STD_ON) || (FTM_ICU_TIMESTAMP_API == STD_ON) || \
     (FTM_ICU_GET_TIME_ELAPSED_API == STD_ON) || (FTM_ICU_GET_DUTY_CYCLE_VALUES_API == STD_ON))
/**
 * @brief Ftm IP function that get the state of the overflow flag
 * 
 * @param instance 
 * 
 * @return      boolean      the state of the overflow flag
 * @retval      TRUE         the overflow flag is set
 * @retval      FALSE        the overflow flag is not set
 */
boolean Ftm_Icu_Ip_GetOverflow(uint8 instance)
{
    /* Get timer overflow flag */
    uint32 statusTOF = ftmIcuBase[instance]->SC & FTM_SC_TOF_MASK;

    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_20();
    /* Clear overflow flag */
    ftmIcuBase[instance]->SC = ftmIcuBase[instance]->SC & (~FTM_SC_TOF_MASK);
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_20();

    /* Get and return the state of the overflow flag */
    return (FTM_SC_TOF_MASK == (statusTOF & FTM_SC_TOF_MASK));
}
#endif
#endif /* (FTM_ICU_OVERFLOW_NOTIFICATION_API == STD_OFF) && (FTM_ICU_IP_DEV_ERROR_DETECT == STD_ON) */

#if (STD_ON == FTM_ICU_GET_INPUT_LEVEL_API)
/**
 * @brief This function returns the actual status of PIN.
 * 
 * @param instance 
 * @param hwChannel 
 * @return Ftm_Icu_Ip_LevelType 
 * @implements Ftm_Icu_Ip_GetInputLevel_Activity
 */
Ftm_Icu_Ip_LevelType Ftm_Icu_Ip_GetInputLevel(uint8 instance, uint8 hwChannel)
{
#if (STD_ON == FTM_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(instance < FTM_INSTANCE_COUNT);
    DevAssert(hwChannel < FTM_CONTROLS_COUNT);
    DevAssert(Ftm_Icu_Ip_ChState[instance][hwChannel].channelMode != FTM_ICU_MODE_NO_MEASUREMENT);
#endif
    /* Return input level. */
    return (((ftmIcuBase[instance]->CONTROLS[hwChannel].CSC & FTM_CSC_CHIS_MASK) == FTM_CSC_CHIS_MASK) ? \
            FTM_ICU_LEVEL_HIGH : FTM_ICU_LEVEL_LOW);
}
#endif /* STD_ON == FTM_ICU_GET_INPUT_LEVEL_API */

#if (STD_ON == FTM_ICU_DEINIT_API)
/**
 * @brief  FTM IP function that de-initialize a specific channel
 * 
 * @param instance
 *
 * @implements Ftm_Icu_Ip_DeInit_Activity
 */
Ftm_Icu_Ip_StatusType Ftm_Icu_Ip_DeInit (uint8 instance)
{
    /* Logical channel. */
    uint8 channel;
    Ftm_Icu_Ip_StatusType   retStatus = FTM_IP_STATUS_SUCCESS;

    if (TRUE == Ftm_Icu_Ip_InsState[instance].instInit)
    {
        /* Reset global configuration. */
        Ftm_Icu_Ip_GlobalConfiguration(instance, (uint8)0U);
        Ftm_Icu_Ip_InsState[instance].instInit = FALSE;
        Ftm_Icu_Ip_InsState[instance].prescaler = (uint8)0U;
        Ftm_Icu_Ip_InsState[instance].prescalerAlternate = (uint8)0U;
        Ftm_Icu_Ip_InsState[instance].spuriousMask = (uint8)0U;

        /* Stop clock. */
        Ftm_Icu_Ip_SetClock(instance, FTM_NO_CLOCK_SELECTED, (uint8)0U);

        for (channel = 0U; channel < FTM_ICU_IP_NUM_OF_CHANNELS; channel++)
        {
            /* Clear filter for each chanel that support this feature. */
            if (channel < (uint8)FTM_FILTER_MAX_NO_CH)
            {
                ftmIcuBase[instance]->FILTER &= ~((uint32)FTM_FILTER_CH0FVAL_MASK << ((uint32)4U * (uint32)channel));
            }
            /* Reset Channel Value, Channel Status And Control and Counter Registers (CV, CSC, CNT) */
            ftmIcuBase[instance]->CONTROLS[channel].CV  = 0U;
            ftmIcuBase[instance]->CONTROLS[channel].CSC = 0U;
            ftmIcuBase[instance]->CNT                   = 0U;

            /* Reset BDMMODE bits. */
            ftmIcuBase[instance]->CONF &= ~(FTM_CONF_BDMMODE_MASK);

            /* Clear COMBINE bit for the FTM channel. */
            if(0U == (channel % 2U))
            {
                ftmIcuBase[instance]->COMBINE &= ~((uint32)FTM_COMBINE_COMBINE0_MASK << ((uint32)8U * ((uint32)channel / (uint32)2U)));
            }
            /* Clear channel state. */
            Ftm_Icu_Ip_ClearChState(instance, channel);
       }
    }
    else
    {
        /* instance already de-initialize - use initialize first */
        retStatus = FTM_IP_STATUS_ERROR;
    }
    return retStatus;

}
#endif /* STD_ON == FTM_ICU_DEINIT_API */

#if (STD_ON == FTM_ICU_SET_MODE_API)
/**
 * @brief 
 * 
 * @param instance 
 * @param hwChannel 
 */
void Ftm_Icu_Ip_SetSleepMode(uint8 instance, uint8 hwChannel)
{
    /* Disable FTM mod on the output pin. */
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_21();
    ftmIcuBase[instance]->CONTROLS[hwChannel].CSC &= ~(FTM_CSC_ELSB_MASK | FTM_CSC_ELSA_MASK);
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_21();
}

/**
 * @brief 
 * 
 * @param instance 
 * @param hwChannel 
 */
void Ftm_Icu_Ip_SetNormalMode(uint8 instance, uint8 hwChannel)
{
    /* Get edge activation mode. */
    Ftm_Icu_Ip_EdgeType edgeTrigger = Ftm_Icu_Ip_ChState[instance][hwChannel].edgeTrigger;
    if (FTM_ICU_NO_PIN_CONTROL != edgeTrigger)
    {
        /* Set the FTM channel to the configured activation type. */
        SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_22();
        ftmIcuBase[instance]->CONTROLS[hwChannel].CSC |= ((FTM_CSC_ELSB_MASK | FTM_CSC_ELSA_MASK) & \
                                                    ((uint32)edgeTrigger << FTM_CSC_ELSA_SHIFT));
        SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_22();
    }
}
#endif  /* STD_ON == FTM_ICU_SET_MODE_API */

#if (STD_ON == FTM_ICU_EDGE_DETECT_API)
/**
 * @brief 
 * 
 * @param instance 
 * @param hwChannel 
 * @implements Ftm_Icu_Ip_EnableEdgeDetection_Activity
 */
void Ftm_Icu_Ip_EnableEdgeDetection(uint8 instance, uint8 hwChannel)
{
    /* Enable interrupts on the FTM channel. */
    /* Clear pending interrupt serviced. */
    Ftm_Icu_Ip_ClearInterruptFlags(instance, hwChannel);
    /* Set activation condition and start. */
    Ftm_Icu_Ip_SetActivationCondition(instance, hwChannel, (Ftm_Icu_Ip_EdgeType)Ftm_Icu_Ip_ChState[instance][hwChannel].edgeTrigger);

    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_23();
    ftmIcuBase[instance]->CONTROLS[hwChannel].CSC |= FTM_CSC_CHIE_MASK;
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_23();
}
#endif /* ICU_EDGE_DETECT_API */

#if (STD_ON == FTM_ICU_EDGE_DETECT_API)
/**
 * @brief 
 * 
 * @param instance 
 * @param hwChannel 
 * @implements Ftm_Icu_Ip_DisableEdgeDetection_Activity
 */
void Ftm_Icu_Ip_DisableEdgeDetection(uint8 instance, uint8 hwChannel)
{
    /* Disable interrupts on the FTM channel. */
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_24();
    ftmIcuBase[instance]->CONTROLS[hwChannel].CSC &= (~FTM_CSC_CHIE_MASK);
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_24();
}
#endif /* STD_ON == FTM_ICU_EDGE_DETECT_API */

/**
 * @brief Enable channel interrupt.
 * 
 * @param[in] instance   Hardware instance of FTM used. 
 * @param[in] hwChannel  Hardware channel of FTM used.
 * @return   void.
 * @implements Ftm_Icu_Ip_EnableInterrupt_Activity
 */
void Ftm_Icu_Ip_EnableInterrupt(uint8 instance, uint8 hwChannel)
{
    ftmIcuBase[instance]->CONTROLS[hwChannel].CSC &= (~(FTM_CSC_CHIE_MASK | FTM_CSC_CHF_MASK));
    ftmIcuBase[instance]->CONTROLS[hwChannel].CSC |= FTM_CSC_CHIE_MASK;
}

/**
 * @brief Disable channel interrupt.
 * 
 * @param[in] instance   Hardware instance of FTM used. 
 * @param[in] hwChannel  Hardware channel of FTM used.
 * @return   void.
 * @implements Ftm_Icu_Ip_DisableInterrupt_Activity
 */
void Ftm_Icu_Ip_DisableInterrupt(uint8 instance, uint8 hwChannel)
{
    ftmIcuBase[instance]->CONTROLS[hwChannel].CSC &= (~(FTM_CSC_CHIE_MASK | FTM_CSC_CHF_MASK));
}

#if (STD_ON == FTM_ICU_TIMESTAMP_API)
/** @implements Ftm_Icu_Ip_StartTimestamp_Activity */
void Ftm_Icu_Ip_StartTimestamp(uint8   instance,
                               uint8   hwChannel,
                               Ftm_Icu_ValueType* bufferPtr,
                               uint16  bufferSize,
                               uint16  notifyInterval)
{
#if (STD_ON == FTM_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(FTM_INSTANCE_COUNT > instance);
    DevAssert(FTM_ICU_IP_NUM_OF_CHANNELS > hwChannel);
    DevAssert(NULL_PTR != ftmIcuBase[instance]);
#endif
    /* Reset bufferPtr. */
    Ftm_Icu_Ip_aBufferPtr[instance][hwChannel] = 0U;
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_25();
    Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBuffer = bufferPtr;
    Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferSize = bufferSize;
    Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferNotify = notifyInterval;
    Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aNotifyCount = (uint16)0U;
    Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferIndex = (uint16)0U;
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_25();

    /* Clear pending interrupt serviced. */
    Ftm_Icu_Ip_ClearInterruptFlags(instance, hwChannel);
    /* Configure the Timestamp mode for the Ftm channel in the configuration array */
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_25();
    {
#if defined(FTM_ICU_OVERFLOW_NOTIFICATION_API) && (FTM_ICU_OVERFLOW_NOTIFICATION_API == STD_ON)
        /* Set Timer Overflow Interrupt Enable. */
        ftmIcuBase[instance]->SC = ftmIcuBase[instance]->SC | FTM_SC_TOIE_MASK;
#endif
        /* Enable interrupts on the FTM channel. */
        ftmIcuBase[instance]->CONTROLS[hwChannel].CSC |= FTM_CSC_CHIE_MASK;
    }
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_25();
    /* Set activation condition and start. */
    Ftm_Icu_Ip_SetActivationCondition(instance, hwChannel, (Ftm_Icu_Ip_EdgeType)Ftm_Icu_Ip_ChState[instance][hwChannel].edgeTrigger);
}

/**
 * @brief Stop a channel which run in timestamp mode.
 * 
 * @param instance  - FTM instance where the current channel is located.
 * @param hwChannel - FTM hardware channel used.
 * @implements Ftm_Icu_Ip_StopTimestamp_Activity
 */
void Ftm_Icu_Ip_StopTimestamp(uint8 instance, uint8 hwchannel)
{
#if (STD_ON == FTM_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(FTM_INSTANCE_COUNT > instance);
    DevAssert(FTM_ICU_IP_NUM_OF_CHANNELS > hwchannel);
    DevAssert(NULL_PTR != ftmIcuBase[instance]);
    DevAssert(Ftm_Icu_Ip_ChState[instance][hwchannel].channelMode == FTM_ICU_MODE_TIMESTAMP);
#endif
    /* Stop channel activity. */
    Ftm_Icu_Ip_StopChannel(instance, hwchannel);

    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_26();
    {
        /* Disable Interrupts on the FTM channel. */
        ftmIcuBase[instance]->CONTROLS[hwchannel].CSC &= ~(FTM_CSC_CHIE_MASK);
#if defined(FTM_ICU_OVERFLOW_NOTIFICATION_API) && (STD_ON == FTM_ICU_OVERFLOW_NOTIFICATION_API)
        /* Disable Timer Overflow Interrupt. */
        ftmIcuBase[instance]->SC &= ~(FTM_SC_TOIE_MASK);
#endif
    }
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_26();
}

/** @implements Ftm_Icu_Ip_GetTimestampIndex_Activity */
uint16 Ftm_Icu_Ip_GetTimestampIndex(uint8 instance, uint8 hwChannel)
{
    uint16 timestampIndex = 0U;

#if (STD_ON == FTM_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(instance < FTM_INSTANCE_COUNT);
#endif
    if (NULL_PTR == Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBuffer)
    {
        timestampIndex = 0U;
    }
    else
    {
        timestampIndex = Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferIndex;
    }
    return timestampIndex;
}

#endif  /* STD_ON == FTM_ICU_TIMESTAMP_API */

#if ((STD_ON == FTM_ICU_TIMESTAMP_USES_DMA) && (STD_ON == FTM_ICU_TIMESTAMP_API))
/**
 * @brief 
 * 
 * @param instance 
 * @param hwChannel 
 * @return uint32 
 * @implements Ftm_Icu_Ip_GetStartAddress_Activity
 */
uint32 Ftm_Icu_Ip_GetStartAddress(uint8 instance, uint8 hwChannel)
{
    return (uint32)(&ftmIcuBase[instance]->CONTROLS[hwChannel].CV);
}
#endif

#if (STD_ON == FTM_ICU_EDGE_COUNT_API)
/**
 * @brief FTM IP function that reset the edge counting for FTM channel.
 * 
 * @param instance  - FTM instance where the current channel is located.
 * @param hwChannel - FTM hardware channel used.
 * @implements Ftm_Icu_Ip_ResetEdgeCount_Activity
 */
void Ftm_Icu_Ip_ResetEdgeCount(uint8 instance, uint8 hwchannel)
{
    Ftm_Icu_Ip_ChState[instance][hwchannel].edgeNumbers = 0U;
}
#endif  /* STD_ON == FTM_ICU_EDGE_COUNT_API */

#if (STD_ON == FTM_ICU_EDGE_COUNT_API)
/**
 * @brief 
 * 
 * @param instance 
 * @param hwChannel
 * @implements Ftm_Icu_Ip_EnableEdgeCount_Activity
 */
Ftm_Icu_Ip_StatusType Ftm_Icu_Ip_EnableEdgeCount(uint8 instance, uint8 hwChannel)
{
    Ftm_Icu_Ip_ChState[instance][hwChannel].edgeNumbers = 0U;

    /* Clear interrupt flags. */
    Ftm_Icu_Ip_ClearInterruptFlags(instance, hwChannel);
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_26();
    {
#if defined(FTM_ICU_OVERFLOW_NOTIFICATION_API) && (FTM_ICU_OVERFLOW_NOTIFICATION_API == STD_ON)
        /* Set TOF interrupt enable. */
        ftmIcuBase[instance]->SC |= FTM_SC_TOIE_MASK;
#endif
        /* Enable Interrupts on the FTM channel. */
        ftmIcuBase[instance]->CONTROLS[hwChannel].CSC |= FTM_CSC_CHIE_MASK;
    }
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_26();

    /* Set activation condition and start. */
    Ftm_Icu_Ip_SetActivationCondition(instance, hwChannel, (Ftm_Icu_Ip_EdgeType)Ftm_Icu_Ip_ChState[instance][hwChannel].edgeTrigger);
    return FTM_IP_STATUS_SUCCESS;
}
#endif  /* STD_ON == FTM_ICU_EDGE_COUNT_API */

#if (STD_ON == FTM_ICU_EDGE_COUNT_API)
/**
* @brief      Ftm_Icu_Ip_DisableEdgeCount
* @details    Ftm IP function that disable the edge counting service for an Ftm channel
*
* @param[in]      hwChannel   - Ftm encoded hardware channel
*
* @return Ftm_Icu_Ip_StatusType
*
* @implements Ftm_Icu_Ip_DisableEdgeCount_Activity
*/
void Ftm_Icu_Ip_DisableEdgeCount(uint8 instance, uint8 hwChannel)
{
    Ftm_Icu_Ip_StopChannel(instance, hwChannel);

    /* Disable Interrupts on the FTM channel. */
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_27();
    {
        ftmIcuBase[instance]->CONTROLS[hwChannel].CSC &= (~FTM_CSC_CHIE_MASK);
#if defined(FTM_ICU_OVERFLOW_NOTIFICATION_API) && (FTM_ICU_OVERFLOW_NOTIFICATION_API == STD_ON)
        /* Disable Timer Overflow Interrupt. */
        ftmIcuBase[instance]->SC &= (~FTM_SC_TOIE_MASK);
#endif
    }
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_27();
}
#endif  /* STD_ON == FTM_ICU_EDGE_COUNT_API */

#if (STD_ON == FTM_ICU_EDGE_COUNT_API)
/**
 * @brief FTM IP function that get the edges number.
 * 
 * @param instance 
 * @param hwChannel 
 * @return uint16 
 * @implements Ftm_Icu_Ip_GetEdgeNumbers_Activity
 */
uint16 Ftm_Icu_Ip_GetEdgeNumbers(uint8 instance, uint8 hwChannel)
{
    return Ftm_Icu_Ip_ChState[instance][hwChannel].edgeNumbers;
}
#endif  /* STD_ON == FTM_ICU_EDGE_COUNT_API */

#if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)
/** @implements Ftm_Icu_Ip_StartSignalMeasurement_Activity */
void Ftm_Icu_Ip_StartSignalMeasurement(uint8 instance, uint8 hwchannel)
{
    Ftm_Icu_Ip_MeasType measProperty = Ftm_Icu_Ip_ChState[instance][hwchannel].measurement;
    /* Find the pair channel of the current channel. */
    uint8 channelPair = (uint8)((0U == (hwchannel % 2U)) ? (hwchannel / 2U) : ((hwchannel - 1U) / 2U));

    Ftm_Icu_Ip_ChState[instance][hwchannel].ftm_Icu_Ip_aPeriod = (uint16)0U;
    Ftm_Icu_Ip_ChState[instance][hwchannel].ftm_Icu_Ip_aActivePulseWidth = (uint16)0U;
    /* Clear status of first edge come to measurement in BOTH_EDGE mode for new session. */
    Ftm_Icu_Ip_ChState[instance][hwchannel].firstEdgePolarity = FALSE;

    if (FTM_ICU_PERIOD_TIME == measProperty)
    {
        
        /* Clear interrupt flags. */
        Ftm_Icu_Ip_ClearInterruptFlags(instance, hwchannel);
        /* Set continuous capture mode - single edge capture. */
        SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_28();
        {
            ftmIcuBase[instance]->CONTROLS[hwchannel].CSC &= ~(FTM_CSC_MSA_MASK | FTM_CSC_MSB_MASK);
            /* Clear Dual Edge Capture on current channel pair. */
            ftmIcuBase[instance]->COMBINE &= ~((uint32)FTM_COMBINE_DECAPEN0_MASK << ((uint32)8U * (uint32)channelPair));
            ftmIcuBase[instance]->COMBINE &= ~((uint32)FTM_COMBINE_DECAP0_MASK << ((uint32)8U * (uint32)channelPair));
            /* Enable channel interrupt. */
            ftmIcuBase[instance]->CONTROLS[hwchannel].CSC |= FTM_CSC_CHIE_MASK;
#if defined(FTM_ICU_OVERFLOW_NOTIFICATION_API) && (FTM_ICU_OVERFLOW_NOTIFICATION_API == STD_ON)
            /* Set Timer Overflow Interrupt Enable */
            ftmIcuBase[instance]->SC |= FTM_SC_TOIE_MASK;
#endif
        }
        SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_28();
        /* Update status of first signal measurement. */
        Ftm_Icu_Ip_ChState[instance][hwchannel].firstEdge = TRUE;
    }
    else
    {
        /* Clear interrupt flags. */
        Ftm_Icu_Ip_ClearInterruptFlags(instance, hwchannel);
        SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_28();
        {
            if(FTM_ICU_DUTY_CYCLE == measProperty)
            {
                /* Enable first channel interrupt. */
                ftmIcuBase[instance]->CONTROLS[hwchannel].CSC |= FTM_CSC_CHIE_MASK;
                /* Update status of first signal measurement. */
                Ftm_Icu_Ip_ChState[instance][hwchannel].firstEdge = TRUE;
                /* Set continuous capture mode - single edge capture. */
                ftmIcuBase[instance]->CONTROLS[hwchannel].CSC &= ~(FTM_CSC_MSA_MASK | FTM_CSC_MSB_MASK);
                /* Clear Dual Edge Capture on current channel pair. */
                ftmIcuBase[instance]->COMBINE &= ~((uint32)FTM_COMBINE_DECAPEN0_MASK << ((uint32)8U * (uint32)channelPair));
                ftmIcuBase[instance]->COMBINE &= ~((uint32)FTM_COMBINE_DECAP0_MASK << ((uint32)8U * (uint32)channelPair));
            }
            else
            {
            /* Set continuous capture mode*/
            ftmIcuBase[instance]->CONTROLS[hwchannel].CSC |= FTM_CSC_MSA_MASK;
            /* Set Dual Edge Capture on current channel pair. */
            ftmIcuBase[instance]->COMBINE |= ((uint32)FTM_COMBINE_DECAPEN0_MASK << ((uint32)8U * (uint32)channelPair));
            ftmIcuBase[instance]->COMBINE |= ((uint32)FTM_COMBINE_DECAP0_MASK << ((uint32)8U * (uint32)channelPair));
            /* Set Interrupt enable flag for the second channel. */
            ftmIcuBase[instance]->CONTROLS[(hwchannel + 1U)].CSC |= FTM_CSC_CHIE_MASK;
        }
#if defined(FTM_ICU_OVERFLOW_NOTIFICATION_API) && (FTM_ICU_OVERFLOW_NOTIFICATION_API == STD_ON)
            /* Set Timer Overflow Interrupt Enable */
            ftmIcuBase[instance]->SC |= FTM_SC_TOIE_MASK;
#endif
        }
        SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_28();

        /* Save second channel state properties. */
        Ftm_Icu_Ip_ChState[instance][(hwchannel + 1U)].measurement = measProperty;
        Ftm_Icu_Ip_ChState[instance][(hwchannel + 1U)].channelMode = FTM_ICU_MODE_SIGNAL_MEASUREMENT;
        /* Set activation condition for the second FTM channel. */
        switch(SignalMeasurementEdgeAlign[instance][hwchannel])
        {
            case FTM_ICU_FALLING_EDGE:
            {
                Ftm_Icu_Ip_SetActivationCondition(instance, (uint8)(hwchannel + 1U), FTM_ICU_RISING_EDGE);
            }
            break;
            case FTM_ICU_RISING_EDGE:
            {
                Ftm_Icu_Ip_SetActivationCondition(instance, (uint8)(hwchannel + 1U), FTM_ICU_FALLING_EDGE);
            }
            break;
            default:
            {
                Ftm_Icu_Ip_SetActivationCondition(instance, (uint8)(hwchannel + 1U), FTM_ICU_BOTH_EDGES);
            }
            break;
        }
    }
    /* Start the first channel. */
    Ftm_Icu_Ip_SetActivationCondition(instance, hwchannel, SignalMeasurementEdgeAlign[instance][hwchannel]);
}

/** @implements Ftm_Icu_Ip_GetTimeElapsed_Activity */
uint16 Ftm_Icu_Ip_GetTimeElapsed(uint8 instance, uint8 hwChannel)
{
    uint16 timeElapsed = (uint16)0U;

#if (STD_ON == FTM_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(instance < FTM_INSTANCE_COUNT);
#endif

    if((FTM_ICU_DUTY_CYCLE != Ftm_Icu_Ip_ChState[instance][hwChannel].measurement) && \
        (FTM_ICU_NO_MEASUREMENT != Ftm_Icu_Ip_ChState[instance][hwChannel].measurement))
    {
        if ((Ftm_Icu_Ip_MeasType)FTM_ICU_PERIOD_TIME == Ftm_Icu_Ip_ChState[instance][hwChannel].measurement)
        {
            timeElapsed = Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aPeriod;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aPeriod = (uint16)0U;
        }
        else
        {
            timeElapsed = Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aActivePulseWidth;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aActivePulseWidth = (uint16)0U;
        }
    }
    #if (STD_ON == FTM_ICU_IP_DEV_ERROR_DETECT)
    else
    {
        DevAssert(TRUE);
    }
    #endif
    return timeElapsed;
}

/** @implements Ftm_Icu_Ip_GetDutyCycleValues_Activity */
void Ftm_Icu_Ip_GetDutyCycleValues(uint8 instance,
                                   uint8 hwChannel,
                                   Ftm_Icu_Ip_DutyCycleType* dutyCycleValues)
{
#if (STD_ON == FTM_ICU_IP_DEV_ERROR_DETECT)
    DevAssert(instance < FTM_INSTANCE_COUNT);
#endif

    if (FTM_ICU_DUTY_CYCLE == Ftm_Icu_Ip_ChState[instance][hwChannel].measurement)
    {
        if ((uint16)0U != Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aPeriod)
        {
            dutyCycleValues->ActiveTime = (uint16)Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aActivePulseWidth;
            dutyCycleValues->PeriodTime = (uint16)Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aPeriod;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aActivePulseWidth = (uint16)0U;
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aPeriod           = (uint16)0U;
        }
        else
        {
            dutyCycleValues->ActiveTime = (uint16)0U;
            dutyCycleValues->PeriodTime = (uint16)0U;
        }
    }
    #if (STD_ON == FTM_ICU_IP_DEV_ERROR_DETECT)
    else
    {
        DevAssert(TRUE);
    }
    #endif 
}

#endif  /* STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API */

#if (STD_ON == FTM_ICU_GET_INPUT_STATE_API)
/**
 * @brief Return input state of the channel.
 * 
 * @param instance 
 * @param hwChannel 
 * @return boolean 
 */
boolean Ftm_Icu_Ip_GetInputState(uint8 instance, uint8 hwChannel)
{
    boolean result = FALSE;
    /* Get value from status and control register. */
    uint32  statusRegister = ftmIcuBase[instance]->CONTROLS[hwChannel].CSC;

    /* Interrupt not enabled, flag bit was set. */
    if (FTM_CSC_CHIE_MASK != (FTM_CSC_CHIE_MASK & statusRegister))
    {
        if (FTM_CSC_CHF_MASK == (FTM_CSC_CHF_MASK & statusRegister))
        {
            result = TRUE;
            /* Clear interrupt flag. */
            SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_29();
            ftmIcuBase[instance]->CONTROLS[hwChannel].CSC &= ~(FTM_CSC_CHF_MASK);
            SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_29();
        }
    }
    return result;
}
#endif /* STD_ON == FTM_ICU_GET_INPUT_STATE_API */

#if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)
/**
 * @brief FTM IP function which stops the signal measurement for a FTM channel.
 * 
 * @param instance 
 * @param hwChannel 
 * @implements Ftm_Icu_Ip_StopSignalMeasurement_Activity
 */
void Ftm_Icu_Ip_StopSignalMeasurement(uint8 instance, uint8 hwchannel)
{
    /* Stop Signal Measurement Mode for both channels used. */
    Ftm_Icu_Ip_StopChannel(instance, hwchannel);
    Ftm_Icu_Ip_StopChannel(instance, (uint8)(hwchannel + 1U));
    
    /* Clear status of first edge come to measurement in BOTH_EDGE mode for new session. */
    Ftm_Icu_Ip_ChState[instance][hwchannel].firstEdgePolarity = FALSE;
    
    Ftm_Icu_Ip_InsState[instance].spuriousMask &= ~((uint8)(1U << hwchannel));
    
    SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_30();
    {
        /* Clear interrupt and status flags for both channels. */
        ftmIcuBase[instance]->CONTROLS[hwchannel].CSC       &= (~(FTM_CSC_CHIE_MASK | FTM_CSC_CHF_MASK));
        ftmIcuBase[instance]->CONTROLS[(hwchannel + 1U)].CSC &= (~(FTM_CSC_CHIE_MASK | FTM_CSC_CHF_MASK));
#if defined(FTM_ICU_OVERFLOW_NOTIFICATION_API) && (FTM_ICU_OVERFLOW_NOTIFICATION_API == STD_ON)
        /* Disable Timer Overflow Interrupt Enable */
        ftmIcuBase[instance]->SC &= ~FTM_SC_TOIE_MASK;
#endif
    }
    SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_30();
}
#endif  /* STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API */

#if (STD_ON == FTM_ICU_DUAL_CLOCK_MODE_API)
/**
 * @brief This function sets the FTM prescaler.
 * 
 * @param instance 
 * @param prescaler 
 */
void Ftm_Icu_Ip_SetPrescaler(uint8 instance, Ftm_Icu_Ip_ClockModeType selectPrescaler)
{
    uint8 prescaler = (uint8)0U;
    uint8 psValue = (uint8)0U;
    if (FTM_ICU_NORMAL_CLK == selectPrescaler)
    {
        prescaler = Ftm_Icu_Ip_InsState[instance].prescaler;
    }
    else
    {
        prescaler = Ftm_Icu_Ip_InsState[instance].prescalerAlternate;
    }
    for (psValue = 0U; psValue < 8U; psValue++)
    {
        if(prescaler == (((uint8)1U) << psValue))
        {
            break;
        }
    }
    /* Set Prescaler value. */
    ftmIcuBase[instance]->SC &= (~FTM_SC_PS_MASK);
    ftmIcuBase[instance]->SC |= FTM_SC_PS(psValue);
}
#endif /* STD_ON == FTM_ICU_DUAL_CLOCK_MODE_API */

/**
 * @brief      Driver function Enable Notification for timestamp.
 *
 * @param[in]  instance  Hardware instance of FTM used. 
 * @param[in]  hwChannel Hardware channel of FTM used.
 * @return     void
 * @implements Ftm_Icu_Ip_EnableNotification_Activity
 */
void Ftm_Icu_Ip_EnableNotification(uint8 instance, uint8 hwChannel)
{
    Ftm_Icu_Ip_ChState[instance][hwChannel].notificationEnable =  TRUE;
}

/**
 * @brief      Driver function Disable Notification for timestamp.
 *
 * @param[in]  instance  Hardware instance of FTM used. 
 * @param[in]  hwChannel Hardware channel of FTM used.
 * @return     void
 * @implements Ftm_Icu_Ip_DisableNotification_Activity
 */
void Ftm_Icu_Ip_DisableNotification(uint8 instance, uint8 hwChannel)
{
    Ftm_Icu_Ip_ChState[instance][hwChannel].notificationEnable =  FALSE;
}

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */
