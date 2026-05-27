/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           : DMA,CACHE,TRGMUX,FLEXIO
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
*   @file CDD_Mcl.c
*
*   @version 2.0.0
*
*   @brief   AUTOSAR Mcl - MCL driver source file.
*   @details
*
*   @addtogroup MCL_DRIVER MCL Driver
*   @{
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
#include "CDD_Mcl.h"
#include "CDD_Mcl_Irq.h"

#if (STD_ON == MCL_DET_IS_AVAILABLE)
#include "Det.h"
#endif

#if (STD_ON == MCL_DEM_IS_AVAILABLE)
#include "Dem.h"
#endif

/*==================================================================================================
 *                                      SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define CDD_MCL_VENDOR_ID_C                       43
#define CDD_MCL_AR_RELEASE_MAJOR_VERSION_C        4
#define CDD_MCL_AR_RELEASE_MINOR_VERSION_C        7
#define CDD_MCL_AR_RELEASE_REVISION_VERSION_C     0
#define CDD_MCL_SW_MAJOR_VERSION_C                2
#define CDD_MCL_SW_MINOR_VERSION_C                0
#define CDD_MCL_SW_PATCH_VERSION_C                0

/*==================================================================================================
 *                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if CDD_Mcl.c file and CDD_Mcl.h file are of the same vendor */
#if (CDD_MCL_VENDOR_ID_C != CDD_MCL_VENDOR_ID)
    #error "CDD_Mcl.c and CDD_Mcl.h have different vendor ids"
#endif

/* Check if CDD_Mcl.c file and CDD_Mcl.h file are of the same Autosar version */
#if ((CDD_MCL_AR_RELEASE_MAJOR_VERSION_C != CDD_MCL_AR_RELEASE_MAJOR_VERSION) || \
     (CDD_MCL_AR_RELEASE_MINOR_VERSION_C != CDD_MCL_AR_RELEASE_MINOR_VERSION) || \
     (CDD_MCL_AR_RELEASE_REVISION_VERSION_C != CDD_MCL_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of CDD_Mcl.c and CDD_Mcl.h are different"
#endif

/* Check if CDD_Mcl.c file and CDD_Mcl.h file are of the same Software version */
#if ((CDD_MCL_SW_MAJOR_VERSION_C != CDD_MCL_SW_MAJOR_VERSION) || \
     (CDD_MCL_SW_MINOR_VERSION_C != CDD_MCL_SW_MINOR_VERSION) || \
     (CDD_MCL_SW_PATCH_VERSION_C != CDD_MCL_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of CDD_Mcl.c and CDD_Mcl.h are different"
#endif

/* Check if CDD_Mcl.c file and CDD_Mcl_Irq.h file are of the same vendor */
#if (CDD_MCL_VENDOR_ID_C != CDD_MCL_IRQ_VENDOR_ID)
    #error "CDD_Mcl.c and CDD_Mcl_Irq.h have different vendor ids"
#endif

/* Check if CDD_Mcl.c file and CDD_Mcl_Irq.h file are of the same Autosar version */
#if ((CDD_MCL_AR_RELEASE_MAJOR_VERSION_C != CDD_MCL_IRQ_AR_RELEASE_MAJOR_VERSION) || \
     (CDD_MCL_AR_RELEASE_MINOR_VERSION_C != CDD_MCL_IRQ_AR_RELEASE_MINOR_VERSION) || \
     (CDD_MCL_AR_RELEASE_REVISION_VERSION_C != CDD_MCL_IRQ_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of CDD_Mcl.c and CDD_Mcl_Irq.h are different"
#endif

/* Check if CDD_Mcl.c file and CDD_Mcl_Irq.h file are of the same Software version */
#if ((CDD_MCL_SW_MAJOR_VERSION_C != CDD_MCL_IRQ_SW_MAJOR_VERSION) || \
     (CDD_MCL_SW_MINOR_VERSION_C != CDD_MCL_IRQ_SW_MINOR_VERSION) || \
     (CDD_MCL_SW_PATCH_VERSION_C != CDD_MCL_IRQ_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of CDD_Mcl.c and CDD_Mcl_Irq.h are different"
#endif

#if (STD_ON == MCL_DET_IS_AVAILABLE)
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
/* Check if CDD_Mcl.c file and Det header file are of the same Autosar version */
#if ((CDD_MCL_AR_RELEASE_MAJOR_VERSION_C != DET_AR_RELEASE_MAJOR_VERSION) || \
     (CDD_MCL_AR_RELEASE_MINOR_VERSION_C != DET_AR_RELEASE_MINOR_VERSION))
    #error "AutoSar Version Numbers of CDD_Mcl.c and Det.h are different"
#endif
#endif
#endif

#if (STD_ON == MCL_DEM_IS_AVAILABLE)
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
/* Check if CDD_Mcl.c file and Dem header file are of the same Autosar version */
#if ((CDD_MCL_AR_RELEASE_MAJOR_VERSION_C != DEM_AR_RELEASE_MAJOR_VERSION) || \
     (CDD_MCL_AR_RELEASE_MINOR_VERSION_C != DEM_AR_RELEASE_MINOR_VERSION))
    #error "AUTOSAR Version Numbers of CDD_Mcl.c and Dem.h are different"
#endif
#endif
#endif

/*==================================================================================================
 *                           LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                          LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
 *                                        LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                        LOCAL VARIABLES
==================================================================================================*/
#define MCL_START_SEC_VAR_CLEARED_UNSPECIFIED_NO_CACHEABLE
#include "Mcl_MemMap.h"

static const Mcl_ConfigType * Mcl_pxConfig;

#define MCL_STOP_SEC_VAR_CLEARED_UNSPECIFIED_NO_CACHEABLE
#include "Mcl_MemMap.h"


/*==================================================================================================
 *                                        GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                        GLOBAL VARIABLES
==================================================================================================*/

#if (STD_ON == MCL_FLEXIO_IS_AVAILABLE)
#define MCL_START_SEC_VAR_INIT_UNSPECIFIED_NO_CACHEABLE
#include "Mcl_MemMap.h"

uint8 Mcl_au8FlexioLogicToHwInstance[FLEXIO_INSTANCE_COUNT];

#define MCL_STOP_SEC_VAR_INIT_UNSPECIFIED_NO_CACHEABLE
#include "Mcl_MemMap.h"
#endif /* FLEXIO_IS_AVAILABLE */

/*==================================================================================================
 *                                    LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

#define MCL_START_SEC_CODE
#include "Mcl_MemMap.h"

#if (STD_ON == MCL_DET_IS_AVAILABLE)
static Mcl_ReturnType Mcl_ValidatePtrInit(const Mcl_ConfigType * const ConfigPtr);
#endif /* STD_ON == MCL_DET_IS_AVAILABLE */

#if (STD_ON == MCL_CACHE_IS_AVAILABLE)
#if (STD_ON == MCL_DEM_IS_AVAILABLE)
static void Mcl_Dem_ReportErrorStatus(const Std_ReturnType Status, const uint32 DemState, uint32 DemId);
#endif /* STD_ON == MCL_DEM_IS_AVAILABLE */
#endif /* STD_ON == MCL_CACHE_IS_AVAILABLE */

#if (STD_ON == MCL_DMA_IS_AVAILABLE)
static void Mcl_DmaReturnCheck(const uint8 Channel, const uint8 Function, const Dma_Ip_ReturnType RetStatus);
#endif /* #if (STD_ON == MCL_DMA_IS_AVAILABLE) */

#define MCL_STOP_SEC_CODE
#include "Mcl_MemMap.h"

/*==================================================================================================
 *                                        LOCAL FUNCTIONS
==================================================================================================*/
#define MCL_START_SEC_CODE
#include "Mcl_MemMap.h"

#if (STD_ON == MCL_DET_IS_AVAILABLE)
static Mcl_ReturnType Mcl_ValidatePtrInit(const Mcl_ConfigType * const ConfigPtr)
{
    Mcl_ReturnType Valid = MCL_STATUS_SUCCESS;

#if (MCL_PRECOMPILE_SUPPORT == STD_OFF)
    if (NULL_PTR == ConfigPtr)
    {
        Valid = MCL_STATUS_ERROR;
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, 0U, MCL_DET_INIT, MCL_E_INIT_FAILED);
    }
    else
    {
        /** do nothing */
    }
#elif (STD_ON == MCL_PRECOMPILE_SUPPORT)
    if (NULL_PTR != ConfigPtr)
    {
        Valid = MCL_STATUS_ERROR;
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, 0U, MCL_DET_INIT, MCL_E_INIT_FAILED);
    }
    else
    {
        /** do nothing */
    }
#endif /* MCL_PRECOMPILE_SUPPORT */

    return Valid;
}
#endif /* STD_ON == MCL_DET_IS_AVAILABLE */

#if (STD_ON == MCL_CACHE_IS_AVAILABLE)
#if (STD_ON == MCL_DEM_IS_AVAILABLE)
static void Mcl_Dem_ReportErrorStatus(const Std_ReturnType Status, const uint32 DemState, uint32 DemId)
{
    if((uint32)STD_ON == DemState)
    {
        if((uint8)E_OK == Status)
        {
            (void)Dem_SetEventStatus((uint16)DemId, DEM_EVENT_STATUS_PASSED);
        }
        else
        {
            (void)Dem_SetEventStatus((uint16)DemId, DEM_EVENT_STATUS_FAILED);
        }
    }
    else
    {
        /* Do nothing */
    }
}
#endif /* STD_ON == MCL_DEM_IS_AVAILABLE */
#endif /* STD_ON == MCL_CACHE_IS_AVAILABLE*/

#if (STD_ON == MCL_DMA_IS_AVAILABLE)
/* Check returned status of the IP-Layer DMA function */
static void Mcl_DmaReturnCheck(const uint8 Channel, const uint8 Function, const Dma_Ip_ReturnType RetStatus)
{
    switch(RetStatus)
    {

#if (STD_ON == MCL_DET_IS_AVAILABLE)
        case DMA_IP_STATUS_ERROR:
            (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, Function, MCL_E_INCONSISTENCY);
        break;
        case DMA_IP_STATUS_WRONG_STATE:
            (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, Function, MCL_E_INVALID_STATE);
        break;
#endif /* #if (STD_ON == MCL_DET_IS_AVAILABLE) */
        case DMA_IP_STATUS_SUCCESS:
            (void)Channel;
            (void)Function;
        break;
        default:
            /* Do nothing... */
        break;
    }
}
#endif /* #if (STD_ON == MCL_DMA_IS_AVAILABLE) */

#define MCL_STOP_SEC_CODE
#include "Mcl_MemMap.h"

/*==================================================================================================
 *                                        GLOBAL FUNCTIONS
==================================================================================================*/
#define MCL_START_SEC_CODE
#include "Mcl_MemMap.h"

void Mcl_Init(const Mcl_ConfigType * const ConfigPtr)
{
#if (STD_ON == MCL_DET_IS_AVAILABLE)
    #if (STD_ON == MCL_DMA_IS_AVAILABLE)
    Dma_Ip_ReturnType DmaIpStatus;
    #endif
    #if (STD_ON == MCL_TRGMUX_IS_AVAILABLE)
    Trgmux_Ip_StatusType TrgmuxIpStatus;
    #endif
    Mcl_ReturnType ValidPtr;

    ValidPtr = Mcl_ValidatePtrInit(ConfigPtr);
    if(MCL_STATUS_SUCCESS == ValidPtr)
    {
#endif /* STD_ON == MCL_DET_IS_AVAILABLE */
        if(NULL_PTR == Mcl_pxConfig)
        {
            #if (STD_ON == MCL_PRECOMPILE_SUPPORT)
                Mcl_pxConfig = &Mcl_Config;
                (void)ConfigPtr;
            #else
                Mcl_pxConfig = ConfigPtr;
            #endif /* STD_ON == MCL_PRECOMPILE_SUPPORT */
                if(NULL_PTR != Mcl_pxConfig)
                {
                #if (STD_ON == MCL_DMA_IS_AVAILABLE)
                    if(NULL_PTR != Mcl_pxConfig->pxDmaInitType)
                    {
                    #if (STD_ON == MCL_DET_IS_AVAILABLE)
                        DmaIpStatus = Dma_Ip_Init(Mcl_pxConfig->pxDmaInitType);
                        if(DMA_IP_STATUS_SUCCESS != DmaIpStatus)
                        {
                            (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)0U, MCL_DET_INIT, MCL_E_UNINIT);
                        }
                    #else
                        (void)Dma_Ip_Init(Mcl_pxConfig->pxDmaInitType);
                    #endif
                    }
                #endif /* #if (STD_ON == MCL_DMA_IS_AVAILABLE) */
                #if (STD_ON == MCL_TRGMUX_IS_AVAILABLE)
                    if(NULL_PTR != Mcl_pxConfig->pxTrgmuxInitType)
                    {
                    #if (STD_ON == MCL_DET_IS_AVAILABLE)
                        TrgmuxIpStatus = Trgmux_Ip_Init(Mcl_pxConfig->pxTrgmuxInitType);
                        if(TRGMUX_IP_STATUS_SUCCESS != TrgmuxIpStatus)
                        {
                            (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)0U, MCL_DET_INIT, MCL_E_UNINIT);
                        }
                    #else
                        (void)Trgmux_Ip_Init(Mcl_pxConfig->pxTrgmuxInitType);
                    #endif
                    }
                #endif /* MCL_TRGMUX_IS_AVAILABLE */
                #if(STD_ON == MCL_FLEXIO_IS_AVAILABLE)
                    if(NULL_PTR != Mcl_pxConfig->pFlexioInitType)
                    {
                        (void)Flexio_Mcl_Ip_InitDevice(Mcl_pxConfig->pFlexioInitType);
                    }
                #endif /* MCL_FLEXIO_IS_AVAILABLE */
                }
            }
        else
        {
#if (STD_ON == MCL_DET_IS_AVAILABLE)
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)0U, MCL_DET_INIT, MCL_E_UNINIT);
#else /* STD_ON == MCL_DET_IS_AVAILABLE */
            /* Do Nothing */
