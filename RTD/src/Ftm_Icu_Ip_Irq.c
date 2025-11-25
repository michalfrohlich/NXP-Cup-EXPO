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
 *     @file       File with source code used to implement ICU driver interrupts on FTM module.
 *     @details    This file contains the source code for all interrupt functions which are using 
 *                 FTM module.
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
#include "Ftm_Icu_Ip_Irq.h"

#include "SchM_Icu.h"
#include "Mcal.h"

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define FTM_ICU_IP_IRQ_VENDOR_ID_C                      43
#define FTM_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C       4
#define FTM_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C       7
#define FTM_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION_C    0
#define FTM_ICU_IP_IRQ_SW_MAJOR_VERSION_C               2
#define FTM_ICU_IP_IRQ_SW_MINOR_VERSION_C               0
#define FTM_ICU_IP_IRQ_SW_PATCH_VERSION_C               0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/

/* Check if source file and ICU header file are of the same vendor */
#if (FTM_ICU_IP_IRQ_VENDOR_ID_C != FTM_ICU_IP_IRQ_VENDOR_ID)
    #error "Ftm_Icu_Ip_Irq.c and Ftm_Icu_Ip_Irq.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((FTM_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C != FTM_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION) || \
     (FTM_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C != FTM_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION) || \
     (FTM_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION_C != FTM_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Ftm_Icu_Ip_Irq.c and Ftm_Icu_Ip_Irq.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((FTM_ICU_IP_IRQ_SW_MAJOR_VERSION_C != FTM_ICU_IP_IRQ_SW_MAJOR_VERSION) || \
     (FTM_ICU_IP_IRQ_SW_MINOR_VERSION_C != FTM_ICU_IP_IRQ_SW_MINOR_VERSION) || \
     (FTM_ICU_IP_IRQ_SW_PATCH_VERSION_C != FTM_ICU_IP_IRQ_SW_PATCH_VERSION))
#error "Software Version Numbers of Ftm_Icu_Ip_Irq.c and Ftm_Icu_Ip_Irq.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (FTM_ICU_IP_IRQ_VENDOR_ID_C != FTM_ICU_IP_VENDOR_ID)
    #error "Ftm_Icu_Ip_Irq.c and Ftm_Icu_Ip.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((FTM_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C != FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION) || \
     (FTM_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C != FTM_ICU_IP_AR_RELEASE_MINOR_VERSION) || \
     (FTM_ICU_IP_IRQ_AR_RELEASE_REVISION_VERSION_C != FTM_ICU_IP_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Ftm_Icu_Ip_Irq.c and Ftm_Icu_Ip.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((FTM_ICU_IP_IRQ_SW_MAJOR_VERSION_C != FTM_ICU_IP_SW_MAJOR_VERSION) || \
     (FTM_ICU_IP_IRQ_SW_MINOR_VERSION_C != FTM_ICU_IP_SW_MINOR_VERSION) || \
     (FTM_ICU_IP_IRQ_SW_PATCH_VERSION_C != FTM_ICU_IP_SW_PATCH_VERSION))
#error "Software Version Numbers of Ftm_Icu_Ip_Irq.c and Ftm_Icu_Ip.h are different"
#endif

#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if this header file and SchM_Icu.h file are of the same Autosar version */
    #if ((FTM_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C != SCHM_ICU_AR_RELEASE_MAJOR_VERSION) || \
        (FTM_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C != SCHM_ICU_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Ftm_Icu_Ip_Irq.c and SchM_Icu.h are different"
    #endif
    
    /* Check if this header file and Mcal.h file are of the same Autosar version */
    #if ((FTM_ICU_IP_IRQ_AR_RELEASE_MAJOR_VERSION_C != MCAL_AR_RELEASE_MAJOR_VERSION) || \
        (FTM_ICU_IP_IRQ_AR_RELEASE_MINOR_VERSION_C != MCAL_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Ftm_Icu_Ip_Irq.c and Mcal.h are different"
    #endif
#endif

/*==================================================================================================
*                                        GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL VARIABLES
==================================================================================================*/
#define ICU_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"

/* State of channels from FTM modules. */
extern Ftm_Icu_Ip_ChStateType Ftm_Icu_Ip_ChState[FTM_INSTANCE_COUNT][FTM_ICU_IP_NUM_OF_CHANNELS];

/* Instance state for FTMs. */
extern Ftm_Icu_Ip_InstStateType Ftm_Icu_Ip_InsState[FTM_INSTANCE_COUNT];

#if (((defined FTM_ICU_0_ISR_USED) || (defined FTM_ICU_1_ISR_USED) || (defined FTM_ICU_2_ISR_USED) || (defined FTM_ICU_3_ISR_USED) || (defined FTM_ICU_4_ISR_USED) || (defined FTM_ICU_5_ISR_USED) || \
    (defined FTM_ICU_0_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_0_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_0_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_0_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_1_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_1_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_1_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_1_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_2_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_2_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_2_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_2_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_3_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_3_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_3_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_3_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_4_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_4_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_4_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_4_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_5_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_5_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_5_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_5_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_6_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_6_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_6_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_6_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_7_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_7_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_7_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_7_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_0_CH_0_ISR_USED) || (defined FTM_ICU_0_CH_1_ISR_USED) || (defined FTM_ICU_0_CH_2_ISR_USED) || \
    (defined FTM_ICU_0_CH_3_ISR_USED) || (defined FTM_ICU_0_CH_4_ISR_USED) || (defined FTM_ICU_0_CH_5_ISR_USED) || \
    (defined FTM_ICU_1_CH_0_ISR_USED) || (defined FTM_ICU_1_CH_1_ISR_USED) || (defined FTM_ICU_1_CH_2_ISR_USED) || \
    (defined FTM_ICU_1_CH_3_ISR_USED) || (defined FTM_ICU_1_CH_4_ISR_USED) || (defined FTM_ICU_1_CH_5_ISR_USED) || \
    (defined FTM_ICU_2_CH_0_ISR_USED) || (defined FTM_ICU_2_CH_1_ISR_USED) || (defined FTM_ICU_2_CH_2_ISR_USED) || \
    (defined FTM_ICU_2_CH_3_ISR_USED) || (defined FTM_ICU_2_CH_4_ISR_USED) || (defined FTM_ICU_2_CH_5_ISR_USED) || \
    (defined FTM_ICU_3_CH_0_ISR_USED) || (defined FTM_ICU_3_CH_1_ISR_USED) || (defined FTM_ICU_3_CH_2_ISR_USED) || \
    (defined FTM_ICU_3_CH_3_ISR_USED) || (defined FTM_ICU_3_CH_4_ISR_USED) || (defined FTM_ICU_3_CH_5_ISR_USED)) && \
    (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API))
static uint16 Icu_Ftm_ActivationPulseTemp[FTM_INSTANCE_COUNT][FTM_ICU_IP_NUM_OF_CHANNELS];
#endif

#define ICU_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Icu_MemMap.h"

#if (STD_ON == FTM_ICU_TIMESTAMP_API)

#define ICU_START_SEC_VAR_CLEARED_16
#include "Icu_MemMap.h"

/* Channels buffer. */
extern uint16 Ftm_Icu_Ip_aBufferPtr[FTM_INSTANCE_COUNT][FTM_ICU_IP_NUM_OF_CHANNELS];

#define ICU_STOP_SEC_VAR_CLEARED_16
#include "Icu_MemMap.h"

#endif

/*==================================================================================================
*                                    LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"
#if ((defined FTM_ICU_0_ISR_USED) || (defined FTM_ICU_1_ISR_USED) || (defined FTM_ICU_2_ISR_USED) || (defined FTM_ICU_3_ISR_USED) || (defined FTM_ICU_4_ISR_USED) || (defined FTM_ICU_5_ISR_USED) || \
    (defined FTM_ICU_0_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_0_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_0_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_0_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_1_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_1_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_1_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_1_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_2_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_2_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_2_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_2_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_3_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_3_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_3_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_3_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_4_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_4_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_4_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_4_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_5_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_5_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_5_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_5_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_6_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_6_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_6_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_6_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_7_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_7_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_7_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_7_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_0_CH_0_ISR_USED) || (defined FTM_ICU_0_CH_1_ISR_USED) || (defined FTM_ICU_0_CH_2_ISR_USED) || \
    (defined FTM_ICU_0_CH_3_ISR_USED) || (defined FTM_ICU_0_CH_4_ISR_USED) || (defined FTM_ICU_0_CH_5_ISR_USED) || \
    (defined FTM_ICU_1_CH_0_ISR_USED) || (defined FTM_ICU_1_CH_1_ISR_USED) || (defined FTM_ICU_1_CH_2_ISR_USED) || \
    (defined FTM_ICU_1_CH_3_ISR_USED) || (defined FTM_ICU_1_CH_4_ISR_USED) || (defined FTM_ICU_1_CH_5_ISR_USED) || \
    (defined FTM_ICU_2_CH_0_ISR_USED) || (defined FTM_ICU_2_CH_1_ISR_USED) || (defined FTM_ICU_2_CH_2_ISR_USED) || \
    (defined FTM_ICU_2_CH_3_ISR_USED) || (defined FTM_ICU_2_CH_4_ISR_USED) || (defined FTM_ICU_2_CH_5_ISR_USED) || \
    (defined FTM_ICU_3_CH_0_ISR_USED) || (defined FTM_ICU_3_CH_1_ISR_USED) || (defined FTM_ICU_3_CH_2_ISR_USED) || \
    (defined FTM_ICU_3_CH_3_ISR_USED) || (defined FTM_ICU_3_CH_4_ISR_USED) || (defined FTM_ICU_3_CH_5_ISR_USED))

/**
 * @brief FTM IP function that get the state of the overflow flag
 * 
 * @param[in] instance     Hardware instance of FTM used. 
 * @return    boolean    State of the overflow flag.
 * @retval    TRUE       The overflow flag is set.
 * @retval    FALSE      The overflow flag is not set.
 */
static inline boolean Ftm_Icu_Ip_GetOverflowIrq(uint8 instance);

static inline void Ftm_Icu_Ip_ProcessInterrupt(uint8 instance, uint8 hwchannel);

#if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)
static inline void Ftm_Icu_Ip_SignalMeasurement(uint8 instance, uint8 hwchannel, boolean overflow);

/**
 * @brief FTM IP function that handle the Signal Measurement
 */
static inline void Ftm_Icu_Ip_SignalMeasurementHandler(uint8  instance,
                                         uint8  hwChannel,
                                         uint16 activePulseWidth,
                                         uint16 period,
                                         boolean bOverflow);
#endif /* STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API */

#if ((STD_ON == FTM_ICU_EDGE_DETECT_API) || (STD_ON == FTM_ICU_EDGE_COUNT_API))
/**
 * @brief 
 * 
 * @param instance 
 * @param hwChannel 
 * @param overflow 
 */
static inline void Ftm_Icu_Ip_ReportEvents(uint8   instance,
                                           uint8   hwChannel,
                                           boolean overflow);
#endif

#if (STD_ON == FTM_ICU_TIMESTAMP_API)
/**
 * @brief      FTM ICU driver function that handles the timestamp measurement in interrupt.
 * @details    This service is called when an interrupt is recognized as a Timestamp
 *             Measurement type.
 * 
 * @param instance  - Hardware channel of FTM used. 
 * @param hwChannel - Hardware instance of FTM used. 
 * @param bufferPtr - Address of the user buffer to store data.
 * @param overflow  - Parameter that indicates the source of report is an overflow
 * @internal
 */
static inline void Ftm_Icu_Ip_TimestampHandler(uint8         instance,
                                               uint8         hwChannel,
                                               const uint16* bufferPtr,
                                               boolean       overflow);
#endif /* FTM_ICU_TIMESTAMP_API */

#if ((STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API) || (STD_ON == FTM_ICU_TIMESTAMP_API))
/**
 * @brief 
 * 
 * @param instance 
 * @param hwChannel 
 * @param overflow 
 */
static inline void Ftm_Icu_Ip_ReportOverflow(uint8   instance,
                                             uint8   hwChannel,
                                             boolean overflow);
#endif
#endif /* All ISRs defines. */

#if ((((defined FTM_ICU_0_ISR_USED) || (defined FTM_ICU_1_ISR_USED) || (defined FTM_ICU_2_ISR_USED) || (defined FTM_ICU_3_ISR_USED) || (defined FTM_ICU_4_ISR_USED) || (defined FTM_ICU_5_ISR_USED)) &&  !defined( FTM_ICU_TOF_DISTINCT_LINE)) || \
     (defined FTM_ICU_0_OVF_ISR_USED) || (defined FTM_ICU_1_OVF_ISR_USED) || \
     (defined FTM_ICU_2_OVF_ISR_USED) || (defined FTM_ICU_3_OVF_ISR_USED) || \
     (defined FTM_ICU_4_OVF_ISR_USED) || (defined FTM_ICU_5_OVF_ISR_USED) || \
     (defined FTM_ICU_6_OVF_ISR_USED) || (defined FTM_ICU_7_OVF_ISR_USED))
    static inline void Ftm_Icu_Ip_ProcessTofInterrupt(uint8 instance);
#endif
/*==================================================================================================
*                                        LOCAL FUNCTIONS
==================================================================================================*/
#if ((defined FTM_ICU_0_ISR_USED) || (defined FTM_ICU_1_ISR_USED) || (defined FTM_ICU_2_ISR_USED) || (defined FTM_ICU_3_ISR_USED) || (defined FTM_ICU_4_ISR_USED) || (defined FTM_ICU_5_ISR_USED) || \
     (defined FTM_ICU_0_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_0_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_0_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_0_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_1_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_1_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_1_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_1_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_2_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_2_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_2_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_2_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_3_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_3_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_3_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_3_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_4_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_4_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_4_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_4_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_5_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_5_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_5_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_5_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_6_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_6_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_6_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_6_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_7_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_7_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_7_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_7_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_0_CH_0_ISR_USED) || (defined FTM_ICU_0_CH_1_ISR_USED) || (defined FTM_ICU_0_CH_2_ISR_USED) || \
     (defined FTM_ICU_0_CH_3_ISR_USED) || (defined FTM_ICU_0_CH_4_ISR_USED) || (defined FTM_ICU_0_CH_5_ISR_USED) || \
     (defined FTM_ICU_1_CH_0_ISR_USED) || (defined FTM_ICU_1_CH_1_ISR_USED) || (defined FTM_ICU_1_CH_2_ISR_USED) || \
     (defined FTM_ICU_1_CH_3_ISR_USED) || (defined FTM_ICU_1_CH_4_ISR_USED) || (defined FTM_ICU_1_CH_5_ISR_USED) || \
     (defined FTM_ICU_2_CH_0_ISR_USED) || (defined FTM_ICU_2_CH_1_ISR_USED) || (defined FTM_ICU_2_CH_2_ISR_USED) || \
     (defined FTM_ICU_2_CH_3_ISR_USED) || (defined FTM_ICU_2_CH_4_ISR_USED) || (defined FTM_ICU_2_CH_5_ISR_USED) || \
     (defined FTM_ICU_3_CH_0_ISR_USED) || (defined FTM_ICU_3_CH_1_ISR_USED) || (defined FTM_ICU_3_CH_2_ISR_USED) || \
     (defined FTM_ICU_3_CH_3_ISR_USED) || (defined FTM_ICU_3_CH_4_ISR_USED) || (defined FTM_ICU_3_CH_5_ISR_USED))

