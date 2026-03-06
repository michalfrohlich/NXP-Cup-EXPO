#ifndef VISION_DEBUG_H
#define VISION_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h"
#include "../src/services/vision_linear_v2.h" /* VisionLinear_ResultType, VisionLinear_DebugOut_t, VLIN_DBG_* */

/* Debug screens */
typedef enum
{
    VDBG_SCREEN_MAIN = 0,
    VDBG_SCREEN_DEBUG_FIXED = 1,
    VDBG_SCREEN_DEBUG_AUTO  = 2
} VisionDebug_Screen_t;

typedef struct
{
    /* FIXED scaling: value >= whiteSat => 100% (top of graph) */
    uint16 whiteSat;
} VisionDebug_Config_t;

typedef struct
{
    VisionDebug_Screen_t screen;

    /* When switching MAIN->DEBUG always enter this mode */
    VisionDebug_Screen_t debugEntryScreen;
    uint8 mainTextMode; //which text is shown on the main screen either 0/1

    VisionDebug_Config_t cfg;
} VisionDebug_State_t;

/* Init state + defaults */
void VisionDebug_Init(VisionDebug_State_t *st, uint16 whiteSat_default);

/* Call every 5ms after Buttons_Update().
   - screenTogglePressed: MAIN <-> DEBUG
   - modeNextPressed: cycles FIXED <-> AUTO (and later more screens)
*/
void VisionDebug_OnTick(VisionDebug_State_t *st,
                        boolean screenTogglePressed,
                        boolean modeNextPressed);

/* Whether we need ProcessFrameEx() to get smooth/threshold/blobs */
boolean VisionDebug_WantsVisionDebugData(const VisionDebug_State_t *st);

/* Prepare dbg request (mask + smooth buffer pointer) */
void VisionDebug_PrepareVisionDbg(const VisionDebug_State_t *st,
                                  VisionLinear_DebugOut_t *dbg,
                                  uint16 *smoothOutBuf /* [VISION_LINEAR_BUFFER_SIZE] */);

/* Draw current screen.
   raw: frame.Values
   smooth/dbg: only valid if VisionDebug_WantsVisionDebugData()==TRUE, otherwise can be NULL.
*/
void VisionDebug_Draw(const VisionDebug_State_t *st,
                      const uint16 *rawU16,
                      const uint16 *smoothU16,
                      const VisionLinear_ResultType *result,
                      const VisionLinear_DebugOut_t *dbg);

#ifdef __cplusplus
}
#endif

#endif
