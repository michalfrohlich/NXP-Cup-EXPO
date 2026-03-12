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
#define LINEAR_CAMERA_SHUTTER_HIGH_TIME_TICKS (100U)
#define LINEAR_CAMERA_DEFAULT_SHUTTER_FREQUENCY_TICKS (100U)

typedef struct
{
    uint16 Values[LINEAR_CAMERA_PIXEL_COUNT];
} LinearCameraFrame;

typedef enum
{
    LINEAR_CAMERA_IDLE = 0,
    LINEAR_CAMERA_CAPTURING
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

/* Streaming API (single buffer) */
boolean LinearCameraStartStream(LinearCameraFrame *Frame);
void LinearCameraStopStream(void);
void LinearCameraSetShutterFrequencyTicks(uint32 shutterFrequencyTicks);
boolean LinearCameraGetLatestFrame(const LinearCameraFrame **Frame);

LinearCameraStatus LinearCameraGetStatus(void);
boolean LinearCameraIsBusy(void);

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_CAMERA_H */
