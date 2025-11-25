/*==================================================================================================
*    Copyright 2021-2024 NXP
*
*    NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be
*    used strictly in accordance with the applicable license terms. By expressly
*    accepting such terms or by downloading, installing, activating and/or otherwise
*    using the software, you are agreeing that you have read, and that you agree to
*    comply with and are bound by, such license terms. If you do not agree to be
*    bound by the applicable license terms, then you may not retain, install,
*    activate or otherwise use the software.
==================================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
 * 1) system and project includes
 * 2) needed interfaces from external units
 * 3) internal and external interfaces from this unit
==================================================================================================*/
#include "main_functions.h"
#include "Mcu.h"
#include "Mcl.h"
#include "Platform.h"
#include "Port.h"
#include "CDD_I2c.h"
#include "Pwm.h"
#include "Icu.h"
#include "Gpt.h"
#include "Adc.h"
#include "Mcal.h"

#include "display.h"
#include "receiver.h"
#include "servo.h"
#include "pixy2.h"
#include "Hbridge.h"
#include "esc.h"
#include "linear_camera.h"
/*==================================================================================================
 *                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
 *                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                      LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
 *                                      GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                      GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
 *                                       LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
==================================================================================================*/
void DriversInit(void){
    uint8 Index;
    /* Init system clock */
#if (MCU_PRECOMPILE_SUPPORT == STD_ON)
    Mcu_Init(NULL_PTR);
#elif (MCU_PRECOMPILE_SUPPORT == STD_OFF)
    Mcu_Init(&Mcu_Config_VS_0);
#endif

    /* Initialize the clock tree and apply PLL as system clock */
    Mcu_InitClock(McuClockSettingConfig_0);
    #if (MCU_NO_PLL == STD_OFF)
    while (MCU_PLL_LOCKED != Mcu_GetPllStatus())
        {
            /* Busy wait until the System PLL is locked */
        }
    Mcu_DistributePllClock();
    #endif
    Mcu_SetMode(McuModeSettingConf_0);

    /* Initialize Platform driver */
    Platform_Init(NULL_PTR);

    /* Initialize Port driver */
    Port_Init(NULL_PTR);

    /* Initialize Mcl */
    Mcl_Init(NULL_PTR);

    /* Init i2c instances */
    I2c_Init(NULL_PTR);

    /* Initialize the Icu driver */
    Icu_Init(NULL_PTR);

    /*Init gpt driver*/
    Gpt_Init(NULL_PTR);

    /*Init adc driver*/
    Adc_Init(NULL_PTR);
    Adc_CalibrationStatusType CalibStatus;
    for(Index = 0; Index <= 5; Index++)
    {
        Adc_Calibrate(0U, &CalibStatus);
        if(CalibStatus.AdcUnitSelfTestStatus == E_OK)
        {
            break;
        }
    }

    /*Init pwm driver*/
    Pwm_Init(NULL_PTR);
}

Vector NormalizePixyVector(Vector PixyVector){
    Vector NormalizedVector;
    NormalizedVector.VectorIndex = PixyVector.VectorIndex;
    NormalizedVector.x0 = PixyVector.x0 * 100U / 78U;
    NormalizedVector.y0 = PixyVector.y0 * 100U / 51U;
    NormalizedVector.x1 = PixyVector.x1 * 100U / 78U;
    NormalizedVector.y1 = PixyVector.y1 * 100U / 51U;
    return NormalizedVector;
}

void Pixy2Test(){
    DetectedVectors PixyVectors;
    volatile uint16 Delay = 10000;
    Pixy2SetLed(0U,255U,0U);
    while(1){
        DisplayClear();
        Pixy2GetVectors(&PixyVectors);
        DisplayValue(0U, PixyVectors.NumberOfVectors, 3U, 0U);
        for(uint8 Index = 0U; Index < PixyVectors.NumberOfVectors; Index++){
            DisplayVector(NormalizePixyVector(PixyVectors.Vectors[Index]));
        }
        DisplayRefresh();
        Delay=10000U;
        while(Delay){
            Delay--;
        }
    }
}

