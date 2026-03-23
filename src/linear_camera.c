#ifdef __cplusplus
extern "C" {
#endif

#include "linear_camera.h"
#include "app/car_config.h"
#include "Adc_Types.h"

static volatile LinearCamera LinearCameraInstance;
static Adc_ValueGroupType AdcResultBuffer;

#define LINEAR_CAMERA_BUFFER_COUNT  (2U)
#define LINEAR_CAMERA_INVALID_INDEX (0xFFU)

typedef enum
{
    LINEAR_CAMERA_SYNC_IDLE = 0,
    LINEAR_CAMERA_SYNC_WAIT_SI_HIGH,
    LINEAR_CAMERA_SYNC_WAIT_SI_LOW,
    LINEAR_CAMERA_SYNC_CAPTURING
} LinearCameraSyncPhase;

static LinearCameraFrame LinearCameraBuffers[LINEAR_CAMERA_BUFFER_COUNT];

static volatile uint32 LinearCameraFrameIntervalTicks =
    CAM_FRAME_INTERVAL_TICKS;
static volatile uint32 LinearCameraDroppedFrameCount = 0U;
static volatile boolean LinearCameraStreamEnabled = FALSE;
static volatile uint32 LinearCameraAdcCallbackCount = 0U;
static volatile uint32 LinearCameraDmaFrameCount = 0U;
static volatile uint32 LinearCameraFrameRequestCount = 0U;
static volatile uint32 LinearCameraSiPulseCount = 0U;
static volatile boolean LinearCameraLatestFrameReady = FALSE;
static volatile uint8 LinearCameraWriteBufferIndex = 0U;
static volatile uint8 LinearCameraReadyBufferIndex = LINEAR_CAMERA_INVALID_INDEX;
static volatile boolean LinearCameraFramePending = FALSE;
static volatile LinearCameraSyncPhase LinearCameraSyncPhaseState =
    LINEAR_CAMERA_SYNC_IDLE;

static void LinearCamera_StopCaptureHw(void);
static void LinearCamera_StartFrameTimer(void);
static void LinearCamera_RequestShutterPulse(void);
static void LinearCamera_FinishFrame(void);

static void LinearCamera_StopCaptureHw(void)
{
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);

    Gpt_StopTimer(LinearCameraInstance.ShutterGptChannel);
    Gpt_DisableNotification(LinearCameraInstance.ShutterGptChannel);

    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
    LinearCameraSyncPhaseState = LINEAR_CAMERA_SYNC_IDLE;
    LinearCameraFramePending = FALSE;
}

static void LinearCamera_StartFrameTimer(void)
{
    if (LinearCameraFrameIntervalTicks != 0U)
    {
        Gpt_EnableNotification(LinearCameraInstance.ShutterGptChannel);
        Gpt_StartTimer(LinearCameraInstance.ShutterGptChannel, LinearCameraFrameIntervalTicks);
    }
}

static void LinearCamera_RequestShutterPulse(void)
{
    LinearCameraSyncPhaseState = LINEAR_CAMERA_SYNC_WAIT_SI_HIGH;
    LinearCameraInstance.CurrentIndex = 0U;
    Pwm_EnableNotification(LinearCameraInstance.ClkPwmChannel, PWM_FALLING_EDGE);
}

static void LinearCamera_FinishFrame(void)
{
    uint8 completedIndex = LinearCameraWriteBufferIndex;
    boolean restartPending = LinearCameraFramePending;

    Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);

    if (LinearCameraLatestFrameReady == TRUE)
    {
        LinearCameraDroppedFrameCount++;
    }

    LinearCameraReadyBufferIndex = completedIndex;
    LinearCameraLatestFrameReady = TRUE;
    LinearCameraWriteBufferIndex =
        (uint8)((LinearCameraWriteBufferIndex + 1U) % LINEAR_CAMERA_BUFFER_COUNT);
    LinearCameraDmaFrameCount++;
    LinearCameraFramePending = FALSE;
    LinearCameraSyncPhaseState = LINEAR_CAMERA_SYNC_IDLE;
    LinearCameraInstance.CurrentIndex = 0U;

    if (LinearCameraFrameIntervalTicks == 0U)
    {
        LinearCamera_RequestShutterPulse();
    }
    else if (restartPending == TRUE)
    {
        LinearCamera_RequestShutterPulse();
        LinearCamera_StartFrameTimer();
    }
}