#endif /* STD_ON == MCL_DET_IS_AVAILABLE */
        }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    }
#endif /* STD_ON == MCL_DET_IS_AVAILABLE */
}


void Mcl_DeInit(void)
{
#if (STD_ON == MCL_DMA_IS_AVAILABLE)
    Dma_Ip_ReturnType RetStatus;
#endif /* #if (STD_ON == MCL_DMA_IS_AVAILABLE) */

    if(NULL_PTR != Mcl_pxConfig)
    {
#if (STD_ON == MCL_DMA_IS_AVAILABLE)
        if(NULL_PTR != Mcl_pxConfig->pxDmaInitType)
        {
            RetStatus = Dma_Ip_Deinit();
            Mcl_DmaReturnCheck(0U, MCL_DET_DEINIT, RetStatus);
        }
#endif /* #if (STD_ON == MCL_DMA_IS_AVAILABLE) */

#if(STD_ON == MCL_FLEXIO_IS_AVAILABLE)
        if(NULL_PTR != Mcl_pxConfig->pFlexioInitType)
        {
            Flexio_Mcl_Ip_DeinitDevice(Mcl_pxConfig->pFlexioInitType->instance);
        }
#endif /* MCL_FLEXIO_IS_AVAILABLE */

        Mcl_pxConfig = NULL_PTR;
    }
    else
    {
#if (STD_ON == MCL_DET_IS_AVAILABLE)
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)0U, MCL_DET_DEINIT, MCL_E_UNINIT);
#else /* STD_ON == MCL_DET_IS_AVAILABLE */
        /* Do Nothing */
