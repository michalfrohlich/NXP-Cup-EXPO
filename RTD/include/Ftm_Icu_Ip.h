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

#ifndef FTM_ICU_IP_H
#define FTM_ICU_IP_H

/**
 *     @file       Ftm_Icu_Ip.h 
 *     @brief      Header file of Flextimer module.
 *     @details    This file contains signatures for all the functions which are related to FTM 
 *                 module.
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
#include "Ftm_Icu_Ip_Types.h"
#include "Ftm_Icu_Ip_Cfg.h"

#if (FTM_ICU_ENABLE_USER_MODE_SUPPORT == STD_ON)
    #include "Reg_eSys.h"
#endif

/*==================================================================================================
*                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define FTM_ICU_IP_VENDOR_ID                    43
#define FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION     4
#define FTM_ICU_IP_AR_RELEASE_MINOR_VERSION     7
#define FTM_ICU_IP_AR_RELEASE_REVISION_VERSION  0
#define FTM_ICU_IP_SW_MAJOR_VERSION             2
#define FTM_ICU_IP_SW_MINOR_VERSION             0
#define FTM_ICU_IP_SW_PATCH_VERSION             0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if source file and ICU header file are of the same vendor */
#if (FTM_ICU_IP_VENDOR_ID != FTM_ICU_IP_TYPES_VENDOR_ID)
    #error "Ftm_Icu_Ip.h and Ftm_Icu_Ip_Types.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION != FTM_ICU_IP_TYPES_AR_RELEASE_MAJOR_VERSION) || \
     (FTM_ICU_IP_AR_RELEASE_MINOR_VERSION != FTM_ICU_IP_TYPES_AR_RELEASE_MINOR_VERSION) || \
     (FTM_ICU_IP_AR_RELEASE_REVISION_VERSION != FTM_ICU_IP_TYPES_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Ftm_Icu_Ip.h and Ftm_Icu_Ip_Types.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((FTM_ICU_IP_SW_MAJOR_VERSION != FTM_ICU_IP_TYPES_SW_MAJOR_VERSION) || \
     (FTM_ICU_IP_SW_MINOR_VERSION != FTM_ICU_IP_TYPES_SW_MINOR_VERSION) || \
     (FTM_ICU_IP_SW_PATCH_VERSION != FTM_ICU_IP_TYPES_SW_PATCH_VERSION))
#error "Software Version Numbers of Ftm_Icu_Ip.h and Ftm_Icu_Ip_Types.h are different"
#endif

/* Check if source file and ICU header file are of the same vendor */
#if (FTM_ICU_IP_VENDOR_ID != FTM_ICU_IP_CFG_VENDOR_ID)
    #error "Ftm_Icu_Ip.h and Ftm_Icu_Ip_Cfg.h have different vendor IDs"
#endif
/* Check if source file and ICU header file are of the same AutoSar version */
#if ((FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION != FTM_ICU_IP_CFG_AR_RELEASE_MAJOR_VERSION) || \
     (FTM_ICU_IP_AR_RELEASE_MINOR_VERSION != FTM_ICU_IP_CFG_AR_RELEASE_MINOR_VERSION) || \
     (FTM_ICU_IP_AR_RELEASE_REVISION_VERSION != FTM_ICU_IP_CFG_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of Ftm_Icu_Ip.h and Ftm_Icu_Ip_Cfg.h are different"
#endif
/* Check if source file and ICU header file are of the same Software version */
#if ((FTM_ICU_IP_SW_MAJOR_VERSION != FTM_ICU_IP_CFG_SW_MAJOR_VERSION) || \
     (FTM_ICU_IP_SW_MINOR_VERSION != FTM_ICU_IP_CFG_SW_MINOR_VERSION) || \
     (FTM_ICU_IP_SW_PATCH_VERSION != FTM_ICU_IP_CFG_SW_PATCH_VERSION))
#error "Software Version Numbers of Ftm_Icu_Ip.h and Ftm_Icu_Ip_Cfg.h are different"
#endif

#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    #if (FTM_ICU_ENABLE_USER_MODE_SUPPORT == STD_ON)
    /* Check if header file and Reg_eSys.h file are of the same Autosar version */
        #if ((FTM_ICU_IP_AR_RELEASE_MAJOR_VERSION != REG_ESYS_AR_RELEASE_MAJOR_VERSION) || \
             (FTM_ICU_IP_AR_RELEASE_MINOR_VERSION != REG_ESYS_AR_RELEASE_MINOR_VERSION))
            #error "AutoSar Version Numbers of Ftm_Icu_Ip.h and Reg_eSys.h are different"
        #endif
    #endif
