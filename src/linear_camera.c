#ifdef __cplusplus
extern "C" {
#endif

#include "linear_camera.h"
#include "app/car_config.h"
#include "Adc_Types.h"
#include "OsIf.h"
#include <string.h>

static volatile LinearCamera LinearCameraInstance;
static Adc_ValueGroupType AdcResultBuffer;

typedef enum
{
    LINEAR_CAMERA_TIMER_PHASE_FRAME_WAIT = 0,
    LINEAR_CAMERA_TIMER_PHASE_SHUTTER_HIGH
} LinearCameraTimerPhase;

#define LINEAR_CAMERA_BUFFER_COUNT  (2U)
#define LINEAR_CAMERA_INVALID_INDEX (0xFFU)

static LinearCameraFrame LinearCameraBuffers[LINEAR_CAMERA_BUFFER_COUNT];

static volatile uint32 LinearCameraFrameIntervalTicks =
    CAM_FRAME_INTERVAL_TICKS;
static volatile uint32 LinearCameraDroppedFrameCount = 0U;
static volatile boolean LinearCameraStreamEnabled = FALSE;
static volatile boolean LinearCameraLatestFrameReady = FALSE;
static volatile uint8 LinearCameraWriteBufferIndex = 0U;
static volatile uint8 LinearCameraReadyBufferIndex = LINEAR_CAMERA_INVALID_INDEX;
static volatile LinearCameraTimerPhase LinearCameraTimerPhaseState =
    LINEAR_CAMERA_TIMER_PHASE_SHUTTER_HIGH;

static void LinearCamera_StopCaptureHw(void)
{
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);

    Gpt_StopTimer(LinearCameraInstance.ShutterGptChannel);
    Gpt_DisableNotification(LinearCameraInstance.ShutterGptChannel);

    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
}

static void LinearCamera_StartShutterHighPulse(void)
{
    LinearCameraTimerPhaseState = LINEAR_CAMERA_TIMER_PHASE_SHUTTER_HIGH;
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_HIGH);
    Gpt_EnableNotification(LinearCameraInstance.ShutterGptChannel);
    Gpt_StartTimer(LinearCameraInstance.ShutterGptChannel, CAM_SHUTTER_HIGH_TIME_TICKS);
}

static void LinearCamera_ScheduleNextFrame(void)
{
    if (LinearCameraFrameIntervalTicks == 0U)
    {
        LinearCamera_StartShutterHighPulse();
    }
    else
    {
        LinearCameraTimerPhaseState = LINEAR_CAMERA_TIMER_PHASE_FRAME_WAIT;
        Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
        Gpt_EnableNotification(LinearCameraInstance.ShutterGptChannel);
        Gpt_StartTimer(LinearCameraInstance.ShutterGptChannel, LinearCameraFrameIntervalTicks);
    }
}

/* Called when GPT timing event is finished */
void NewCameraFrame(void)
{
    if ((LinearCameraStreamEnabled != TRUE) ||
        (LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING))
    {
        return;
    }

    if (LinearCameraTimerPhaseState == LINEAR_CAMERA_TIMER_PHASE_FRAME_WAIT)
    {
        /* End of between-frame wait: begin shutter high pulse */
        LinearCamera_StartShutterHighPulse();
    }
    else
    {
        /* End of shutter high pulse: begin pixel clock readout */
        Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0x4000U);
        Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
        Pwm_EnableNotification(LinearCameraInstance.ClkPwmChannel, PWM_FALLING_EDGE);
    }
}

/* Called on each pixel clock edge */
void CameraClock(void)
{
    if ((LinearCameraStreamEnabled != TRUE) ||
        (LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING))
    {
        return;
    }

    if (LinearCameraInstance.CurrentIndex < LINEAR_CAMERA_PIXEL_COUNT)
    {
        Adc_StartGroupConversion(LinearCameraInstance.InputAdcGroup);
    }
    else
    {
        uint8 completedIndex = LinearCameraWriteBufferIndex;

        /* Frame readout finished */
        Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
        Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);

        if (LinearCameraLatestFrameReady == TRUE)
        {
            LinearCameraDroppedFrameCount++;
        }

        LinearCameraReadyBufferIndex = completedIndex;
        LinearCameraLatestFrameReady = TRUE;

        /* Switch writer to the other internal buffer before starting next frame. */
        LinearCameraWriteBufferIndex =
            (uint8)((LinearCameraWriteBufferIndex + 1U) % LINEAR_CAMERA_BUFFER_COUNT);

        /* Free-running: wait, then start next frame */
        LinearCameraInstance.CurrentIndex = 0U;
        LinearCamera_ScheduleNextFrame();
    }
}

/* Called when one ADC sample is finished */
void CameraAdcFinished(void)
{
    if ((LinearCameraStreamEnabled != TRUE) ||
        (LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING))
    {
        return;
    }

    if (LinearCameraInstance.CurrentIndex < LINEAR_CAMERA_PIXEL_COUNT)
    {
        LinearCameraBuffers[LinearCameraWriteBufferIndex]
            .Values[LinearCameraInstance.CurrentIndex] = (uint16)AdcResultBuffer;

        LinearCameraInstance.CurrentIndex++;
    }
}