#endif /* STD_ON == MCL_DET_IS_AVAILABLE */
    }
}


#if (STD_ON == MCL_DMA_IS_AVAILABLE)
void Mcl_SetDmaInstanceCommand(const uint32 Instance, const Mcl_DmaInstanceCmdType Command)
{
    Dma_Ip_ReturnType RetStatus;
    boolean InstanceValidation = TRUE;
    boolean CommandValidation = TRUE;

    if(DMA_IP_NOF_CFG_LOGIC_INSTANCES <= Instance)
    {
        InstanceValidation = FALSE;
    }

    if(MCL_DMA_INST_RESUME < Command)
    {
        CommandValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != InstanceValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Instance, MCL_DET_DMA_INSTANCE_COMMAND, MCL_E_INVALID_INSTANCE);
    }

    if (TRUE != CommandValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Instance, MCL_DET_DMA_INSTANCE_COMMAND, MCL_E_INVALID_COMMAND);
    }
#endif

    if ((FALSE != InstanceValidation) && (FALSE != CommandValidation))
    {
        RetStatus = Dma_Ip_SetLogicInstanceCommand(Instance, (Dma_Ip_LogicInstanceCmdType)Command);
        Mcl_DmaReturnCheck((uint8)Instance, MCL_DET_DMA_INSTANCE_COMMAND, RetStatus);
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_GetDmaInstanceStatus(const uint32 Instance, Mcl_DmaInstanceStatusType * const Status)
{
    Dma_Ip_ReturnType RetStatus;
    Dma_Ip_LogicInstanceStatusType LocInstStatus = {0U, 0U, FALSE, 0U};
    boolean InstanceValidation = TRUE;
    boolean StatusValidation = TRUE;

    if(DMA_IP_NOF_CFG_LOGIC_INSTANCES <= Instance)
    {
        InstanceValidation = FALSE;
    }

    if(NULL_PTR == Status)
    {
        StatusValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != InstanceValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Instance, MCL_DET_DMA_INSTANCE_STATUS, MCL_E_INVALID_INSTANCE);
    }

    if (TRUE != StatusValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Instance, MCL_DET_DMA_INSTANCE_STATUS, MCL_E_PARAM_POINTER);
    }
