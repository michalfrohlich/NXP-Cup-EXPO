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
    LINEAR_CAMERA_IDLE = 0,
    LINEAR_CAMERA_CAPTURING,
    LINEAR_CAMERA_READY
} LinearCameraStatus;

typedef struct
{
    Pwm_ChannelType ClkPwmChannel;
    Gpt_ChannelType ShutterGptChannel;
    Adc_GroupType InputAdcGroup;
    Dio_ChannelType ShutterDioChannel;

    volatile uint16 CurrentIndex;
    volatile LinearCameraStatus Status;

    LinearCameraFrame *BufferReference;
} LinearCamera;

void LinearCameraInit(Pwm_ChannelType ClkPwmChannel,
                      Gpt_ChannelType ShutterGptChannel,
                      Adc_GroupType InputAdcGroup,
                      Dio_ChannelType ShutterDioChannel);

void LinearCameraGetFrame(LinearCameraFrame *Frame, uint32 exposureTicks);

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_CAMERA_H */