void LinearCameraInit(Pwm_ChannelType ClkPwmChannel,
                      Gpt_ChannelType ShutterGptChannel,
                      Adc_GroupType InputAdcGroup,
                      Dio_ChannelType ShutterDioChannel)
{
    LinearCameraInstance.ClkPwmChannel = ClkPwmChannel;
    LinearCameraInstance.ShutterGptChannel = ShutterGptChannel;
    LinearCameraInstance.InputAdcGroup = InputAdcGroup;
    LinearCameraInstance.ShutterDioChannel = ShutterDioChannel;
    LinearCameraInstance.CurrentIndex = 0U;
    LinearCameraInstance.Status = LINEAR_CAMERA_IDLE;

    LinearCameraStreamEnabled = FALSE;
    LinearCameraLatestFrameReady = FALSE;
    LinearCameraWriteBufferIndex = 0U;
    LinearCameraReadyBufferIndex = LINEAR_CAMERA_INVALID_INDEX;
    LinearCameraDroppedFrameCount = 0U;
    LinearCameraTimerPhaseState = LINEAR_CAMERA_TIMER_PHASE_SHUTTER_HIGH;
    LinearCameraFrameIntervalTicks = CAM_FRAME_INTERVAL_TICKS;

    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);

    Adc_SetupResultBuffer(LinearCameraInstance.InputAdcGroup, &AdcResultBuffer);
    Adc_EnableGroupNotification(LinearCameraInstance.InputAdcGroup);

    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);
    Gpt_DisableNotification(LinearCameraInstance.ShutterGptChannel);
}

boolean LinearCameraStartStream(void)
{
    if (LinearCameraInstance.Status != LINEAR_CAMERA_IDLE)
    {
        return FALSE;
    }

    LinearCameraInstance.CurrentIndex = 0U;
    LinearCameraInstance.Status = LINEAR_CAMERA_CAPTURING;

    LinearCameraLatestFrameReady = FALSE;
    LinearCameraReadyBufferIndex = LINEAR_CAMERA_INVALID_INDEX;
    LinearCameraWriteBufferIndex = 0U;
    LinearCameraDroppedFrameCount = 0U;
    LinearCameraStreamEnabled = TRUE;

    LinearCamera_StartShutterHighPulse();

    return TRUE;
}

void LinearCameraStopStream(void)
{
    LinearCameraStreamEnabled = FALSE;
    LinearCamera_StopCaptureHw();
    LinearCameraInstance.CurrentIndex = 0U;
    LinearCameraLatestFrameReady = FALSE;
    LinearCameraReadyBufferIndex = LINEAR_CAMERA_INVALID_INDEX;
    LinearCameraWriteBufferIndex = 0U;
    LinearCameraInstance.Status = LINEAR_CAMERA_IDLE;
}

void LinearCameraSetFrameIntervalTicks(uint32 frameIntervalTicks)
{
    if (LinearCameraInstance.Status == LINEAR_CAMERA_IDLE)
    {
        LinearCameraFrameIntervalTicks = frameIntervalTicks;
    }
}

boolean LinearCameraGetLatestFrame(const LinearCameraFrame **Frame)
{
    if (Frame == (const LinearCameraFrame **)0)
    {
        return FALSE;
    }

    if ((LinearCameraLatestFrameReady == TRUE) &&
        (LinearCameraReadyBufferIndex != LINEAR_CAMERA_INVALID_INDEX))
    {
        *Frame = &LinearCameraBuffers[LinearCameraReadyBufferIndex];
        LinearCameraLatestFrameReady = FALSE;
        return TRUE;
    }

    return FALSE;
}

boolean LinearCameraCopyLatestFrame(LinearCameraFrame *Frame)
{
    boolean hasFrame = FALSE;

    if (Frame == (LinearCameraFrame *)0)
    {
        return FALSE;
    }

    OsIf_SuspendAllInterrupts();

    if ((LinearCameraLatestFrameReady == TRUE) &&
        (LinearCameraReadyBufferIndex != LINEAR_CAMERA_INVALID_INDEX))
    {
        (void)memcpy(Frame,
                     &LinearCameraBuffers[LinearCameraReadyBufferIndex],
                     sizeof(*Frame));
        LinearCameraLatestFrameReady = FALSE;
        hasFrame = TRUE;
    }

    OsIf_ResumeAllInterrupts();

    return hasFrame;
}

LinearCameraStatus LinearCameraGetStatus(void)
{
    return LinearCameraInstance.Status;
}

boolean LinearCameraIsBusy(void)
{
    return (LinearCameraInstance.Status == LINEAR_CAMERA_CAPTURING) ? TRUE : FALSE;
}

uint32 LinearCameraGetPixelClockHz(void)
{
    if (LINEAR_CAMERA_TIMING_PWM_PERIOD_TICKS == 0UL)
    {
        return 0UL;
    }

    return (LINEAR_CAMERA_TIMING_PWM_SOURCE_CLOCK_HZ /
            LINEAR_CAMERA_TIMING_PWM_PERIOD_TICKS);
}

uint32 LinearCameraGetFrameReadoutUs(void)
{
    uint32 pixelClockHz = LinearCameraGetPixelClockHz();
    uint32 clockCount = (uint32)LINEAR_CAMERA_PIXEL_COUNT + 1UL;

    if (pixelClockHz == 0UL)
    {
        return 0UL;
    }

    return (clockCount * 1000000UL) / pixelClockHz;
}

uint32 LinearCameraGetDroppedFrameCount(void)
{
    return LinearCameraDroppedFrameCount;
}

#ifdef __cplusplus
}
#endif
