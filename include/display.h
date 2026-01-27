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
#ifndef DISPLAY_H
#define DISPLAY_H
/*These functions assume the relevant drivers are already initialised*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "PlatformTypes.h"
#include "main_types.h"
#include "CDD_I2c.h"

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
#define CharacterColumns 16U
#define CharacterRows 4U
typedef struct{
    I2c_AddressType I2cAddress;
    uint8 I2cChannel;
    /*this is a buffer for the displayed characters, on calling DisplayRefresh() its contents are shown on screen*/
    unsigned char ScreenCharBuffer[CharacterRows][CharacterColumns];
}Display;
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
void DisplayRefresh(void);
void DisplayText(uint16 DisplayLine, const char Text[16], uint16 TextLength, uint16 TextOffset);
void DisplayValue(uint16 DisplayLine, int Value, uint16 TextLength, uint16 TextOffset);
void DisplayGraph(uint8 DisplayLine, uint8 Values[128], uint16 ValuesCount, uint8 LinesSpan);
void DisplayBarGraph(uint8 DisplayLine, uint8 Values[128], uint16 ValuesCount, uint8 LinesSpan);
void DisplayOverlayVerticalLine(uint8 DisplayLine, uint8 LinesSpan, uint8 x); //for camera testing
void DisplayOverlayHorizontalLine(uint8 DisplayLine, uint8 LinesSpan, uint8 yPx); //for camera testing
void DisplayOverlayHorizontalSegment(uint8 DisplayLine, uint8 LinesSpan, uint8 yPx, uint8 x0, uint8 x1); //for camera testing
void DisplayVector(Vector VectorCoordinates);
void DisplayClear(void);
void RotateFontmap(void);
void DisplayInit(uint8 I2cChannel, uint8 FontmapRotation);

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/** @} */
