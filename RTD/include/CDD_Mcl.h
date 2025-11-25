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

#ifndef CDD_MCL_H_
#define CDD_MCL_H_

/**
*   @file    CDD_Mcl.h
*
*   @version 2.0.0
*
*   @brief   AUTOSAR Mcl - MCL driver header file.
*   @details
*
*   @addtogroup MCL_DRIVER MCL Driver
*   @{
*/

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @page misra_violations MISRA-C:2012 violations
*/

/*==================================================================================================
*                                          INCLUDE FILES
*  1) system and project includes
*  2) needed interfaces from external units
*  3) internal and external interfaces from this unit
==================================================================================================*/
#include "CDD_Mcl_Cfg.h"
#include "CDD_Mcl_Ipw.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define CDD_MCL_MODULE_ID                       255
#define CDD_MCL_VENDOR_ID                       43
#define CDD_MCL_AR_RELEASE_MAJOR_VERSION        4
#define CDD_MCL_AR_RELEASE_MINOR_VERSION        7
#define CDD_MCL_AR_RELEASE_REVISION_VERSION     0
#define CDD_MCL_SW_MAJOR_VERSION                2
#define CDD_MCL_SW_MINOR_VERSION                0
#define CDD_MCL_SW_PATCH_VERSION                0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if header file and CDD_Mcl_Cfg.h file are of the same vendor */
#if (CDD_MCL_VENDOR_ID != CDD_MCL_CFG_VENDOR_ID)
    #error "CDD_Mcl.h and CDD_Mcl_Cfg.h have different vendor ids"
#endif

