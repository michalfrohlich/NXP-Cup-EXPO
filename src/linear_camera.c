#ifdef __cplusplus
extern "C" {
#endif

#include "linear_camera.h"
#include "Adc_Types.h"

static volatile LinearCamera LinearCameraInstance;
static Adc_ValueGroupType AdcResultBuffer;

typedef enum
{
    LINEAR_CAMERA_PHASE_IDLE = 0,
    LINEAR_CAMERA_PHASE_WAIT_START,
    LINEAR_CAMERA_PHASE_SHIFTING,
    LINEAR_CAMERA_PHASE_WAIT_LAST_SAMPLE
} LinearCameraPhaseType;

static volatile LinearCameraPhaseType LinearCameraPhase = LINEAR_CAMERA_PHASE_IDLE;
static volatile uint16 LinearCameraClockCount = 0U;
static volatile uint16 LinearCameraAdcDoneCount = 0U;

#define LINEAR_CAMERA_SI_SETUP_DELAY_LOOPS   (4U)
/* Optional safety timeout for missing final ADC completion after clock 129 */
#define LINEAR_CAMERA_FINAL_SAMPLE_TIMEOUT   (100000U)

static void LinearCamera_DelayLoops(volatile uint32 loops)
{
    while (loops > 0U)
    {
        __asm volatile ("nop");
        loops--;
    }
}

/* Stop only the pixel clock generation.
 * Do not disable ADC HW trigger here yet, because the final conversion may still be in flight.
 */
static void LinearCamera_StopClockOnly(void)
{
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);
}

/* Full stop of all capture-related hardware */
static void LinearCamera_StopCaptureHw(void)
{
    Adc_DisableHardwareTrigger(LinearCameraInstance.InputAdcGroup);

    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);

    Gpt_StopTimer(LinearCameraInstance.ShutterGptChannel);
    Gpt_DisableNotification(LinearCameraInstance.ShutterGptChannel);

    /* SI idle low */
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
}

static void LinearCamera_FinishReady(void)
{
    LinearCamera_StopCaptureHw();
    LinearCameraPhase = LINEAR_CAMERA_PHASE_IDLE;
    LinearCameraInstance.Status = LINEAR_CAMERA_READY;
}

static void LinearCamera_FailSafeStop(void)
{
    LinearCamera_StopCaptureHw();
    LinearCameraPhase = LINEAR_CAMERA_PHASE_IDLE;
    LinearCameraInstance.Status = LINEAR_CAMERA_IDLE;
}

void NewCameraFrame(void)
{
    if ((LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING) ||
        (LinearCameraPhase != LINEAR_CAMERA_PHASE_WAIT_START))
    {
        return;
    }

    LinearCameraClockCount = 0U;
    LinearCameraAdcDoneCount = 0U;
    LinearCameraInstance.CurrentIndex = 0U;

    /* SI high before first clock */
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_HIGH);

    LinearCamera_DelayLoops(LINEAR_CAMERA_SI_SETUP_DELAY_LOOPS);

    /* Arm ADC group to accept HW triggers from FTM/TRGMUX/PDB */
    Adc_EnableHardwareTrigger(LinearCameraInstance.InputAdcGroup);

    LinearCameraPhase = LINEAR_CAMERA_PHASE_SHIFTING;

    /* Start camera clock, 50% duty */
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0x4000U);
    Pwm_EnableNotification(LinearCameraInstance.ClkPwmChannel, PWM_FALLING_EDGE);
}

void CameraClock(void)
{
    if ((LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING) ||
        ((LinearCameraPhase != LINEAR_CAMERA_PHASE_SHIFTING) &&
         (LinearCameraPhase != LINEAR_CAMERA_PHASE_WAIT_LAST_SAMPLE)))
    {
        return;
    }

    /* Ignore late PWM callbacks after the clock has already been stopped */
    if (LinearCameraPhase == LINEAR_CAMERA_PHASE_WAIT_LAST_SAMPLE)
    {
        return;
    }

    LinearCameraClockCount++;

    /* SI high before clock 1, SI low before clock 2 */
    if (LinearCameraClockCount == 1U)
    {
        Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
        return;
    }

    /* Clocks 2..128: nothing to do here.
     * ADC completions are asynchronous and intentionally delayed by hardware.
     */
    if (LinearCameraClockCount <= LINEAR_CAMERA_PIXEL_COUNT)
    {
        return;
    }

    /* Clock 129: flush clock. Stop further clocks, then wait for any final ADC completion. */
    if (LinearCameraClockCount == (LINEAR_CAMERA_PIXEL_COUNT + 1U))
    {
        LinearCamera_StopClockOnly();
        LinearCameraPhase = LINEAR_CAMERA_PHASE_WAIT_LAST_SAMPLE;

        /* If all 128 samples are already in, finish immediately */
        if (LinearCameraAdcDoneCount >= LINEAR_CAMERA_PIXEL_COUNT)
        {
            LinearCamera_FinishReady();
        }

        return;
    }

    /* More than 129 clock callbacks is invalid */
    LinearCamera_FailSafeStop();
}

