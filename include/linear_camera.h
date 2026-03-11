#ifndef LINEAR_CAMERA_H
#define LINEAR_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Pwm.h"
#include "Adc.h"
#include "Gpt.h"
#include "Dio.h"
#include "Platform_Types.h"

#define LINEAR_CAMERA_PIXEL_COUNT (128U)

typedef struct
{
    uint16 Values[LINEAR_CAMERA_PIXEL_COUNT];
} LinearCameraFrame;

typedef enum
{
    LINEAR_CAMERA_STATUS_IDLE = 0,
    LINEAR_CAMERA_STATUS_ARMED,
    LINEAR_CAMERA_STATUS_CAPTURING
} LinearCameraStatus;

typedef struct
{
    volatile uint32 ExposureIsrCount;
    volatile uint32 ClockEdgeCbCount;
    volatile uint32 BurstStopCbCount;
    volatile uint32 DmaDoneCbCount;
    volatile uint32 PublishedFrameCount;
    volatile uint32 CopyLatestFrameCount;
    volatile uint32 ArmNextExposureCount;
    volatile uint32 AbortCount;
    volatile uint32 LastStatus;
    volatile uint32 LastPhase;
    volatile uint32 LastReadyIndex;
    volatile uint32 LastWriteIndex;
} LinearCameraDebugCounters;

void LinearCamera_Init(Pwm_ChannelType clkPwmChannel,
                       Gpt_ChannelType exposureGptChannel,
                       Gpt_ChannelType burstStopGptChannel,
                       Adc_GroupType adcGroup,
                       Dio_ChannelType siDioChannel);

/* Start continuous acquisition.
 * The exposure GPT remains one-shot, but is re-armed internally after each frame.
 */
boolean LinearCamera_StartStreaming(uint32 exposureUs);

/* Stop continuous acquisition immediately. */
void LinearCamera_StopStreaming(void);

/* Update exposure for the next re-armed cycle. */
void LinearCamera_SetExposureUs(uint32 exposureUs);

/* Returns TRUE if at least one completed frame is waiting. */
boolean LinearCamera_IsFrameReady(void);

/* Copy the latest completed frame into caller-owned storage.
 * Returns FALSE if no frame is available.
 */
boolean LinearCamera_CopyLatestFrame(LinearCameraFrame *dst);

LinearCameraStatus LinearCamera_GetStatus(void);
boolean LinearCamera_IsBusy(void);
void LinearCamera_Abort(void);

/* Debug helpers */
void LinearCamera_DebugResetCounters(void);
const LinearCameraDebugCounters *LinearCamera_DebugGetCounters(void);

/*
 * Callbacks referenced by Config Tools / generated code
 * Keep these exact names if the .mex refers to them.
 */
void NewCameraFrame(void);
void CameraClock(void);
void CameraBurstStop(void);
void CameraDmaFinished(void);

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_CAMERA_H */