#if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)
/**
 * @brief FTM IP function that handle the Signal Measurement
 */
static inline void Ftm_Icu_Ip_SignalMeasurementHandler(uint8  instance,
                                                       uint8  hwChannel,
                                                       uint16 activePulseWidth,
                                                       uint16 period,
                                                       boolean bOverflow)
{
    Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aActivePulseWidth = activePulseWidth;
    Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aPeriod = period;
    Ftm_Icu_Ip_ReportOverflow(instance, hwChannel, bOverflow);
    if (NULL_PTR != Ftm_Icu_Ip_ChState[instance][hwChannel].logicChStateCallback)
    {
        Ftm_Icu_Ip_ChState[instance][hwChannel].logicChStateCallback(Ftm_Icu_Ip_ChState[instance][hwChannel].callbackParam, (1U << 1), TRUE);
    }
}
#endif /* STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API */
#endif

#if ((((defined FTM_ICU_0_ISR_USED) || (defined FTM_ICU_1_ISR_USED) || (defined FTM_ICU_2_ISR_USED) || (defined FTM_ICU_3_ISR_USED) || (defined FTM_ICU_4_ISR_USED) || (defined FTM_ICU_5_ISR_USED)) &&  !defined( FTM_ICU_TOF_DISTINCT_LINE)) || \
     (defined FTM_ICU_0_OVF_ISR_USED) || (defined FTM_ICU_1_OVF_ISR_USED) || \
     (defined FTM_ICU_2_OVF_ISR_USED) || (defined FTM_ICU_3_OVF_ISR_USED) || \
     (defined FTM_ICU_4_OVF_ISR_USED) || (defined FTM_ICU_5_OVF_ISR_USED) || \
     (defined FTM_ICU_6_OVF_ISR_USED) || (defined FTM_ICU_7_OVF_ISR_USED))