#endif

    if ((FALSE != InstanceValidation) && (FALSE != StatusValidation))
    {
        RetStatus = Dma_Ip_GetLogicInstanceStatus(Instance, &LocInstStatus);
        Status->Errors   = LocInstStatus.Errors;
        Status->ActiveId = LocInstStatus.ActiveId;
        Status->Active   = LocInstStatus.Active;
        Mcl_DmaReturnCheck((uint8)Instance, MCL_DET_DMA_INSTANCE_STATUS, RetStatus);
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_SetDmaChannelCommand(const uint32 Channel, const Mcl_DmaChannelCmdType Command)
{
    Dma_Ip_ReturnType RetStatus;
    boolean ChannelValidation = TRUE;
    boolean CommandValidation = TRUE;

    if(DMA_IP_NOF_CFG_LOGIC_CHANNELS <= Channel)
    {
        ChannelValidation = FALSE;
    }

    if(MCL_DMA_CH_ACK_ERROR < Command)
    {
        CommandValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != ChannelValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_CHANNEL_COMMAND, MCL_E_INVALID_CHANNEL);
    }

    if (TRUE != CommandValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_CHANNEL_COMMAND, MCL_E_INVALID_COMMAND);
    }
#endif

    if ((FALSE != ChannelValidation) && (FALSE != CommandValidation))
    {
        RetStatus = Dma_Ip_SetLogicChannelCommand(Channel, (Dma_Ip_LogicChannelCmdType)Command);
        Mcl_DmaReturnCheck((uint8)Channel, MCL_DET_DMA_CHANNEL_COMMAND, RetStatus);
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_GetDmaChannelStatus(const uint32 Channel, Mcl_DmaChannelStatusType * const Status)
{
    Dma_Ip_ReturnType RetStatus;
    Dma_Ip_LogicChannelStatusType LocChStatus = {DMA_IP_CH_RESET_STATE, 0U, FALSE, FALSE};
    boolean ChannelValidation = TRUE;
    boolean StatusValidation = TRUE;

    if(DMA_IP_NOF_CFG_LOGIC_CHANNELS <= Channel)
    {
        ChannelValidation = FALSE;
    }

    if(NULL_PTR == Status)
    {
        StatusValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != ChannelValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_CHANNEL_STATUS, MCL_E_INVALID_CHANNEL);
    }

    if (TRUE != StatusValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_CHANNEL_STATUS, MCL_E_PARAM_POINTER);
    }
