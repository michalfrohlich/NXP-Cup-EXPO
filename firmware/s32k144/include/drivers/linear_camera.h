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
} LinearCameraFrame_t;

typedef struct
{
    uint32 frameRequestCount;
    uint32 frameStartCount;
    uint32 frameCompleteCount;
    uint32 droppedFrameCount;
    uint32 captureEventCount;
} LinearCameraDebugCounters_t;

typedef enum
{
    LINEAR_CAMERA_IDLE = 0,
    LINEAR_CAMERA_CAPTURING
} LinearCameraStatus_t;

void LinearCamera_Init(Pwm_ChannelType ClkPwmChannel,
                      Gpt_ChannelType ShutterGptChannel,
                      Adc_GroupType InputAdcGroup,
                      Dio_ChannelType ShutterDioChannel);

/* Starts free-running camera clock with SI-aligned manual ADC sampling. */
boolean LinearCamera_StartStream(void);
void LinearCameraStopStream(void);
void LinearCameraSetFrameIntervalTicks(uint32 frameIntervalTicks);
boolean LinearCamera_GetLatestFrame(const LinearCameraFrame_t **Frame);

LinearCameraStatus_t LinearCameraGetStatus(void);
boolean LinearCameraIsBusy(void);

/* Timing/debug helpers */
uint32 LinearCameraGetPixelClockHz(void);
uint32 LinearCameraGetFrameReadoutUs(void);
void LinearCameraGetDebugCounters(LinearCameraDebugCounters_t *counters);

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_CAMERA_H */