static inline void Ftm_Icu_Ip_ProcessTofInterrupt(uint8 instance)
{
    uint8  hwchannel;
    Ftm_Icu_Ip_ModeType measMode;

    for (hwchannel = 0U; hwchannel < (uint8)FTM_ICU_IP_NUM_OF_CHANNELS; hwchannel++)
    {
        /* Get mode of measurement for each hwchannel. */
        measMode = Ftm_Icu_Ip_ChState[instance][hwchannel].channelMode;

        if ((FTM_ICU_MODE_TIMESTAMP == measMode) || \
            (FTM_ICU_MODE_SIGNAL_MEASUREMENT == measMode))
        {
            /* Call ReportEvents for ASR or User_Callback for Edge Detection */
            if(Ftm_Icu_Ip_ChState[instance][hwchannel].callback != NULL_PTR)
            {
                Ftm_Icu_Ip_ChState[instance][hwchannel].callback(Ftm_Icu_Ip_ChState[instance][hwchannel].callbackParam, TRUE);
            }
            else
            {
                /* Calling User Overflow Notification for the IPL channel */
                if (NULL_PTR != Ftm_Icu_Ip_ChState[instance][hwchannel].ftmOverflowNotification)
                {
                    Ftm_Icu_Ip_ChState[instance][hwchannel].ftmOverflowNotification();
                }
            }
        }
    }
}
#endif

#if ((defined FTM_ICU_0_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_0_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_0_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_0_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_1_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_1_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_1_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_1_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_2_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_2_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_2_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_2_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_3_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_3_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_3_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_3_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_4_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_4_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_4_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_4_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_5_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_5_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_5_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_5_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_6_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_6_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_6_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_6_CH_6_CH_7_ISR_USED) || \
     (defined FTM_ICU_7_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_7_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_7_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_7_CH_6_CH_7_ISR_USED))
/* @implements Ftm_Icu_Ip_PairChHandler_Activity */
static inline void Ftm_Icu_Ip_PairChHandler(uint8 moduleIndex, uint8 channelIndex)
{
    /* Read data from status and control register. */
    uint8  isrStatus    = (uint8)ftmIcuBase[moduleIndex]->STATUS;
    uint8  nextChannelIndex = channelIndex + (uint8)1U;
    
    if (TRUE == Ftm_Icu_Ip_InsState[moduleIndex].instInit)
    {
        if (((isrStatus & (uint8)(1U << channelIndex)) != (uint8)0U) && \
             ((ftmIcuBase[moduleIndex]->CONTROLS[channelIndex].CSC & FTM_CSC_CHIE_MASK) != (uint8)0U))
        {
            /* Clear interrupt flag from STATUS flags. */
            ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
            Ftm_Icu_Ip_ProcessInterrupt(moduleIndex, channelIndex);
        }
        
        if (nextChannelIndex < FTM_CONTROLS_COUNT)
        {
            if (((isrStatus & (uint8)(1U << nextChannelIndex)) != (uint8)0U) && \
                 ((ftmIcuBase[moduleIndex]->CONTROLS[nextChannelIndex].CSC & FTM_CSC_CHIE_MASK) != (uint8)0U))
            {
                /* Clear interrupt flag from STATUS flags. */
                ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << nextChannelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
                Ftm_Icu_Ip_ProcessInterrupt(moduleIndex, nextChannelIndex);
            }
        }
    }
    else
    {
        /* Clear interrupt flag from STATUS flags. */
        ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
        if (nextChannelIndex < FTM_CONTROLS_COUNT)
        {
            /* Clear interrupt flag from STATUS flags. */
            ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << nextChannelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
        }
    }
}
#endif /* All pair channels defines. */

#if ((defined FTM_ICU_0_CH_0_ISR_USED) || (defined FTM_ICU_0_CH_1_ISR_USED) || (defined FTM_ICU_0_CH_2_ISR_USED) || \
     (defined FTM_ICU_0_CH_3_ISR_USED) || (defined FTM_ICU_0_CH_4_ISR_USED) || (defined FTM_ICU_0_CH_5_ISR_USED) || \
     (defined FTM_ICU_1_CH_0_ISR_USED) || (defined FTM_ICU_1_CH_1_ISR_USED) || (defined FTM_ICU_1_CH_2_ISR_USED) || \
     (defined FTM_ICU_1_CH_3_ISR_USED) || (defined FTM_ICU_1_CH_4_ISR_USED) || (defined FTM_ICU_1_CH_5_ISR_USED) || \
     (defined FTM_ICU_2_CH_0_ISR_USED) || (defined FTM_ICU_2_CH_1_ISR_USED) || (defined FTM_ICU_2_CH_2_ISR_USED) || \
     (defined FTM_ICU_2_CH_3_ISR_USED) || (defined FTM_ICU_2_CH_4_ISR_USED) || (defined FTM_ICU_2_CH_5_ISR_USED) || \
     (defined FTM_ICU_3_CH_0_ISR_USED) || (defined FTM_ICU_3_CH_1_ISR_USED) || (defined FTM_ICU_3_CH_2_ISR_USED) || \
     (defined FTM_ICU_3_CH_3_ISR_USED) || (defined FTM_ICU_3_CH_4_ISR_USED) || (defined FTM_ICU_3_CH_5_ISR_USED))
static inline void Ftm_Icu_Ip_ChHandler(uint8 moduleIndex, uint8 channelIndex)
{
    /* Read data from status and control register. */
    uint8  isrStatus    = (uint8)ftmIcuBase[moduleIndex]->STATUS;

    if (TRUE == Ftm_Icu_Ip_InsState[moduleIndex].instInit)
    {
        if (((isrStatus & (uint8)(1U << channelIndex)) != (uint8)0U) && \
             ((ftmIcuBase[moduleIndex]->CONTROLS[channelIndex].CSC & FTM_CSC_CHIE_MASK) != (uint8)0U))
        {
            /* Clear interrupt flag from STATUS flags. */
            ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
            Ftm_Icu_Ip_ProcessInterrupt(moduleIndex, channelIndex);
        }
    }
    else
    {
        /* Clear interrupt flag from STATUS flags. */
        ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
    }
}
#endif /* All channels defines. */

#if ((defined FTM_ICU_0_OVF_ISR_USED) || (defined FTM_ICU_1_OVF_ISR_USED) || (defined FTM_ICU_2_OVF_ISR_USED) || (defined FTM_ICU_3_OVF_ISR_USED) || \
     (defined FTM_ICU_4_OVF_ISR_USED) || (defined FTM_ICU_5_OVF_ISR_USED) || (defined FTM_ICU_6_OVF_ISR_USED) || (defined FTM_ICU_7_OVF_ISR_USED))
