#ifdef __cplusplus
extern "C" {
#endif

#include "linear_camera.h"
#include "Adc_Types.h"

static volatile LinearCamera LinearCameraInstance;
static Adc_ValueGroupType AdcResultBuffer;

/* Called when exposure time is finished */
void NewCameraFrame(void)
{
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0x4000U);
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
    Pwm_EnableNotification(LinearCameraInstance.ClkPwmChannel, PWM_FALLING_EDGE);
}

/* Called on each pixel clock edge */
void CameraClock(void)
{
    if (LinearCameraInstance.CurrentIndex < LINEAR_CAMERA_PIXEL_COUNT)
    {
        Adc_StartGroupConversion(LinearCameraInstance.InputAdcGroup);
    }
    else
    {
        Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
        Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);
        Gpt_StopTimer(LinearCameraInstance.ShutterGptChannel);
        Gpt_DisableNotification(LinearCameraInstance.ShutterGptChannel);
        LinearCameraInstance.Status = LINEAR_CAMERA_READY;
    }
}

/* Called when one ADC sample is finished */
void CameraAdcFinished(void)
{
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
}

void LinearCameraGetFrame(LinearCameraFrame *Frame, uint32 exposureTicks)
{
    LinearCameraInstance.BufferReference = Frame;
    LinearCameraInstance.CurrentIndex = 0U;
    LinearCameraInstance.Status = LINEAR_CAMERA_CAPTURING;

    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_HIGH);
    Gpt_EnableNotification(LinearCameraInstance.ShutterGptChannel);
    Gpt_StartTimer(LinearCameraInstance.ShutterGptChannel, exposureTicks);


    while (LinearCameraInstance.Status != LINEAR_CAMERA_READY)
    {
        ;
    }

    LinearCameraInstance.Status = LINEAR_CAMERA_IDLE;
}
#ifdef __cplusplus
}
#endif
