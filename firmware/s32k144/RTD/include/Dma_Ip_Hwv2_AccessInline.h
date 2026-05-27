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

/* Prevention from multiple including the same header */
#ifndef DMA_IP_HWV2_ACCESSINLINE_H_
#define DMA_IP_HWV2_ACCESSINLINE_H_

/**
*   @file    Dma_Ip_Hwv2_AccessInline.h
*
*   @version 2.0.0
*
*   @brief   AUTOSAR Mcl - Dma Ip driver source file.
*   @details
*
*   @addtogroup DMA_IP_DRIVER DMA IP Driver
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
#include "Dma_Ip.h"
#include "Dma_Ip_Devassert.h"

/*==================================================================================================
                               SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define DMA_IP_HWV2_ACCESSINLINE_VENDOR_ID                       43
#define DMA_IP_HWV2_ACCESSINLINE_AR_RELEASE_MAJOR_VERSION        4
#define DMA_IP_HWV2_ACCESSINLINE_AR_RELEASE_MINOR_VERSION        7
#define DMA_IP_HWV2_ACCESSINLINE_AR_RELEASE_REVISION_VERSION     0
#define DMA_IP_HWV2_ACCESSINLINE_SW_MAJOR_VERSION                2
#define DMA_IP_HWV2_ACCESSINLINE_SW_MINOR_VERSION                0
#define DMA_IP_HWV2_ACCESSINLINE_SW_PATCH_VERSION                0

/*==================================================================================================
                                      FILE VERSION CHECKS
==================================================================================================*/
/* Check if header file and Dma_Ip.h file are of the same vendor */
#if (DMA_IP_HWV2_ACCESSINLINE_VENDOR_ID != DMA_IP_VENDOR_ID)
    #error "Dma_Ip_Hwv2_AccessInline.h and Dma_Ip.h have different vendor ids"
#endif