#endif
/*===============================================================================================
*                                       DEFINES AND MACROS
===============================================================================================*/

/*==================================================================================================
*                                        GLOBAL VARIABLES
==================================================================================================*/
#define ICU_START_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"

/* Array with FTM modules base addresses. */
extern Ftm_Icu_Ip_BaseType * const ftmIcuBase[];

#define ICU_STOP_SEC_CONST_UNSPECIFIED
#include "Icu_MemMap.h"

#if (defined FTM_ICU_CONFIG_EXT)
#define ICU_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Icu_MemMap.h"

/* Macro used to import FTM generated configurations. */
FTM_ICU_CONFIG_EXT

#define ICU_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Icu_MemMap.h"
#endif
/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/
#define ICU_START_SEC_CODE
#include "Icu_MemMap.h"

/**
 * @details This function configures the channel in the Input Capture mode for either getting
 * time-stamps on edge detection or on signal measurement. When the edge specified in the captureMode
 * argument occurs on the channel and then the FTM counter is captured into the CnV register.
 * The user have to read the CnV register separately to get this value. The filter
 * function is disabled if the filterVal argument passed as 0. The filter feature.
 * is available only on channels 0,1,2,3.
 * 
 * @param[in] instance  Hardware instance of FTM used.
 * @param[in] userConfig     Configuration of the input capture channel.
 * @return Ftm_Icu_Ip_StatusType
 *        - FTM_IP_STATUS_SUCCESS : Completed successfully.
 *        - FTM_IP_STATUS_ERROR : Error occurred.
 */
Ftm_Icu_Ip_StatusType Ftm_Icu_Ip_Init (uint8 instance, const  Ftm_Icu_Ip_ConfigType *userConfig);

#if (STD_ON == FTM_ICU_DEINIT_API)
/**
 * @brief     Disables input capture mode and clears FTM timer configuration
 * 
 * @param[in] instance  Hardware instance of FTM used.
 */
Ftm_Icu_Ip_StatusType Ftm_Icu_Ip_DeInit (uint8 instance);
#endif /* STD_ON == FTM_ICU_DEINIT_API */

#if (STD_ON == FTM_ICU_SET_MODE_API)
/**
 * @brief      Driver function sets FTM hardware channel into SLEEP mode.
 *
 * @param[in]  instance  Hardware instance of FTM used. 
 * @param[in]  hwChannel Hardware channel of FTM used.
 * @return     void
 */
void Ftm_Icu_Ip_SetSleepMode(uint8 instance, uint8 hwChannel);

/**
 * @brief      Driver function sets FTM hardware channel into NORMAL mode.
 *
 * @param[in]  instance  Hardware instance of FTM used. 
 * @param[in]  hwChannel Hardware channel of FTM used.
 * @return     void
 */
void Ftm_Icu_Ip_SetNormalMode(uint8 instance, uint8 hwChannel);
#endif  /* STD_ON == FTM_ICU_SET_MODE_API */

#if (FTM_ICU_CAPTURERGISTER_API == STD_ON)
/**
 * @brief      Capture the value of counter register for a specified channel.
 * 
 * @details   The API shall return the value stored in capture register.
 *            The API is the equivalent of AUTOSAR API GetCaptureRegisterValue.
 * 
 * @param[in]  instance  Hardware instance of FTM used. 
 * @param[in]  hwChannel Hardware channel of FTM used.
 * @return     uint32  Value of the register captured.
 */
uint32 Ftm_Icu_Ip_GetCaptureRegisterValue(uint8 instance, uint8 hwChannel);
#endif
/**
 * @details   This function enables the requested activation condition(rising, falling or both
 *            edges) for corresponding FTM channels.
 * 
 * @param[in] instance     Hardware instance of FTM used. 
 * @param[in] hwChannel    Hardware channel of FTM used.
 * @param[in] activation Edge activation type used.
 * @return    void
 */
