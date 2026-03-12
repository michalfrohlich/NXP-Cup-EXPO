#ifdef __cplusplus
extern "C" {
#endif

#include "linear_camera.h"
#include "Adc_Types.h"

static volatile LinearCamera LinearCameraInstance;
static Adc_ValueGroupType AdcResultBuffer;

typedef enum
{
    LINEAR_CAMERA_TIMER_PHASE_FRAME_WAIT = 0,
    LINEAR_CAMERA_TIMER_PHASE_SHUTTER_HIGH
} LinearCameraTimerPhase;

static volatile uint32 LinearCameraShutterFrequencyTicks = LINEAR_CAMERA_DEFAULT_SHUTTER_FREQUENCY_TICKS;
static volatile boolean LinearCameraStreamEnabled = FALSE;
static volatile boolean LinearCameraLatestFrameReady = FALSE;
static volatile LinearCameraTimerPhase LinearCameraTimerPhaseState = LINEAR_CAMERA_TIMER_PHASE_SHUTTER_HIGH;

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
    Gpt_StartTimer(LinearCameraInstance.ShutterGptChannel, LINEAR_CAMERA_SHUTTER_HIGH_TIME_TICKS);
}

static void LinearCamera_ScheduleNextFrame(void)
{
    if (LinearCameraShutterFrequencyTicks == 0U)
    {
        LinearCamera_StartShutterHighPulse();
    }
    else
    {
        LinearCameraTimerPhaseState = LINEAR_CAMERA_TIMER_PHASE_FRAME_WAIT;
        Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
        Gpt_EnableNotification(LinearCameraInstance.ShutterGptChannel);
        Gpt_StartTimer(LinearCameraInstance.ShutterGptChannel, LinearCameraShutterFrequencyTicks);
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
        /* Frame readout finished */
        Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
        Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);

        LinearCameraLatestFrameReady = TRUE;

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

    if ((LinearCameraInstance.BufferReference != (LinearCameraFrame *)0) &&
        (LinearCameraInstance.CurrentIndex < LINEAR_CAMERA_PIXEL_COUNT))
    {
        LinearCameraInstance.BufferReference->Values[LinearCameraInstance.CurrentIndex] =
            (uint16)AdcResultBuffer;

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
    LinearCameraInstance.BufferReference = (LinearCameraFrame *)0;

    LinearCameraStreamEnabled = FALSE;
    LinearCameraLatestFrameReady = FALSE;
    LinearCameraTimerPhaseState = LINEAR_CAMERA_TIMER_PHASE_SHUTTER_HIGH;
    LinearCameraShutterFrequencyTicks = LINEAR_CAMERA_DEFAULT_SHUTTER_FREQUENCY_TICKS;

    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);

    Adc_SetupResultBuffer(LinearCameraInstance.InputAdcGroup, &AdcResultBuffer);
    Adc_EnableGroupNotification(LinearCameraInstance.InputAdcGroup);

    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);
    Gpt_DisableNotification(LinearCameraInstance.ShutterGptChannel);
}

boolean LinearCameraStartStream(LinearCameraFrame *Frame)
{
    if (Frame == (LinearCameraFrame *)0)
    {
        return FALSE;
    }

    if (LinearCameraInstance.Status != LINEAR_CAMERA_IDLE)
    {
        return FALSE;
    }

    LinearCameraInstance.BufferReference = Frame;
    LinearCameraInstance.CurrentIndex = 0U;
    LinearCameraInstance.Status = LINEAR_CAMERA_CAPTURING;

    LinearCameraLatestFrameReady = FALSE;
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
    LinearCameraInstance.Status = LINEAR_CAMERA_IDLE;
}

void LinearCameraSetShutterFrequencyTicks(uint32 shutterFrequencyTicks)
{
    if ((shutterFrequencyTicks != 0U) &&
        (LinearCameraInstance.Status == LINEAR_CAMERA_IDLE))
    {
        LinearCameraShutterFrequencyTicks = shutterFrequencyTicks;
    }
}

boolean LinearCameraGetLatestFrame(const LinearCameraFrame **Frame)
{
    if (Frame == (const LinearCameraFrame **)0)
    {
        return FALSE;
    }

    if ((LinearCameraLatestFrameReady == TRUE) &&
        (LinearCameraInstance.BufferReference != (LinearCameraFrame *)0))
    {
        *Frame = LinearCameraInstance.BufferReference;
        LinearCameraLatestFrameReady = FALSE;
        return TRUE;
    }

    return FALSE;
}

LinearCameraStatus LinearCameraGetStatus(void)
{
    return LinearCameraInstance.Status;
}

boolean LinearCameraIsBusy(void)
{
    return (LinearCameraInstance.Status == LINEAR_CAMERA_CAPTURING) ? TRUE : FALSE;
}

#ifdef __cplusplus
}
#endif