/* Check if header file and Dma_Ip.h file are of the same Autosar version */
#if ((DMA_IP_HWV2_ACCESSINLINE_AR_RELEASE_MAJOR_VERSION != DMA_IP_AR_RELEASE_MAJOR_VERSION) || \
     (DMA_IP_HWV2_ACCESSINLINE_AR_RELEASE_MINOR_VERSION != DMA_IP_AR_RELEASE_MINOR_VERSION) || \
     (DMA_IP_HWV2_ACCESSINLINE_AR_RELEASE_REVISION_VERSION != DMA_IP_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of Dma_Ip_Hwv2_AccessInline.h and Dma_Ip.h are different"
#endif

/* Check if header file and Dma_Ip.h file are of the same Software version */
#if ((DMA_IP_HWV2_ACCESSINLINE_SW_MAJOR_VERSION != DMA_IP_SW_MAJOR_VERSION) || \
     (DMA_IP_HWV2_ACCESSINLINE_SW_MINOR_VERSION != DMA_IP_SW_MINOR_VERSION) || \
     (DMA_IP_HWV2_ACCESSINLINE_SW_PATCH_VERSION != DMA_IP_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of Dma_Ip_Hwv2_AccessInline.h and Dma_Ip.h are different"
#endif

/* Check if header file and Dma_Ip_Devassert.h file are of the same vendor */
#if (DMA_IP_HWV2_ACCESSINLINE_VENDOR_ID != DMA_IP_DEVASSERT_VENDOR_ID)
    #error "Dma_Ip_Hwv2_AccessInline.h and Dma_Ip_Devassert.h have different vendor ids"
#endif

/* Check if header file and Dma_Ip_Devassert.h file are of the same Autosar version */
#if ((DMA_IP_HWV2_ACCESSINLINE_AR_RELEASE_MAJOR_VERSION != DMA_IP_DEVASSERT_AR_RELEASE_MAJOR_VERSION) || \
     (DMA_IP_HWV2_ACCESSINLINE_AR_RELEASE_MINOR_VERSION != DMA_IP_DEVASSERT_AR_RELEASE_MINOR_VERSION) || \
     (DMA_IP_HWV2_ACCESSINLINE_AR_RELEASE_REVISION_VERSION != DMA_IP_DEVASSERT_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of Dma_Ip_Hwv2_AccessInline.h and Dma_Ip_Devassert.h are different"
#endif

/* Check if header file and Dma_Ip_Devassert.h file are of the same Software version */
#if ((DMA_IP_HWV2_ACCESSINLINE_SW_MAJOR_VERSION != DMA_IP_DEVASSERT_SW_MAJOR_VERSION) || \
     (DMA_IP_HWV2_ACCESSINLINE_SW_MINOR_VERSION != DMA_IP_DEVASSERT_SW_MINOR_VERSION) || \
     (DMA_IP_HWV2_ACCESSINLINE_SW_PATCH_VERSION != DMA_IP_DEVASSERT_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of Dma_Ip_Hwv2_AccessInline.h and Dma_Ip_Devassert.h are different"
#endif

#define MCL_START_SEC_CODE
#include "Mcl_MemMap.h"

#if (STD_ON == DMA_IP_IS_AVAILABLE)

#if (STD_ON == DMA_IP_HWV2_IS_AVAILABLE)
/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                       DEFINES AND MACROS
==================================================================================================*/
#define DMA_IP_HWV2_DOES_NOT_SUPPORT    (uint8)0xFFU

/*==================================================================================================
*                                              ENUMS
==================================================================================================*/

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
 * DMA INSTANCE CONFIGURATION FUNCTION - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaInst_SetConfig(Dma_Ip_Hwv2InstRegType * const ptInst, const Dma_Ip_LogicInstanceConfigType * const pxConfig)
{
    uint32 reg = ptInst->reg_CR;
    reg = ((reg & (~(DMA_CR_EDBG_MASK))) | (DMA_CR_EDBG(pxConfig->EnDebug ? TRUE : FALSE)));
    reg = ((reg & (~(DMA_CR_ERCA_MASK))) | (DMA_CR_ERCA(pxConfig->EnRoundRobin ? TRUE : FALSE)));
    reg = ((reg & (~(DMA_CR_HOE_MASK)))  | (DMA_CR_HOE(pxConfig->EnHaltAfterError ? TRUE : FALSE)));
    reg = ((reg & (~(DMA_CR_EMLM_MASK))) | (DMA_CR_EMLM(TRUE)));
    reg = ((reg & (~(DMA_CR_CLM_MASK))));
    ptInst->reg_CR = reg;
}

/*==================================================================================================
 * DMA INSTANCE CMD FUNCTIONS - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaInst_CmdCancelTransfer(Dma_Ip_Hwv2InstRegType * const ptInst)
{
    uint32 reg = ptInst->reg_CR;
    /* Set CX bit to 1 */
    reg = ((reg & (~(DMA_CR_CX_MASK))) | (DMA_CR_CX(TRUE)));
    ptInst->reg_CR = reg;
}
static inline void hwv2AccInlineDmaInst_CmdCancelTransferWithError(Dma_Ip_Hwv2InstRegType * const ptInst)
{
    uint32 reg = ptInst->reg_CR;
    /* Set ECX bit to 1 */
    reg = ((reg & (~(DMA_CR_ECX_MASK))) | (DMA_CR_ECX(TRUE)));
    ptInst->reg_CR = reg;
}
static inline void hwv2AccInlineDmaInst_CmdHalt(Dma_Ip_Hwv2InstRegType * const ptInst)
{
    uint32 reg = ptInst->reg_CR;
    /* Set HALT bit to 1 */
    reg = ((reg & (~(DMA_CR_HALT_MASK))) | (DMA_CR_HALT(TRUE)));
    ptInst->reg_CR = reg;
}
static inline void hwv2AccInlineDmaInst_CmdResume(Dma_Ip_Hwv2InstRegType * const ptInst)
{
    uint32 reg = ptInst->reg_CR;
    /* Set HALT bit to 0 */
    reg = ((reg & (~(DMA_CR_HALT_MASK))) | (DMA_CR_HALT(FALSE)));
    ptInst->reg_CR = reg;
}

/*==================================================================================================
 * DMA INSTANCE STATUS FUNCTIONS - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaInst_GetErrorStatus(const Dma_Ip_Hwv2InstRegType * const ptInst, uint32 * const pValue)
{
    /* Get value from ES register */
    *pValue = (ptInst->reg_ES & DMA_IP_MP_ES_MASK);
}
static inline void hwv2AccInlineDmaInst_GetActiveIdStatus(const Dma_Ip_Hwv2InstRegType * const ptInst, uint8 * const pValue)
{
    /* Hardware version 2 doesn't support get active ID status */
    (void)ptInst;
    *pValue = DMA_IP_HWV2_DOES_NOT_SUPPORT;
}
static inline void hwv2AccInlineDmaInst_GetActiveStatus(const Dma_Ip_Hwv2InstRegType * const ptInst, boolean * const pBool)
{
    /* Get value from CR register */
    *pBool = ((((ptInst->reg_CR & DMA_CR_ACTIVE_MASK) >> DMA_CR_ACTIVE_SHIFT) != 0U) ? TRUE : FALSE);
}
static inline void hwv2AccInlineDmaInst_GetEdmaVersion(const Dma_Ip_Hwv2InstRegType * const ptInst, uint8 * const pValue)
{
    /* Hardware version 2 doesn't support get EDMA version */
    (void)ptInst;
    *pValue = DMA_IP_HWV2_DOES_NOT_SUPPORT;
}

/*==================================================================================================
 * DMA CHANNEL CMD FUNCTIONS - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaCh_CmdSetRequest(Dma_Ip_Hwv2InstRegType * ptCh, const uint32 Channel)
{
    /* Set SERQ bit */
    uint8 reg = DMA_SERQ_SERQ(Channel);
    ptCh->reg_SERQ = ((uint8)reg);
}
static inline void hwv2AccInlineDmaCh_CmdClearRequest(Dma_Ip_Hwv2InstRegType * ptCh, const uint32 Channel)
{
    /* Set CERQ bit */
    uint8 reg = DMA_CERQ_CERQ(Channel);
    ptCh->reg_CERQ = ((uint8)reg);
}
static inline void hwv2AccInlineDmaCh_CmdSwReqStart(Dma_Ip_Hwv2InstRegType * ptCh, const uint32 Channel)
{
    /* Set SSRT bit */
    uint8 reg = DMA_SSRT_SSRT(Channel);
    ptCh->reg_SSRT = ((uint8)reg);
}
static inline void hwv2AccInlineDmaCh_CmdClearDone(Dma_Ip_Hwv2InstRegType * ptCh, const uint32 Channel)
{
    /* Set CDNE bit */
    uint8 reg = DMA_CDNE_CDNE(Channel);
    ptCh->reg_CDNE = ((uint8)reg);
}
static inline void hwv2AccInlineDmaCh_CmdClearError(Dma_Ip_Hwv2InstRegType * ptCh, const uint32 Channel)
{
    /* Set CERR bit */
    uint8 reg = DMA_CERR_CERR(Channel);
    ptCh->reg_CERR = ((uint8)reg);
}

/*==================================================================================================
 * DMA CHANNEL STATUS FUNCTIONS - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaCh_GetErrorStatus(uint32 LocHwCh, const Dma_Ip_Hwv2InstRegType * ptCh, uint32 * const pValue)
{
    if(ptCh->reg_ERR >> LocHwCh)
    {
        /* Get ES register value */
        *pValue = (ptCh->reg_ES & DMA_IP_MP_ES_MASK);
    }
    else
    {
        *pValue = 0U;
    }
}