void CameraAdcFinished(void)
{
    if ((LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING) ||
        ((LinearCameraPhase != LINEAR_CAMERA_PHASE_SHIFTING) &&
         (LinearCameraPhase != LINEAR_CAMERA_PHASE_WAIT_LAST_SAMPLE)))
    {
        return;
    }

    if ((LinearCameraInstance.BufferReference == (LinearCameraFrame *)0) ||
        (LinearCameraInstance.CurrentIndex >= LINEAR_CAMERA_PIXEL_COUNT))
    {
        /* Unexpected extra conversion or invalid buffer */
        return;
    }

    LinearCameraInstance.BufferReference->Values[LinearCameraInstance.CurrentIndex] =
        (uint16)AdcResultBuffer;

    LinearCameraInstance.CurrentIndex++;
    LinearCameraAdcDoneCount++;

    /* When the clock train has completed, the last ADC completion ends the scan */
    if ((LinearCameraPhase == LINEAR_CAMERA_PHASE_WAIT_LAST_SAMPLE) &&
        (LinearCameraAdcDoneCount >= LINEAR_CAMERA_PIXEL_COUNT))
    {
        LinearCamera_FinishReady();
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

    LinearCameraPhase = LINEAR_CAMERA_PHASE_IDLE;
    LinearCameraClockCount = 0U;
    LinearCameraAdcDoneCount = 0U;

    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);

    (void)Adc_SetupResultBuffer(LinearCameraInstance.InputAdcGroup, &AdcResultBuffer);
    Adc_EnableGroupNotification(LinearCameraInstance.InputAdcGroup);
    Adc_DisableHardwareTrigger(LinearCameraInstance.InputAdcGroup);

    /* Ensure capture-related HW starts stopped */
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);

    Gpt_StopTimer(LinearCameraInstance.ShutterGptChannel);
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

    LinearCameraPhase = LINEAR_CAMERA_PHASE_WAIT_START;
    LinearCameraClockCount = 0U;
    LinearCameraAdcDoneCount = 0U;

    /* SI low while integrating / waiting */
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);

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

    LinearCameraInstance.BufferReference = (LinearCameraFrame *)0;
    LinearCameraInstance.Status = LINEAR_CAMERA_IDLE;
    return TRUE;
}

void LinearCameraAbort(void)
{
    LinearCamera_StopCaptureHw();
    LinearCameraInstance.CurrentIndex = 0U;
    LinearCameraInstance.BufferReference = (LinearCameraFrame *)0;
    LinearCameraInstance.Status = LINEAR_CAMERA_IDLE;
    LinearCameraPhase = LINEAR_CAMERA_PHASE_IDLE;
    LinearCameraClockCount = 0U;
    LinearCameraAdcDoneCount = 0U;
}

void LinearCameraGetFrame(LinearCameraFrame *Frame, uint32 exposureTicks)
{
    volatile uint32 timeout = LINEAR_CAMERA_FINAL_SAMPLE_TIMEOUT;

    if (LinearCameraStartCapture(Frame, exposureTicks) == FALSE)
    {
        return;
    }

    while ((LinearCameraInstance.Status == LINEAR_CAMERA_CAPTURING) &&
           (timeout > 0U))
    {
        timeout--;
    }

    if (LinearCameraInstance.Status == LINEAR_CAMERA_READY)
    {
        (void)LinearCameraConsumeFrame();
    }
    else if (LinearCameraInstance.Status == LINEAR_CAMERA_CAPTURING)
    {
        LinearCameraAbort();
    }
}

#ifdef __cplusplus
}
#endif
