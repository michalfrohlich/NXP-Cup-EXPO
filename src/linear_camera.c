#ifdef __cplusplus
extern "C" {
#endif

#include "linear_camera.h"
#include "Adc_Types.h"

static volatile LinearCamera LinearCameraInstance;
static Adc_ValueGroupType AdcResultBuffer;

/* ------------------------------------------------------------------------------------------------
 * Internal helper: stop/cleanup active capture-related peripherals.
 * This is intentionally minimal and keeps the existing hardware flow intact.
 * -----------------------------------------------------------------------------------------------*/
static void LinearCamera_StopCaptureHw(void)
{
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);

    Gpt_StopTimer(LinearCameraInstance.ShutterGptChannel);
    Gpt_DisableNotification(LinearCameraInstance.ShutterGptChannel);

    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW); //is this needed?
}

/* Called when exposure time is finished */
void NewCameraFrame(void)
{
    /* Ignore stray callback unless a capture is actually active */
    if (LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING)
    {
        return;
    }

    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0x4000U);
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
    Pwm_EnableNotification(LinearCameraInstance.ClkPwmChannel, PWM_FALLING_EDGE);
}

/* Called on each pixel clock edge */
void CameraClock(void)
{
    if (LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING)
    {
        return;
    }

    if (LinearCameraInstance.CurrentIndex < LINEAR_CAMERA_PIXEL_COUNT)
    {
        Adc_StartGroupConversion(LinearCameraInstance.InputAdcGroup);
    }
    else
    {
        LinearCamera_StopCaptureHw();
        LinearCameraInstance.Status = LINEAR_CAMERA_READY;
    }
}

/* Called when one ADC sample is finished */
void CameraAdcFinished(void)
{
    if (LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING)
    {
        return;
    }

    if ((LinearCameraInstance.BufferReference != (LinearCameraFrame *)0) &&
        (LinearCameraInstance.CurrentIndex < LINEAR_CAMERA_PIXEL_COUNT))
    {
        /* Store raw ADC value directly, no scaling */
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

    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);

    Adc_SetupResultBuffer(LinearCameraInstance.InputAdcGroup, &AdcResultBuffer);
    Adc_EnableGroupNotification(LinearCameraInstance.InputAdcGroup);

    /* Ensure capture-related HW starts from a stopped state */
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);
    Gpt_DisableNotification(LinearCameraInstance.ShutterGptChannel);
}

boolean LinearCameraStartCapture(LinearCameraFrame *Frame, uint32 exposureTicks)
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

    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_HIGH);
    Gpt_EnableNotification(LinearCameraInstance.ShutterGptChannel);
    Gpt_StartTimer(LinearCameraInstance.ShutterGptChannel, exposureTicks);

    return TRUE;
}

LinearCameraStatus LinearCameraGetStatus(void)
{
    return LinearCameraInstance.Status;
}

boolean LinearCameraIsBusy(void)
{
    return (LinearCameraInstance.Status == LINEAR_CAMERA_CAPTURING) ? TRUE : FALSE;
}

boolean LinearCameraIsFrameReady(void)
{
    return (LinearCameraInstance.Status == LINEAR_CAMERA_READY) ? TRUE : FALSE;
}

boolean LinearCameraConsumeFrame(void)
{
    if (LinearCameraInstance.Status != LINEAR_CAMERA_READY)
    {
        return FALSE;
    }

    LinearCameraInstance.Status = LINEAR_CAMERA_IDLE;
    return TRUE;
}

void LinearCameraAbort(void)
{
    LinearCamera_StopCaptureHw();
    LinearCameraInstance.CurrentIndex = 0U;
    LinearCameraInstance.BufferReference = (LinearCameraFrame *)0;
    LinearCameraInstance.Status = LINEAR_CAMERA_IDLE;
}

/* Legacy blocking version */
void LinearCameraGetFrame(LinearCameraFrame *Frame, uint32 exposureTicks)
{
    if (LinearCameraStartCapture(Frame, exposureTicks) == FALSE)
    {
        return;
    }

    while (LinearCameraIsFrameReady() == FALSE)
    {
        ;
    }

    (void)LinearCameraConsumeFrame();
}

#ifdef __cplusplus
}
#endif