#endif

    if ((FALSE != ChannelValidation) && (FALSE != StatusValidation))
    {
        RetStatus = Dma_Ip_GetLogicChannelStatus(Channel, &LocChStatus);
        Status->ChannelState = (Mcl_DmaChannelStateType)LocChStatus.ChStateValue;
        Status->Errors = LocChStatus.Errors;
        Status->Active = LocChStatus.Active;
        Status->Done   = LocChStatus.Done;
        Mcl_DmaReturnCheck((uint8)Channel, MCL_DET_DMA_CHANNEL_STATUS, RetStatus);
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_SetDmaChannelGlobalList(const uint32 Channel, const Mcl_DmaChannelGlobalListType List[], const uint32 ListDimension)
{
    Dma_Ip_ReturnType RetStatus;
    uint32 Idx;
    boolean ChannelValidation = TRUE;
    boolean ListDimensionValidation = TRUE;
    boolean ParamValidation = TRUE;

    if(DMA_IP_NOF_CFG_LOGIC_CHANNELS <= Channel)
    {
        ChannelValidation = FALSE;
    }

    if(((uint32)MCL_DMA_CH_SET_DIS_PREEMPT_PRIORITY + 1U) < ListDimension)
    {
        ListDimensionValidation = FALSE;
    }

    for(Idx = 0U; Idx < ListDimension; Idx++)
    {
        if(MCL_DMA_CH_SET_DIS_PREEMPT_PRIORITY < List[Idx].Param)
        {
            ParamValidation = FALSE;
            break;
        }
    }


#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != ChannelValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_GLOBAL, MCL_E_INVALID_CHANNEL);
    }

    if (TRUE != ListDimensionValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_GLOBAL, MCL_E_INVALID_PARAMETER);
    }

    if (TRUE != ParamValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_GLOBAL, MCL_E_INVALID_PARAMETER);
    }