static inline void hwv2AccInlineDmaCh_GetActiveStatus(const Dma_Ip_TcdRegType * ptTcd, boolean * const pBool)
{
    /* Get ACTIVE bit value */
    *pBool = ((((ptTcd->reg_CSR & DMA_TCD_CSR_ACTIVE_MASK) >> DMA_TCD_CSR_ACTIVE_SHIFT) != 0U) ? TRUE : FALSE);
}
static inline void hwv2AccInlineDmaCh_GetDoneStatus(const Dma_Ip_TcdRegType * ptTcd, boolean * const pBool)
{
    /* Get DONE bit value */
    *pBool = ((((ptTcd->reg_CSR & DMA_TCD_CSR_DONE_MASK) >> DMA_TCD_CSR_DONE_SHIFT) != 0U) ? TRUE : FALSE);
}

/*==================================================================================================
 * DMA CHANNEL GLOBAL FUNCTIONS - REGISTER ACCESS
==================================================================================================*/
#if (STD_ON == DMA_IP_MASTER_ID_REPLICATION_IS_AVAILABLE)
static inline void hwv2AccInlineDmaCh_SetControl_EnMasterIdReplication(Dma_Ip_Hwv2InstRegType * ptCh, uint32 Value)
{
    /* To avoid misra violation */
    (void)ptCh;
    (void)Value;
}
#endif
#if (STD_ON == DMA_IP_BUFFERED_WRITES_IS_AVAILABLE)
static inline void hwv2AccInlineDmaCh_SetControl_EnBufferedWrites(Dma_Ip_Hwv2InstRegType * ptCh, uint32 Value)
{
     /* To avoid misra violation */
    (void)ptCh;
    (void)Value;
}
#endif

static inline void hwv2AccInlineDmaCh_SetRequest_EnHwRequest(Dma_Ip_Hwv2InstRegType * ptCh, uint32 Channel, uint32 Value)
{
    /* Set value for ERQ register */
    uint32 reg = ptCh->reg_ERQ;
    ptCh->reg_ERQ = ((reg & (~((uint32)((uint32)DMA_ERQ_ERQ0_MASK << Channel)))) | (Value << Channel));
}
static inline void hwv2AccInlineDmaCh_SetInterrupt_EnError(Dma_Ip_Hwv2InstRegType * ptCh, uint32 Channel, uint32 Value)
{
    /* Set value for EEI register */
    uint32 reg = ptCh->reg_EEI;
    ptCh->reg_EEI = ((reg & (~((uint32)((uint32)DMA_EEI_EEI0_MASK << Channel)))) | (Value << Channel));
}
static inline void hwv2AccInlineDmaCh_SetPriority_Group(volatile const uint32 * const pGrpri, uint32 Value)
{
    /* To avoid misra violations */
    (void)pGrpri;
    (void)Value;
}
static inline void hwv2AccInlineDmaCh_SetPriority_Level(Dma_Ip_Hwv2InstRegType * ptCh, uint32 Channel, uint32 Value)
{
    /* Set value for CHPRI bit */
    uint8 reg = ptCh->reg_DCHPRI[Channel];
    ptCh->reg_DCHPRI[Channel] = ((uint8)((reg & (~(DMA_DCHPRI_CHPRI_MASK))) | (DMA_DCHPRI_CHPRI(Value))));
}
#if (STD_ON == DMA_IP_PREEMPTION_IS_AVAILABLE)
static inline void hwv2AccInlineDmaCh_SetPriority_EnPreemption(Dma_Ip_Hwv2InstRegType * ptCh, uint32 Channel, uint32 Value)
{
    /* Set value for ECP bit */
    uint8 reg = ptCh->reg_DCHPRI[Channel];
    ptCh->reg_DCHPRI[Channel] = ((uint8)((reg & (~(DMA_DCHPRI_ECP_MASK))) | (DMA_DCHPRI_ECP(Value))));
}
#endif
#if (STD_ON == DMA_IP_DISABLE_PREEMPT_IS_AVAILABLE)
static inline void hwv2AccInlineDmaCh_SetPriority_DisPreempt(Dma_Ip_Hwv2InstRegType * ptCh, uint32 Channel, uint32 Value)
{
    /* Set value for DPA bit */
    uint8 reg = ptCh->reg_DCHPRI[Channel];
    ptCh->reg_DCHPRI[Channel] = ((uint8)((reg & (~(DMA_DCHPRI_DPA_MASK))) | (DMA_DCHPRI_DPA(Value))));
}
#endif

