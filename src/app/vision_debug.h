#ifndef VISION_DEBUG_H
#define VISION_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h"
#include "../src/services/vision_linear_v2.h"

/* Debug screens */
typedef enum
{
    VDBG_SCREEN_MAIN = 0,
    VDBG_SCREEN_DEBUG_FIXED = 1,
    VDBG_SCREEN_DEBUG_AUTO  = 2,
    VDBG_SCREEN_DEBUG_FINISH = 3
} VisionDebug_Screen_t;

typedef struct
{
    /* DEBUG_FIXED scaling: value >= whiteSat => 100% (top of graph) */
    uint16 whiteSat;
    boolean paused;
} VisionDebug_Config_t;

typedef struct
{
    VisionDebug_Screen_t screen;
    VisionDebug_Screen_t debugEntryScreen;
    uint8 mainTextMode;
    VisionDebug_Config_t cfg;
} VisionDebug_State_t;

void VisionDebug_Init(VisionDebug_State_t *st, uint16 whiteSat_default);
void VisionDebug_OnTick(VisionDebug_State_t *st,
                        boolean screenTogglePressed,
                        boolean modeNextPressed);
boolean VisionDebug_WantsVisionDebugData(const VisionDebug_State_t *st);
void VisionDebug_PrepareVisionDbg(const VisionDebug_State_t *st,
                                  VisionLinear_DebugOut_t *dbg,
                                  uint16 *filteredOutBuf,
                                  sint16 *gradientOutBuf);
void VisionDebug_Draw(const VisionDebug_State_t *st,
                      const uint16 *rawU16,
                      const uint16 *filteredU16,
                      const sint16 *gradientS16,
                      const VisionOutput_t *result,
                      const VisionLinear_DebugOut_t *dbg);

#ifdef __cplusplus
}
#endif

#endif