/* @implements Ftm_Icu_Ip_TofHandler_Activity */
static inline void Ftm_Icu_Ip_TofHandler(uint8 moduleIndex)
{
    /* Read TOF value and verify if it was an overflow. */
    if ((FTM_SC_TOF_MASK  == (ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOF_MASK)) && \
        (FTM_SC_TOIE_MASK == (ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOIE_MASK)))
    {
        /* Clear timer overflow flag */
        ftmIcuBase[moduleIndex]->SC &= ~(FTM_SC_TOF_MASK);
        Ftm_Icu_Ip_ProcessTofInterrupt(moduleIndex);
    }
}
#endif /* All timeroveflow defines. */

#if ((defined FTM_ICU_0_ISR_USED) || (defined FTM_ICU_1_ISR_USED) || (defined FTM_ICU_2_ISR_USED) || (defined FTM_ICU_3_ISR_USED) || (defined FTM_ICU_4_ISR_USED) || (defined FTM_ICU_5_ISR_USED) || \
    (defined FTM_ICU_0_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_0_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_0_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_0_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_1_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_1_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_1_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_1_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_2_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_2_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_2_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_2_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_3_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_3_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_3_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_3_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_4_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_4_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_4_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_4_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_5_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_5_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_5_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_5_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_6_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_6_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_6_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_6_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_7_CH_0_CH_1_ISR_USED) || (defined FTM_ICU_7_CH_2_CH_3_ISR_USED) || (defined FTM_ICU_7_CH_4_CH_5_ISR_USED) || (defined FTM_ICU_7_CH_6_CH_7_ISR_USED) || \
    (defined FTM_ICU_0_CH_0_ISR_USED) || (defined FTM_ICU_0_CH_1_ISR_USED) || (defined FTM_ICU_0_CH_2_ISR_USED) || \
    (defined FTM_ICU_0_CH_3_ISR_USED) || (defined FTM_ICU_0_CH_4_ISR_USED) || (defined FTM_ICU_0_CH_5_ISR_USED) || \
    (defined FTM_ICU_1_CH_0_ISR_USED) || (defined FTM_ICU_1_CH_1_ISR_USED) || (defined FTM_ICU_1_CH_2_ISR_USED) || \
    (defined FTM_ICU_1_CH_3_ISR_USED) || (defined FTM_ICU_1_CH_4_ISR_USED) || (defined FTM_ICU_1_CH_5_ISR_USED) || \
    (defined FTM_ICU_2_CH_0_ISR_USED) || (defined FTM_ICU_2_CH_1_ISR_USED) || (defined FTM_ICU_2_CH_2_ISR_USED) || \
    (defined FTM_ICU_2_CH_3_ISR_USED) || (defined FTM_ICU_2_CH_4_ISR_USED) || (defined FTM_ICU_2_CH_5_ISR_USED) || \
    (defined FTM_ICU_3_CH_0_ISR_USED) || (defined FTM_ICU_3_CH_1_ISR_USED) || (defined FTM_ICU_3_CH_2_ISR_USED) || \
    (defined FTM_ICU_3_CH_3_ISR_USED) || (defined FTM_ICU_3_CH_4_ISR_USED) || (defined FTM_ICU_3_CH_5_ISR_USED))

#if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)

static inline void Ftm_Icu_Ip_PeriodTime(uint8 instance, uint8 hwchannel, boolean overflow)
{
    uint16 firstChannelCV    = 0U;
    uint16 period            = 0U;
    /* Save hwchannel Value (CV) for first hardware channels */
    firstChannelCV  =  (uint16)ftmIcuBase[instance]->CONTROLS[(hwchannel)].CV;

    if(TRUE == Ftm_Icu_Ip_ChState[instance][hwchannel].firstEdge)
    {
        /* If the first edge is detected, save the value of the counter. */
        Icu_Ftm_ActivationPulseTemp[instance][hwchannel]  = (uint16)firstChannelCV;
        /* Prepare the second measurement. */
        Ftm_Icu_Ip_ChState[instance][hwchannel].firstEdge = FALSE;
    }
    else
    {
        /* Calculate period and make the callback. */
        if(firstChannelCV > Icu_Ftm_ActivationPulseTemp[instance][hwchannel])
        {
            period = firstChannelCV - Icu_Ftm_ActivationPulseTemp[instance][hwchannel];
        }
        else
        {
            period = (uint16)((uint16)FTM_MAX_VAL_COUNTER  - (uint16)Icu_Ftm_ActivationPulseTemp[instance][hwchannel]) + firstChannelCV + (uint16)1U;
        }
        Icu_Ftm_ActivationPulseTemp[instance][hwchannel]  = (uint16)firstChannelCV;

        /* Callback to process data from interrupt. */
        Ftm_Icu_Ip_SignalMeasurementHandler(instance, hwchannel, 0U, period, overflow);
    }
}

static inline void Ftm_Icu_Ip_DutyCycle(uint8 instance, uint8 hwchannel, boolean overflow)
{
    uint16 firstChannelCV    = 0U;
    uint16 secondChannelCV   = 0U;
    uint16 firstChannelEdge  = 0U;
    uint16 secondChannelEdge = 0U;
    uint16 period            = 0U;
    uint16 pulseWidth        = 0U;
    /* If it's the first activation edge remove from queue. */
    if(TRUE == Ftm_Icu_Ip_ChState[instance][hwchannel].firstEdge)
    {
        /* Save hwchannel Status and Control (CSC) for the n and n+1 hardware channels */
        firstChannelEdge  =  (uint16)ftmIcuBase[instance]->CONTROLS[hwchannel].CSC;
        secondChannelEdge =  (uint16)ftmIcuBase[instance]->CONTROLS[(hwchannel + 1U)].CSC;
        /* Save Activation Pulse Temp */
        Icu_Ftm_ActivationPulseTemp[instance][hwchannel] = (uint16)ftmIcuBase[instance]->CONTROLS[hwchannel].CV;

        if (((firstChannelEdge & FTM_CSC_ELSB_MASK) > 0U) && ((firstChannelEdge & FTM_CSC_ELSA_MASK) > 0U))
        {
            if ((ftmIcuBase[instance]->CONTROLS[hwchannel].CSC & FTM_CSC_CHIS_MASK) > 0U)
            {
                firstChannelEdge  &= ~((uint16)FTM_CSC_ELSB_MASK);
                secondChannelEdge &= ~((uint16)FTM_CSC_ELSA_MASK);
            }
            else
            {
                firstChannelEdge  &= ~((uint16)FTM_CSC_ELSA_MASK);
                secondChannelEdge &= ~((uint16)FTM_CSC_ELSB_MASK);
                Ftm_Icu_Ip_ChState[instance][hwchannel].firstEdgePolarity = TRUE;
            }
        }

        SchM_Enter_Icu_ICU_EXCLUSIVE_AREA_33();
        {
            /* Set the combine mode */
            ftmIcuBase[instance]->COMBINE |= ((uint32)FTM_COMBINE_DECAPEN0_MASK << ((uint32)8U * ((uint32)hwchannel / (uint32)2U)));
            ftmIcuBase[instance]->COMBINE |= ((uint32)FTM_COMBINE_DECAP0_MASK << ((uint32)8U * ((uint32)hwchannel / (uint32)2U)));
            /* Dual Edge Capture - Continouos Capture Mode. */
            ftmIcuBase[instance]->CONTROLS[hwchannel].CSC |= FTM_CSC_MSA_MASK;
            /* Edge Polarity Selection for hwchannel. */
            ftmIcuBase[instance]->CONTROLS[hwchannel].CSC &= ~(FTM_CSC_ELSB_MASK | FTM_CSC_ELSA_MASK);
            ftmIcuBase[instance]->CONTROLS[hwchannel].CSC |= (uint32)((FTM_CSC_ELSB_MASK | FTM_CSC_ELSA_MASK) & ((uint32)secondChannelEdge));
            ftmIcuBase[instance]->CONTROLS[(hwchannel + 1U)].CSC &= ~(FTM_CSC_ELSB_MASK | FTM_CSC_ELSA_MASK);
            ftmIcuBase[instance]->CONTROLS[(hwchannel + 1U)].CSC |= (uint32)((FTM_CSC_ELSB_MASK | FTM_CSC_ELSA_MASK) & ((uint32)firstChannelEdge));
            /* Clear the flag and enable interrupts for the n+1 hwchannel */
            ftmIcuBase[instance]->CONTROLS[(hwchannel + 1U)].CSC &= ~(FTM_CSC_CHF_MASK);
            ftmIcuBase[instance]->CONTROLS[(hwchannel + 1U)].CSC |= FTM_CSC_CHIE_MASK;
            /* Clear enable interrupt on the n hwchannel */
            ftmIcuBase[instance]->CONTROLS[hwchannel].CSC &= ~(FTM_CSC_CHIE_MASK);
        }
        SchM_Exit_Icu_ICU_EXCLUSIVE_AREA_33();

        /* Prepare the second measurement. */
        Ftm_Icu_Ip_ChState[instance][hwchannel].firstEdge = FALSE;
        
        /* Mark channel which is not verified for spurious interrupt. */
        Ftm_Icu_Ip_InsState[instance].spuriousMask |= ((uint8)(1U << hwchannel));
    }
    else
    {
        if (hwchannel >= (uint8)1U)
        {
            /* Save the Counter Value (CV) for the n and the n+1 channels */
            firstChannelCV  =  (uint16)ftmIcuBase[instance]->CONTROLS[(hwchannel - 1U)].CV;
            secondChannelCV =  (uint16)ftmIcuBase[instance]->CONTROLS[(hwchannel)].CV;

            /* At the 2nd edge, compute the Period and Active Pulse Width and make the callback. */
            if (secondChannelCV > Icu_Ftm_ActivationPulseTemp[instance][(hwchannel - 1U)])
            {
                period = secondChannelCV - Icu_Ftm_ActivationPulseTemp[instance][(hwchannel - 1U)];
            }
            else
            {
                period = (uint16)((uint16)FTM_MAX_VAL_COUNTER - Icu_Ftm_ActivationPulseTemp[instance][(hwchannel - 1U)]) + secondChannelCV + (uint16)1U;
            }
            if (firstChannelCV > Icu_Ftm_ActivationPulseTemp[instance][(hwchannel - 1U)])
            {
                pulseWidth = firstChannelCV - Icu_Ftm_ActivationPulseTemp[instance][(hwchannel - 1U)];
            }
            else
            {
                pulseWidth = (uint16)((uint16)FTM_MAX_VAL_COUNTER - (uint16)Icu_Ftm_ActivationPulseTemp[instance][(hwchannel - 1U)]) + firstChannelCV +  (uint16)1U;
            }
            if (TRUE == Ftm_Icu_Ip_ChState[instance][(hwchannel - 1U)].firstEdgePolarity)
            {
                pulseWidth = period - pulseWidth;
            }
            Icu_Ftm_ActivationPulseTemp[instance][(hwchannel - 1U)] = (uint16)secondChannelCV;
            /* Callback to process data from interrupt. */
            Ftm_Icu_Ip_SignalMeasurementHandler(instance, (hwchannel - 1U), pulseWidth, period, overflow);
        }
        else
        {
            /* Nothing to do */
        }
    }
}

