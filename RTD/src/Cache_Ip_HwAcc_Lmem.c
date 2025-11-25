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
*   @file    Cache_Ip_HwAcc_Lmem.c
*
*   @version 2.0.0
*
*   @brief   AUTOSAR Mcl - Cache Ip driver header file.
*   @details
*
*   @addtogroup CACHE_IP_DRIVER CACHE IP Driver
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
#include "Cache_Ip_HwAcc_Lmem.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define CACHE_IP_HWACC_LMEM_VENDOR_ID_C                       43
#define CACHE_IP_HWACC_LMEM_AR_RELEASE_MAJOR_VERSION_C        4
#define CACHE_IP_HWACC_LMEM_AR_RELEASE_MINOR_VERSION_C        7
#define CACHE_IP_HWACC_LMEM_AR_RELEASE_REVISION_VERSION_C     0
#define CACHE_IP_HWACC_LMEM_SW_MAJOR_VERSION_C                2
#define CACHE_IP_HWACC_LMEM_SW_MINOR_VERSION_C                0
#define CACHE_IP_HWACC_LMEM_SW_PATCH_VERSION_C                0


/*==================================================================================================
                                      FILE VERSION CHECKS
==================================================================================================*/
/* Check if header file and Cache_Ip_HwAcc_Lmem.h file are of the same vendor */
#if (CACHE_IP_HWACC_LMEM_VENDOR_ID_C != CACHE_IP_HWACC_LMEM_VENDOR_ID)
    #error "Cache_Ip_HwAcc_Lmem.c and Cache_Ip_HwAcc_Lmem.h have different vendor ids"
#endif

/* Check if header file and Cache_Ip_HwAcc_Lmem.h file are of the same Autosar version */
#if ((CACHE_IP_HWACC_LMEM_AR_RELEASE_MAJOR_VERSION_C != CACHE_IP_HWACC_LMEM_AR_RELEASE_MAJOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_AR_RELEASE_MINOR_VERSION_C != CACHE_IP_HWACC_LMEM_AR_RELEASE_MINOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_AR_RELEASE_REVISION_VERSION_C != CACHE_IP_HWACC_LMEM_AR_RELEASE_REVISION_VERSION) \
    )
    #error "AutoSar Version Numbers of Cache_Ip_HwAcc_Lmem.c and Cache_Ip_HwAcc_Lmem.h are different"
#endif