/*==================================================================================================
 * DMA CHANNEL TRANSFER FUNCTIONS - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaCh_SetSource_Address(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for SADDR bit */
    ptTcd->reg_SADDR = (DMA_TCD_SADDR_SADDR(Value));
}
static inline void hwv2AccInlineDmaCh_SetSource_SignedOffset(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for SOFF bit */
    uint32 reg = ptTcd->reg_SOFF;
    ptTcd->reg_SOFF = ((uint16)(reg & (~(DMA_TCD_SOFF_SOFF_MASK))) | (DMA_TCD_SOFF_SOFF(Value)));
}
static inline void hwv2AccInlineDmaCh_SetSource_SignedLastAddrAdj(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for SLAST bit */
    ptTcd->reg_SLAST_SDA = (DMA_TCD_SLAST_SLAST(Value));
}
static inline void hwv2AccInlineDmaCh_SetSource_Params(Dma_Ip_TcdRegType * ptTcd, const uint32 Value1, const uint32 Value2, const uint32 Value3)
{
    uint32 reg_val2 = ptTcd->reg_SOFF;

    /* Set value for SADDR bit */
    ptTcd->reg_SADDR = (DMA_TCD_SADDR_SADDR(Value1));
    /* Set value for SOFF bit */
    ptTcd->reg_SOFF = ((uint16)(reg_val2 & (~(DMA_TCD_SOFF_SOFF_MASK))) | (DMA_TCD_SOFF_SOFF(Value2)));

    /* Set value for SLAST bit */
    ptTcd->reg_SLAST_SDA = (DMA_TCD_SLAST_SLAST(Value3));
}
static inline void hwv2AccInlineDmaCh_SetSource_TransferSize(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for SSIZE bit */
    uint32 reg = ptTcd->reg_ATTR;
    ptTcd->reg_ATTR = ((uint16)(reg & (~(DMA_TCD_ATTR_SSIZE_MASK))) | (DMA_TCD_ATTR_SSIZE(Value)));
}
static inline void hwv2AccInlineDmaCh_SetSource_Modulo(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for SMOD bit */
    uint32 reg = ptTcd->reg_ATTR;
    ptTcd->reg_ATTR = ((uint16)(reg & (~(DMA_TCD_ATTR_SMOD_MASK))) | (DMA_TCD_ATTR_SMOD(Value)));
}
static inline void hwv2AccInlineDmaCh_SetDestination_Address(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for DADDR bit */
    ptTcd->reg_DADDR = (DMA_TCD_DADDR_DADDR(Value));
}
static inline void hwv2AccInlineDmaCh_SetDestination_SignedOffset(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for DOFF bit */
    uint16 reg = ptTcd->reg_DOFF;
    ptTcd->reg_DOFF = ((uint16)(reg & (~(DMA_TCD_DOFF_DOFF_MASK))) | (DMA_TCD_DOFF_DOFF(Value)));
}
static inline void hwv2AccInlineDmaCh_SetDestination_Params(Dma_Ip_TcdRegType * ptTcd, const uint32 Value1, const uint32 Value2)
{
    uint16 reg_val2 = ptTcd->reg_DOFF;

    /* Set value for DADDR bit */
    ptTcd->reg_DADDR = (DMA_TCD_DADDR_DADDR(Value1));
    /* Set value for DOFF bit */
    ptTcd->reg_DOFF = ((uint16)(reg_val2 & (~(DMA_TCD_DOFF_DOFF_MASK))) | (DMA_TCD_DOFF_DOFF(Value2)));
}
static inline void hwv2AccInlineDmaCh_SetDestination_LastAddrAdj(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for DLASTSGA bit */
    ptTcd->reg_DLAST_SGA = (DMA_TCD_DLASTSGA_DLASTSGA(Value));
}
static inline void hwv2AccInlineDmaCh_SetDestination_TransferSize(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for DSIZE bit */
    uint16 reg = ptTcd->reg_ATTR;
    ptTcd->reg_ATTR = ((uint16)(reg & (~(DMA_TCD_ATTR_DSIZE_MASK))) | (DMA_TCD_ATTR_DSIZE(Value)));
}
static inline void hwv2AccInlineDmaCh_SetDestination_Modulo(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for DMOD bit */
    uint16 reg = ptTcd->reg_ATTR;
    ptTcd->reg_ATTR = ((uint16)(reg & (~(DMA_TCD_ATTR_DMOD_MASK))) | (DMA_TCD_ATTR_DMOD(Value)));
}
static inline void hwv2AccInlineDmaCh_SetMinorLoop_enSrcOffset(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for SMLOE bit */
    uint32 reg = ptTcd->reg_NBYTES.reg_MLOFFYES;
    ptTcd->reg_NBYTES.reg_MLOFFYES = ((reg & (~(DMA_TCD_NBYTES_MLOFFYES_SMLOE_MASK))) | (DMA_TCD_NBYTES_MLOFFYES_SMLOE(Value)));
}
static inline void hwv2AccInlineDmaCh_SetMinorLoop_enDstOffset(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for DMLOE bit */
    uint32 reg = ptTcd->reg_NBYTES.reg_MLOFFYES;
    ptTcd->reg_NBYTES.reg_MLOFFYES = ((reg & (~(DMA_TCD_NBYTES_MLOFFYES_DMLOE_MASK))) | (DMA_TCD_NBYTES_MLOFFYES_DMLOE(Value)));
}
static inline void hwv2AccInlineDmaCh_SetMinorLoop_SignedOffset(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for MLOFF bit */
    uint32 reg = ptTcd->reg_NBYTES.reg_MLOFFYES;
    ptTcd->reg_NBYTES.reg_MLOFFYES = ((reg & (~(DMA_TCD_NBYTES_MLOFFYES_MLOFF_MASK))) | (DMA_TCD_NBYTES_MLOFFYES_MLOFF(Value)));
}
static inline void hwv2AccInlineDmaCh_SetMinorLoop_EnLink(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for ELINK bit */
    uint16 reg = ptTcd->reg_BITER.reg_ELINKYES;
    ptTcd->reg_BITER.reg_ELINKYES = ((uint16)(reg & (~(DMA_TCD_BITER_ELINKYES_ELINK_MASK))) | (DMA_TCD_BITER_ELINKYES_ELINK(Value)));
    reg = ptTcd->reg_CITER.reg_ELINKYES;
    ptTcd->reg_CITER.reg_ELINKYES = ((uint16)(reg & (~(DMA_TCD_CITER_ELINKYES_ELINK_MASK))) | (DMA_TCD_CITER_ELINKYES_ELINK(Value)));
}
static inline void hwv2AccInlineDmaCh_SetMinorLoop_LinkCh(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for LINKCH bit */
    uint16 reg = ptTcd->reg_BITER.reg_ELINKYES;
    ptTcd->reg_BITER.reg_ELINKYES = ((uint16)(reg & (~(DMA_TCD_BITER_ELINKYES_LINKCH_MASK))) | (DMA_TCD_BITER_ELINKYES_LINKCH(Value)));
    reg = ptTcd->reg_CITER.reg_ELINKYES;
    ptTcd->reg_CITER.reg_ELINKYES = ((uint16)(reg & (~(DMA_TCD_CITER_ELINKYES_LINKCH_MASK))) | (DMA_TCD_CITER_ELINKYES_LINKCH(Value)));
}
static inline void hwv2AccInlineDmaCh_SetMinorLoop_Size(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for NBYTES bit */
    uint32 reg = ptTcd->reg_NBYTES.reg_MLOFFNO;
    if(0U != (reg & (DMA_TCD_NBYTES_MLOFFYES_SMLOE_MASK | DMA_TCD_NBYTES_MLOFFYES_DMLOE_MASK)))
    {
        ptTcd->reg_NBYTES.reg_MLOFFNO = ((reg & (~(DMA_TCD_NBYTES_MLOFFYES_NBYTES_MASK))) | (DMA_TCD_NBYTES_MLOFFYES_NBYTES(Value)));
    }
    else
    {
        ptTcd->reg_NBYTES.reg_MLOFFNO = ((reg & (~(DMA_TCD_NBYTES_MLOFFNO_NBYTES_MASK))) | (DMA_TCD_NBYTES_MLOFFNO_NBYTES(Value)));
    }
}
static inline void hwv2AccInlineDmaCh_SetMinorLoop_Params(Dma_Ip_TcdRegType * ptTcd, const uint32 Value1, const uint32 Value2, const uint32 Value3)
{
    uint16 reg_val1 = ptTcd->reg_BITER.reg_ELINKYES;
    uint32 reg_val2 = ptTcd->reg_NBYTES.reg_MLOFFNO;

    /* Set value for ELINK bit */
    ptTcd->reg_BITER.reg_ELINKYES = ((uint16)(reg_val1 & (~(DMA_TCD_BITER_ELINKYES_ELINK_MASK))) | (DMA_TCD_BITER_ELINKYES_ELINK(Value1)));
    reg_val1 = ptTcd->reg_BITER.reg_ELINKYES;
    ptTcd->reg_BITER.reg_ELINKYES = ((uint16)(reg_val1 & (~(DMA_TCD_BITER_ELINKYES_LINKCH_MASK))) | (DMA_TCD_BITER_ELINKYES_LINKCH(Value2)));

    reg_val1 = ptTcd->reg_CITER.reg_ELINKYES;
    /* Set value for LINKCH bit */
    ptTcd->reg_CITER.reg_ELINKYES = ((uint16)(reg_val1 & (~(DMA_TCD_CITER_ELINKYES_ELINK_MASK))) | (DMA_TCD_CITER_ELINKYES_ELINK(Value1)));
    reg_val1 = ptTcd->reg_CITER.reg_ELINKYES;
    ptTcd->reg_CITER.reg_ELINKYES = ((uint16)(reg_val1 & (~(DMA_TCD_CITER_ELINKYES_LINKCH_MASK))) | (DMA_TCD_CITER_ELINKYES_LINKCH(Value2)));

    /* Set value for NBYTES bit */
    if(0U != (reg_val2 & (DMA_TCD_NBYTES_MLOFFYES_SMLOE_MASK | DMA_TCD_NBYTES_MLOFFYES_DMLOE_MASK)))
    {
        ptTcd->reg_NBYTES.reg_MLOFFNO = ((reg_val2 & (~(DMA_TCD_NBYTES_MLOFFYES_NBYTES_MASK))) | (DMA_TCD_NBYTES_MLOFFYES_NBYTES(Value3)));
    }
    else
    {
        ptTcd->reg_NBYTES.reg_MLOFFNO = ((reg_val2 & (~(DMA_TCD_NBYTES_MLOFFNO_NBYTES_MASK))) | (DMA_TCD_NBYTES_MLOFFNO_NBYTES(Value3)));
    }
}
static inline void hwv2AccInlineDmaCh_SetMajorLoop_EnLink(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for MAJORELINK bit */
    uint16 reg = ptTcd->reg_CSR;
    ptTcd->reg_CSR = ((uint16)(reg & (~(DMA_TCD_CSR_MAJORELINK_MASK))) | (DMA_TCD_CSR_MAJORELINK(Value)));
}
static inline void hwv2AccInlineDmaCh_SetMajorLoop_LinkCh(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for MAJORLINKCH bit */
    uint16 reg = ptTcd->reg_CSR;
    ptTcd->reg_CSR = ((uint16)(reg & (~(DMA_TCD_CSR_MAJORLINKCH_MASK))) | (DMA_TCD_CSR_MAJORLINKCH(Value)));
}
static inline void hwv2AccInlineDmaCh_SetMajorLoop_Count(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for BITER and CITER bit */
    uint16 reg = (ptTcd->reg_BITER.reg_ELINKYES & (DMA_TCD_BITER_ELINKYES_ELINK_MASK)) |
                 (ptTcd->reg_CITER.reg_ELINKYES & (DMA_TCD_CITER_ELINKYES_ELINK_MASK));
    if(0U != reg)
    {
        reg = ptTcd->reg_BITER.reg_ELINKYES;
        ptTcd->reg_BITER.reg_ELINKYES = ((uint16)(reg & (~(DMA_TCD_BITER_ELINKYES_BITER_MASK))) | (DMA_TCD_BITER_ELINKYES_BITER(Value)));
        reg = ptTcd->reg_CITER.reg_ELINKYES;
        ptTcd->reg_CITER.reg_ELINKYES = ((uint16)(reg & (~(DMA_TCD_CITER_ELINKYES_CITER_MASK))) | (DMA_TCD_CITER_ELINKYES_CITER(Value)));
    }
    else
    {
        reg = ptTcd->reg_BITER.reg_ELINKNO;
        ptTcd->reg_BITER.reg_ELINKNO = ((uint16)(reg & (~(DMA_TCD_BITER_ELINKNO_BITER_MASK))) | (DMA_TCD_BITER_ELINKNO_BITER(Value)));
        reg = ptTcd->reg_CITER.reg_ELINKNO;
        ptTcd->reg_CITER.reg_ELINKNO = ((uint16)(reg & (~(DMA_TCD_CITER_ELINKNO_CITER_MASK))) | (DMA_TCD_CITER_ELINKNO_CITER(Value)));
    }
}
static inline void hwv2AccInlineDmaCh_SetMajorLoop_Params(Dma_Ip_TcdRegType * ptTcd, const uint32 Value1, const uint32 Value2, const uint32 Value3)
{
    uint16 reg_val1 = ptTcd->reg_CSR;
    uint16 reg_val2;
    uint16 reg_val3 = (ptTcd->reg_BITER.reg_ELINKYES & (DMA_TCD_BITER_ELINKYES_ELINK_MASK)) |
                 (ptTcd->reg_CITER.reg_ELINKYES & (DMA_TCD_CITER_ELINKYES_ELINK_MASK));

    /* Set value for MAJORELINK bit */
    ptTcd->reg_CSR = ((uint16)(reg_val1 & (~(DMA_TCD_CSR_MAJORELINK_MASK))) | (DMA_TCD_CSR_MAJORELINK(Value1)));

    /* Set value for MAJORLINKCH bit */
    reg_val2 = ptTcd->reg_CSR;
    ptTcd->reg_CSR = ((uint16)(reg_val2 & (~(DMA_TCD_CSR_MAJORLINKCH_MASK))) | (DMA_TCD_CSR_MAJORLINKCH(Value2)));

    /* Set value for BITER and CITER bit */
    if(0U != reg_val3)
    {
        reg_val3 = ptTcd->reg_BITER.reg_ELINKYES;
        ptTcd->reg_BITER.reg_ELINKYES = ((uint16)(reg_val3 & (~(DMA_TCD_BITER_ELINKYES_BITER_MASK))) | (DMA_TCD_BITER_ELINKYES_BITER(Value3)));
        reg_val3 = ptTcd->reg_CITER.reg_ELINKYES;
        ptTcd->reg_CITER.reg_ELINKYES = ((uint16)(reg_val3 & (~(DMA_TCD_CITER_ELINKYES_CITER_MASK))) | (DMA_TCD_CITER_ELINKYES_CITER(Value3)));
    }
    else
    {
        reg_val3 = ptTcd->reg_BITER.reg_ELINKNO;
        ptTcd->reg_BITER.reg_ELINKNO = ((uint16)(reg_val3 & (~(DMA_TCD_BITER_ELINKNO_BITER_MASK))) | (DMA_TCD_BITER_ELINKNO_BITER(Value3)));
        reg_val3 = ptTcd->reg_CITER.reg_ELINKNO;
        ptTcd->reg_CITER.reg_ELINKNO = ((uint16)(reg_val3 & (~(DMA_TCD_CITER_ELINKNO_CITER_MASK))) | (DMA_TCD_CITER_ELINKNO_CITER(Value3)));
    }
}
static inline void hwv2AccInlineDmaCh_SetControl_ScatterGatherAddress(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for DLASTSGA bit */
    ptTcd->reg_DLAST_SGA = (DMA_TCD_DLASTSGA_DLASTSGA(Value));
}
#if (STD_ON == DMA_IP_STORE_DST_ADDR_IS_AVAILABLE)
static inline void hwv2AccInlineDmaCh_SetControl_StoreDestinationAddress(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* To avoid misra violations */
    (void)ptTcd;
    (void)Value;
}
#endif
static inline void hwv2AccInlineDmaCh_SetControl_EnStart(Dma_Ip_TcdRegType * ptTcd, const boolean Value)
{
    /* Set value for START bit */
    uint16 reg = ptTcd->reg_CSR;
    ptTcd->reg_CSR = ((uint16)(reg & (~(DMA_TCD_CSR_START_MASK))) | (DMA_TCD_CSR_START(Value ? 1U : 0U)));
}
static inline void hwv2AccInlineDmaCh_SetControl_EnMajor(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for INTMAJOR bit */
    uint16 reg = ptTcd->reg_CSR;
    ptTcd->reg_CSR = ((uint16)(reg & (~(DMA_TCD_CSR_INTMAJOR_MASK))) | (DMA_TCD_CSR_INTMAJOR(Value)));
}
static inline void hwv2AccInlineDmaCh_SetControl_EnHalfMajor(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for INTHALF bit */
    uint16 reg = ptTcd->reg_CSR;
    ptTcd->reg_CSR = ((uint16)(reg & (~(DMA_TCD_CSR_INTHALF_MASK))) | (DMA_TCD_CSR_INTHALF(Value)));
}
static inline void hwv2AccInlineDmaCh_SetControl_DisAutoHwRequest(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for DREQ bit */
    uint16 reg = ptTcd->reg_CSR;
    ptTcd->reg_CSR = ((uint16)(reg & (~(DMA_TCD_CSR_DREQ_MASK))) | (DMA_TCD_CSR_DREQ(Value)));
}
#if (STD_ON == DMA_IP_END_OF_PACKET_SIGNAL_IS_AVAILABLE)
static inline void hwv2AccInlineDmaCh_SetControl_EnEndOfPacketSignal(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* To avoid misra violations */
    (void)ptTcd;
    (void)Value;
}
#endif
static inline void hwv2AccInlineDmaCh_SetControl_BandwidthControl(Dma_Ip_TcdRegType * ptTcd, const uint32 Value)
{
    /* Set value for BWC bit */
    uint16 reg = ptTcd->reg_CSR;
    ptTcd->reg_CSR = ((uint16)(reg & (~(DMA_TCD_CSR_BWC_MASK))) | (DMA_TCD_CSR_BWC(Value)));
}