#endif

    if ((FALSE != ChannelValidation) && (FALSE != ListDimensionValidation) && (FALSE != ParamValidation))
    {
        RetStatus = Dma_Ip_SetLogicChannelGlobalList(Channel, (const Dma_Ip_LogicChannelGlobalListType*)List, ListDimension);
        Mcl_DmaReturnCheck((uint8)Channel, MCL_DET_DMA_GLOBAL, RetStatus);
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_SetDmaChannelTransferList(const uint32 Channel, const Mcl_DmaChannelTransferListType List[], const uint32 ListDimension)
{
    Dma_Ip_ReturnType RetStatus;
    uint32 Idx;
    boolean ChannelValidation = TRUE;
    boolean ListDimensionValidation = TRUE;
    boolean ParamValidation = TRUE;

    if(DMA_IP_NOF_CFG_LOGIC_CHANNELS <= Channel)
    {
        ChannelValidation = FALSE;
    }

#if (STD_ON == MCL_DMA_BANDWIDTH_CONTROL_IS_AVAILABLE)
    if(((uint32)MCL_DMA_CH_SET_CONTROL_BANDWIDTH + 1U) < ListDimension)
    {
        ListDimensionValidation = FALSE;
    }

    for(Idx = 0; Idx < ListDimension; Idx++)
    {
        if(MCL_DMA_CH_SET_CONTROL_BANDWIDTH < List[Idx].Param)
        {
            ParamValidation = FALSE;
            break;
        }
    }
#endif

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != ChannelValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_TRANSFER, MCL_E_INVALID_CHANNEL);
    }

    if (TRUE != ListDimensionValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_TRANSFER, MCL_E_INVALID_PARAMETER);
    }

    if (TRUE != ParamValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_TRANSFER, MCL_E_INVALID_PARAMETER);
    }
#endif

    if ((FALSE != ChannelValidation) && (FALSE != ListDimensionValidation) && (FALSE != ParamValidation))
    {
        RetStatus = Dma_Ip_SetLogicChannelTransferList(Channel, (const Dma_Ip_LogicChannelTransferListType*)List, ListDimension);
        Mcl_DmaReturnCheck((uint8)Channel, MCL_DET_DMA_TRANSFER, RetStatus);
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_SetDmaChannelScatterGatherList(const uint32 Channel, const uint32 Element, const Mcl_DmaChannelScatterGatherListType List[], const uint32 ListDimension)
{
    Dma_Ip_ReturnType RetStatus;
    uint32 Idx;
    boolean ChannelValidation = TRUE;
    boolean ListDimensionValidation = TRUE;
    boolean ParamValidation = TRUE;

    if(DMA_IP_NOF_CFG_LOGIC_CHANNELS <= Channel)
    {
        ChannelValidation = FALSE;
    }

#if (STD_ON == MCL_DMA_BANDWIDTH_CONTROL_IS_AVAILABLE)
    if(((uint32)MCL_DMA_CH_SET_CONTROL_BANDWIDTH + 1U) < ListDimension)
    {
        ListDimensionValidation = FALSE;
    }

    for(Idx = 0; Idx < ListDimension; Idx++)
    {
        if(MCL_DMA_CH_SET_CONTROL_BANDWIDTH < List[Idx].Param)
        {
            ParamValidation = FALSE;
            break;
        }
    }
#endif

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != ChannelValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_SCATTER_GATHER_LIST, MCL_E_INVALID_CHANNEL);
    }

    if (TRUE != ListDimensionValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_SCATTER_GATHER_LIST, MCL_E_INVALID_PARAMETER);
    }

    if (TRUE != ParamValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_SCATTER_GATHER_LIST, MCL_E_INVALID_PARAMETER);
    }
#endif

    if ((FALSE != ChannelValidation) && (FALSE != ListDimensionValidation) && (FALSE != ParamValidation))
    {
        RetStatus = Dma_Ip_SetLogicChannelScatterGatherList(Channel, Element, (const Dma_Ip_LogicChannelScatterGatherListType*)List, ListDimension);
        Mcl_DmaReturnCheck((uint8)Channel, MCL_DET_DMA_SCATTER_GATHER_LIST, RetStatus);
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_GetDmaChannelParam(const uint32 Channel, const Mcl_DmaChannelInfoParamType Param, uint32 * const Value)
{
    Dma_Ip_ReturnType RetStatus;
    boolean ChannelValidation = TRUE;
    boolean ParamValidation = TRUE;

    if(DMA_IP_NOF_CFG_LOGIC_CHANNELS <= Channel)
    {
        ChannelValidation = FALSE;
    }

    if(MCL_DMA_CH_GET_HALF_MAJOR_INTERRUPT < Param)
    {
        ParamValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != ChannelValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_INFORMATION, MCL_E_INVALID_CHANNEL);
    }

    if (TRUE != ParamValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_INFORMATION, MCL_E_INVALID_PARAMETER);
    }
#endif

    if ((FALSE != ChannelValidation) && (FALSE != ParamValidation))
    {
        RetStatus = Dma_Ip_GetLogicChannelParam(Channel, (Dma_Ip_LogicChannelInfoParamType)Param, Value);
        Mcl_DmaReturnCheck((uint8)Channel, MCL_DET_DMA_INFORMATION, RetStatus);
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_SetDmaChannelScatterGatherConfig(const uint32 Channel, const uint32 Element)
{
    Dma_Ip_ReturnType RetStatus;
    boolean ChannelValidation = TRUE;

    if(DMA_IP_NOF_CFG_LOGIC_CHANNELS <= Channel)
    {
        ChannelValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != ChannelValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)Channel, MCL_DET_DMA_SCATTER_GATHER_CONFIG, MCL_E_INVALID_CHANNEL);
    }