/* Check if header file and Cache_Ip_HwAcc_Lmem.h file are of the same Software version */
#if ((CACHE_IP_HWACC_LMEM_SW_MAJOR_VERSION_C != CACHE_IP_HWACC_LMEM_SW_MAJOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_SW_MINOR_VERSION_C != CACHE_IP_HWACC_LMEM_SW_MINOR_VERSION) || \
     (CACHE_IP_HWACC_LMEM_SW_PATCH_VERSION_C != CACHE_IP_HWACC_LMEM_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of Cache_Ip_HwAcc_Lmem.c and Cache_Ip_HwAcc_Lmem.h are different"
#endif


#if (STD_ON == CACHE_IP_IS_AVAILABLE)

#if (STD_ON == CACHE_IP_LMEM_IS_AVAILABLE)
/*==================================================================================================
*                                            CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                       DEFINES AND MACROS
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
#define MCL_START_SEC_CONST_UNSPECIFIED
#include "Mcl_MemMap.h"

static LMEM_Type * const g_ptLmemBasePtrArray[LMEM_INSTANCE_COUNT] = IP_LMEM_BASE_PTRS;

#define MCL_STOP_SEC_CONST_UNSPECIFIED
#include "Mcl_MemMap.h"

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
 *                                       LOCAL FUNCTIONS
==================================================================================================*/
#define MCL_START_SEC_CODE
/* @violates @ref Mcl_Dma_h_REF_1 MISRA 2012 Required Directive 4.10, Precautions shall be taken in order to prevent the contents of a header file being included more than once. */
#include "Mcl_MemMap.h"

#if (STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE)
LOCAL_INLINE Std_ReturnType hwAcc_Lmem_ProcessorCodeControlReg(LMEM_Type * pInst, uint32 ControlMask, uint32 GoMask)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locTimeoutTicks = OsIf_MicrosToTicks(CACHE_IP_LMEM_COMMAND_TIMEOUT_VAL, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locCurrentTicks = OsIf_GetCounter(CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locElapsedTicks = 0U;
    uint32 locReg;

    /* Set value for PCCCR register */
    locReg = pInst->PCCCR;
    locReg &= ~(LMEM_PCCCR_INVW0_MASK | LMEM_PCCCR_INVW1_MASK | LMEM_PCCCR_PUSHW0_MASK | LMEM_PCCCR_PUSHW1_MASK);
    locReg |= (ControlMask | GoMask);
    pInst->PCCCR = locReg;

    if (0UL != GoMask)
    {
        while ((GoMask == (GoMask & (pInst->PCCCR))) && (locElapsedTicks < locTimeoutTicks))
        {
            locElapsedTicks += OsIf_GetElapsed(&locCurrentTicks, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
        }
        if (locElapsedTicks >= locTimeoutTicks)
        {
            locStatus = (Std_ReturnType)E_NOT_OK;
        }
    }

    return locStatus;
}

LOCAL_INLINE Std_ReturnType hwAcc_Lmem_ProcessorCodeLineControlReg(LMEM_Type * pInst, uint32 LineControlMask, uint32 GoMask)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locTimeoutTicks = OsIf_MicrosToTicks(CACHE_IP_LMEM_COMMAND_TIMEOUT_VAL, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locCurrentTicks = OsIf_GetCounter(CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locElapsedTicks = 0U;
    uint32 locReg;

    /* Set value for PCCLCR register */
    locReg = pInst->PCCLCR;
    locReg &= ~(LineControlMask | LMEM_PCCLCR_LCMD_MASK);
    locReg |= (LineControlMask | GoMask);
    pInst->PCCLCR = locReg;

    if (0UL != GoMask)
    {
        while ((GoMask == (GoMask & (pInst->PCCLCR))) && (locElapsedTicks < locTimeoutTicks))
        {
            locElapsedTicks += OsIf_GetElapsed(&locCurrentTicks, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
        }
        if (locElapsedTicks >= locTimeoutTicks)
        {
            locStatus = (Std_ReturnType)E_NOT_OK;
        }
    }

    return locStatus;
}

LOCAL_INLINE Std_ReturnType hwAcc_Lmem_ProcessorCodeAddressReg(LMEM_Type * pInst, uint32 Address, uint32 GoMask)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locTimeoutTicks = OsIf_MicrosToTicks(CACHE_IP_LMEM_COMMAND_TIMEOUT_VAL, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locCurrentTicks = OsIf_GetCounter(CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locElapsedTicks = 0U;
    uint32 locReg;

    /* Set value for PCCSAR register */
    locReg = pInst->PCCSAR;
    locReg &= (~LMEM_PCCSAR_PHYADDR_MASK);
    locReg |= ((Address & LMEM_PCCSAR_PHYADDR_MASK) | GoMask);
    pInst->PCCSAR = locReg;

    if (0UL != GoMask)
    {
        while ((GoMask == (GoMask & (pInst->PCCSAR))) && (locElapsedTicks < locTimeoutTicks))
        {
            locElapsedTicks += OsIf_GetElapsed(&locCurrentTicks, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
        }
        if (locElapsedTicks >= locTimeoutTicks)
        {
            locStatus = (Std_ReturnType)E_NOT_OK;
        }
    }

    return locStatus;
}
#endif /* STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE */

#if (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE)
LOCAL_INLINE Std_ReturnType hwAcc_Lmem_ProcessorSystemControlReg(LMEM_Type * pInst, uint32 ControlMask, uint32 GoMask)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locTimeoutTicks = OsIf_MicrosToTicks(CACHE_IP_LMEM_COMMAND_TIMEOUT_VAL, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locCurrentTicks = OsIf_GetCounter(CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locElapsedTicks = 0U;
    uint32 locReg;

    /* Set value for PSCCR register */
    locReg = pInst->PSCCR;
    locReg &= ~(LMEM_PSCCR_INVW0_MASK | LMEM_PSCCR_INVW1_MASK | LMEM_PSCCR_PUSHW0_MASK | LMEM_PSCCR_PUSHW1_MASK);
    locReg |= (ControlMask | GoMask);
    pInst->PSCCR = locReg;

    if (0UL != GoMask)
    {
        while ((GoMask == (GoMask & (pInst->PSCCR))) && (locElapsedTicks < locTimeoutTicks))
        {
            locElapsedTicks += OsIf_GetElapsed(&locCurrentTicks, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
        }
        if (locElapsedTicks >= locTimeoutTicks)
        {
            locStatus = (Std_ReturnType)E_NOT_OK;
        }
    }

    return locStatus;
}

LOCAL_INLINE Std_ReturnType hwAcc_Lmem_ProcessorSystemLineControlReg(LMEM_Type * pInst, uint32 LineControlMask, uint32 GoMask)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locTimeoutTicks = OsIf_MicrosToTicks(CACHE_IP_LMEM_COMMAND_TIMEOUT_VAL, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locCurrentTicks = OsIf_GetCounter(CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locElapsedTicks = 0U;
    uint32 locReg;

    /* Set value for PSCLCR register */
    locReg = pInst->PSCLCR;
    locReg &= ~(LineControlMask | LMEM_PSCLCR_LCMD_MASK);
    locReg |= (LineControlMask | GoMask);
    pInst->PSCLCR = locReg;

    if (0UL != GoMask)
    {
        while ((GoMask == (GoMask & (pInst->PSCLCR))) && (locElapsedTicks < locTimeoutTicks))
        {
            locElapsedTicks += OsIf_GetElapsed(&locCurrentTicks, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
        }
        if (locElapsedTicks >= locTimeoutTicks)
        {
            locStatus = (Std_ReturnType)E_NOT_OK;
        }
    }

    return locStatus;
}

LOCAL_INLINE Std_ReturnType hwAcc_Lmem_ProcessorSystemAddressReg(LMEM_Type * pInst, uint32 Address, uint32 GoMask)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locTimeoutTicks = OsIf_MicrosToTicks(CACHE_IP_LMEM_COMMAND_TIMEOUT_VAL, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locCurrentTicks = OsIf_GetCounter(CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locElapsedTicks = 0U;
    uint32 locReg;

    /* Set value for PSCSAR register */
    locReg = pInst->PSCSAR;
    locReg &= (~LMEM_PSCSAR_PHYADDR_MASK);
    locReg |= ((Address & LMEM_PSCSAR_PHYADDR_MASK) | GoMask);
    pInst->PSCSAR = locReg;

    if (0UL != GoMask)
    {
        while ((GoMask == (GoMask & (pInst->PSCSAR))) && (locElapsedTicks < locTimeoutTicks))
        {
            locElapsedTicks += OsIf_GetElapsed(&locCurrentTicks, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
        }
        if (locElapsedTicks >= locTimeoutTicks)
        {
            locStatus = (Std_ReturnType)E_NOT_OK;
        }
    }

    return locStatus;
}
#endif /* STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE */

#define MCL_STOP_SEC_CODE
/* @violates @ref Mcl_Dma_h_REF_1 MISRA 2012 Required Directive 4.10, Precautions shall be taken in order to prevent the contents of a header file being included more than once. */
#include "Mcl_MemMap.h"

/*==================================================================================================
 *                                       INLINE FUNCTIONS
==================================================================================================*/
#define MCL_START_SEC_CODE
/* @violates @ref Mcl_Dma_h_REF_1 MISRA 2012 Required Directive 4.10, Precautions shall be taken in order to prevent the contents of a header file being included more than once. */
#include "Mcl_MemMap.h"

#if (STD_ON == CACHE_IP_ALL_IS_AVAILABLE)

#if (STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE)
Std_ReturnType hwAcc_Lmem_ProcessorCodeEnable(void)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locTimeoutTicks = OsIf_MicrosToTicks(CACHE_IP_LMEM_COMMAND_TIMEOUT_VAL, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locCurrentTicks = OsIf_GetCounter(CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locElapsedTicks = 0U;
    uint32 locReg = g_ptLmemBasePtrArray[0U]->PCCCR;

    locReg |= LMEM_PCCCR_ENCACHE_MASK;
    /* Save value into PCCR register */
    g_ptLmemBasePtrArray[0U]->PCCCR = locReg;

    /* Waiting ENACHE bit is set to 1 */
    while ((LMEM_PCCCR_ENCACHE_MASK != (LMEM_PCCCR_ENCACHE_MASK & (g_ptLmemBasePtrArray[0U]->PCCCR))) && (locElapsedTicks < locTimeoutTicks))
    {
        locElapsedTicks += OsIf_GetElapsed(&locCurrentTicks, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    }
    /* Return E_MOT_OK if exceed timeout*/
    if (locElapsedTicks >= locTimeoutTicks)
    {
        locStatus = (Std_ReturnType)E_NOT_OK;
    }

    return locStatus;
}

Std_ReturnType hwAcc_Lmem_ProcessorCodeDisable(void)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locTimeoutTicks = OsIf_MicrosToTicks(CACHE_IP_LMEM_COMMAND_TIMEOUT_VAL, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locCurrentTicks = OsIf_GetCounter(CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locElapsedTicks = 0U;
    uint32 locReg = g_ptLmemBasePtrArray[0U]->PCCCR;

    /* Save value into PCCR register */
    locReg &= ~(LMEM_PCCCR_ENCACHE_MASK);
    g_ptLmemBasePtrArray[0U]->PCCCR = locReg;

    /* Waiting ENACHE bit is set to 0 */
    while ((LMEM_PCCCR_ENCACHE_MASK == (LMEM_PCCCR_ENCACHE_MASK & (g_ptLmemBasePtrArray[0U]->PCCCR))) && (locElapsedTicks < locTimeoutTicks))
    {
        locElapsedTicks += OsIf_GetElapsed(&locCurrentTicks, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    }
    /* Return E_MOT_OK if exceed timeout*/
    if (locElapsedTicks >= locTimeoutTicks)
    {
        locStatus = (Std_ReturnType)E_NOT_OK;
    }

    return locStatus;
}

Std_ReturnType hwAcc_Lmem_ProcessorCodeInvalidate(void)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;

    /* Invalidate Cache */
    locStatus = hwAcc_Lmem_ProcessorCodeControlReg(g_ptLmemBasePtrArray[0U],
                                                  (LMEM_PCCCR_INVW0_MASK | LMEM_PCCCR_INVW1_MASK),
                                                   LMEM_PCCCR_GO_MASK);

    return locStatus;
}

Std_ReturnType hwAcc_Lmem_ProcessorCodeClean(const boolean enInvalidate)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;

    /* Clean Cache */
    if (FALSE == enInvalidate)
    {
        locStatus = hwAcc_Lmem_ProcessorCodeControlReg(g_ptLmemBasePtrArray[0U],
                                                      (LMEM_PCCCR_PUSHW0_MASK | LMEM_PCCCR_PUSHW1_MASK),
                                                      LMEM_PCCCR_GO_MASK);
    }
    else
    {
        locStatus = hwAcc_Lmem_ProcessorCodeControlReg(g_ptLmemBasePtrArray[0U],
                                                      (LMEM_PCCCR_PUSHW0_MASK | LMEM_PCCCR_INVW0_MASK | LMEM_PCCCR_PUSHW1_MASK | LMEM_PCCCR_INVW1_MASK),
                                                       LMEM_PCCCR_GO_MASK);
    }

    return locStatus;
}

Std_ReturnType hwAcc_Lmem_ProcessorCodeInvalidateByAddr(const uint32 addr, const uint32 length)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locStartAddr = addr & ~(CACHE_IP_LMEM_LINE_SIZE - 1U);
    uint32 locEndAddr = addr + length;

    /* Invalidate Cache by address */
    locStatus = hwAcc_Lmem_ProcessorCodeLineControlReg(g_ptLmemBasePtrArray[0U],
                                                      (LMEM_PCCLCR_LADSEL_MASK | LMEM_PCCLCR_LCMD(1U) | LMEM_PCCLCR_TDSEL_MASK),
                                                       0UL);

    while((locStartAddr < locEndAddr) && (locStatus == (Std_ReturnType)E_OK))
    {

            locStatus = hwAcc_Lmem_ProcessorCodeAddressReg(g_ptLmemBasePtrArray[0U],
                                                           locStartAddr,
                                                           LMEM_PCCSAR_LGO_MASK);
            locStartAddr += CACHE_IP_LMEM_LINE_SIZE;
    }

    return locStatus;
}

Std_ReturnType hwAcc_Lmem_ProcessorCodeCleanByAddr(const boolean enInvalidate, const uint32 addr, const uint32 length)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locStartAddr = addr & ~(CACHE_IP_LMEM_LINE_SIZE - 1U);
    uint32 locEndAddr = addr + length;

    /* Clean Cache by address */
    if(TRUE == enInvalidate)
    {
        locStatus = hwAcc_Lmem_ProcessorCodeLineControlReg(g_ptLmemBasePtrArray[0U],
                                                          (LMEM_PCCLCR_LADSEL_MASK | LMEM_PCCLCR_LCMD(3U) | LMEM_PCCLCR_TDSEL_MASK),
                                                           0UL);
    }
    else
    {
        locStatus = hwAcc_Lmem_ProcessorCodeLineControlReg(g_ptLmemBasePtrArray[0U],
                                                          (LMEM_PCCLCR_LADSEL_MASK | LMEM_PCCLCR_LCMD(2U) | LMEM_PCCLCR_TDSEL_MASK),
                                                           0UL);
    }

    while((locStartAddr < locEndAddr) && (locStatus == (Std_ReturnType)E_OK))
    {

            locStatus = hwAcc_Lmem_ProcessorCodeAddressReg(g_ptLmemBasePtrArray[0U],
                                                           locStartAddr,
                                                           LMEM_PCCSAR_LGO_MASK);
            locStartAddr += CACHE_IP_LMEM_LINE_SIZE;
    }

    return locStatus;
}
#endif /* CACHE_IP_LMEM_PC_IS_AVAILABLE */

#if (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE)
Std_ReturnType hwAcc_Lmem_ProcessorSystemEnable(void)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locTimeoutTicks = OsIf_MicrosToTicks(CACHE_IP_LMEM_COMMAND_TIMEOUT_VAL, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locCurrentTicks = OsIf_GetCounter(CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locElapsedTicks = 0U;
    uint32 locReg = g_ptLmemBasePtrArray[0U]->PSCCR;

    locReg |= LMEM_PSCCR_ENCACHE_MASK;
    /* Save value into PSCCR register */
    g_ptLmemBasePtrArray[0U]->PSCCR = locReg;

    /* Waiting ENACHE bit is set to 1 */
    while ((LMEM_PSCCR_ENCACHE_MASK != (LMEM_PSCCR_ENCACHE_MASK & (g_ptLmemBasePtrArray[0U]->PSCCR))) && (locElapsedTicks < locTimeoutTicks))
    {
        locElapsedTicks += OsIf_GetElapsed(&locCurrentTicks, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    }
    /* Return E_MOT_OK if exceed timeout*/
    if (locElapsedTicks >= locTimeoutTicks)
    {
        locStatus = (Std_ReturnType)E_NOT_OK;
    }

    return locStatus;
}

Std_ReturnType hwAcc_Lmem_ProcessorSystemDisable(void)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locTimeoutTicks = OsIf_MicrosToTicks(CACHE_IP_LMEM_COMMAND_TIMEOUT_VAL, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locCurrentTicks = OsIf_GetCounter(CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    uint32 locElapsedTicks = 0U;
    uint32 locReg = g_ptLmemBasePtrArray[0U]->PSCCR;

    /* Save value into PSCCR register */
    locReg &= ~(LMEM_PSCCR_ENCACHE_MASK);
    g_ptLmemBasePtrArray[0U]->PSCCR = locReg;

    /* Waiting ENACHE bit is set to 0 */
    while ((LMEM_PSCCR_ENCACHE_MASK == (LMEM_PSCCR_ENCACHE_MASK & (g_ptLmemBasePtrArray[0U]->PSCCR))) && (locElapsedTicks < locTimeoutTicks))
    {
        locElapsedTicks += OsIf_GetElapsed(&locCurrentTicks, CACHE_IP_LMEM_COMMAND_TIMEOUT_TYPE);
    }
    /* Return E_MOT_OK if exceed timeout*/
    if (locElapsedTicks >= locTimeoutTicks)
    {
        locStatus = (Std_ReturnType)E_NOT_OK;
    }

    return locStatus;
}

Std_ReturnType hwAcc_Lmem_ProcessorSystemInvalidate(void)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;

    /* Invalidate Cache */
    locStatus = hwAcc_Lmem_ProcessorSystemControlReg(g_ptLmemBasePtrArray[0U],
                                                    (LMEM_PSCCR_INVW0_MASK | LMEM_PSCCR_INVW1_MASK),
                                                     LMEM_PSCCR_GO_MASK);

    return locStatus;
}

Std_ReturnType hwAcc_Lmem_ProcessorSystemClean(const boolean enInvalidate)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;

    /* Clean Cache */
    if (FALSE == enInvalidate)
    {
        locStatus = hwAcc_Lmem_ProcessorSystemControlReg(g_ptLmemBasePtrArray[0U],
                                                        (LMEM_PSCCR_PUSHW0_MASK | LMEM_PSCCR_PUSHW1_MASK),
                                                         LMEM_PSCCR_GO_MASK);
    }
    else
    {
        locStatus = hwAcc_Lmem_ProcessorSystemControlReg(g_ptLmemBasePtrArray[0U],
                                                        (LMEM_PSCCR_PUSHW0_MASK | LMEM_PSCCR_INVW0_MASK | LMEM_PSCCR_PUSHW1_MASK | LMEM_PSCCR_INVW1_MASK),
                                                         LMEM_PSCCR_GO_MASK);
    }

    return locStatus;
}

Std_ReturnType hwAcc_Lmem_ProcessorSystemInvalidateByAddr(const uint32 addr, const uint32 length)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locStartAddr = addr & ~(CACHE_IP_LMEM_LINE_SIZE - 1U);
    uint32 locEndAddr = addr + length;

    /* Invalidate Cache by address */
    locStatus = hwAcc_Lmem_ProcessorSystemLineControlReg(g_ptLmemBasePtrArray[0U],
                                                        (LMEM_PSCLCR_LADSEL_MASK | LMEM_PSCLCR_LCMD(1U) | LMEM_PSCLCR_TDSEL_MASK),
                                                         0UL);

    while((locStartAddr < locEndAddr) && (locStatus == (Std_ReturnType)E_OK))
    {

            locStatus = hwAcc_Lmem_ProcessorSystemAddressReg(g_ptLmemBasePtrArray[0U],
                                                             locStartAddr,
                                                             LMEM_PSCSAR_LGO_MASK);
            locStartAddr += CACHE_IP_LMEM_LINE_SIZE;
    }

    return locStatus;
}

Std_ReturnType hwAcc_Lmem_ProcessorSystemCleanByAddr(const boolean enInvalidate, const uint32 addr, const uint32 length)
{
    Std_ReturnType locStatus = (Std_ReturnType)E_OK;
    uint32 locStartAddr = addr & ~(CACHE_IP_LMEM_LINE_SIZE - 1U);
    uint32 locEndAddr = addr + length;

    /* Clean Cache by address */
    if(TRUE == enInvalidate)
    {
        locStatus = hwAcc_Lmem_ProcessorSystemLineControlReg(g_ptLmemBasePtrArray[0U],
                                                            (LMEM_PSCLCR_LADSEL_MASK | LMEM_PSCLCR_LCMD(3U) | LMEM_PSCLCR_TDSEL_MASK),
                                                             0UL);
    }
    else
    {
        locStatus = hwAcc_Lmem_ProcessorSystemLineControlReg(g_ptLmemBasePtrArray[0U],
                                                            (LMEM_PSCLCR_LADSEL_MASK | LMEM_PSCLCR_LCMD(2U) | LMEM_PSCLCR_TDSEL_MASK),
                                                             0UL);
    }

    while((locStartAddr < locEndAddr) && (locStatus == (Std_ReturnType)E_OK))
    {

            locStatus = hwAcc_Lmem_ProcessorSystemAddressReg(g_ptLmemBasePtrArray[0U],
                                                             locStartAddr,
                                                             LMEM_PSCSAR_LGO_MASK);
            locStartAddr += CACHE_IP_LMEM_LINE_SIZE;
    }

    return locStatus;
}
#endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */

Std_ReturnType hwAcc_Lmem_Enable(const uint8 BusType)
{
    Std_ReturnType locStatus;
    Std_ReturnType retStatus = (Std_ReturnType)E_OK;

    switch(BusType)
    {
        case 0U:{
        #if (STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorCodeEnable();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PC_IS_AVAILABLE */
        break;}
        case 1U:{
        #if (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorSystemEnable();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        case 2U:{
        #if ((STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE) && (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE))
            locStatus = hwAcc_Lmem_ProcessorCodeEnable();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
            locStatus = hwAcc_Lmem_ProcessorSystemEnable();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        default:{
            CACHE_IP_DEV_ASSERT(FALSE);
        break;}
    }

    return retStatus;
}

Std_ReturnType hwAcc_Lmem_Disable(const uint8 BusType)
{
    Std_ReturnType locStatus;
    Std_ReturnType retStatus = (Std_ReturnType)E_OK;

    switch(BusType)
    {
        case 0U:{
        #if (STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorCodeDisable();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PC_IS_AVAILABLE */
        break;}
        case 1U:{
        #if (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorSystemDisable();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        case 2U:{
        #if ((STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE) && (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE))
            locStatus = hwAcc_Lmem_ProcessorCodeDisable();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
            locStatus = hwAcc_Lmem_ProcessorSystemDisable();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        default:{
            CACHE_IP_DEV_ASSERT(FALSE);
        break;}
    }

    return retStatus;
}

Std_ReturnType hwAcc_Lmem_Invalidate(const uint8 BusType)
{
    Std_ReturnType locStatus;
    Std_ReturnType retStatus = (Std_ReturnType)E_OK;

    switch(BusType)
    {
        case 0U:{
        #if (STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorCodeInvalidate();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PC_IS_AVAILABLE */
        break;}
        case 1U:{
        #if (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorSystemInvalidate();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        case 2U:{
        #if ((STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE) && (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE))
            locStatus = hwAcc_Lmem_ProcessorCodeInvalidate();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
            locStatus = hwAcc_Lmem_ProcessorSystemInvalidate();
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        default:{
            CACHE_IP_DEV_ASSERT(FALSE);
        break;}
    }

    return retStatus;
}

Std_ReturnType hwAcc_Lmem_Clean(const uint8 BusType, const boolean enInvalidate)
{
    Std_ReturnType locStatus;
    Std_ReturnType retStatus = (Std_ReturnType)E_OK;

    switch(BusType)
    {
        case 0U:{
        #if (STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorCodeClean(enInvalidate);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PC_IS_AVAILABLE */
        break;}
        case 1U:{
        #if (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorSystemClean(enInvalidate);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        case 2U:{
        #if ((STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE) && (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE))
            locStatus = hwAcc_Lmem_ProcessorCodeClean(enInvalidate);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
            locStatus = hwAcc_Lmem_ProcessorSystemClean(enInvalidate);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        default:{
            CACHE_IP_DEV_ASSERT(FALSE);
        break;}
    }

    return retStatus;
}

Std_ReturnType hwAcc_Lmem_InvalidateByAddr(const uint8 BusType, const uint32 addr, const uint32 length)
{
    Std_ReturnType locStatus;
    Std_ReturnType retStatus = (Std_ReturnType)E_OK;

    switch(BusType)
    {
        case 0U:{
        #if (STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorCodeInvalidateByAddr(addr, length);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PC_IS_AVAILABLE */
        break;}
        case 1U:{
        #if (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorSystemInvalidateByAddr(addr, length);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        case 2U:{
        #if ((STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE) && (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE))
            locStatus = hwAcc_Lmem_ProcessorCodeInvalidateByAddr(addr, length);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
            locStatus = hwAcc_Lmem_ProcessorSystemInvalidateByAddr(addr, length);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        default:{
            CACHE_IP_DEV_ASSERT(FALSE);
        break;}
    }

    return retStatus;
}

Std_ReturnType hwAcc_Lmem_CleanByAddr(const uint8 BusType, const boolean enInvalidate, const uint32 addr, const uint32 length)
{
    Std_ReturnType locStatus;
    Std_ReturnType retStatus = (Std_ReturnType)E_OK;

    switch(BusType)
    {
        case 0U:{
        #if (STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorCodeCleanByAddr(enInvalidate, addr, length);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PC_IS_AVAILABLE */
        break;}
        case 1U:{
        #if (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE)
            locStatus = hwAcc_Lmem_ProcessorSystemCleanByAddr(enInvalidate, addr, length);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        case 2U:{
        #if ((STD_ON == CACHE_IP_LMEM_PC_IS_AVAILABLE) && (STD_ON == CACHE_IP_LMEM_PS_IS_AVAILABLE))
            locStatus = hwAcc_Lmem_ProcessorCodeCleanByAddr(enInvalidate, addr, length);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
            locStatus = hwAcc_Lmem_ProcessorSystemCleanByAddr(enInvalidate, addr, length);
            if((Std_ReturnType)E_NOT_OK == locStatus)
            {
                retStatus = E_NOT_OK;
            }
        #else
            CACHE_IP_DEV_ASSERT(FALSE);
        #endif /* CACHE_IP_LMEM_PS_IS_AVAILABLE */
        break;}
        default:{
            CACHE_IP_DEV_ASSERT(FALSE);
        break;}
    }

    return retStatus;
}


#endif /* CACHE_IP_ALL_IS_AVAILABLE */

#define MCL_STOP_SEC_CODE
/* @violates @ref Mcl_Dma_h_REF_1 MISRA 2012 Required Directive 4.10, Precautions shall be taken in order to prevent the contents of a header file being included more than once. */
#include "Mcl_MemMap.h"

#endif /* #if (STD_ON == CACHE_IP_LMEM_IS_AVAILABLE) */

#endif /* #if (STD_ON == CACHE_IP_IS_AVAILABLE) */

#ifdef __cplusplus
}
#endif

/** @} */