/*==================================================================================================
 * DMA CHANNEL GET PARAMETER - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaCh_GetSourceAddress(const Dma_Ip_TcdRegType * ptTcd, uint32 * const retValue)
{
    /* Get SADDR register value */
    *retValue = (uint32)ptTcd->reg_SADDR;
}
static inline void hwv2AccInlineDmaCh_GetDestinationAddress(const Dma_Ip_TcdRegType * ptTcd, uint32 * const retValue)
{
    /* Get DADDR register value */
    *retValue = (uint32)ptTcd->reg_DADDR;
}
static inline void hwv2AccInlineDmaCh_GetBeginIterCount(const Dma_Ip_TcdRegType * ptTcd, uint32 * const retValue)
{
    /* Get BITER bit value */
    uint16 reg = ptTcd->reg_BITER.reg_ELINKYES & (DMA_TCD_BITER_ELINKYES_ELINK_MASK);
    if(0U != reg)
    {
        *retValue = (uint32)((ptTcd->reg_BITER.reg_ELINKYES & (uint32)DMA_TCD_BITER_ELINKYES_BITER_MASK) >> DMA_TCD_BITER_ELINKYES_BITER_SHIFT);
    }
    else
    {
        *retValue = (uint32)((ptTcd->reg_BITER.reg_ELINKYES & (uint32)DMA_TCD_BITER_ELINKNO_BITER_MASK) >> DMA_TCD_BITER_ELINKNO_BITER_SHIFT);
    }
}
static inline void hwv2AccInlineDmaCh_GetCurrentIterCount(const Dma_Ip_TcdRegType * ptTcd, uint32 * const retValue)
{
    /* Get CITER bit value */
    uint16 reg = (ptTcd->reg_CITER.reg_ELINKYES & (DMA_TCD_CITER_ELINKYES_ELINK_MASK));
    if(0U != reg)
    {
        *retValue = (uint32)((ptTcd->reg_CITER.reg_ELINKYES & (uint32)DMA_TCD_CITER_ELINKYES_CITER_MASK) >> DMA_TCD_CITER_ELINKYES_CITER_SHIFT);
    }
    else
    {
        *retValue = (uint32)((ptTcd->reg_CITER.reg_ELINKNO & (uint32)DMA_TCD_CITER_ELINKNO_CITER_MASK) >> DMA_TCD_CITER_ELINKNO_CITER_SHIFT);
    }
}
#if (STD_ON == DMA_IP_STORE_DST_ADDR_IS_AVAILABLE)
static inline void hwv2AccInlineDmaCh_GetStoreDstAddress(Dma_Ip_TcdRegType * ptTcd, uint32 * const retValue)
{
    /* Hardware version 2 doesn't have this feature */
    (void)ptTcd;
    *retValue = (uint32)0U;
}
#endif
#if (STD_ON == DMA_IP_MASTER_ID_REPLICATION_IS_AVAILABLE)
static inline void hwv2AccInlineDmaCh_GetMasterId(Dma_Ip_ChRegType * ptCh, uint32 * const retValue)
{
    /* Hardware version 2 doesn't have this feature */
    (void)ptCh;
    *retValue = (uint32)0U;
}
#endif
static inline void hwv2AccInlineDmaCh_GetIntMajor(const Dma_Ip_TcdRegType * ptTcd, uint32 * const retValue)
{
    /* Get INTMAJOR bit value */
    *retValue = (uint32)((ptTcd->reg_CSR & (uint32)DMA_TCD_CSR_INTMAJOR_MASK) >> DMA_TCD_CSR_INTMAJOR_SHIFT);
}
static inline void hwv2AccInlineDmaCh_GetIntHalfMajor(const Dma_Ip_TcdRegType * ptTcd, uint32 * const retValue)
{
    /* Get INTHALF bit value */
    *retValue = (uint32)((ptTcd->reg_CSR & (uint32)DMA_TCD_CSR_INTHALF_MASK) >> DMA_TCD_CSR_INTHALF_SHIFT);
}