void Ftm_Icu_Ip_SetActivationCondition (uint8 instance, uint8 hwChannel, Ftm_Icu_Ip_EdgeType activation);

#if (STD_ON == FTM_ICU_TIMESTAMP_API)
/**
 * @brief FTM IP layer function which starts timestamp measure mode.
 * 
 * @param[in] instance      Hardware instance of FTM used. 
 * @param[in] hwChannel   Hardware channel of FTM used.
 * @param[in] bufferPtr                buffer pointer for results
 * @param[in] bufferSize              size of buffer results
 * @param[in] notifyInterval       interval for calling notification
 * @return    void
 */
void Ftm_Icu_Ip_StartTimestamp
(
    uint8 instance,
    uint8 hwChannel,
    Ftm_Icu_ValueType * bufferPtr,
    uint16 bufferSize,
    uint16 notifyInterval
);

/**
 * @brief FTM IP layer function which stops timestamp measure mode for a given instance and channel.
 * 
 * @param[in] instance  Hardware instance of FTM used. 
 * @param[in] hwChannel Hardware channel of FTM used.
 * @return    void
 */
void Ftm_Icu_Ip_StopTimestamp(uint8 instance, uint8 hwchannel);

/**
 * @brief   This function reads the timestamp index of the given channel.
 * @details The API shall return the index to be used for next timestamp measurement to be stored.
 * 
 * @param[in] instance  Hardware instance of FTM used. 
 * @param[in] channel   Hardware channel of FTM used.
 *
 * @return     uint16 - Timestamp index of the given channel, next index to be used for storring timestamps.
 * @pre        Ftm_Icu_Ip_Init must be called before. Icu_StartTimestamp must be called before.
 */
uint16 Ftm_Icu_Ip_GetTimestampIndex(uint8 instance, uint8 hwChannel);

#if (STD_ON == FTM_ICU_TIMESTAMP_USES_DMA)
/**
 * @brief FTM IP layer function which get address of CVn register.
 * 
 * @param[in] instance  Hardware instance of FTM used. 
 * @param[in] hwChannel Hardware channel of FTM used.
 * @return    void
 */
uint32 Ftm_Icu_Ip_GetStartAddress(uint8 instance, uint8 hwChannel);
#endif

#endif /* STD_ON == FTM_ICU_TIMESTAMP_API */

#if (STD_ON == FTM_ICU_EDGE_DETECT_API)
/**
 * @brief FTM IP layer function which enable edge detection measure mode for a given instance and channel.
 * 
 * @param[in] instance  Hardware instance of FTM used.
 * @param[in] hwChannel Hardware channel of FTM used.
 * @return    Ftm_Icu_Ip_StatusType
 */
void Ftm_Icu_Ip_EnableEdgeDetection(uint8 instance, uint8 hwChannel);
#endif /* STD_ON == ICU_EDGE_DETECT_API */

#if (STD_ON == FTM_ICU_EDGE_DETECT_API)
/**
 * @brief FTM IP layer function which disable edge detection measure mode for a given instance and channel.
 * 
 * @param[in] instance  Hardware instance of FTM used. 
 * @param[in] hwChannel Hardware channel of FTM used.
 * @return    Ftm_Icu_Ip_StatusType
 */
void Ftm_Icu_Ip_DisableEdgeDetection(uint8 instance, uint8 hwChannel);
#endif /* STD_ON == ICU_EDGE_DETECT_API */

#if (STD_ON == FTM_ICU_EDGE_COUNT_API)
/**
 * @brief FTM IP layer function which reset edge count measure mode for a given instance and channel.
 * 
 * @param[in] instance  Hardware instance of FTM used. 
 * @param[in] hwChannel Hardware channel of FTM used.
 * @return    void
 */
void Ftm_Icu_Ip_ResetEdgeCount(uint8 instance, uint8 hwchannel);

/**
 * @brief FTM IP layer function which enable edge count measure mode for a given instance and channel.
 * 
 * @param[in] instance  module instance number 
 * @param[in] hwChannel Hardware channel of FTM used.
 * @return    Ftm_Icu_Ip_StatusType
 */