/**
 * @brief Ftm_Icu_Ip_SignalMeasurement - process the signal measurement data
 * 
 * @param instance 
 * @param hwchannel 
 * @param bOverflow 
 */
static inline void Ftm_Icu_Ip_SignalMeasurement(uint8 instance, uint8 hwchannel, boolean overflow)
{
    uint16 firstChannelCV    = 0U;
    uint16 secondChannelCV   = 0U;

    uint16 pulseWidth        = 0U;
    Ftm_Icu_Ip_MeasType measProperty = Ftm_Icu_Ip_ChState[instance][hwchannel].measurement;

    if (((FTM_ICU_HIGH_TIME == measProperty) ||  (FTM_ICU_LOW_TIME == measProperty)) && (hwchannel >= (uint8)1U))
    {
        /* Save hwchannel Value (CV) for the n and n+1 hardware channels */
        firstChannelCV  =  (uint16)ftmIcuBase[instance]->CONTROLS[(hwchannel - 1U)].CV;
        secondChannelCV =  (uint16)ftmIcuBase[instance]->CONTROLS[(hwchannel)].CV;

        /* Calculate Active Pulse Width and make the callback. */
        if(secondChannelCV > firstChannelCV)
        {
            pulseWidth = (uint16)(secondChannelCV - firstChannelCV);
        }
        else
        {
            pulseWidth = (uint16)(FTM_MAX_VAL_COUNTER - firstChannelCV) + secondChannelCV + (uint16)1U;
        }
        /* Callback to process data from interrupt. */
        Ftm_Icu_Ip_SignalMeasurementHandler(instance, (hwchannel - 1U), pulseWidth, 0U, overflow);
    }
    else if (FTM_ICU_PERIOD_TIME == measProperty)
    {
        Ftm_Icu_Ip_PeriodTime(instance, hwchannel, overflow);
    }
    else if (FTM_ICU_DUTY_CYCLE == measProperty)
    {
        Ftm_Icu_Ip_DutyCycle(instance, hwchannel, overflow);
    }
    else
    {
        /* Do nothing. */
    }
}
#endif /* STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API */

/**
 * @brief Ftm IP function that get the state of the overflow flag
 * 
 * @param instance 
 * 
 * @return      boolean      the state of the overflow flag
 * @retval      TRUE         the overflow flag is set
 * @retval      FALSE        the overflow flag is not set
 */
static inline boolean Ftm_Icu_Ip_GetOverflowIrq(uint8 instance)
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

static inline void Ftm_Icu_Ip_ProcessInterrupt(uint8 instance, uint8 hwchannel)
{
    /* Measurement mode of the current hwchannel. */
    Ftm_Icu_Ip_ModeType measMode = Ftm_Icu_Ip_ChState[instance][hwchannel].channelMode;
    boolean bOverflow = Ftm_Icu_Ip_GetOverflowIrq(instance);

    switch (measMode)
    {
#if (STD_ON == FTM_ICU_EDGE_DETECT_API)
        case FTM_ICU_MODE_SIGNAL_EDGE_DETECT:
        {
            Ftm_Icu_Ip_ReportEvents(instance, hwchannel, bOverflow);
        }
        break;
#endif
#if (STD_ON == FTM_ICU_TIMESTAMP_API)
        case FTM_ICU_MODE_TIMESTAMP:
        {
            /* Copy the Counter value in the Timestamp Buffer. */
            Ftm_Icu_Ip_aBufferPtr[instance][hwchannel] = (uint16)ftmIcuBase[instance]->CONTROLS[hwchannel].CV;

            /* Call timestamp handler. */
            Ftm_Icu_Ip_TimestampHandler(instance, hwchannel, &Ftm_Icu_Ip_aBufferPtr[instance][hwchannel], bOverflow);
        }
        break;
#endif
#if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)
        case FTM_ICU_MODE_SIGNAL_MEASUREMENT:
        {
            Ftm_Icu_Ip_SignalMeasurement(instance, hwchannel, bOverflow);
        }
        break;
#endif
#if (STD_ON == FTM_ICU_EDGE_COUNT_API)
        case FTM_ICU_MODE_EDGE_COUNTER:
        {
            Ftm_Icu_Ip_ChState[instance][hwchannel].edgeNumbers++;
            if (0U == Ftm_Icu_Ip_ChState[instance][hwchannel].edgeNumbers)
            {
                bOverflow = TRUE;
            }
            else
            {
                bOverflow = FALSE;
            }
            Ftm_Icu_Ip_ReportEvents(instance, hwchannel, bOverflow);
        }
        break;
#endif /* STD_ON == ICU_EDGE_COUNT_API */
        default:
        {
            /* Do nothing. */
        }
        break;
    }
}