void NewCameraFrame(void)
{
    if ((LinearCameraStreamEnabled != TRUE) ||
        (LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING))
    {
        return;
    }

    LinearCameraFrameRequestCount++;

    if (LinearCameraSyncPhaseState == LINEAR_CAMERA_SYNC_IDLE)
    {
        LinearCamera_RequestShutterPulse();
    }
    else
    {
        LinearCameraFramePending = TRUE;
    }

    LinearCamera_StartFrameTimer();
}

void CameraClock(void)
{
    if ((LinearCameraStreamEnabled != TRUE) ||
        (LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING))
    {
        return;
    }

    if (LinearCameraSyncPhaseState == LINEAR_CAMERA_SYNC_WAIT_SI_HIGH)
    {
        Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_HIGH);
        LinearCameraSyncPhaseState = LINEAR_CAMERA_SYNC_WAIT_SI_LOW;
    }
    else if (LinearCameraSyncPhaseState == LINEAR_CAMERA_SYNC_WAIT_SI_LOW)
    {
        Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
        LinearCameraSiPulseCount++;
        LinearCameraSyncPhaseState = LINEAR_CAMERA_SYNC_CAPTURING;
    }
    else if (LinearCameraSyncPhaseState == LINEAR_CAMERA_SYNC_CAPTURING)
    {
        if (LinearCameraInstance.CurrentIndex < LINEAR_CAMERA_PIXEL_COUNT)
        {
            Adc_StartGroupConversion(LinearCameraInstance.InputAdcGroup);
        }
        else
        {
            LinearCamera_FinishFrame();
        }
    }
}

void CameraAdcFinished(void)
{
    if ((LinearCameraStreamEnabled != TRUE) ||
        (LinearCameraInstance.Status != LINEAR_CAMERA_CAPTURING))
    {
        return;
    }

    LinearCameraAdcCallbackCount++;

    if ((LinearCameraSyncPhaseState == LINEAR_CAMERA_SYNC_CAPTURING) &&
        (LinearCameraInstance.CurrentIndex < LINEAR_CAMERA_PIXEL_COUNT))
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
    LinearCameraDroppedFrameCount = 0U;
    LinearCameraAdcCallbackCount = 0U;
    LinearCameraDmaFrameCount = 0U;
    LinearCameraFrameRequestCount = 0U;
    LinearCameraSiPulseCount = 0U;
    LinearCameraLatestFrameReady = FALSE;
    LinearCameraWriteBufferIndex = 0U;
    LinearCameraReadyBufferIndex = LINEAR_CAMERA_INVALID_INDEX;
    LinearCameraFramePending = FALSE;
    LinearCameraFrameIntervalTicks = CAM_FRAME_INTERVAL_TICKS;
    LinearCameraSyncPhaseState = LINEAR_CAMERA_SYNC_IDLE;

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

    LinearCameraDroppedFrameCount = 0U;
    LinearCameraAdcCallbackCount = 0U;
    LinearCameraDmaFrameCount = 0U;
    LinearCameraFrameRequestCount = 0U;
    LinearCameraSiPulseCount = 0U;
    LinearCameraLatestFrameReady = FALSE;
    LinearCameraWriteBufferIndex = 0U;
    LinearCameraReadyBufferIndex = LINEAR_CAMERA_INVALID_INDEX;
    LinearCameraStreamEnabled = TRUE;
    LinearCameraFramePending = FALSE;
    LinearCameraSyncPhaseState = LINEAR_CAMERA_SYNC_IDLE;

    Adc_SetupResultBuffer(LinearCameraInstance.InputAdcGroup, &AdcResultBuffer);
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0x4000U);
    if (LinearCameraFrameIntervalTicks == 0U)
    {
        LinearCamera_RequestShutterPulse();
    }
    else
    {
        LinearCamera_StartFrameTimer();
    }

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

void LinearCameraGetDebugCounters(LinearCameraDebugCounters *counters)
{
    if (counters == (LinearCameraDebugCounters *)0)
    {
        return;
    }

    counters->frameRequestCount = LinearCameraFrameRequestCount;
    counters->frameStartCount = LinearCameraSiPulseCount;
    counters->frameCompleteCount = LinearCameraDmaFrameCount;
    counters->droppedFrameCount = LinearCameraDroppedFrameCount;
    counters->captureEventCount = LinearCameraAdcCallbackCount;
}

#ifdef __cplusplus
}
#endif
