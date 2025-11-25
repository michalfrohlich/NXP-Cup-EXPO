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

/*==================================================================================================
    Fontmap present in the code as static unsigned char Font8x8Basic[128][8] is taken from:
    https://github.com/ajones74/font8x8/blob/master/font8x8_basic.h
==================================================================================================*/

/*    This library is written for the 0.91" monochrome oled display that comes packaged with the
*    MR-CANHUBK344 development board. This is a 128x32 display based on the SSD1306 chip. The
*    library can be made to work with a 128x64 display with minimal changes.
*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
 * 1) system and project includes
 * 2) needed interfaces from external units
 * 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Mcu.h"
#include "Platform.h"
#include "Port.h"
#include "CDD_I2c.h"
#include "display.h"
#include "main_types.h"
/*==================================================================================================
 *                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/
#define DisplayI2cAddress 0x3C

/*==================================================================================================
 *                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                      LOCAL VARIABLES
==================================================================================================*/
static volatile Display DisplayInstance;
/*==================================================================================================
 *                                      GLOBAL CONSTANTS
==================================================================================================*/

/* Constant: Font8x8Basic
Contains an 8x8 font map for unicode points U+0000 - U+007F (basic latin)*/
static unsigned char Font8x8Basic[128][8] = {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0000 (nul)*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0001*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0002*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0003*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0004*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0005*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0006*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0007*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0008*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0009*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+000A*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+000B*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+000C*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+000D*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+000E*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+000F*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0010*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0011*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0012*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0013*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0014*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0015*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0016*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0017*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0018*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0019*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+001A*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+001B*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+001C*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+001D*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+001E*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+001F*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0020 (space)*/
        { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   /* U+0021 (!)*/
        { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0022 (")*/
        { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},   /* U+0023 (#)*/
        { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},   /* U+0024 ($)*/
        { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},   /* U+0025 (%)*/
        { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},   /* U+0026 (&)*/
        { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0027 (')*/
        { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},   /* U+0028 (()*/
        { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},   /* U+0029 ())*/
        { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},   /* U+002A (*)*/
        { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},   /* U+002B (+)*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},   /* U+002C (,)*/
        { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},   /* U+002D (-)*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},   /* U+002E (.)*/
        { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},   /* U+002F (/)*/
        { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   /* U+0030 (0)*/
        { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   /* U+0031 (1)*/
        { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   /* U+0032 (2)*/
        { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   /* U+0033 (3)*/
        { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   /* U+0034 (4)*/
        { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   /* U+0035 (5)*/
        { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   /* U+0036 (6)*/
        { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   /* U+0037 (7)*/
        { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   /* U+0038 (8)*/
        { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   /* U+0039 (9)*/
        { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},   /* U+003A (:)*/
        { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},   /* U+003B (;)*/
        { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},   /* U+003C (<)*/
        { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},   /* U+003D (=)*/
        { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},   /* U+003E (>)*/
        { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},   /* U+003F (?)*/
        { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},   /* U+0040 (@)*/
        { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   /* U+0041 (A)*/
        { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   /* U+0042 (B)*/
        { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},   /* U+0043 (C)*/
        { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},   /* U+0044 (D)*/
        { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   /* U+0045 (E)*/
        { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},   /* U+0046 (F)*/
        { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},   /* U+0047 (G)*/
        { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   /* U+0048 (H)*/
        { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   /* U+0049 (I)*/
        { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},   /* U+004A (J)*/
        { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   /* U+004B (K)*/
        { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},   /* U+004C (L)*/
        { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   /* U+004D (M)*/
        { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   /* U+004E (N)*/
        { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   /* U+004F (O)*/
        { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   /* U+0050 (P)*/
        { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},   /* U+0051 (Q)*/
        { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},   /* U+0052 (R)*/
        { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},   /* U+0053 (S)*/
        { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   /* U+0054 (T)*/
        { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},   /* U+0055 (U)*/
        { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   /* U+0056 (V)*/
        { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   /* U+0057 (W)*/
        { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   /* U+0058 (X)*/
        { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   /* U+0059 (Y)*/
        { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   /* U+005A (Z)*/
        { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},   /* U+005B ([)*/
        { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},   /* U+005C (\)*/
        { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},   /* U+005D (])*/
        { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},   /* U+005E (^)*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   /* U+005F (_)*/
        { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+0060 (`)*/
        { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},   /* U+0061 (a)*/
        { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},   /* U+0062 (b)*/
        { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},   /* U+0063 (c)*/
        { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},   /* U+0064 (d)*/
        { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},   /* U+0065 (e)*/
        { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},   /* U+0066 (f)*/
        { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},   /* U+0067 (g)*/
        { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},   /* U+0068 (h)*/
        { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   /* U+0069 (i)*/
        { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},   /* U+006A (j)*/
        { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},   /* U+006B (k)*/
        { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   /* U+006C (l)*/
        { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},   /* U+006D (m)*/
        { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},   /* U+006E (n)*/
        { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   /* U+006F (o)*/
        { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},   /* U+0070 (p)*/
        { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},   /* U+0071 (q)*/
        { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},   /* U+0072 (r)*/
        { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},   /* U+0073 (s)*/
        { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   /* U+0074 (t)*/
        { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},   /* U+0075 (u)*/
        { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   /* U+0076 (v)*/
        { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},   /* U+0077 (w)*/
        { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},   /* U+0078 (x)*/
        { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},   /* U+0079 (y)*/
        { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},   /* U+007A (z)*/
        { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},   /* U+007B ({)*/
        { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   /* U+007C (|)*/
        { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},   /* U+007D (})*/
        { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* U+007E (~)*/
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    /* U+007F*/
};
/*==================================================================================================
 *                                      GLOBAL VARIABLES
==================================================================================================*/
I2c_DataType OneByteCommandBuffer[2U] = {0x80};/*control byte for one byte commands  is 0x80*/
I2c_DataType TwoByteCommandBuffer[3U] = {0x00};/*control byte for 2 or more byte commands is 0x00*/
I2c_DataType ThreeByteCommandBuffer[4U] = {0x00};
I2c_DataType DataBuffer[2U] = {0xC0};/*control byte for one data byte is 0xC0*/
I2c_DataType AllDataBuffer[CharacterRows*CharacterColumns * 8U + 1U]= {0x40};/*control byte for multiple data bytes is 0x40*/

I2c_RequestType OneByteCommandRequest ={DisplayI2cAddress, FALSE, FALSE, FALSE, FALSE, 2U, I2C_SEND_DATA, OneByteCommandBuffer};
I2c_RequestType TwoByteCommandRequest = {DisplayI2cAddress, FALSE, FALSE, FALSE, FALSE, 3U, I2C_SEND_DATA, TwoByteCommandBuffer};
I2c_RequestType ThreeByteCommandRequest = {DisplayI2cAddress, FALSE, FALSE, FALSE, FALSE, 4U, I2C_SEND_DATA, ThreeByteCommandBuffer};
I2c_RequestType OptimizedDataWriteRequest = {DisplayI2cAddress, FALSE, FALSE, FALSE, FALSE, CharacterRows*CharacterColumns*8U+1U, I2C_SEND_DATA, AllDataBuffer};

/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
 *                                       LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
==================================================================================================*/
void DisplayRefresh(void){
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &OptimizedDataWriteRequest);
    /*Setting page and column ranges again resets the internal cursor to the beginning*/
    /*set page range*/
    ThreeByteCommandBuffer[1] = 0x22;
    ThreeByteCommandBuffer[2] = 0x00;
    ThreeByteCommandBuffer[3] = 0x03;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &ThreeByteCommandRequest);
    /*set column range*/
    ThreeByteCommandBuffer[1] = 0x21;
    ThreeByteCommandBuffer[2] = 0x00;
    ThreeByteCommandBuffer[3] = 0x7F;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &ThreeByteCommandRequest);
}

void DisplayText(uint16 DisplayLine, const char Text[16], uint16 TextLength, uint16 TextOffset){
    uint16 StartIndex;
    uint16 StopIndex;
    unsigned char AsciiCharacter;
    for(uint8 i = TextOffset; i < 16U; i++){
        DisplayInstance.ScreenCharBuffer[DisplayLine][i]=Text[i - TextOffset];
    }
    StartIndex = DisplayLine * 16U + TextOffset;
    StopIndex = DisplayLine * 16U + TextLength + TextOffset;
    for(uint16 DisplayCharacterIndex = StartIndex; DisplayCharacterIndex < StopIndex; DisplayCharacterIndex++){
        /*save the character from the screen buffer to use as an index to get its corresponding fontmap*/
        AsciiCharacter = DisplayInstance.ScreenCharBuffer[DisplayCharacterIndex / 16U][DisplayCharacterIndex % 16U];
        /*go through all fontmap lines and add them to the request buffer*/
        for(uint8 BitmapLineIndex = 0U; BitmapLineIndex < 8U; BitmapLineIndex++){
            /*TextOffset by one because the first position holds the control byte*/
            AllDataBuffer[BitmapLineIndex+DisplayCharacterIndex * 8U + 1U] = Font8x8Basic[AsciiCharacter][BitmapLineIndex];
        }
    }
}

void DisplayValue(uint16 DisplayLine, int Number, uint16 TextLength, uint16 TextOffset){
    uint16 StartIndex;
    uint16 StopIndex;
    unsigned char AsciiCharacter;
    unsigned char AsciiNumberString[16]={"                "};
    int AuxNumber = Number;
    uint8 DigitCount = 0U;
    unsigned char AsciiDigitIndex = 0U;
    /*transform the number in ascii and show it on the display*/
    while(AuxNumber >= 10 || AuxNumber <= -10){
        AuxNumber /= 10;
        DigitCount++;
    }
    DigitCount++;
    AuxNumber = Number;
    if(Number < 0){
        AsciiNumberString[DigitCount] = '-';/*appended at the end because we build the string backwards*/
        AuxNumber *= -1;
    }
    do{/*build the string backwards for easier calculations*/
        AsciiNumberString[AsciiDigitIndex] = AuxNumber % 10 + '0';
        AuxNumber /= 10;
        AsciiDigitIndex++;
    }while(AuxNumber);
    /*take into account negative numbers' '-' character*/
    if(Number >= 0){
        DigitCount -= 1U;
    }
    /*write the built string in display character buffer on the selected line*/
    for(uint8 i = 0; i < TextLength; i++){/*the string is written backwards again to restore its orientation*/
        if(DigitCount - i >= 0){
            DisplayInstance.ScreenCharBuffer[DisplayLine][i + TextOffset] = AsciiNumberString[DigitCount - i];
        }
        else{
            DisplayInstance.ScreenCharBuffer[DisplayLine][i + TextOffset] = ' ';
        }
    }
    StartIndex = DisplayLine * 16 + TextOffset;
    StopIndex = DisplayLine * 16 + TextLength + TextOffset;
    for(uint8 DisplayCharacterIndex = StartIndex; DisplayCharacterIndex < StopIndex; DisplayCharacterIndex++){
        /*save the character from the screen buffer to use as an index to get its corresponding fontmap*/
        AsciiCharacter = DisplayInstance.ScreenCharBuffer[DisplayCharacterIndex / 16U][DisplayCharacterIndex % 16U];
        /*go through all fontmap lines and add them to the request buffer*/
        for(uint8 BitmapLineIndex = 0U; BitmapLineIndex < 8U; BitmapLineIndex++){
            /*TextOffset by one because the first position holds the control byte*/
            AllDataBuffer[BitmapLineIndex + DisplayCharacterIndex * 8U + 1U] = Font8x8Basic[AsciiCharacter][BitmapLineIndex];
        }
    }
}

void DisplayGraph(uint8 DisplayLine, uint8 Values[128], uint16 ValuesCount, uint8 LinesSpan){
    uint8 CurrentBitsShiftPositions;
    uint8 PreviousBitsShiftPositions;
    uint32 CurrentBitsMask;
    uint32 PreviousBitsMask;
    uint32 ColumnPixels;
    uint8 CurrentPixels;
    /*Calculate the position of the pixel on the graph column, build the graph column by shifting and store it*/
    ColumnPixels = 1U << ((100U - (uint32)Values[0]) * (uint32)LinesSpan * 8U / 100U);
    for(uint8 RowIndex = DisplayLine; RowIndex < LinesSpan + DisplayLine; RowIndex++){
        /*first column print does not need/can not print trail*/
        /*splits the column into 8-bit values and inserts them in the display i2c buffer*/
        CurrentPixels = (uint8)(ColumnPixels >> (8U * (RowIndex - DisplayLine)));
        AllDataBuffer[RowIndex * 128U + 1U] = CurrentPixels;
    }
    for(uint16 ColumnIndex = 1U; ColumnIndex < ValuesCount; ColumnIndex++){
        /*Calculate the position of the pixel on the graph column and store it*/
        CurrentBitsShiftPositions = ((100U - (uint32)Values[ColumnIndex]) * (uint32)LinesSpan * 8U / 100U);
        PreviousBitsShiftPositions = ((100U - (uint32)Values[ColumnIndex - 1U])*(uint32)LinesSpan * 8U / 100U);
        /*high and low bits masks are used for printing vertical trails for values that are far apart*/
        CurrentBitsMask = 0xFFFFFFFF << CurrentBitsShiftPositions;
        PreviousBitsMask = 0xFFFFFFFF << PreviousBitsShiftPositions;
        /*bitwise XOR prints the vertical trail for big differences, added shift prints the actual point*/
        ColumnPixels = (CurrentBitsMask ^ PreviousBitsMask) | (1UL << CurrentBitsShiftPositions);
        for(uint8 RowIndex = DisplayLine; RowIndex < LinesSpan + DisplayLine; RowIndex++){
            /*splits the column into 8-bit values and inserts them in the display i2c buffer*/
            CurrentPixels = (uint8)(ColumnPixels >> (8U * (RowIndex - DisplayLine)));
            AllDataBuffer[RowIndex * 128U + ColumnIndex + 1U] = CurrentPixels;
        }
    }
}

void DisplayBarGraph(uint8 DisplayLine, uint8 Values[128], uint16 ValuesCount, uint8 LinesSpan)
{
    uint32 ColumnPixels;
    uint8  CurrentPixels;

    /* Total vertical pixels for this graph region */
    uint8 totalPixels      = (uint8)(LinesSpan * 8U);          /* e.g. 3*8 = 24 */
    uint8 maxHeightPixels  = (uint8)((2U * totalPixels) / 3U); /* 2/3 of total → e.g. 16 */

    /* Precompute horizontal reference line at "max height".
     *
     * Vertically, bit index 0 is the TOP of this graph region,
     * and bit (totalPixels - 1) is the BOTTOM.
     * Bars fill from bottom upwards. If a maximal bar has height H, it uses
     * bits [ totalPixels - H .. totalPixels-1 ].
     *
     * So the TOP of that allowed bar region (the "2/3 line") is bit:
     *   refBitIndex = totalPixels - maxHeightPixels
     */
    uint32 refLineMask = 0U;
    if (maxHeightPixels > 0U)
    {
        uint8 refBitIndex = (uint8)(totalPixels - maxHeightPixels);
        refLineMask = (uint32)1U << refBitIndex;
    }

    /* For each column, build a vertical bar from bottom upwards,
     * but clamp to maxHeightPixels, and OR with the reference line.
     */
    for (uint16 ColumnIndex = 0U; ColumnIndex < ValuesCount; ColumnIndex++)
    {
        uint8 v = Values[ColumnIndex];
        if (v > 100U)
        {
            v = 100U;
        }

        /* Map [0..100] → [0..maxHeightPixels] instead of full height */
        uint8 heightPixels = (uint8)(((uint32)v * (uint32)maxHeightPixels) / 100U);

        uint32 barMask = 0U;

        if (heightPixels > 0U)
        {
            uint8 firstLitBit = (uint8)(totalPixels - heightPixels); /* top of bar */
            uint32 baseMask   = (1UL << heightPixels) - 1UL;         /* H bits at LSB */
            barMask           = baseMask << firstLitBit;             /* shift to correct vertical position */
        }

        /* Combine bar + reference line */
        ColumnPixels = barMask | refLineMask;

        /* Split ColumnPixels into 8-bit chunks and store into AllDataBuffer,
         * same row/column layout as DisplayGraph().
         */
        for (uint8 RowIndex = DisplayLine; RowIndex < (uint8)(LinesSpan + DisplayLine); RowIndex++)
        {
            CurrentPixels = (uint8)(ColumnPixels >> (8U * (RowIndex - DisplayLine)));
            AllDataBuffer[RowIndex * 128U + ColumnIndex + 1U] = CurrentPixels;
        }
    }
}




void DisplayClear(){
    for(uint16 Index = 1U; Index < CharacterRows * 8U * CharacterColumns + 1U; Index++){
        AllDataBuffer[Index] = 0U;
    }
}

void DisplayVector(Vector VectorCoordinates){
    volatile uint16 HorizontalIndexStart, HorizontalIndexEnd, VerticalIndexStart, VerticalIndexEnd, HorizontalIndex, VerticalIndex;
    /*scale the coordinates from -100 100 to display dimensions*/
    VectorCoordinates.x0 = VectorCoordinates.x0 * (CharacterColumns * 8U - 1U) / 100U;
    VectorCoordinates.x1 = VectorCoordinates.x1 * (CharacterColumns * 8U - 1U) / 100U;
    VectorCoordinates.y0 = VectorCoordinates.y0 * (CharacterRows * 8U - 1U) / 100U;
    VectorCoordinates.y1 = VectorCoordinates.y1 * (CharacterRows * 8U - 1U) / 100U;
    /*printing the horizontal points along the vector*/
    if(VectorCoordinates.x0 <= VectorCoordinates.x1){
        HorizontalIndexStart = VectorCoordinates.x0;
        HorizontalIndexEnd = VectorCoordinates.x1;
        VerticalIndexStart = VectorCoordinates.y0;
        VerticalIndexEnd = VectorCoordinates.y1;
    }
    else{
        HorizontalIndexStart = VectorCoordinates.x1;
        HorizontalIndexEnd = VectorCoordinates.x0;
        VerticalIndexStart = VectorCoordinates.y1;
        VerticalIndexEnd = VectorCoordinates.y0;
    }
    if(VerticalIndexStart <= VerticalIndexEnd){
        for(HorizontalIndex = HorizontalIndexStart; HorizontalIndex < HorizontalIndexEnd; HorizontalIndex++){
            VerticalIndex = VerticalIndexStart + ((HorizontalIndex - HorizontalIndexStart) * (VerticalIndexEnd - VerticalIndexStart) / (HorizontalIndexEnd - HorizontalIndexStart));
            AllDataBuffer[HorizontalIndex + (VerticalIndex / 8U)* 128U + 1U] |= 1 << (VerticalIndex % 8U);
        }
    }
    else if(VerticalIndexStart > VerticalIndexEnd)
    {
        for(HorizontalIndex = HorizontalIndexStart; HorizontalIndex < HorizontalIndexEnd; HorizontalIndex++){
            VerticalIndex = VerticalIndexStart - ((HorizontalIndex - HorizontalIndexStart) * (VerticalIndexStart - VerticalIndexEnd) / (HorizontalIndexEnd - HorizontalIndexStart));
            AllDataBuffer[HorizontalIndex + (VerticalIndex / 8U)* 128U + 1U] |= 1 << (VerticalIndex % 8U);
        }
    }
    /*printing the vertical points along the vector*/
    if(VectorCoordinates.y0 <= VectorCoordinates.y1){
        HorizontalIndexStart = VectorCoordinates.x0;
        HorizontalIndexEnd = VectorCoordinates.x1;
        VerticalIndexStart = VectorCoordinates.y0;
        VerticalIndexEnd = VectorCoordinates.y1;
    }
    else{
        HorizontalIndexStart = VectorCoordinates.x1;
        HorizontalIndexEnd = VectorCoordinates.x0;
        VerticalIndexStart = VectorCoordinates.y1;
        VerticalIndexEnd = VectorCoordinates.y0;
    }
    if(HorizontalIndexStart <= HorizontalIndexEnd){
        for(VerticalIndex = VerticalIndexStart; VerticalIndex < VerticalIndexEnd; VerticalIndex++){
            /*put the value in the buffer*/
            HorizontalIndex = HorizontalIndexStart + ((VerticalIndex - VerticalIndexStart) * (HorizontalIndexEnd - HorizontalIndexStart)) / (VerticalIndexEnd - VerticalIndexStart);
            AllDataBuffer[HorizontalIndex + (VerticalIndex / 8U) * 128U + 1U] |= (uint8)(1 << (VerticalIndex % 8U));
        }
    }
    else if (HorizontalIndexStart > HorizontalIndexEnd){
        for(VerticalIndex = VerticalIndexStart; VerticalIndex < VerticalIndexEnd; VerticalIndex++){
            HorizontalIndex = HorizontalIndexStart - ((VerticalIndex - VerticalIndexStart)* (HorizontalIndexStart - HorizontalIndexEnd)) / (VerticalIndexEnd - VerticalIndexStart);
            AllDataBuffer[HorizontalIndex + (VerticalIndex / 8U) * 128U + 1U] |= (uint8)(1 << (VerticalIndex % 8U));
        }
    }
}

void RotateFontmap(void){
    unsigned char CharactersLine;
    unsigned char TempCharactersLine[8];
    for(uint8 BitmapCharacterIndex = 0U; BitmapCharacterIndex < 128U; BitmapCharacterIndex++){
        /*go through all character positions from the 8x8 bitmap*/
        for(uint8 BitmapLineIndex = 0U; BitmapLineIndex < 8U; BitmapLineIndex++){
            /*rotate all characters by 90 degrees*/
            CharactersLine = 0;
            for(int i = 7; i >= 0; i--){
                /*go through every line from the bitmap and take the bit from the position BitmapLineIndex*/
                CharactersLine = CharactersLine << 1U;
                CharactersLine += ((Font8x8Basic[BitmapCharacterIndex][i]) >> BitmapLineIndex) % 2U;
            }
            /*save in a temporary variable because the operations need the original values too*/
            TempCharactersLine[BitmapLineIndex] = CharactersLine;
        }
        /*override the original fontmap*/
        for(uint8 i = 0U; i < 8U; i++){
            Font8x8Basic[BitmapCharacterIndex][i]=TempCharactersLine[i];
        }
    }
}

void DisplayInit(uint8 I2cChannel, uint8 FontmapRotation){
    DisplayInstance.I2cChannel = I2cChannel;
    OneByteCommandBuffer[1] = 0xAF;//on
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &OneByteCommandRequest);
    /*display clock divide ratio*/
    TwoByteCommandBuffer[1] = 0xD5;
    TwoByteCommandBuffer[2] = 0x80;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &TwoByteCommandRequest);
    /*multiplex ratio*/
    TwoByteCommandBuffer[1] = 0xA8;
    TwoByteCommandBuffer[2] = 0x1F;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &TwoByteCommandRequest);
    /*set start line*/
    OneByteCommandBuffer[1] = 0x40;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &OneByteCommandRequest);
    /*set charge pump*/
    TwoByteCommandBuffer[1] = 0x8D;
    TwoByteCommandBuffer[2] = 0x14;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &TwoByteCommandRequest);
    /*set segment re-map*/
    TwoByteCommandBuffer[1] = 0xA1;
    TwoByteCommandBuffer[2] = 0xC8;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &TwoByteCommandRequest);
    /*set com hw config*/
    TwoByteCommandBuffer[1] = 0xDA;
    TwoByteCommandBuffer[2] = 0x02;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &TwoByteCommandRequest);
    /*offset*/
    TwoByteCommandBuffer[1] = 0xD3;
    TwoByteCommandBuffer[2] = 0x00;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &TwoByteCommandRequest);
    /*contrast*/
    TwoByteCommandBuffer[1] = 0x81;
    TwoByteCommandBuffer[2] = 0x05;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &TwoByteCommandRequest);
    /*set precharge period*/
    TwoByteCommandBuffer[1] = 0xD9;
    TwoByteCommandBuffer[2] = 0xF1;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &TwoByteCommandRequest);
    /*set VCOMh deselect level*/
    TwoByteCommandBuffer[1] = 0xDB;
    TwoByteCommandBuffer[2] = 0x20;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &TwoByteCommandRequest);
    /*set page range*/
    ThreeByteCommandBuffer[1] = 0x22;
    ThreeByteCommandBuffer[2] = 0x00;
    ThreeByteCommandBuffer[3] = 0x03;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &ThreeByteCommandRequest);
    /*set column range*/
    ThreeByteCommandBuffer[1] = 0x21;
    ThreeByteCommandBuffer[2] = 0x00;
    ThreeByteCommandBuffer[3] = 0x7F;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &ThreeByteCommandRequest);
    /*all pixels on*/
    OneByteCommandBuffer[1] = 0xA4;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &OneByteCommandRequest);
    /*not inverted*/
    OneByteCommandBuffer[1] = 0xA6;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &OneByteCommandRequest);
    /*display on*/
    OneByteCommandBuffer[1] = 0xAF;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &OneByteCommandRequest);
    /*set address mode (horizontal, vertical etc)*/
    TwoByteCommandBuffer[1] = 0x20;
    TwoByteCommandBuffer[2] = 0x00;
    I2c_SyncTransmit(DisplayInstance.I2cChannel, &TwoByteCommandRequest);
    /*rotate the fontmap if requested*/
    if(FontmapRotation != STD_OFF){
        RotateFontmap();
    }
    /*send the first refresh here*/
    DisplayRefresh();
}

#ifdef __cplusplus
}
#endif

/** @} */
