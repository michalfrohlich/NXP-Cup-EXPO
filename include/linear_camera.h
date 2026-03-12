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
 * Timing values below describe the currently generated peripheral setup:
 * - camera clock PWM runs from an 8 MHz source with period 1000 ticks
 * - shutter GPT runs from an 8 MHz source
 *
 * Keeping these as named constants makes the effective frame timing explicit
 * and prevents the driver from relying on hidden clock assumptions.
 */
#define LINEAR_CAMERA_PWM_CLOCK_HZ              (8000000UL)
#define LINEAR_CAMERA_PWM_PERIOD_TICKS          (1000UL)
#define LINEAR_CAMERA_GPT_CLOCK_HZ              (8000000UL)

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
} LinearCamera;

void LinearCameraInit(Pwm_ChannelType ClkPwmChannel,
                      Gpt_ChannelType ShutterGptChannel,
                      Adc_GroupType InputAdcGroup,
                      Dio_ChannelType ShutterDioChannel);

/* Starts free-running capture into internal ping-pong buffers. */
boolean LinearCameraStartStream(void);
void LinearCameraStopStream(void);
void LinearCameraSetShutterFrequencyTicks(uint32 shutterFrequencyTicks);
boolean LinearCameraGetLatestFrame(const LinearCameraFrame **Frame);

LinearCameraStatus LinearCameraGetStatus(void);
boolean LinearCameraIsBusy(void);

/* Timing/debug helpers */
uint32 LinearCameraGetPixelClockHz(void);
uint32 LinearCameraGetFrameReadoutUs(void);
uint32 LinearCameraGetDroppedFrameCount(void);

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_CAMERA_H */