/*==================================================================================================
 * DMA CHANNEL AUXILIARY FUNCTIONS - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaCh_SetAuxiliary_EnScatterGatherProcessing(Dma_Ip_TcdRegType * ptTcd, boolean Value)
{
    /* Set ESG bit value */
    uint16 reg = ptTcd->reg_CSR;
    ptTcd->reg_CSR = ((uint16)(reg & (~(DMA_TCD_CSR_ESG_MASK))) | (DMA_TCD_CSR_ESG(Value ? 1U : 0U)));
}
#if (STD_ON == DMA_IP_STORE_DST_ADDR_IS_AVAILABLE)
static inline void hwv2AccInlineDmaCh_SetAuxiliary_EnDestinationStoreAddress(Dma_Ip_TcdRegType * ptTcd, boolean Value)
{
    /* To avoid misra violations */
    (void)ptTcd;
    (void)Value;
}
#endif

/*==================================================================================================
 * TCD CONTROL AND STATUS (TCD_CSR) - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaCh_SetControlAndStatus(Dma_Ip_TcdRegType * ptTcd, const Dma_Ip_ScatterGatherConfigType * const pxLocScatterGather)
{
    /* Set value for TCD_CSR register*/
    uint16 reg = ptTcd->reg_CSR;
    reg = ((uint16)(reg & (~(DMA_TCD_CSR_START_MASK))) | (DMA_TCD_CSR_START(pxLocScatterGather->TransferConfig->Control.EnStart ? 1U : 0U)));
    reg = ((uint16)(reg & (~(DMA_TCD_CSR_BWC_MASK))) | (DMA_TCD_CSR_BWC(pxLocScatterGather->TransferConfig->Control.BandwidthControl)));
    reg = ((uint16)(reg & (~(DMA_TCD_CSR_INTMAJOR_MASK))) | (DMA_TCD_CSR_INTMAJOR(pxLocScatterGather->TransferConfig->Control.EnMajorInt ? 1U : 0U)));
    reg = ((uint16)(reg & (~(DMA_TCD_CSR_INTHALF_MASK))) | (DMA_TCD_CSR_INTHALF(pxLocScatterGather->TransferConfig->Control.EnHalfMajorInt ? 1U : 0U)));
    reg = ((uint16)(reg & (~(DMA_TCD_CSR_DREQ_MASK))) | (DMA_TCD_CSR_DREQ(pxLocScatterGather->TransferConfig->Control.DisAutoHwRequest ? 1U : 0U)));
    ptTcd->reg_CSR= reg;
}