void DisplayTest(){
    uint16 DisplayValueTest = 0UL;
    uint8 DisplayGraphTest[128];
    for(uint8 i=0; i<128U;i++){
        DisplayGraphTest[i] = i/2;
    }
    while(1){
        DisplayText(0U, "Display test", 12U, 0U);
        DisplayValue(0U, DisplayValueTest, 4U, 12U);
        DisplayGraph(1U, DisplayGraphTest, 128U, 3U);
        volatile int Delay = 100000;
        while(Delay){
            Delay--;
        }

        DisplayValueTest++;
        for(uint8 i=0; i<128U;i++){
            DisplayGraphTest[i]++;
            if(DisplayGraphTest[i]>=100U){
                DisplayGraphTest[i] = 0U;
            }
        }
        DisplayRefresh();
    }
}

void ReceiverTest(){
    int ReceiverChannels[8];
    uint8 DisplayValueOffset;
    /*Display static text first*/
    DisplayText(0U, "Ch0:    Ch1:", 12U, 0U);
    DisplayText(1U, "Ch2:    Ch3:", 12U, 0U);
    DisplayText(2U, "Ch4:    Ch5:", 12U, 0U);
    DisplayText(3U, "Ch6:    Ch7:", 12U, 0U);

    while(1){
        for(uint8 i=0U; i<8U; i++){
            ReceiverChannels[i] = GetReceiverChannel(i);/*Save all receiver channel values*/
            DisplayValueOffset = 4U+8U*(i%2U);
            DisplayValue(i/2U, ReceiverChannels[i], 4U, DisplayValueOffset);
        }
        DisplayRefresh();
    }

}

void ServoTest() {
    volatile int Delay;
    volatile int SteerStrength;
    while(1){
        SteerRight();
        Delay = 5000000;
        while(Delay){
            Delay--;
        }
        SteerStraight();
        Delay = 5000000;
        while(Delay){
            Delay--;
        }
        SteerLeft();
        Delay = 5000000;
        while(Delay){
            Delay--;
        }
        for(SteerStrength = -100; SteerStrength <=100; SteerStrength++){
            Delay = 100000;
            while(Delay){
                Delay--;
            }
            Steer(SteerStrength);
        }
    }
}

void LinearCameraTest(){
    LinearCameraFrame FrameBuffer;
    while(1){
        LinearCameraGetFrame(&FrameBuffer);
        DisplayGraph(0U, FrameBuffer.Values, 128U, 4U);
        DisplayRefresh();
    }
}

void HbridgeTest(){
    volatile int Delay;
    volatile int Speed;
    while(1){
        for(Speed = 0; Speed <=100; Speed++){
            Delay = 500000;
            while(Delay){
                Delay--;
            }
            HbridgeSetSpeed(Speed);
        }
        HbridgeSetBrake(1U);
        HbridgeSetSpeed(0);
        Delay = 5000000;
        while(Delay){
            Delay--;
        }
        HbridgeSetBrake(0U);
        for(Speed = 0; Speed >= -100; Speed--){
            Delay = 500000;
            while(Delay){
                Delay--;
            }
            HbridgeSetSpeed(Speed);
        }
        HbridgeSetBrake(1U);
        HbridgeSetSpeed(0);
        Delay = 5000000;
        while(Delay){
            Delay--;
        }
        HbridgeSetBrake(0U);
    }
}

void EscTest(){
    volatile int Delay;
    volatile int Speed;
    while(1){
        for(Speed = 0; Speed <=100; Speed++){
            Delay = 500000;
            while(Delay){
                Delay--;
            }
            EscSetSpeed(Speed);
        }
        EscSetBrake(1U);
        EscSetSpeed(0);
        Delay = 5000000;
        while(Delay){
            Delay--;
        }
        EscSetBrake(0U);
        for(Speed = 0; Speed >= -100; Speed--){
            Delay = 500000;
            while(Delay){
                Delay--;
            }
            EscSetSpeed(Speed);
        }
        EscSetBrake(1U);
        EscSetSpeed(0);
        Delay = 5000000;
        while(Delay){
            Delay--;
        }
        EscSetBrake(0U);
    }
}

#ifdef __cplusplus
}
#endif

/** @} */