#if ((STD_ON == FTM_ICU_EDGE_DETECT_API) || (STD_ON == FTM_ICU_EDGE_COUNT_API))
static inline void Ftm_Icu_Ip_ReportEvents(uint8   instance,
                                           uint8   hwChannel,
                                           boolean overflow)
{   
    /* Calling HLD Report Events for the logical channel */
    if(Ftm_Icu_Ip_ChState[instance][hwChannel].callback != NULL_PTR)
    {
        Ftm_Icu_Ip_ChState[instance][hwChannel].callback(Ftm_Icu_Ip_ChState[instance][hwChannel].callbackParam, overflow);
    }
    else
    {
        /* Calling Notification for the IPL channel */
        if ((NULL_PTR != Ftm_Icu_Ip_ChState[instance][hwChannel].ftmChannelNotification) && \
           ((boolean)TRUE == Ftm_Icu_Ip_ChState[instance][hwChannel].notificationEnable))
        {
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftmChannelNotification();
        }

        /* Calling User Overflow Notification for the IPL channel */
        if ( overflow && (NULL_PTR != Ftm_Icu_Ip_ChState[instance][hwChannel].ftmOverflowNotification))
        {
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftmOverflowNotification();
        }
    }
}
#endif

#if (STD_ON == FTM_ICU_TIMESTAMP_API)
/** @implements Ftm_Icu_Ip_TimestampHandler_Activity */
static inline void Ftm_Icu_Ip_TimestampHandler(uint8         instance,
                                               uint8         hwChannel,
                                               const uint16* bufferPtr,
                                               boolean       overflow)
{
    Ftm_Icu_Ip_ReportOverflow(instance, hwChannel, overflow);

    Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBuffer[Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferIndex] = (Ftm_Icu_ValueType)bufferPtr[0U];
    Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferIndex++;

    if (Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferIndex >= Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferSize)
    {
        if(FTM_ICU_CIRCULAR_BUFFER == Ftm_Icu_Ip_ChState[instance][hwChannel].timestampBufferType)
        {
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferIndex = 0U;
        }
        else
        {
            Ftm_Icu_Ip_StopTimestamp(instance, hwChannel);
            if (NULL_PTR != Ftm_Icu_Ip_ChState[instance][hwChannel].logicChStateCallback)
            {
                Ftm_Icu_Ip_ChState[instance][hwChannel].logicChStateCallback(Ftm_Icu_Ip_ChState[instance][hwChannel].callbackParam, (1U << 3), FALSE);
            }
        }
    }

    if (0U != Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferNotify)
    {
        Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aNotifyCount++;
        if (Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aNotifyCount >= Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aBufferNotify)
        {
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftm_Icu_Ip_aNotifyCount = 0U;
            /* Call User Notification Function */
            if ( (NULL_PTR != Ftm_Icu_Ip_ChState[instance][hwChannel].ftmChannelNotification) && \
                 ((boolean)TRUE == Ftm_Icu_Ip_ChState[instance][hwChannel].notificationEnable))
            {
                Ftm_Icu_Ip_ChState[instance][hwChannel].ftmChannelNotification();
            }
        }
    }
}
#endif /* FTM_ICU_TIMESTAMP_API */

#if ((STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API) || (STD_ON == FTM_ICU_TIMESTAMP_API))
static inline void Ftm_Icu_Ip_ReportOverflow(uint8   instance,
                                             uint8   hwChannel,
                                             boolean overflow)
{
    /* Calling HLD Report wakeup and overflow for the logical channel */
    if(Ftm_Icu_Ip_ChState[instance][hwChannel].callback != NULL_PTR)
    {
        Ftm_Icu_Ip_ChState[instance][hwChannel].callback(Ftm_Icu_Ip_ChState[instance][hwChannel].callbackParam, overflow);
    }
    else
    {
        /* Calling User Overflow Notification for the IPL channel */
        if ((NULL_PTR != Ftm_Icu_Ip_ChState[instance][hwChannel].ftmOverflowNotification) && overflow)
        {
            Ftm_Icu_Ip_ChState[instance][hwChannel].ftmOverflowNotification();
        }
    }
}
#endif
#endif

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

/*==================================================================================================
*                                        GLOBAL FUNCTIONS
==================================================================================================*/

#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"

#if (defined FTM_ICU_0_ISR_USED)
ISR(FTM_0_ISR)
{
    uint8  moduleIndex  = 0U;
    uint8  channelIndex = 0U;
    /* Read data from status and control register. */
    uint8  isrStatus    = (uint8)ftmIcuBase[moduleIndex]->STATUS;
#ifndef FTM_ICU_TOF_DISTINCT_LINE
    uint32 ovfStatus = ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOF_MASK;
#endif

    /* Clear timer overflow flag. */
    ftmIcuBase[moduleIndex]->SC &= ~(FTM_SC_TOF_MASK);

#ifndef FTM_ICU_TOF_DISTINCT_LINE
    /* Read TOF value and verify if it was an overflow. */
    if ((ovfStatus != 0U) && (FTM_SC_TOIE_MASK == (ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOIE_MASK)))
    {
        Ftm_Icu_Ip_ProcessTofInterrupt(moduleIndex);
    }
#endif

    /* Verify status for each hwchannel and process interrupt. */
    for (channelIndex = 0U; channelIndex < FTM_ICU_IP_NUM_OF_CHANNELS; channelIndex++)
    {
        if ((isrStatus & (uint8)(1U << channelIndex)) != (uint8)0U)
        {
            if (TRUE == Ftm_Icu_Ip_InsState[moduleIndex].instInit)
            {
                if (FTM_CSC_CHIE_MASK == (ftmIcuBase[moduleIndex]->CONTROLS[channelIndex].CSC & FTM_CSC_CHIE_MASK))
                {
                    /* Clear channels interrupt flag. */
                    ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
                    Ftm_Icu_Ip_ProcessInterrupt(moduleIndex, channelIndex);
                }
            }
            else
            {
                /* Clear channels interrupt flag. */
                ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
            }
        }
    }
    EXIT_INTERRUPT();
}
#endif /* FTM_ICU_0_ISR_USED */

#if (defined FTM_ICU_1_ISR_USED)
ISR(FTM_1_ISR)
{
    uint8  moduleIndex  = 1U;
    uint8  channelIndex = 0U;
    /* Read data from status and control register. */
    uint8  isrStatus    = (uint8)ftmIcuBase[moduleIndex]->STATUS;
#ifndef FTM_ICU_TOF_DISTINCT_LINE
    uint32 ovfStatus = ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOF_MASK;
#endif

    /* Clear timer overflow flag. */
    ftmIcuBase[moduleIndex]->SC &= ~(FTM_SC_TOF_MASK);

#ifndef FTM_ICU_TOF_DISTINCT_LINE
    /* Read TOF value and verify if it was an overflow. */
    if ((ovfStatus != 0U) && (FTM_SC_TOIE_MASK == (ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOIE_MASK)))
    {
        Ftm_Icu_Ip_ProcessTofInterrupt(moduleIndex);
    }
#endif

    /* Verify status for each hwchannel and process interrupt. */
    for (channelIndex = 0U; channelIndex < FTM_ICU_IP_NUM_OF_CHANNELS; channelIndex++)
    {
        if ((isrStatus & (uint8)(1U << channelIndex)) != (uint8)0U)
        {
            if (TRUE == Ftm_Icu_Ip_InsState[moduleIndex].instInit)
            {
                if (FTM_CSC_CHIE_MASK == (ftmIcuBase[moduleIndex]->CONTROLS[channelIndex].CSC & FTM_CSC_CHIE_MASK))
                {
                    /* Clear channels interrupt flag. */
                    ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
                    Ftm_Icu_Ip_ProcessInterrupt(moduleIndex, channelIndex);
                }
            }
            else
            {
                /* Clear channels interrupt flag. */
                ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
            }
        }
    }
    EXIT_INTERRUPT();
}
#endif /* FTM_ICU_1_ISR_USED */