/*==================================================================================================
 * TCD TRANSFER ATTRIBUTES (TCD_ATTR) - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaCh_SetTransferAttributes(Dma_Ip_TcdRegType * ptTcd, const Dma_Ip_ScatterGatherConfigType * const pxLocScatterGather)
{
    /* Set value for TCD_ATTR register*/
    uint16 reg = ptTcd->reg_ATTR;
    reg = ((uint16)(reg & (~(DMA_TCD_ATTR_SSIZE_MASK))) | (DMA_TCD_ATTR_SSIZE(pxLocScatterGather->TransferConfig->Source.TransferSize)));
    reg = ((uint16)(reg & (~(DMA_TCD_ATTR_SMOD_MASK))) | (DMA_TCD_ATTR_SMOD(pxLocScatterGather->TransferConfig->Source.Modulo)));
    reg = ((uint16)(reg & (~(DMA_TCD_ATTR_DSIZE_MASK))) | (DMA_TCD_ATTR_DSIZE(pxLocScatterGather->TransferConfig->Destination.TransferSize)));
    reg = ((uint16)(reg & (~(DMA_TCD_ATTR_DMOD_MASK))) | (DMA_TCD_ATTR_DMOD(pxLocScatterGather->TransferConfig->Destination.Modulo)));
    ptTcd->reg_ATTR= reg;
}