Ftm_Icu_Ip_StatusType Ftm_Icu_Ip_EnableEdgeCount(uint8 instance, uint8 hwChannel);

/**
 * @brief FTM IP layer function which disable edge count measure mode for a given instance and channel.
 * 
 * @param[in] instance  Hardware instance of FTM used. 
 * @param[in] hwChannel Hardware channel of FTM used.
 * @return    Ftm_Icu_Ip_StatusType
 */
void Ftm_Icu_Ip_DisableEdgeCount(uint8 instance, uint8 hwChannel);

/**
 * @brief FTM IP layer function which gets the number of edges for a given instance and channel.
 * 
 * @param[in] instance  Hardware instance of FTM used. 
 * @param[in] hwChannel Hardware channel of FTM used.
 * @return    uint16  Number of edges counted.
 */
uint16 Ftm_Icu_Ip_GetEdgeNumbers(uint8 instance, uint8 hwChannel);
#endif /* STD_ON == FTM_ICU_EDGE_COUNT_API */

#if (STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API)
/**
 * @brief FTM IP layer function which starts signal measurement mode for a given instance and channel.
 * 
 * @param[in] instance  Hardware instance of FTM used. 
 * @param[in] hwChannel Hardware channel of FTM used.
 * @return    void
 */
void Ftm_Icu_Ip_StartSignalMeasurement(uint8 instance, uint8 hwchannel);

/**
 * @brief FTM IP layer function which stops signal measurement mode for a given instance and channel.
 * 
 * @param[in] instance  Hardware instance of FTM used. 
 * @param[in] hwChannel Hardware channel of FTM used.
 * @return    void
 */
void Ftm_Icu_Ip_StopSignalMeasurement(uint8 instance,uint8 hwchannel);

/**
 * @brief   This function reads the elapsed Signal Low, High or Period Time for the given channel.
 * @details This service is reentrant and reads the elapsed Signal Low Time for the given channel
 *          that is configured  in Measurement Mode Signal Measurement, Signal Low Time.
 *          The elapsed time is measured between a falling edge and the consecutive rising edge of
 *          the channel.
 *          This service reads the elapsed Signal High Time for the given channel that is configured
 *          in Measurement Mode Signal Measurement,Signal High Time.The elapsed time is measured
 *          between a rising edge and the consecutive falling edge of the channel.
 *          This service reads the elapsed Signal Period Time for the given channel that is
 *          configured in Measurement Mode Signal Measurement,  Signal Period Time.The elapsed time
 *          is measured between consecutive  rising (or falling) edges  of the  channel. The  period
 *          start edge is
 *
 *          configurable.
 *
 * @param[in]  instance   Hardware instance of FTM used.
 * @param[in]  hwChannel  Hardware channel of FTM used.
 *
 * @return     uint16 -  the elapsed Signal Low Time for the given channel that is configured in
 *             Measurement Mode Signal Measurement, Signal Low Time
 * @pre        Ftm_Icu_Ip_Init must be called before. The channel must be configured in Measurement Mode Signal
 *             Measurement.
 */
uint16 Ftm_Icu_Ip_GetTimeElapsed(uint8 instance, uint8 hwChannel);

/**
 * @brief   This function reads the coherent active time and period time for the given ICU Channel.
 * @details The function is reentrant and reads the coherent active time and period time for
 *          the given ICU Channel, if it is configured in Measurement Mode Signal Measurement, Duty
 *          Cycle Values.
 *
 * @param[in]  instance   Hardware instance of FTM used.
 * @param[in]  hwChannel  Hardware channel of FTM used.
 * @param[out] dutyCycleValues  structure where duty cycle values are stored 
 *
 * @return     void
 * @pre        Ftm_Icu_Ip_Init must be called before. The given channel must be configured in Measurement Mode
 *             Signal Measurement, Duty Cycle Values.
 */
void Ftm_Icu_Ip_GetDutyCycleValues(uint8 instance,
                                   uint8 hwChannel,
                                   Ftm_Icu_Ip_DutyCycleType* dutyCycleValues);