#if (defined FTM_ICU_2_ISR_USED)
ISR(FTM_2_ISR)
{
    uint8  moduleIndex  = 2U;
    uint8  channelIndex = 0U;
    /* Read data from status and control register. */
    uint8  isrStatus    = (uint8)ftmIcuBase[moduleIndex]->STATUS;
#ifndef FTM_ICU_TOF_DISTINCT_LINE
    uint32 ovfStatus = ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOF_MASK;
#endif

    /* Clear timer overflow flag. */
    ftmIcuBase[moduleIndex]->SC &= ~(FTM_SC_TOF_MASK);

#ifndef FTM_ICU_TOF_DISTINCT_LINE
    /* Read TOF value and verify if it was an overflow. */
    if ((ovfStatus != 0U) && (FTM_SC_TOIE_MASK == (ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOIE_MASK)))
    {
        Ftm_Icu_Ip_ProcessTofInterrupt(moduleIndex);
    }
#endif

    /* Verify status for each hwchannel and process interrupt. */
    for (channelIndex = 0U; channelIndex < FTM_ICU_IP_NUM_OF_CHANNELS; channelIndex++)
    {
        if ((isrStatus & (uint8)(1U << channelIndex)) != (uint8)0U)
        {
            if (TRUE == Ftm_Icu_Ip_InsState[moduleIndex].instInit)
            {
                if (FTM_CSC_CHIE_MASK == (ftmIcuBase[moduleIndex]->CONTROLS[channelIndex].CSC & FTM_CSC_CHIE_MASK))
                {
                    /* Clear channels interrupt flag. */
                    ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
                    Ftm_Icu_Ip_ProcessInterrupt(moduleIndex, channelIndex);
                }
            }
            else
            {
                /* Clear channels interrupt flag. */
                ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
            }
        }
    }
    EXIT_INTERRUPT();
}
#endif /* FTM_ICU_2_ISR_USED */

#if (defined FTM_ICU_3_ISR_USED)
ISR(FTM_3_ISR)
{
    uint8  moduleIndex  = 3U;
    uint8  channelIndex = 0U;
    /* Read data from status and control register. */
    uint8  isrStatus    = (uint8)ftmIcuBase[moduleIndex]->STATUS;
#ifndef FTM_ICU_TOF_DISTINCT_LINE
    uint32 ovfStatus = ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOF_MASK;
#endif

    /* Clear timer overflow flag. */
    ftmIcuBase[moduleIndex]->SC &= ~(FTM_SC_TOF_MASK);

#ifndef FTM_ICU_TOF_DISTINCT_LINE
    /* Read TOF value and verify if it was an overflow. */
    if ((ovfStatus != 0U) && (FTM_SC_TOIE_MASK == (ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOIE_MASK)))
    {
        Ftm_Icu_Ip_ProcessTofInterrupt(moduleIndex);
    }
#endif

    /* Verify status for each hwchannel and process interrupt. */
    for (channelIndex = 0U; channelIndex < FTM_ICU_IP_NUM_OF_CHANNELS; channelIndex++)
    {
        if ((isrStatus & (uint8)(1U << channelIndex)) != (uint8)0U)
        {
            if (TRUE == Ftm_Icu_Ip_InsState[moduleIndex].instInit)
            {
                if (FTM_CSC_CHIE_MASK == (ftmIcuBase[moduleIndex]->CONTROLS[channelIndex].CSC & FTM_CSC_CHIE_MASK))
                {
                    /* Clear channels interrupt flag. */
                    ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
                    Ftm_Icu_Ip_ProcessInterrupt(moduleIndex, channelIndex);
                }
            }
            else
            {
                /* Clear channels interrupt flag. */
                ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
            }
        }
    }
    EXIT_INTERRUPT();
}
#endif /* FTM_ICU_3_ISR_USED */

#if (defined FTM_ICU_4_ISR_USED)
ISR(FTM_4_ISR)
{
    uint8  moduleIndex  = 4U;
    uint8  channelIndex = 0U;
    /* Read data from status and control register. */
    uint8  isrStatus    = (uint8)ftmIcuBase[moduleIndex]->STATUS;
#ifndef FTM_ICU_TOF_DISTINCT_LINE
    uint32 ovfStatus = ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOF_MASK;
#endif

    /* Clear timer overflow flag. */
    ftmIcuBase[moduleIndex]->SC &= ~(FTM_SC_TOF_MASK);

#ifndef FTM_ICU_TOF_DISTINCT_LINE
    /* Read TOF value and verify if it was an overflow. */
    if ((ovfStatus != 0U) && (FTM_SC_TOIE_MASK == (ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOIE_MASK)))
    {
        Ftm_Icu_Ip_ProcessTofInterrupt(moduleIndex);
    }
#endif

    /* Verify status for each hwchannel and process interrupt. */
    for (channelIndex = 0U; channelIndex < FTM_ICU_IP_NUM_OF_CHANNELS; channelIndex++)
    {
        if ((isrStatus & (uint8)(1U << channelIndex)) != (uint8)0U)
        {
            if (TRUE == Ftm_Icu_Ip_InsState[moduleIndex].instInit)
            {
                if (FTM_CSC_CHIE_MASK == (ftmIcuBase[moduleIndex]->CONTROLS[channelIndex].CSC & FTM_CSC_CHIE_MASK))
                {
                    /* Clear channels interrupt flag. */
                    ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
                    Ftm_Icu_Ip_ProcessInterrupt(moduleIndex, channelIndex);
                }
            }
            else
            {
                /* Clear channels interrupt flag. */
                ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
            }
        }
    }
    EXIT_INTERRUPT();
}
#endif /* FTM_ICU_4_ISR_USED */

#if (defined FTM_ICU_5_ISR_USED)
ISR(FTM_5_ISR)
{
    uint8  moduleIndex  = 5U;
    uint8  channelIndex = 0U;
    /* Read data from status and control register. */
    uint8  isrStatus    = (uint8)ftmIcuBase[moduleIndex]->STATUS;
#ifndef FTM_ICU_TOF_DISTINCT_LINE
    uint32 ovfStatus = ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOF_MASK;
#endif

    /* Clear timer overflow flag. */
    ftmIcuBase[moduleIndex]->SC &= ~(FTM_SC_TOF_MASK);

#ifndef FTM_ICU_TOF_DISTINCT_LINE
    /* Read TOF value and verify if it was an overflow. */
    if ((ovfStatus != 0U) && (FTM_SC_TOIE_MASK == (ftmIcuBase[moduleIndex]->SC & (uint32)FTM_SC_TOIE_MASK)))
    {
        Ftm_Icu_Ip_ProcessTofInterrupt(moduleIndex);
    }
#endif

    /* Verify status for each hwchannel and process interrupt. */
    for (channelIndex = 0U; channelIndex < FTM_ICU_IP_NUM_OF_CHANNELS; channelIndex++)
    {
        if ((isrStatus & (uint8)(1U << channelIndex)) != (uint8)0U)
        {
            if (TRUE == Ftm_Icu_Ip_InsState[moduleIndex].instInit)
            {
                if (FTM_CSC_CHIE_MASK == (ftmIcuBase[moduleIndex]->CONTROLS[channelIndex].CSC & FTM_CSC_CHIE_MASK))
                {
                    /* Clear channels interrupt flag. */
                    ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
                    Ftm_Icu_Ip_ProcessInterrupt(moduleIndex, channelIndex);
                }
            }
            else
            {
                /* Clear channels interrupt flag. */
                ftmIcuBase[moduleIndex]->STATUS &= (uint32)((~((uint32)0x01U << channelIndex)) | (uint32)Ftm_Icu_Ip_InsState[moduleIndex].spuriousMask);
            }
        }
    }
    EXIT_INTERRUPT();
}
#endif /* FTM_ICU_5_ISR_USED */

#if (defined FTM_ICU_0_CH_0_CH_1_ISR_USED)
/**
 * @brief          Independent interrupt handler.
 * @details        Interrupt handler for FTM module 0 channel 0 - channel 1.
 */