#endif

    if (FALSE != ChannelValidation)
    {
        RetStatus = Dma_Ip_SetLogicChannelScatterGatherConfig(Channel, Element);
        Mcl_DmaReturnCheck((uint8)Channel, MCL_DET_DMA_SCATTER_GATHER_CONFIG, RetStatus);
    }
    else
    {
        /* Do nothing */
    }
}


#endif /* #if (STD_ON == MCL_DMA_IS_AVAILABLE) */

#if (STD_ON == MCL_CACHE_IS_AVAILABLE)

void Mcl_CacheEnable(Mcl_CacheType CacheType, Mcl_CacheBusType BusType)
{
#if (STD_ON == MCL_DEM_IS_AVAILABLE)
    Std_ReturnType Status = E_OK;
#endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    boolean CacheTypeValidation = TRUE;

    if((MCL_CACHE_CORE < CacheType) || (MCL_CACHE_DATA < BusType))
    {
        CacheTypeValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != CacheTypeValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)CacheType, MCL_DET_CACHE_ENABLE, MCL_E_INVALID_INSTANCE);
    }
#endif

    if (FALSE != CacheTypeValidation)
    {
    #if (STD_ON == MCL_DEM_IS_AVAILABLE)
        Status = Cache_Ip_Enable((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType);
        Mcl_Dem_ReportErrorStatus(Status, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.state, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.id);
    #else
        (void)Cache_Ip_Enable((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType);
    #endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_CacheDisable(Mcl_CacheType CacheType, Mcl_CacheBusType BusType)
{
#if (STD_ON == MCL_DEM_IS_AVAILABLE)
    Std_ReturnType Status = E_OK;
#endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    boolean CacheTypeValidation = TRUE;

    if((MCL_CACHE_CORE < CacheType) || (MCL_CACHE_DATA < BusType))
    {
        CacheTypeValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != CacheTypeValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)CacheType, MCL_DET_CACHE_DISABLE, MCL_E_INVALID_INSTANCE);
    }
#endif

    if (FALSE != CacheTypeValidation)
    {
    #if (STD_ON == MCL_DEM_IS_AVAILABLE)
        Status = Cache_Ip_Disable((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType);
        Mcl_Dem_ReportErrorStatus(Status, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.state, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.id);
    #else
        (void)Cache_Ip_Disable((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType);
    #endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_CacheInvalidate(Mcl_CacheType CacheType, Mcl_CacheBusType BusType)
{
#if (STD_ON == MCL_DEM_IS_AVAILABLE)
    Std_ReturnType Status = E_OK;
#endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    boolean CacheTypeValidation = TRUE;

    if((MCL_CACHE_CORE < CacheType) || (MCL_CACHE_DATA < BusType))
    {
        CacheTypeValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != CacheTypeValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)CacheType, MCL_DET_CACHE_INVALIDATE, MCL_E_INVALID_INSTANCE);
    }
#endif

    if (FALSE != CacheTypeValidation)
    {
    #if (STD_ON == MCL_DEM_IS_AVAILABLE)
        Status = Cache_Ip_Invalidate((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType);
        Mcl_Dem_ReportErrorStatus(Status, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.state, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.id);
    #else
        (void)Cache_Ip_Invalidate((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType);
    #endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_CacheClean(Mcl_CacheType CacheType, Mcl_CacheBusType BusType, boolean EnInvalidate)
{
#if (STD_ON == MCL_DEM_IS_AVAILABLE)
    Std_ReturnType Status = E_OK;
#endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    boolean CacheTypeValidation = TRUE;

    if((MCL_CACHE_CORE < CacheType) || (MCL_CACHE_DATA < BusType))
    {
        CacheTypeValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != CacheTypeValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)CacheType, MCL_DET_CACHE_CLEAN, MCL_E_INVALID_INSTANCE);
    }
#endif

    if (FALSE != CacheTypeValidation)
    {
    #if (STD_ON == MCL_DEM_IS_AVAILABLE)
        Status = Cache_Ip_Clean((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType, EnInvalidate);
        Mcl_Dem_ReportErrorStatus(Status, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.state, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.id);
    #else
        (void)Cache_Ip_Clean((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType, EnInvalidate);
    #endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_CacheInvalidateByAddr(Mcl_CacheType CacheType, Mcl_CacheBusType BusType, uint32 Addr, uint32 Length)
{
#if (STD_ON == MCL_DEM_IS_AVAILABLE)
    Std_ReturnType Status = E_OK;
#endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    boolean CacheTypeValidation = TRUE;

    if((MCL_CACHE_CORE < CacheType) || (MCL_CACHE_DATA < BusType))
    {
        CacheTypeValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != CacheTypeValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)CacheType, MCL_DET_CACHE_INVALIDATE_BY_ADDRESS, MCL_E_INVALID_INSTANCE);
    }
#endif

    if (FALSE != CacheTypeValidation)
    {
    #if (STD_ON == MCL_DEM_IS_AVAILABLE)
        Status = Cache_Ip_InvalidateByAddr((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType, Addr, Length);
        Mcl_Dem_ReportErrorStatus(Status, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.state, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.id);
    #else
        (void)Cache_Ip_InvalidateByAddr((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType, Addr, Length);
    #endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    }
    else
    {
        /* Do nothing */
    }
}

void Mcl_CacheCleanByAddr(Mcl_CacheType CacheType, Mcl_CacheBusType BusType, boolean EnInvalidate, uint32 Addr, uint32 Length)
{
#if (STD_ON == MCL_DEM_IS_AVAILABLE)
    Std_ReturnType Status = E_OK;
#endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    boolean CacheTypeValidation = TRUE;

    if((MCL_CACHE_CORE < CacheType) || (MCL_CACHE_DATA < BusType))
    {
        CacheTypeValidation = FALSE;
    }

#if (STD_ON == MCL_DET_IS_AVAILABLE)
    if (TRUE != CacheTypeValidation)
    {
        (void)Det_ReportError((uint16)CDD_MCL_MODULE_ID, (uint8)CacheType, MCL_DET_CACHE_CLEAN_BY_ADDRESS, MCL_E_INVALID_INSTANCE);
    }
#endif

    if (FALSE != CacheTypeValidation)
    {
    #if (STD_ON == MCL_DEM_IS_AVAILABLE)
        Status = Cache_Ip_CleanByAddr((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType, EnInvalidate, Addr, Length);
        Mcl_Dem_ReportErrorStatus(Status, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.state, Mcl_Dem_Config.Mcl_E_TimeoutFailureCfg.id);
    #else
        (void)Cache_Ip_CleanByAddr((Cache_Ip_Type)CacheType, (Cache_Ip_BusType)BusType, EnInvalidate, Addr, Length);
    #endif /* #if (STD_ON == MCL_DEM_IS_AVAILABLE) */
    }
    else
    {
        /* Do nothing */
    }
}

#endif /* #if (STD_ON == MCL_CACHE_IS_AVAILABLE) */

#if (STD_ON == MCL_TRGMUX_IS_AVAILABLE)

void Mcl_SetTrgMuxInput(const uint32 Trigger, const uint32 Input)
{
    /* Set input */
    (void)Trgmux_Ip_SetInput(Trigger, Input);


}

void Mcl_SetTrgMuxLock(const uint32 Trigger)
{

    /* Set Lock */
    (void)Trgmux_Ip_SetLock(Trigger);

}


#endif /* MCL_TRGMUX_IS_AVAILABLE */


#if (STD_ON == MCL_COMMON_TIMEBASE_IS_AVAILABLE)
/** @implements Mcl_SelectCommonTimebase_Activity */
void Mcl_SelectCommonTimebase(uint8 instance, uint16 elementSyncList)
{
    Ftm_Mcl_Ip_SelectCommonTimebase(instance, ((uint32)(elementSyncList)));
}
#endif /* MCL_COMMON_TIMEBASE_IS_AVAILABLE */




#if (STD_ON == MCL_VERSION_INFO_API_IS_AVAILABLE)
void Mcl_GetVersionInfo(Std_VersionInfoType * const VersionInfo)
{
    /* Save the version information into VersionInfo pointer */
    (VersionInfo)->vendorID = (uint16)CDD_MCL_VENDOR_ID;
    (VersionInfo)->moduleID = (uint16)CDD_MCL_MODULE_ID;
    (VersionInfo)->sw_major_version = (uint8)CDD_MCL_SW_MAJOR_VERSION;
    (VersionInfo)->sw_minor_version = (uint8)CDD_MCL_SW_MINOR_VERSION;
    (VersionInfo)->sw_patch_version = (uint8)CDD_MCL_SW_PATCH_VERSION;
}
#endif /* STD_ON == MCL_VERSION_INFO_API_IS_AVAILABLE */

#define MCL_STOP_SEC_CODE
#include "Mcl_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */

/*==================================================================================================
 *                                        END OF FILE
==================================================================================================*/

