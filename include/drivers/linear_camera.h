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
/*
 * Timing helper values below are used only for driver-side calculations such
 * as reporting effective pixel clock and frame readout time. They document the
 * currently generated peripheral setup, but they do not configure hardware.
 */
#define LINEAR_CAMERA_TIMING_PWM_SOURCE_CLOCK_HZ   (8000000UL)
#define LINEAR_CAMERA_TIMING_PWM_PERIOD_TICKS      (1000UL)
#define LINEAR_CAMERA_TIMING_GPT_SOURCE_CLOCK_HZ   (8000000UL)

typedef struct
{
    uint16 Values[LINEAR_CAMERA_PIXEL_COUNT];
} LinearCameraFrame;

typedef struct
{
    uint32 frameRequestCount;
    uint32 frameStartCount;
    uint32 frameCompleteCount;
    uint32 droppedFrameCount;
    uint32 captureEventCount;
} LinearCameraDebugCounters;

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
} LinearCamera;

void LinearCameraInit(Pwm_ChannelType ClkPwmChannel,
                      Gpt_ChannelType ShutterGptChannel,
                      Adc_GroupType InputAdcGroup,
                      Dio_ChannelType ShutterDioChannel);

/* Starts free-running camera clock with SI-aligned manual ADC sampling. */
boolean LinearCameraStartStream(void);
void LinearCameraStopStream(void);
void LinearCameraSetFrameIntervalTicks(uint32 frameIntervalTicks);
boolean LinearCameraGetLatestFrame(const LinearCameraFrame **Frame);

LinearCameraStatus LinearCameraGetStatus(void);
boolean LinearCameraIsBusy(void);

/* Timing/debug helpers */
uint32 LinearCameraGetPixelClockHz(void);
uint32 LinearCameraGetFrameReadoutUs(void);
void LinearCameraGetDebugCounters(LinearCameraDebugCounters *counters);

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_CAMERA_H */