/* Check if header file and CDD_Mcl_Cfg.h file are of the same Autosar version */
#if ((CDD_MCL_AR_RELEASE_MAJOR_VERSION != CDD_MCL_CFG_AR_RELEASE_MAJOR_VERSION) || \
     (CDD_MCL_AR_RELEASE_MINOR_VERSION != CDD_MCL_CFG_AR_RELEASE_MINOR_VERSION) || \
     (CDD_MCL_AR_RELEASE_REVISION_VERSION != CDD_MCL_CFG_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of CDD_Mcl.h and CDD_Mcl_Cfg.h are different"
#endif

/* Check if header file and CDD_Mcl_Cfg.h file are of the same Software version */
#if ((CDD_MCL_SW_MAJOR_VERSION != CDD_MCL_CFG_SW_MAJOR_VERSION) || \
     (CDD_MCL_SW_MINOR_VERSION != CDD_MCL_CFG_SW_MINOR_VERSION) || \
     (CDD_MCL_SW_PATCH_VERSION != CDD_MCL_CFG_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of CDD_Mcl.h and CDD_Mcl_Cfg.h are different"
#endif

/* Check if header file and CDD_Mcl_Ipw.h file are of the same vendor */
#if (CDD_MCL_VENDOR_ID != CDD_MCL_IPW_VENDOR_ID)
    #error "CDD_Mcl.h and CDD_Mcl_Ipw.h have different vendor ids"
#endif

/* Check if header file and CDD_Mcl_Ipw.h file are of the same Autosar version */
#if ((CDD_MCL_AR_RELEASE_MAJOR_VERSION != CDD_MCL_IPW_AR_RELEASE_MAJOR_VERSION) || \
     (CDD_MCL_AR_RELEASE_MINOR_VERSION != CDD_MCL_IPW_AR_RELEASE_MINOR_VERSION) || \
     (CDD_MCL_AR_RELEASE_REVISION_VERSION != CDD_MCL_IPW_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of CDD_Mcl.h and CDD_Mcl_Ipw.h are different"
#endif

/* Check if header file and CDD_Mcl_Ipw.h file are of the same Software version */
#if ((CDD_MCL_SW_MAJOR_VERSION != CDD_MCL_IPW_SW_MAJOR_VERSION) || \
     (CDD_MCL_SW_MINOR_VERSION != CDD_MCL_IPW_SW_MINOR_VERSION) || \
     (CDD_MCL_SW_PATCH_VERSION != CDD_MCL_IPW_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of CDD_Mcl.h and CDD_Mcl_Ipw.h are different"
#endif

/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
/* DET APIs */
/**
* @brief API service ID for Mcl_Init function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_INIT                                 ((uint8)0x00U)

#if (STD_ON == MCL_DMA_IS_AVAILABLE)

/**
* @brief API service ID for Mcl_SetDmaInstanceCommand function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_DMA_INSTANCE_COMMAND                 ((uint8)0x01U)

/**
* @brief API service ID for Mcl_GetDmaInstanceStatus function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_DMA_INSTANCE_STATUS                  ((uint8)0x02U)

/**
* @brief API service ID for Mcl_SetDmaChannelCommand function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_DMA_CHANNEL_COMMAND                  ((uint8)0x03U)

/**
* @brief API service ID for Mcl_GetDmaChannelStatus function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_DMA_CHANNEL_STATUS                   ((uint8)0x04U)

/**
* @brief API service ID for Mcl_SetDmaChannelGlobalList function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_DMA_GLOBAL                           ((uint8)0x05U)

/**
* @brief API service ID for Mcl_SetDmaChannelTransferList function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_DMA_TRANSFER                         ((uint8)0x06U)

/**
* @brief API service ID for Mcl_SetDmaChannelScatterGatherList function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_DMA_SCATTER_GATHER_LIST              ((uint8)0x07U)

/**
* @brief API service ID for Mcl_GetDmaChannelParam function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_DMA_INFORMATION                      ((uint8)0x08U)

/**
* @brief API service ID for Mcl_SetDmaChannelScatterGatherConfig function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_DMA_SCATTER_GATHER_CONFIG            ((uint8)0x09U)



#endif /* (STD_ON == MCL_DMA_IS_AVAILABLE) */

/**
* @brief API service ID for Mcl_DeInit function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_DEINIT                               ((uint8)0x0BU)

#if (STD_ON == MCL_CACHE_IS_AVAILABLE)
/**
* @brief API service ID for Mcl_CacheEnable function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_CACHE_ENABLE                         ((uint8)0x10U)

/**
* @brief API service ID for Mcl_CacheDisable function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_CACHE_DISABLE                        ((uint8)0x11U)

/**
* @brief API service ID for Mcl_CacheInvalidate function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_CACHE_INVALIDATE                     ((uint8)0x12U)

/**
* @brief API service ID for Mcl_CacheClean function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_CACHE_CLEAN                          ((uint8)0x13U)

/**
* @brief API service ID for Mcl_CacheInvalidateByAddr function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_CACHE_INVALIDATE_BY_ADDRESS          ((uint8)0x14U)

/**
* @brief API service ID for Mcl_CacheCleanByAddr function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_CACHE_CLEAN_BY_ADDRESS               ((uint8)0x15U)
#endif /* if (STD_ON == MCL_CACHE_IS_AVAILABLE) */


#if (STD_ON == MCL_TRGMUX_IS_AVAILABLE)

/**
* @brief API service ID for Mcl_SetTrgMuxInput function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_TRGMUX_INPUT                         ((uint8)0x20U)

/**
* @brief API service ID for Mcl_SetTrgMuxLock function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_TRGMUX_LOCK                          ((uint8)0x21U)

#endif /* MCL_TRGMUX_IS_AVAILABLE */




#if (STD_ON == MCL_TRGMUX_IS_AVAILABLE)

/**
* @brief API service ID for Mcl_SetTrgMuxInput function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_TRGMUX_SET_INPUT                       ((uint8)0x52)

/**
* @brief API service ID for Mcl_SetTrgMuxLock function
* @details Parameters used when raising an error/exception
* */
#define MCL_DET_TRGMUX_SET_LOC                         ((uint8)0x53)

#endif /* STD_ON == MCL_TRGMUX_IS_AVAILABLE */



/* DET ERRORS */
/**
* @brief   All API's having pointers as parameters shall return this error if
*          called with with a NULL value
* @implements MCL_E_UNINIT_define
* */
#define MCL_E_UNINIT                         ((uint8)0x00)

/**
* @brief   All API's having pointers as parameters shall return this error if
*          called with with a NULL value
* @implements MCL_E_PARAM_POINTER_define
* */
#define MCL_E_PARAM_POINTER                  ((uint8)0x01)

/**
* @brief   All API's called with wrong instance shall return this error
* @implements MCL_E_INVALID_INSTANCE_define
* */
#define MCL_E_INVALID_INSTANCE               ((uint8)0x02)

/**
* @brief   All API's called with wrong channel shall return this error
* @implements MCL_E_INVALID_CHANNEL_define
* */
#define MCL_E_INVALID_CHANNEL                ((uint8)0x03)

/**
* @brief   All API's called with wrong instance shall return this error
* @implements MCL_E_INVALID_COMMAND_define
* */
#define MCL_E_INVALID_COMMAND                ((uint8)0x04)

/**
* @brief   All API's called with wrong read parameter shall return this error
* @implements MCL_E_INVALID_PARAMETER_define
* */
#define MCL_E_INVALID_PARAMETER              ((uint8)0x05)

/**
* @brief   All API's called in wrong sequence shall return this error
* @implements MCL_E_INVALID_STATE_define
* */
#define MCL_E_INVALID_STATE                  ((uint8)0x06)

/**
* @brief   All API's called while hardware has error status shall return this error
* @implements MCL_E_INCONSISTENCY_define
* */
#define MCL_E_INCONSISTENCY                  ((uint8)0x07)

/**
* @brief   All API's called with a timeout value shall return this error if execution
*          is not finished in the allocated timeframe
*
* */
#define MCL_E_TIMEOUT                        ((uint8)0x08)


/**
* @brief   If DET error reporting is enabled, the MCL will check if registers are protected
* */
#define MCL_E_PROTECTED                      ((uint8)0x0A)

/**
* @brief   If VariantPreCompile is used, the configuration pointer shall have a NULL_PTR value.
*          If VariantPostBuild is used, the configuration pointer shall be different from NULL_PTR.
*          And in case of violate will return MCL_E_INIT_FAILED.
* */
#define MCL_E_INIT_FAILED                    ((uint8)0x0B)

/*==================================================================================================
*                                              ENUMS
==================================================================================================*/
#if (STD_ON == MCL_DMA_IS_AVAILABLE)
/**
 * @brief This type contains the Mcl Dma Instance Commands.
 * @details The Commands trigger specific actions in the Dma Instance.
 *
 * @implements Mcl_DmaInstanceCmdType_enum
 * */
typedef enum{
    MCL_DMA_INST_STOP       = 0U, /**< @brief The Stop Command stops the executing channel and forces the Minor Loop to finish. */
    MCL_DMA_INST_STOP_ERROR = 1U, /**< @brief The StopError Command stops the executing channel, forces the Minor Loop to finish and generates an error interrupt. */
    MCL_DMA_INST_PAUSE      = 2U, /**< @brief The Pause Command allows the ongoing transfer to finish and pauses any new transfer. */
    MCL_DMA_INST_RESUME     = 3U, /**< @brief The Resume Command allows the transfer to continue. */
}Mcl_DmaInstanceCmdType;

/**
 * @brief This type contains the Mcl Dma Channel Commands.
 * @details The Commands trigger specific actions in the Dma Channel.
 *
 * @implements Mcl_DmaChannelCmdType_enum
 * */
typedef enum{
    MCL_DMA_CH_START_REQUEST = 0U, /**< @brief The Start Request Command enables the Dma Channel to be triggered by hardware requests. */
    MCL_DMA_CH_STOP_REQUEST  = 1U, /**< @brief The Stop Request Command disables the Dma Channel to be triggered by hardware requests. */
    MCL_DMA_CH_START_SERVICE = 2U, /**< @brief The Start Service Command sends a start request to the Dma Channel. */
    MCL_DMA_CH_ACK_DONE      = 3U, /**< @brief The Ack Done Command resets the Dma Channel Done status. */
    MCL_DMA_CH_ACK_ERROR     = 4U, /**< @brief The Ack Error Command resets the Dma Channel Error status. */
}Mcl_DmaChannelCmdType;

/**
 * @brief This type contains the Mcl Dma Channel Global Parameters.
 * @details The Parameters set specific functionalities.
 *
 * @implements Mcl_DmaChannelGlobalParamType_enum
 * */
typedef enum{
#if (STD_ON == MCL_DMA_MASTER_ID_REPLICATION_IS_AVAILABLE)
    MCL_DMA_CH_SET_EN_MASTER_ID_REPLICATION =  0U, /**< @brief [BOOLEAN] The EnMasterIdReplication Parameter sets the Dma Channel to use the same protection level and system bus ID of the master programming the Dma Channel. */
#endif
#if (STD_ON == MCL_DMA_BUFFERED_WRITES_IS_AVAILABLE)
    MCL_DMA_CH_SET_EN_BUFFERED_WRITES       =  1U, /**< @brief [BOOLEAN] The EnBufferedWrites Parameter sets the Dma Channel writes to be bufferable. */
#endif
    MCL_DMA_CH_SET_EN_HARDWARE_REQ          =  2U, /**< @brief [BOOLEAN] The EnRequest Parameter enables the Dma Channel Request. */
    MCL_DMA_CH_SET_EN_ERROR_INTERRUPT       =  3U, /**< @brief [BOOLEAN] The EnError Parameter enables the Dma Channel Error Interrupt. */
    MCL_DMA_CH_SET_GROUP_PRIORITY           =  4U, /**< @brief [VALUE]   The Group Parameter sets the Dma Channel Group Priority. */
    MCL_DMA_CH_SET_LEVEL_PRIORITY           =  5U, /**< @brief [VALUE]   The Level Parameter sets the Dma Channel Level Priority. */
#if (STD_ON == MCL_DMA_PREEMPTION_IS_AVAILABLE)
    MCL_DMA_CH_SET_EN_PREEMPTION_PRIORITY   =  6U, /**< @brief [BOOLEAN] The EnPreemption Parameter enables the Dma Channel Preemption. */
#endif
#if (STD_ON == MCL_DMA_DISABLE_PREEMPT_IS_AVAILABLE)
    MCL_DMA_CH_SET_DIS_PREEMPT_PRIORITY     = 7U, /**< @brief [BOOLEAN] The DisPreempt Parameter disables the Dma Channel Preempt. */
#endif
}Mcl_DmaChannelGlobalParamType;

/**
 * @brief This type contains the Mcl Dma Channel Transfer Parameters.
 * @details The Parameters set specific functionalities.
 *
 * @implements Mcl_DmaChannelTransferParamType_enum
 * */
typedef enum{
    MCL_DMA_CH_SET_SOURCE_ADDRESS                        =  0U, /**< @brief [VALUE]   The Source Address Parameter sets the Dma Channel source address value. */
    MCL_DMA_CH_SET_SOURCE_SIGNED_OFFSET                  =  1U, /**< @brief [VALUE]   The Source Signed Offset Parameter sets the Dma Channel source signed offset value. */
    MCL_DMA_CH_SET_SOURCE_SIGNED_LAST_ADDR_ADJ           =  2U, /**< @brief [VALUE]   The Source Signed Last Address Adjustment Parameter sets the Dma Channel source signed last address adjustment. */
    MCL_DMA_CH_SET_SOURCE_TRANSFER_SIZE                  =  3U, /**< @brief [VALUE]   The Source Transfer Size Parameter sets the Dma Channel source transfer size. */
    MCL_DMA_CH_SET_SOURCE_MODULO                         =  4U, /**< @brief [VALUE]   The Source Modulo Parameter sets the Dma Channel source modulo. */
    MCL_DMA_CH_SET_DESTINATION_ADDRESS                   =  5U, /**< @brief [VALUE]   The Destination Address Parameter sets the Dma Channel destination address value. */
    MCL_DMA_CH_SET_DESTINATION_SIGNED_OFFSET             =  6U, /**< @brief [VALUE]   The Destination Signed Offset Parameter sets the Dma Channel destination signed offset value. */
    MCL_DMA_CH_SET_DESTINATION_SIGNED_LAST_ADDR_ADJ      =  7U, /**< @brief [VALUE]   The Destination Signed Last Address Adjustment Parameter sets the Dma Channel destination signed last address adjustment. */
    MCL_DMA_CH_SET_DESTINATION_TRANSFER_SIZE             =  8U, /**< @brief [VALUE]   The Destination Transfer Size Parameter sets the Dma Channel destination transfer size. */
    MCL_DMA_CH_SET_DESTINATION_MODULO                    =  9U, /**< @brief [VALUE]   The Destination Modulo Parameter sets the Dma Channel destination modulo. */
    MCL_DMA_CH_SET_MINORLOOP_EN_SRC_OFFSET               = 10U, /**< @brief [BOOLEAN] The Minor Loop Enable Source Offset Parameter enables the Dma Channel minor loop source offset. */
    MCL_DMA_CH_SET_MINORLOOP_EN_DST_OFFSET               = 11U, /**< @brief [BOOLEAN] The Minor Loop Enable Destination Offset Parameter enables the Dma Channel minor loop destination offset. */
    MCL_DMA_CH_SET_MINORLOOP_SIGNED_OFFSET               = 12U, /**< @brief [VALUE]   The Minor Loop Signed Offset Parameter sets the Dma Channel minor loop signed offset. */
    MCL_DMA_CH_SET_MINORLOOP_EN_LINK                     = 13U, /**< @brief [BOOLEAN] The Minor Loop Enable Link Parameter enables the Dma Channel minor loop logic channel linking. */
    MCL_DMA_CH_SET_MINORLOOP_LOGIC_LINK_CH               = 14U, /**< @brief [VALUE]   The Minor Loop Logic Channel Link Parameter sets the Dma Channel minor loop logic channel link. */
    MCL_DMA_CH_SET_MINORLOOP_SIZE                        = 15U, /**< @brief [VALUE]   The Minor Loop Size Parameter sets the Dma Channel minor loop transfer size. */
    MCL_DMA_CH_SET_MAJORLOOP_EN_LINK                     = 16U, /**< @brief [BOOLEAN] The Major Loop Enable Link Parameter enables the Dma Channel major loop logic channel linking. */
    MCL_DMA_CH_SET_MAJORLOOP_LOGIC_LINK_CH               = 17U, /**< @brief [VALUE]   The Major Loop Logic Channel Link Parameter sets the Dma Channel major loop logic channel link. */
    MCL_DMA_CH_SET_MAJORLOOP_COUNT                       = 18U, /**< @brief [VALUE]   The Major Loop Count Parameter sets the Dma Channel major loop count. */
#if (STD_ON == MCL_DMA_STORE_DST_ADDR_IS_AVAILABLE)
    MCL_DMA_CH_SET_CONTROL_STORE_DST_ADDR                = 19U, /**< @brief [VALUE]   The Store Destination Address Parameter saves the final destination address in system memory. */
#endif
    MCL_DMA_CH_SET_CONTROL_SOFTWARE_REQUEST              = 20U, /**< @brief [BOOLEAN] The Enable Start Parameter enables the Dma Channel start service request. */
    MCL_DMA_CH_SET_CONTROL_EN_MAJOR_INTERRUPT            = 21U, /**< @brief [BOOLEAN] The Enable Major Interrupt Parameter enables the Dma Channel major interrupt. */
    MCL_DMA_CH_SET_CONTROL_EN_HALF_MAJOR_INTERRUPT       = 22U, /**< @brief [BOOLEAN] The Enable Half Interrupt Parameter enables the Dma Channel half major interrupt. */
    MCL_DMA_CH_SET_CONTROL_DIS_AUTO_REQUEST              = 23U, /**< @brief [BOOLEAN] The Disable Automatic Request Parameter disables the Dma Channel automatic request. */
#if (STD_ON == MCL_DMA_END_OF_PACKET_SIGNAL_IS_AVAILABLE)
    MCL_DMA_CH_SET_CONTROL_EN_END_OF_PACKET_SIGNAL       = 24U, /**< @brief [BOOLEAN] The Enable End Of Packet Signal Parameter enables the Dma Channel end of packet signal. */
#endif
#if (STD_ON == MCL_DMA_BANDWIDTH_CONTROL_IS_AVAILABLE)
    MCL_DMA_CH_SET_CONTROL_BANDWIDTH                     = 25U, /**< @brief [VALUE]   The Bandwidth Control Parameter sets the Dma Channel bandwidth control. */
#endif
}Mcl_DmaChannelTransferParamType;

/**
 * @brief This type contains the Mcl Dma Channel State values.
 * @details The states represent the Channel status during runtime.
 *
 * @implements Mcl_DmaChannelStateType_enum
 * */
typedef enum{
    MCL_DMA_CH_RESET_STATE         = 0U,
    MCL_DMA_CH_READY_STATE         = 1U,
    MCL_DMA_CH_TRANSFER_STATE      = 2U,
    MCL_DMA_CH_SCATTERGATHER_STATE = 3U,
    MCL_DMA_CH_ERROR_STATE         = 4U,
}Mcl_DmaChannelStateType;


/**
 * @brief This type contains the Mcl Dma Channel Information Parameters.
 * @details The Parameters get specific information.
 *
 * @implements Mcl_DmaChannelInfoParamType_enum
 * */
typedef enum{
    MCL_DMA_CH_GET_SOURCE_ADDRESS       = 0U, /**< @brief [VALUE]   The Source Address Parameter gets the Dma Channel source address. */
    MCL_DMA_CH_GET_DESTINATION_ADDRESS  = 1U, /**< @brief [VALUE]   The Destination Address Parameter gets the Dma Channel destination address. */
    MCL_DMA_CH_GET_BEGIN_ITER_COUNT     = 2U, /**< @brief [VALUE]   The Begin Iteration Count Parameter gets the Dma Channel begin iteration count. */
    MCL_DMA_CH_GET_CURRENT_ITER_COUNT   = 3U, /**< @brief [VALUE]   The Current Iteration Count Parameter gets the Dma Channel current iteration count. */
#if (STD_ON == MCL_DMA_STORE_DST_ADDR_IS_AVAILABLE)
    MCL_DMA_CH_GET_STORE_DST_ADDR       = 4U, /**< @brief [VALUE]   The Store Destination Address Parameter gets the Dma Channel stored destination address. */
#endif
#if (STD_ON == MCL_DMA_MASTER_ID_REPLICATION_IS_AVAILABLE)
    MCL_DMA_CH_GET_MASTER_ID            = 5U, /**< @brief [VALUE]   The Master Id Parameter gets the Dma Channel master id. */
#endif
    MCL_DMA_CH_GET_MAJOR_INTERRUPT      = 6U, /**< @brief [BOOLEAN] The Major Interrupt Parameter gets the Dma Channel major interrupt. */
    MCL_DMA_CH_GET_HALF_MAJOR_INTERRUPT = 7U, /**< @brief [BOOLEAN] The Half Major Interrupt Parameter gets the Dma Channel half major interrupt. */
}Mcl_DmaChannelInfoParamType;
#endif /* #if (STD_ON == MCL_DMA_IS_AVAILABLE) */

#if (STD_ON == MCL_CACHE_IS_AVAILABLE)
/**
 * @brief This type contains the Mcl Cache Type selection.
 * @details The Cache Types select specific cache memory types.
 *
 * @implements Mcl_CacheType_enum
 * */
typedef enum{
    MCL_CACHE_LMEM         = 0U, /**< @brief The Cache Lmem Parameter selects LMEM cache types. */
    MCL_CACHE_CORE         = 1U, /**< @brief The Cache Core Parameter selects CORE cache types. */
}Mcl_CacheType;

/**
 * @brief This type contains the Mcl Cache Bus Type selection.
 * @details The Cache Bus Types select Code and System caches and bus.
 *
 * @implements Mcl_CacheBusType_enum
 * */
typedef enum{
    MCL_CACHE_PC_BUS         = 0U, /**< @brief The Cache PC Bus selects Processor Code (PC) bus (used with Cache Lmem). */
    MCL_CACHE_PS_BUS         = 1U, /**< @brief The Cache PS Bus selects Processor System (PS) bus (used with Cache Lmem). */
    MCL_CACHE_ALL_BUS        = 2U, /**< @brief The Cache All Bus selects PC and PS bus (used with Cache Lmem). */
    MCL_CACHE_INSTRUCTION    = 3U, /**< @brief The Cache Instruction Parameter selects instruction cache (used with Cache Core). */
    MCL_CACHE_DATA           = 4U, /**< @brief The Cache Data Parameter selects data cache (used with Cache Core). */
}Mcl_CacheBusType;
#endif /* #if (STD_ON == MCL_CACHE_IS_AVAILABLE) */


/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
#if (STD_ON == MCL_DMA_IS_AVAILABLE)
/**
 * @brief This type contains the Mcl Dma Channel Global List.
 * @details The Mcl Dma Channel Global List contains a pair composed from Dma Channel Global Parameter
 *          Type and the Value of the parameter.
 *
 * @implements Mcl_DmaChannelGlobalListType_struct
 * */
typedef struct{
    Mcl_DmaChannelGlobalParamType Param;   /**< @brief The Mcl Dma Channel Global Parameter Type selects a parameter form the Global Parameter enum type. */
    uint32 Value;                          /**< @brief The Value stores the parameter's value. */
}Mcl_DmaChannelGlobalListType;

/**
 * @brief This type contains the Mcl Dma Channel Transfer List.
 * @details The Mcl Dma Channel Transfer List contains a pair composed from Dma Channel Transfer
 *          Parameter Type and the Value of the parameter.
 *
 * @implements Mcl_DmaChannelTransferListType_struct
 * */
typedef struct{
    Mcl_DmaChannelTransferParamType Param; /**< @brief The Mcl Dma Channel Transfer Parameter Type selects a parameter form the Transfer Parameter enum type. */
    uint32 Value;                          /**< @brief The Value stores the parameter's value. */
}Mcl_DmaChannelTransferListType;

/**
 * @brief This type contains the Mcl Dma Channel Scatter/Gather List.
 * @details The Mcl Dma Channel Scatter/Gather List contains a pair composed from Dma Channel Scatter/Gather
 *          Parameter Type and the Value of the parameter.
 *
 * @implements Mcl_DmaChannelScatterGatherListType_struct
 * */
typedef struct{
    Mcl_DmaChannelTransferParamType Param; /**< @brief The Mcl Dma Channel Transfer Parameter Type selects a parameter form the Transfer Parameter enum type. */
    uint32 Value;                          /**< @brief The Value stores the parameter's value. */
}Mcl_DmaChannelScatterGatherListType;


/**
 * @brief This type contains the Mcl Dma Instance Status.
 * @details The Mcl Dma Instance Status contains the Hardware Errors, Active Id and Active indication for
 *          the running Dma Channel.
 *
 * @implements Mcl_DmaInstanceStatusType_struct
 * */
typedef struct{
    uint32  Errors;                        /**< @brief [VALUE]   The Errors value is read from the DMA Instance Error Register (ES) as it is. */
    uint8   ActiveId;                      /**< @brief [VALUE]   The ActiveId value is read from the DMA Instance Control Register (CR) field ACTIVE_ID. */
    boolean Active;                        /**< @brief [BOOLEAN] The Active value is read from the DMA Instance Control Register (CR) field ACTIVE. */
    uint8 Version;                         /**< @brief [VALUE]   The Version value is read from the DMA Instance Control Register (CR) field VER. */
}Mcl_DmaInstanceStatusType;

/**
 * @brief This type contains the Mcl Dma Channel Status.
 * @details The Mcl Dma Channel Status contains the Hardware Errors, Active status and Done indication for
 *          the running Dma Channel.
 *
 * @implements Mcl_DmaChannelStatusType_struct
 * */
typedef struct{
    Mcl_DmaChannelStateType ChannelState;  /**< @brief [VALUE]   The ChStateValue value is read from the internal DMA Driver Channel State Machine. Check UM for additional information. */
    uint32  Errors;                        /**< @brief [VALUE]   The Errors value is read from the DMA Channel Error Register (CHx_ES) as it is. */
    boolean Active;                        /**< @brief [BOOLEAN] The Active value is read from the DMA Channel Control and Status Register (CHx_CSR) field ACTIVE. */
    boolean Done;                          /**< @brief [BOOLEAN] The Active value is read from the DMA Channel Control and Status Register (CHx_CSR) field DONE. */
}Mcl_DmaChannelStatusType;
#endif /* #if (STD_ON == MCL_DMA_IS_AVAILABLE) */


/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/
#define MCL_START_SEC_CODE
#include "Mcl_MemMap.h"
/**
 * @brief This function initializes the Mcl Driver.
 * @details This service is a non reentrant function that shall initialize the Mcl driver.
 *             The initialization is applied for the enabled IPs, configured statically.
 *
 * @param[in]  ConfigPtr         Pointer to the configuration structure.
 *
 * @return void
 *
 * @implements Mcl_Init_Activity
 * */
void Mcl_Init(const Mcl_ConfigType * const ConfigPtr);

/**
 * @brief This function deinitializes the Mcl Driver.
 * @details This service is a non reentrant function that shall deinitialize the Mcl driver.
 *          The deinitialization is applied for the enabled IPs, configured statically.
 *
 * @return void
 *
 * @implements Mcl_DeInit_Activity
 * */
void Mcl_DeInit(void);

#if (STD_ON == MCL_DMA_IS_AVAILABLE)
/**
 * @brief This function sets Dma Instance Command.
 * @details This service is a reentrant function that shall command the Dma Instance.
 *          The command shall trigger specific functionalities of the Dma Instance.
 *
 * @param[in]  Instance          Selection value of the Logic Instance.
 * @param[in]  Command           The command for the Logic Instance.
 *
 * @return void
 *
 * @implements Mcl_SetDmaInstanceCommand_Activity
 * */
void Mcl_SetDmaInstanceCommand(const uint32 Instance, const Mcl_DmaInstanceCmdType Command);

/**
 * @brief This function gets Dma Instance Status.
 * @details This service is a reentrant function that shall get the Dma Instance status.
 *          The command shall read specific functionalities of the Dma Instance.
 *
 * @param[in]  Instance          Selection value of the Logic Instance.
 * @param[out] Status            Pointer to the Dma Instance status.
 *
 * @return void
 *
 * @implements Mcl_GetDmaInstanceStatus_Activity
 * */
void Mcl_GetDmaInstanceStatus(const uint32 Instance, Mcl_DmaInstanceStatusType * const Status);

/**
 * @brief This function sets Dma Channel Command.
 * @details This service is a reentrant function that shall command the Dma Channel.
 *          The command shall trigger specific functionalities of the Dma Channel.
 *
 * @param[in]  Channel           Specifies the Logic Channel Tag defined by the user.
 * @param[in]  Command           The command for the Logic Channel.
 *
 * @return void
 *
 * @implements Mcl_SetDmaChannelCommand_Activity
 * */
void Mcl_SetDmaChannelCommand(const uint32 Channel, const Mcl_DmaChannelCmdType Command);

/**
 * @brief This function gets Dma Channel Status.
 * @details This service is a reentrant function that shall get the Dma Channel status.
 *          The command shall read specific functionalities of the Dma Channel.
 *
 * @param[in]  Channel           Specifies the Logic Channel Tag defined by the user.
 * @param[out] Status            Pointer to the Dma Channel status.
 *
 * @return void
 *
 * @implements Mcl_GetDmaChannelStatus_Activity
 * */
void Mcl_GetDmaChannelStatus(const uint32 Channel, Mcl_DmaChannelStatusType * const Status);

/**
 * @brief This function sets Dma Channel Global List settings.
 * @details This service is a reentrant function that shall set the Dma Channel
 *          global parameters list.
 *          The list is composed of an array of Dma Channel global parameters settings.
 *          The settings list(array) is defined by the user needs: it contains the
 *          desired parameters to be configured, in any desired order.
 *
 *          How to use this interface:
 *          1. Use the "Mcl_DmaChannelGlobalListType" to create a list(array) with the desired
 *          paramaters to configure (see parameters: "Mcl_DmaChannelGlobalParamType")
 *            The list can declared globally or locally:
 *          Global example:
 *              Mcl_DmaChannelGlobalListType global_Mcl_DmaChannelGlobalList0[NUMBER_OF_PARAMETERS] = {...};
 *          Local example:
 *              Mcl_DmaChannelGlobalListType Mcl_DmaChannelGlobalList[NUMBER_OF_PARAMETERS];
 *              Mcl_DmaChannelGlobalList[PARAMETER0].Param = MCL_DMA_CH_SET_EN_BUFFERED_WRITES;
 *              Mcl_DmaChannelGlobalList[PARAMETER0].Value = TRUE;
 *              Mcl_DmaChannelGlobalList[PARAMETER1].Param = ...;
 *              Mcl_DmaChannelGlobalList[PARAMETER1].Value = ...;
 *          2. Call the "Mcl_SetDmaChannelGlobalList()" interface:
 *              Mcl_SetDmaChannelGlobalList(LOGIC_CHANNELx, Mcl_DmaChannelGlobalList, NUMBER_OF_PARAMETERS);
 *
 * @param[in]  Channel           Specifies the Logic Channel Tag defined by the user.
 * @param[in]  List              Pointer to the Global List Array.
 * @param[in]  ListDimension     Number of entries in the List.
 *
 * @return void
 *
 * @implements Mcl_SetDmaChannelGlobalList_Activity
 * */
void Mcl_SetDmaChannelGlobalList(const uint32 Channel, const Mcl_DmaChannelGlobalListType List[], const uint32 ListDimension);

/**
 * @brief This function sets Dma Channel Transfer List settings.
 * @details This service is a reentrant function that shall set the Dma Channel
 *          transfer parameters list.
 *          The list is composed of an array of Dma Channel transfer parameters settings.
 *          The settings array is defined by the user needs: it contains entries for each desired
 *          parameter to be configured, in any suitable order.
 *
 *          How to use this interface:
 *          1. Use the "Mcl_DmaChannelTransferListType" to create a list(array) with the desired
 *          paramaters to configure (see parameters: "Mcl_DmaChannelTransferParamType")
 *            The list can declared globally or locally:
 *          Global example:
 *              Mcl_DmaChannelTransferListType global_Mcl_DmaChannelTransferList0[NUMBER_OF_PARAMETERS] = {...};
 *          Local example:
 *              Mcl_DmaChannelTransferListType Mcl_DmaChannelTransferList[NUMBER_OF_PARAMETERS];
 *              Mcl_DmaChannelTransferList[PARAMETER0].Param = MCL_DMA_CH_SET_SOURCE_ADDRESS;
 *              Mcl_DmaChannelTransferList[PARAMETER0].Value = &SourceBuffer;
 *              Mcl_DmaChannelTransferList[PARAMETER1].Param = MCL_DMA_CH_SET_DESTINATION_ADDRESS;
 *              Mcl_DmaChannelTransferList[PARAMETER1].Value = &DestinationBuffer;
 *          2. Call the "Mcl_SetDmaChannelTransferList()" interface:
 *              Mcl_SetDmaChannelTransferList(LOGIC_CHANNELx, Mcl_DmaChannelTransferList, NUMBER_OF_PARAMETERS);
 *
 * @param[in]  Channel           Specifies the Logic Channel Tag defined by the user.
 * @param[in]  List              Pointer to the Transfer List Array.
 * @param[in]  ListDimension     Number of entries in the List.
 *
 * @return void
 *
 * @implements Mcl_SetDmaChannelTransferList_Activity
 * */
void Mcl_SetDmaChannelTransferList(const uint32 Channel, const Mcl_DmaChannelTransferListType List[], const uint32 ListDimension);

/**
 * @brief This function sets Dma Channel Scatter/Gather List settings.
 * @details This service is a reentrant function that shall set the Dma Channel
 *          scatter/gather parameters list.
 *          The Scatter/Gather List configures Logic Elements belonging to the same
 *          Dma Logic Channel.
 *          The settings array is defined by the user needs: it contains entries for
 *          each desired parameter to be configured, in any suitable order.
 *
 *          How to use this interface:
 *          1. Use the "Mcl_DmaChannelScatterGatherListType" to create a list(array) with the desired
 *          paramaters to configure (see parameters: "Mcl_DmaChannelTransferParamType")
 *            The list can declared globally or locally:
 *          Global example:
 *              Mcl_DmaChannelScatterGatherListType global_Mcl_DmaChannelScatterGatherList0[NUMBER_OF_PARAMETERS] = {...};
 *          Local example:
 *              Mcl_DmaChannelScatterGatherListType Mcl_DmaChannelScatterGatherList[NUMBER_OF_PARAMETERS];
 *              Mcl_DmaChannelScatterGatherList[PARAMETER0].Param = MCL_DMA_CH_SET_SOURCE_ADDRESS;
 *              Mcl_DmaChannelScatterGatherList[PARAMETER0].Value = &SourceBuffer;
 *              Mcl_DmaChannelScatterGatherList[PARAMETER1].Param = MCL_DMA_CH_SET_DESTINATION_ADDRESS;
 *              Mcl_DmaChannelScatterGatherList[PARAMETER1].Value = &DestinationBuffer;
 *          2. Call the "Mcl_SetDmaChannelScatterGatherList()" interface:
 *              Mcl_SetDmaChannelScatterGatherList(LOGIC_CHANNELx, LOGIC_ELEMENTy, Mcl_DmaChannelScatterGatherList, NUMBER_OF_PARAMETERS);
 *
 * @param[in]  Channel           Specifies the Logic Channel Tag defined by the user.
 * @param[in]  Element           Specifies the Logic Element Id.
 * @param[in]  List              Pointer to the Scatter/Gather List Array.
 * @param[in]  ListDimension     Number of entries in the List.
 *
 * @return void
 *
 * @implements Mcl_SetDmaChannelScatterGatherList_Activity
 * */
void Mcl_SetDmaChannelScatterGatherList(const uint32 Channel, const uint32 Element, const Mcl_DmaChannelScatterGatherListType List[],const uint32 ListDimension);


/**
 * @brief This function gets the Dma Channel Parameter value.
 * @details This service is a reentrant function that shall get the Dma Channel
 *          parameters value.
 *
 * @param[in]  Channel           Specifies the Logic Channel Tag defined by the user.
 * @param[in]  Param             Selection parameter.
 * @param[out] Value             Pointer to the parameter value.
 *
 * @return void
 *
 * @implements Mcl_GetDmaChannelParam_Activity
 * */
void Mcl_GetDmaChannelParam(const uint32 Channel, const Mcl_DmaChannelInfoParamType Param, uint32 * const Value);

/**
 * @brief This function configures the Dma Channel Scatter/Gather.
 * @details This service is a reentrant function that shall configure the Dma Channel
 *          scatter/gather functionality.
 *          The Scatter/Gather settings, for the specified Dma Logic Channel, are loaded
 *          into the Software TCDs. Each software TCD corresponds to a Logic Element.
 *          The specified Logic Element shall be loaded into the Dma Logic Channel's
 *          Hardware TCD.
 *          The Logic Elements (describing the Software TCDs) form a simple chained list,
 *          the "Element" function parameter representing the lists's head.
 *
 * @param[in]  Channel           Specifies the Logic Channel Tag defined by the user.
 * @param[in]  Element           Specifies the Logic Element Id representing the
 *                               list's head.
 *
 * @return void
 *
 * @implements Mcl_SetDmaChannelScatterGatherConfig_Activity
 * */
void Mcl_SetDmaChannelScatterGatherConfig(const uint32 Channel, const uint32 Element);
#endif /* #if (STD_ON == MCL_DMA_IS_AVAILABLE) */



#if (STD_ON == MCL_TRGMUX_IS_AVAILABLE)
/**
 * @brief This function sets the Trgmux Trigger Input selection.
 * @details This service is a reentrant function that shall configure the Trgmux Trigger
 *          functionality.
 *
 * @param[in]  Trigger           Selection value of the Logic Trigger.
 * @param[in]  Input             Selection value for the Logic Trigger's Input.
 *
 * @return void
 *
 * @implements Mcl_SetTrgMuxInput_Activity
 * */
void Mcl_SetTrgMuxInput(const uint32 Trigger, const uint32 Input);

/**
 * @brief This function sets the Trgmux Trigger Lock.
 * @details This service is a reentrant function that shall configure the Trgmux Trigger
 *          Lock functionality.
 *
 * @param[in]  Trigger           Selection value of the Logic Trigger.
 *
 * @return void
 *
 * @implements Mcl_SetTrgMuxLock_Activity
 * */
void Mcl_SetTrgMuxLock(const uint32 Trigger);
#endif /* MCL_TRGMUX_IS_AVAILABLE */


#if (STD_ON == MCL_CACHE_IS_AVAILABLE)
/**
 * @brief This function enables the Cache.
 * @details This service is a reentrant function that shall enable the Cache functionality.
 *
 * @param[in]  CacheType         Selection value of the Cache Type.
 * @param[in]  BusType           Selection value of the Bus Type.
 *
 * @return void
 *
 * @implements Mcl_CacheEnable_Activity
 * */
void Mcl_CacheEnable(Mcl_CacheType CacheType, Mcl_CacheBusType BusType);


/**
 * @brief This function disables the Cache.
 * @details This service is a reentrant function that shall disable the Cache functionality.
 *
 * @param[in]  CacheType         Selection value of the Cache Type.
 * @param[in]  BusType           Selection value of the Bus Type.
 *
 * @return void
 *
 * @implements Mcl_CacheDisable_Activity
 * */
void Mcl_CacheDisable(Mcl_CacheType CacheType, Mcl_CacheBusType BusType);


/**
 * @brief This function Invalidates the Cache.
 * @details This service is a reentrant function that shall Invalidate the Cache functionality.
 *          The Invalidation applies to the entire Cache.
 *
 * @param[in]  CacheType         Selection value of the Cache Type.
 * @param[in]  BusType           Selection value of the Bus Type.
 *
 * @return void
 *
 * @implements Mcl_CacheInvalidate_Activity
 * */
void Mcl_CacheInvalidate(Mcl_CacheType CacheType, Mcl_CacheBusType BusType);


/**
 * @brief This function Cleans the Cache.
 * @details This service is a reentrant function that shall Clean the Cache functionality.
 *          The Clean applies to the entire Cache.
 *          By enabling the Invalidation, the function shall execute specific Cache
 *          Clean&Invalidate function.
 *
 * @param[in]  CacheType         Selection value of the Cache Type.
 * @param[in]  BusType           Selection value of the Bus Type.
 * @param[in]  EnInvalidate      Enable the Invalidation specific functionality.
 *
 * @return void
 *
 * @implements Mcl_CacheClean_Activity
 * */
void Mcl_CacheClean(Mcl_CacheType CacheType, Mcl_CacheBusType BusType, boolean EnInvalidate);


/**
 * @brief This function Invalidates the Cache by address.
 * @details This service is a reentrant function that shall Invalidate the Cache
 *          by address.
 *          The Invalidation applies to the area in Cache specified by the address and length.
 *          The buffer shall be aligned to the Cache Line size.
 *
 * @param[in]  CacheType         Selection value of the Cache Type.
 * @param[in]  BusType           Selection value of the Bus Type.
 * @param[in]  Addr              Address value of the buffer.
 * @param[in]  Length            Length value of the buffer.
 *
 * @return void
 *
 * @implements Mcl_CacheInvalidateByAddr_Activity
 * */
void Mcl_CacheInvalidateByAddr(Mcl_CacheType CacheType, Mcl_CacheBusType BusType, uint32 Addr, uint32 Length);


/**
 * @brief This function Cleans the Cache by address.
 * @details This service is a reentrant function that shall Clean the Cache by address.
 *          The Clean applies to the area in Cache specified by the address and length.
 *          By enabling the Invalidation, the function shall execute specific Cache
 *          Clean&Invalidate function.
 *
 * @param[in]  CacheType         Selection value of the Cache Type.
 * @param[in]  BusType           Selection value of the Bus Type.
 * @param[in]  EnInvalidate      Enable the Invalidation specific functionality.
 * @param[in]  Addr              Address value of the buffer.
 * @param[in]  Length            Length value of the buffer.
 *
 * @return void
 *
 * @implements Mcl_CacheCleanByAddr_Activity
 * */
void Mcl_CacheCleanByAddr(Mcl_CacheType CacheType, Mcl_CacheBusType BusType, boolean EnInvalidate, uint32 Addr, uint32 Length);

#endif /* #if (STD_ON == MCL_CACHE_IS_AVAILABLE) */


#if (STD_ON == MCL_COMMON_TIMEBASE_IS_AVAILABLE)
/**
 * @brief       Implementation specific function to updates the Global Timebase bits of configured modules.
 * @details     This function is used to set the global timebase bits for modules that support
 *              the global timebase feature. The function selects the module that gives the common timebase
 *              and the modules that are use this timebase (as bits in elementSyncList).
 *              Then it synchronizes the modules.
 *              example:
 *                     elementSyncList is 0x0003 - modules 0 and 1 use the timebase given by instance
 *                     elementSyncList is 0x0005 - modules 0 and 2 use the timebase given by instance
 *
 * @param[in]   instance        FTM module id
 * @param[in]   elementSyncList FTM module mask value
 * @return      void
 */
void Mcl_SelectCommonTimebase(uint8 instance, uint16 elementSyncList);
#endif /* MCL_COMMON_TIMEBASE_IS_AVAILABLE */



#if (STD_ON == MCL_VERSION_INFO_API_IS_AVAILABLE)
/**
 * @brief   Returns the version information of this module.
 * @details Returns the version information of MCL module.
 *
 * @param[out]  VersionInfo    A pointer to a structure used to get version information.
 *
 * @return void
 *
 * @implements Mcl_GetVersionInfo_Activity
 * */
void Mcl_GetVersionInfo(Std_VersionInfoType * const VersionInfo);
#endif /* STD_ON == MCL_VERSION_INFO_API_IS_AVAILABLE */

#define MCL_STOP_SEC_CODE
#include "Mcl_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* CDD_MCL_H_ */

/*==================================================================================================
 *                                        END OF FILE
==================================================================================================*/