/*==================================================================================================
 * TCD SIGNED MINOR LOOP OFFSET (TCD_NBYTES_MLOFFYES) - REGISTER ACCESS
==================================================================================================*/
static inline void hwv2AccInlineDmaCh_SetSignedMinorLoopOffset(Dma_Ip_TcdRegType * ptTcd, const Dma_Ip_ScatterGatherConfigType * const pxLocScatterGather)
{
    /* Set value for TCD_NBYTES_MLOFFYES register*/
    uint32 reg = ptTcd->reg_NBYTES.reg_MLOFFYES;
    reg = ((reg & (~(DMA_TCD_NBYTES_MLOFFYES_SMLOE_MASK))) | (DMA_TCD_NBYTES_MLOFFYES_SMLOE(pxLocScatterGather->TransferConfig->MinorLoop.EnSrcOffset ? 1U : 0U)));
    reg = ((reg & (~(DMA_TCD_NBYTES_MLOFFYES_DMLOE_MASK))) | (DMA_TCD_NBYTES_MLOFFYES_DMLOE(pxLocScatterGather->TransferConfig->MinorLoop.EnDstOffset ? 1U : 0U)));
    reg = ((reg & (~(DMA_TCD_NBYTES_MLOFFYES_MLOFF_MASK))) | (DMA_TCD_NBYTES_MLOFFYES_MLOFF((uint32)pxLocScatterGather->TransferConfig->MinorLoop.Offset)));
    ptTcd->reg_NBYTES.reg_MLOFFYES= reg;
}

#define MCL_STOP_SEC_CODE
#include "Mcl_MemMap.h"

#endif /* #if (STD_ON == DMA_IP_HWV2_IS_AVAILABLE) */
#endif /* #if (STD_ON == DMA_IP_IS_AVAILABLE) */

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* #ifndef DMA_IP_HWV2_ACCESSINLINE_H_ */