ISR(FTM_0_CH_0_CH_1_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(0U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_0_CH_2_CH_3_ISR_USED)
/**
 * @brief          Independent interrupt handler.
 * @details        Interrupt handler for FTM module 0 channel 2 - channel 3.
 */
ISR(FTM_0_CH_2_CH_3_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(0U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_0_CH_4_CH_5_ISR_USED)
/**
 * @brief          Independent interrupt handler.
 * @details        Interrupt handler for FTM module 0 channel 4 - channel 5.
 */
ISR(FTM_0_CH_4_CH_5_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(0U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_0_CH_6_CH_7_ISR_USED)
ISR(FTM_0_CH_6_CH_7_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(0U, 6U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_1_CH_0_CH_1_ISR_USED)
/**
 * @brief          Independent interrupt handler.
 * @details        Interrupt handler for FTM module 1 channel 0 - channel 1.
 */
ISR(FTM_1_CH_0_CH_1_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(1U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_1_CH_2_CH_3_ISR_USED)
/**
 * @brief          Independent interrupt handler.
 * @details        Interrupt handler for FTM module 1 channel 2 - channel 3.
 */
ISR(FTM_1_CH_2_CH_3_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(1U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_1_CH_4_CH_5_ISR_USED)
/**
 * @brief          Independent interrupt handler.
 * @details        Interrupt handler for FTM module 1 channel 4 - channel 5.
 */
ISR(FTM_1_CH_4_CH_5_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(1U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_1_CH_6_CH_7_ISR_USED)
ISR(FTM_1_CH_6_CH_7_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(1U, 6U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_CH_0_CH_1_ISR_USED)
ISR(FTM_2_CH_0_CH_1_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(2U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_CH_2_CH_3_ISR_USED)
ISR(FTM_2_CH_2_CH_3_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(2U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_CH_4_CH_5_ISR_USED)
ISR(FTM_2_CH_4_CH_5_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(2U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_CH_6_CH_7_ISR_USED)
ISR(FTM_2_CH_6_CH_7_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(2U, 6U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_CH_0_CH_1_ISR_USED)
ISR(FTM_3_CH_0_CH_1_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(3U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_CH_2_CH_3_ISR_USED)
ISR(FTM_3_CH_2_CH_3_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(3U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_CH_4_CH_5_ISR_USED)
ISR(FTM_3_CH_4_CH_5_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(3U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_CH_6_CH_7_ISR_USED)
ISR(FTM_3_CH_6_CH_7_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(3U, 6U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_4_CH_0_CH_1_ISR_USED)
ISR(FTM_4_CH_0_CH_1_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(4U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_4_CH_2_CH_3_ISR_USED)
ISR(FTM_4_CH_2_CH_3_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(4U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_4_CH_4_CH_5_ISR_USED)
ISR(FTM_4_CH_4_CH_5_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(4U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_4_CH_6_CH_7_ISR_USED)
ISR(FTM_4_CH_6_CH_7_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(4U, 6U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_5_CH_0_CH_1_ISR_USED)
ISR(FTM_5_CH_0_CH_1_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(5U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_5_CH_2_CH_3_ISR_USED)
ISR(FTM_5_CH_2_CH_3_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(5U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_5_CH_4_CH_5_ISR_USED)
ISR(FTM_5_CH_4_CH_5_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(5U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_5_CH_6_CH_7_ISR_USED)
ISR(FTM_5_CH_6_CH_7_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(5U, 6U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_6_CH_0_CH_1_ISR_USED)
ISR(FTM_6_CH_0_CH_1_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(6U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_6_CH_2_CH_3_ISR_USED)
ISR(FTM_6_CH_2_CH_3_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(6U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_6_CH_4_CH_5_ISR_USED)
ISR(FTM_6_CH_4_CH_5_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(6U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_6_CH_6_CH_7_ISR_USED)
ISR(FTM_6_CH_6_CH_7_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(6U, 6U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_7_CH_0_CH_1_ISR_USED)
ISR(FTM_7_CH_0_CH_1_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(7U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_7_CH_2_CH_3_ISR_USED)
ISR(FTM_7_CH_2_CH_3_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(7U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_7_CH_4_CH_5_ISR_USED)
ISR(FTM_7_CH_4_CH_5_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(7U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_7_CH_6_CH_7_ISR_USED)
ISR(FTM_7_CH_6_CH_7_ISR)
{
    /*calling process interrupt function of corresponding pair channel*/
    Ftm_Icu_Ip_PairChHandler(7U, 6U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_0_CH_0_ISR_USED)
ISR(FTM_0_CH_0_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(0U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_0_CH_1_ISR_USED)
ISR(FTM_0_CH_1_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(0U, 1U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_0_CH_2_ISR_USED)
ISR(FTM_0_CH_2_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(0U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_0_CH_3_ISR_USED)
ISR(FTM_0_CH_3_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(0U, 3U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_0_CH_4_ISR_USED)
ISR(FTM_0_CH_4_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(0U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_0_CH_5_ISR_USED)
ISR(FTM_0_CH_5_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(0U, 5U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_1_CH_0_ISR_USED)
ISR(FTM_1_CH_0_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(1U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_1_CH_1_ISR_USED)
ISR(FTM_1_CH_1_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(1U, 1U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_1_CH_2_ISR_USED)
ISR(FTM_1_CH_2_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(1U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_1_CH_3_ISR_USED)
ISR(FTM_1_CH_3_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(1U, 3U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_1_CH_4_ISR_USED)
ISR(FTM_1_CH_4_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(1U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_1_CH_5_ISR_USED)
ISR(FTM_1_CH_5_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(1U, 5U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_CH_0_ISR_USED)
ISR(FTM_2_CH_0_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(2U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_CH_1_ISR_USED)
ISR(FTM_2_CH_1_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(2U, 1U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_CH_2_ISR_USED)
ISR(FTM_2_CH_2_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(2U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_CH_3_ISR_USED)
ISR(FTM_2_CH_3_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(2U, 3U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_CH_4_ISR_USED)
ISR(FTM_2_CH_4_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(2U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_CH_5_ISR_USED)
ISR(FTM_2_CH_5_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(2U, 5U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_CH_0_ISR_USED)
ISR(FTM_3_CH_0_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(3U, 0U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_CH_1_ISR_USED)
ISR(FTM_3_CH_1_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(3U, 1U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_CH_2_ISR_USED)
ISR(FTM_3_CH_2_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(3U, 2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_CH_3_ISR_USED)
ISR(FTM_3_CH_3_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(3U, 3U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_CH_4_ISR_USED)
ISR(FTM_3_CH_4_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(3U, 4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_CH_5_ISR_USED)
ISR(FTM_3_CH_5_ISR)
{
    /*calling process interrupt function of channel*/
    Ftm_Icu_Ip_ChHandler(3U, 5U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_0_OVF_ISR_USED)
ISR(FTM_0_OVF_ISR)
{
    /*calling FTM 0 overflow interrupt function*/
    Ftm_Icu_Ip_TofHandler(0U);
    EXIT_INTERRUPT();
}
#endif /* FTM_ICU_0_OVF_ISR_USED */

#if (defined FTM_ICU_1_OVF_ISR_USED)
ISR(FTM_1_OVF_ISR)
{
    /*calling FTM 1 overflow interrupt function*/
    Ftm_Icu_Ip_TofHandler(1U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_2_OVF_ISR_USED)
ISR(FTM_2_OVF_ISR)
{
    /*calling FTM 2 overflow interrupt function*/
    Ftm_Icu_Ip_TofHandler(2U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_3_OVF_ISR_USED)
ISR(FTM_3_OVF_ISR)
{
    /*calling FTM 3 overflow interrupt function*/
    Ftm_Icu_Ip_TofHandler(3U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_4_OVF_ISR_USED)
ISR(FTM_4_OVF_ISR)
{
    /*calling FTM 4 overflow interrupt function*/
    Ftm_Icu_Ip_TofHandler(4U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_5_OVF_ISR_USED)
ISR(FTM_5_OVF_ISR)
{
    /*calling FTM 5 overflow interrupt function*/
    Ftm_Icu_Ip_TofHandler(5U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_6_OVF_ISR_USED)
ISR(FTM_6_OVF_ISR)
{
    /*calling FTM 6 overflow interrupt function*/
    Ftm_Icu_Ip_TofHandler(6U);
    EXIT_INTERRUPT();
}
#endif

#if (defined FTM_ICU_7_OVF_ISR_USED)
ISR(FTM_7_OVF_ISR)
{
    /*calling FTM 7 overflow interrupt function*/
    Ftm_Icu_Ip_TofHandler(7U);
    EXIT_INTERRUPT();
}
#endif

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#ifdef __cplusplus
}
#endif

/** @} */