#endif /* STD_ON == FTM_ICU_SIGNAL_MEASUREMENT_API */

#if (STD_ON == FTM_ICU_DUAL_CLOCK_MODE_API)
/**
 * @brief FTM IP layer that sets the prescaler value for a given instance.
 * 
 * @param[in] instance     Hardware instance of FTM used. 
 * @param[in] prescaler  Value of the prescaler.
 */
void Ftm_Icu_Ip_SetPrescaler(uint8 instance, Ftm_Icu_Ip_ClockModeType selectPrescaler);
#endif /* STD_ON == FTM_ICU_DUAL_CLOCK_MODE_API */

#if ((FTM_ICU_OVERFLOW_NOTIFICATION_API == STD_OFF) && (FTM_ICU_IP_DEV_ERROR_DETECT == STD_ON))
#if ((FTM_ICU_EDGE_COUNT_API == STD_ON) || (FTM_ICU_TIMESTAMP_API == STD_ON) || \
     (FTM_ICU_GET_TIME_ELAPSED_API == STD_ON) || (FTM_ICU_GET_DUTY_CYCLE_VALUES_API == STD_ON))
/**
 * @brief FTM IP function that get the state of the overflow flag
 * 
 * @param[in] instance     Hardware instance of FTM used. 
 * @return    boolean    State of the overflow flag.
 * @retval    TRUE       The overflow flag is set.
 * @retval    FALSE      The overflow flag is not set.
 */
boolean Ftm_Icu_Ip_GetOverflow(uint8 instance);
#endif
#endif /* (FTM_ICU_OVERFLOW_NOTIFICATION_API == STD_OFF) && (FTM_ICU_IP_DEV_ERROR_DETECT == STD_ON) */

#if (STD_ON == FTM_ICU_GET_INPUT_LEVEL_API)
/**
 * @brief The API shall return the input read value for the selected pin, if hardware support the functionality.
 * 
 * @param[in] instance                Hardware instance of FTM used. 
 * @param[in] hwChannel               Hardware channel of FTM used.
 * @return    Ftm_Icu_Ip_LevelType  - Level detected at this time on the input pin
 */
Ftm_Icu_Ip_LevelType Ftm_Icu_Ip_GetInputLevel(uint8 instance, uint8 hwChannel);
#endif

#if (STD_ON == FTM_ICU_GET_INPUT_STATE_API)
/**
 * @brief Return input state of the channel.
 * 
 * @param[in] instance   Hardware instance of FTM used. 
 * @param[in] hwChannel  Hardware channel of FTM used.
 * @return    boolean  Channel level type.
 */
boolean Ftm_Icu_Ip_GetInputState(uint8 instance, uint8 hwChannel);
#endif

/**
 * @brief Enable channel interrupt.
 * 
 * @param[in] instance   Hardware instance of FTM used. 
 * @param[in] hwChannel  Hardware channel of FTM used.
 * @return   void.
 */
void Ftm_Icu_Ip_EnableInterrupt(uint8 instance, uint8 hwChannel);

/**
 * @brief Disable channel interrupt.
 * 
 * @param[in] instance   Hardware instance of FTM used. 
 * @param[in] hwChannel  Hardware channel of FTM used.
 * @return   void.
 */
void Ftm_Icu_Ip_DisableInterrupt(uint8 instance, uint8 hwChannel);

/**
 * @brief      Driver function Enable Notification for timestamp.
 *
 * @param[in]  instance  Hardware instance of FTM used. 
 * @param[in]  hwChannel Hardware channel of FTM used.
 * @return     void
 */
void Ftm_Icu_Ip_EnableNotification(uint8 instance, uint8 hwChannel);

/**
 * @brief      Driver function Disable Notification for timestamp.
 *
 * @param[in]  instance  Hardware instance of FTM used. 
 * @param[in]  hwChannel Hardware channel of FTM used.
 * @return     void
 */
void Ftm_Icu_Ip_DisableNotification(uint8 instance, uint8 hwChannel);

#define ICU_STOP_SEC_CODE
#include "Icu_MemMap.h"

#if defined(__cplusplus)
}
#endif

/** @} */

#endif /* FTM_ICU_IP_H */
