#ifdef __cplusplus
extern "C" {
#endif

#include "linear_camera.h"
#include <string.h>

/* Replace these with your platform's interrupt primitives if different */
#ifndef LINEAR_CAMERA_ENTER_CRITICAL
#define LINEAR_CAMERA_ENTER_CRITICAL()   SuspendAllInterrupts()
#define LINEAR_CAMERA_EXIT_CRITICAL()    ResumeAllInterrupts()
#endif

typedef enum
{
    LINEAR_CAMERA_PHASE_IDLE = 0,
    LINEAR_CAMERA_PHASE_WAIT_START,
    LINEAR_CAMERA_PHASE_SHIFTING,
    LINEAR_CAMERA_PHASE_WAIT_COMPLETE
} LinearCameraPhaseType;

typedef struct
{
    Pwm_ChannelType ClkPwmChannel;
    Gpt_ChannelType ExposureGptChannel;
    Gpt_ChannelType BurstStopGptChannel;
    Adc_GroupType AdcGroup;
    Dio_ChannelType SiDioChannel;

    volatile LinearCameraStatus Status;
    volatile LinearCameraPhaseType Phase;

    volatile boolean StreamEnabled;
    volatile boolean BurstDone;
    volatile boolean DmaDone;
    volatile boolean SiPendingLow;

    volatile boolean ReadyValid;
    volatile uint8   WriteIndex;
    volatile uint8   ReadyIndex;

    volatile uint32 ExposureTicks;

    LinearCameraFrame Buffers[2];
} LinearCameraContext;

static volatile LinearCameraContext gLinearCamera;
static volatile LinearCameraDebugCounters gLinearCameraDbg;

/*
 * Timing assumptions for current setup:
 * - camera CLK = 500 kHz => 2 us period
 * - GPT clock = 48 MHz
 * - 129 clocks total per readout => 258 us
 * - fixed integration already contributed by sensor timing = 220 us
 * - minimum extra delay after previous readout = 20 us
 */
#define LINEAR_CAMERA_TIMER_TICKS_PER_US      (48UL)
#define LINEAR_CAMERA_FIXED_INTEGRATION_US    (220UL)
#define LINEAR_CAMERA_MIN_EXTRA_DELAY_US      (20UL)
#define LINEAR_CAMERA_BURST_STOP_TICKS        (12384UL) /* 258 us * 48 MHz */
#define LINEAR_CAMERA_SI_SETUP_DELAY_LOOPS    (4U)

#define LINEAR_CAMERA_BUFFER_A                (0U)
#define LINEAR_CAMERA_BUFFER_B                (1U)
#define LINEAR_CAMERA_INVALID_INDEX           (0xFFU)

static void LinearCamera_DebugSnapshotState(void)
{
    gLinearCameraDbg.LastStatus    = (uint32)gLinearCamera.Status;
    gLinearCameraDbg.LastPhase     = (uint32)gLinearCamera.Phase;
    gLinearCameraDbg.LastReadyIndex = (uint32)gLinearCamera.ReadyIndex;
    gLinearCameraDbg.LastWriteIndex = (uint32)gLinearCamera.WriteIndex;
}

void LinearCamera_DebugResetCounters(void)
{
    gLinearCameraDbg.ExposureIsrCount     = 0U;
    gLinearCameraDbg.ClockEdgeCbCount     = 0U;
    gLinearCameraDbg.BurstStopCbCount     = 0U;
    gLinearCameraDbg.DmaDoneCbCount       = 0U;
    gLinearCameraDbg.PublishedFrameCount  = 0U;
    gLinearCameraDbg.CopyLatestFrameCount = 0U;
    gLinearCameraDbg.ArmNextExposureCount = 0U;
    gLinearCameraDbg.AbortCount           = 0U;
    gLinearCameraDbg.LastStatus           = 0U;
    gLinearCameraDbg.LastPhase            = 0U;
    gLinearCameraDbg.LastReadyIndex       = 0U;
    gLinearCameraDbg.LastWriteIndex       = 0U;
}

const LinearCameraDebugCounters *LinearCamera_DebugGetCounters(void)
{
    return (const LinearCameraDebugCounters *)&gLinearCameraDbg;
}

static void LinearCamera_DelayLoops(volatile uint32 loops)
{
    while (loops > 0U)
    {
        __asm volatile ("nop");
        loops--;
    }
}

static uint32 LinearCamera_ExposureUsToTimerTicks(uint32 exposureUs)
{
    uint32 waitUs;

    if (exposureUs <= LINEAR_CAMERA_FIXED_INTEGRATION_US)
    {
        waitUs = LINEAR_CAMERA_MIN_EXTRA_DELAY_US;
    }
    else
    {
        waitUs = exposureUs - LINEAR_CAMERA_FIXED_INTEGRATION_US;
        if (waitUs < LINEAR_CAMERA_MIN_EXTRA_DELAY_US)
        {
            waitUs = LINEAR_CAMERA_MIN_EXTRA_DELAY_US;
        }
    }

    return (waitUs * LINEAR_CAMERA_TIMER_TICKS_PER_US);
}

static void LinearCamera_StopAllHw(void)
{
    Adc_DisableHardwareTrigger(gLinearCamera.AdcGroup);

    Pwm_SetDutyCycle(gLinearCamera.ClkPwmChannel, 0U);
    Pwm_DisableNotification(gLinearCamera.ClkPwmChannel);

    Gpt_StopTimer(gLinearCamera.ExposureGptChannel);
    Gpt_DisableNotification(gLinearCamera.ExposureGptChannel);

    Gpt_StopTimer(gLinearCamera.BurstStopGptChannel);
    Gpt_DisableNotification(gLinearCamera.BurstStopGptChannel);

    Dio_WriteChannel(gLinearCamera.SiDioChannel, (Dio_LevelType)STD_LOW);
}


static void LinearCamera_StopClockOnly(void)
{
    Pwm_SetDutyCycle(gLinearCamera.ClkPwmChannel, 0U);
    Pwm_DisableNotification(gLinearCamera.ClkPwmChannel);
    //Pwm_ForceOutputToZero(gLinearCamera.ClkPwmChannel);
}


static void LinearCamera_ResetFlags(void)
{
    gLinearCamera.BurstDone = FALSE;
    gLinearCamera.DmaDone = FALSE;
    gLinearCamera.SiPendingLow = FALSE;
}

static void LinearCamera_ArmNextExposure(void)
{
    uint8 writeIdx = gLinearCamera.WriteIndex;

    gLinearCameraDbg.ArmNextExposureCount++;

    LinearCamera_ResetFlags();

    gLinearCamera.Status = LINEAR_CAMERA_STATUS_ARMED;
    gLinearCamera.Phase  = LINEAR_CAMERA_PHASE_WAIT_START;

    Dio_WriteChannel(gLinearCamera.SiDioChannel, (Dio_LevelType)STD_LOW);

    (void)Adc_SetupResultBuffer(
        gLinearCamera.AdcGroup,
        (Adc_ValueGroupType *)(&gLinearCamera.Buffers[writeIdx].Values[0]));

    Gpt_StopTimer(gLinearCamera.ExposureGptChannel);
    Gpt_EnableNotification(gLinearCamera.ExposureGptChannel);
    Gpt_StartTimer(gLinearCamera.ExposureGptChannel, gLinearCamera.ExposureTicks);

    LinearCamera_DebugSnapshotState();
}

static void LinearCamera_TryPublishFrame(void)
{
    uint8 completedIdx;
    uint8 nextWriteIdx;

    if ((gLinearCamera.Status != LINEAR_CAMERA_STATUS_CAPTURING) ||
        (gLinearCamera.BurstDone != TRUE) ||
        (gLinearCamera.DmaDone != TRUE))
    {
        return;
    }

    /* End this line readout cleanly */
    LinearCamera_StopAllHw();

    completedIdx = gLinearCamera.WriteIndex;
    nextWriteIdx = (completedIdx == LINEAR_CAMERA_BUFFER_A) ?
                   LINEAR_CAMERA_BUFFER_B : LINEAR_CAMERA_BUFFER_A;

    /* Publish the just-completed frame as the latest frame. */
    gLinearCamera.ReadyIndex = completedIdx;
    gLinearCamera.ReadyValid = TRUE;

    /* Switch DMA target for the next line. */
    gLinearCamera.WriteIndex = nextWriteIdx;

    gLinearCameraDbg.PublishedFrameCount++;

    if (gLinearCamera.StreamEnabled == TRUE)
    {
        /* Re-arm the one-shot exposure timer immediately here.
         * This keeps SI cadence driver-owned instead of main-loop-owned.
         */
        LinearCamera_ArmNextExposure();
    }
    else
    {
        gLinearCamera.Status = LINEAR_CAMERA_STATUS_IDLE;
        gLinearCamera.Phase  = LINEAR_CAMERA_PHASE_IDLE;
    }

    LinearCamera_DebugSnapshotState();
}

void LinearCamera_Init(Pwm_ChannelType clkPwmChannel,
                       Gpt_ChannelType exposureGptChannel,
                       Gpt_ChannelType burstStopGptChannel,
                       Adc_GroupType adcGroup,
                       Dio_ChannelType siDioChannel)
{
    gLinearCamera.ClkPwmChannel       = clkPwmChannel;
    gLinearCamera.ExposureGptChannel  = exposureGptChannel;
    gLinearCamera.BurstStopGptChannel = burstStopGptChannel;
    gLinearCamera.AdcGroup            = adcGroup;
    gLinearCamera.SiDioChannel        = siDioChannel;

    gLinearCamera.Status        = LINEAR_CAMERA_STATUS_IDLE;
    gLinearCamera.Phase         = LINEAR_CAMERA_PHASE_IDLE;
    gLinearCamera.StreamEnabled = FALSE;
    gLinearCamera.ReadyValid    = FALSE;
    gLinearCamera.WriteIndex    = LINEAR_CAMERA_BUFFER_A;
    gLinearCamera.ReadyIndex    = LINEAR_CAMERA_INVALID_INDEX;
    gLinearCamera.ExposureTicks = LinearCamera_ExposureUsToTimerTicks(1000U);

    LinearCamera_ResetFlags();
    LinearCamera_DebugResetCounters();

    Dio_WriteChannel(gLinearCamera.SiDioChannel, (Dio_LevelType)STD_LOW);

    Adc_DisableHardwareTrigger(gLinearCamera.AdcGroup);
    Adc_EnableGroupNotification(gLinearCamera.AdcGroup);

    Pwm_SetDutyCycle(gLinearCamera.ClkPwmChannel, 0U);
    Pwm_DisableNotification(gLinearCamera.ClkPwmChannel);

    Gpt_StopTimer(gLinearCamera.ExposureGptChannel);
    Gpt_DisableNotification(gLinearCamera.ExposureGptChannel);

    Gpt_StopTimer(gLinearCamera.BurstStopGptChannel);
    Gpt_DisableNotification(gLinearCamera.BurstStopGptChannel);

    LinearCamera_DebugSnapshotState();
}

boolean LinearCamera_StartStreaming(uint32 exposureUs)
{
    if (gLinearCamera.StreamEnabled == TRUE)
    {
        return FALSE;
    }

    gLinearCamera.ExposureTicks = LinearCamera_ExposureUsToTimerTicks(exposureUs);
    gLinearCamera.StreamEnabled = TRUE;
    gLinearCamera.ReadyValid    = FALSE;
    gLinearCamera.ReadyIndex    = LINEAR_CAMERA_INVALID_INDEX;
    gLinearCamera.WriteIndex    = LINEAR_CAMERA_BUFFER_A;

    LinearCamera_ArmNextExposure();
    LinearCamera_DebugSnapshotState();
    return TRUE;
}

void LinearCamera_StopStreaming(void)
{
    gLinearCamera.StreamEnabled = FALSE;
    LinearCamera_StopAllHw();
    gLinearCamera.Status = LINEAR_CAMERA_STATUS_IDLE;
    gLinearCamera.Phase  = LINEAR_CAMERA_PHASE_IDLE;
    gLinearCamera.ReadyValid = FALSE;
    gLinearCamera.ReadyIndex = LINEAR_CAMERA_INVALID_INDEX;
    LinearCamera_ResetFlags();
    LinearCamera_DebugSnapshotState();
}

void LinearCamera_SetExposureUs(uint32 exposureUs)
{
    gLinearCamera.ExposureTicks = LinearCamera_ExposureUsToTimerTicks(exposureUs);
}

LinearCameraStatus LinearCamera_GetStatus(void)
{
    return gLinearCamera.Status;
}

boolean LinearCamera_IsBusy(void)
{
    return (gLinearCamera.Status == LINEAR_CAMERA_STATUS_ARMED) ||
           (gLinearCamera.Status == LINEAR_CAMERA_STATUS_CAPTURING);
}

boolean LinearCamera_IsFrameReady(void)
{
    return gLinearCamera.ReadyValid;
}

boolean LinearCamera_CopyLatestFrame(LinearCameraFrame *dst)
{
    uint8 readyIdx;

    if (dst == (LinearCameraFrame *)0)
    {
        return FALSE;
    }

    if (gLinearCamera.ReadyValid != TRUE)
    {
        return FALSE;
    }

    /* Critical section reason:
     * - prevent ISR from changing ReadyIndex/ReadyValid while we snapshot
     * - active DMA is always writing the other buffer, not ReadyIndex
     */
    LINEAR_CAMERA_ENTER_CRITICAL();

    if (gLinearCamera.ReadyValid != TRUE)
    {
        LINEAR_CAMERA_EXIT_CRITICAL();
        return FALSE;
    }

    readyIdx = gLinearCamera.ReadyIndex;
    (void)memcpy((void *)dst,
                 (const void *)&gLinearCamera.Buffers[readyIdx],
                 sizeof(LinearCameraFrame));

    gLinearCamera.ReadyValid = FALSE;
    gLinearCameraDbg.CopyLatestFrameCount++;

    LINEAR_CAMERA_EXIT_CRITICAL();

    LinearCamera_DebugSnapshotState();
    return TRUE;
}

void LinearCamera_Abort(void)
{
    gLinearCameraDbg.AbortCount++;

    gLinearCamera.StreamEnabled = FALSE;
    LinearCamera_StopAllHw();
    gLinearCamera.Status     = LINEAR_CAMERA_STATUS_IDLE;
    gLinearCamera.Phase      = LINEAR_CAMERA_PHASE_IDLE;
    gLinearCamera.ReadyValid = FALSE;
    gLinearCamera.ReadyIndex = LINEAR_CAMERA_INVALID_INDEX;
    gLinearCamera.WriteIndex = LINEAR_CAMERA_BUFFER_A;
    LinearCamera_ResetFlags();

    LinearCamera_DebugSnapshotState();
}

/*
 * Exposure timer callback:
 * starts a new line acquisition.
 */
void NewCameraFrame(void)
{
    if ((gLinearCamera.Status != LINEAR_CAMERA_STATUS_ARMED) ||
        (gLinearCamera.Phase != LINEAR_CAMERA_PHASE_WAIT_START))
    {
        return;
    }

    gLinearCameraDbg.ExposureIsrCount++;

    gLinearCamera.Status = LINEAR_CAMERA_STATUS_CAPTURING;
    gLinearCamera.Phase  = LINEAR_CAMERA_PHASE_SHIFTING;
    LinearCamera_ResetFlags();
    gLinearCamera.SiPendingLow = TRUE;

    LinearCamera_DebugSnapshotState();

    /* SI high before first clock */
    Dio_WriteChannel(gLinearCamera.SiDioChannel, (Dio_LevelType)STD_HIGH);
    LinearCamera_DelayLoops(LINEAR_CAMERA_SI_SETUP_DELAY_LOOPS);

    /* One callback only: drop SI after the first clock edge */
    Pwm_EnableNotification(gLinearCamera.ClkPwmChannel, PWM_FALLING_EDGE);

    /* Enable ADC HW trigger path before the burst starts */
    Adc_EnableHardwareTrigger(gLinearCamera.AdcGroup);

    /* Start 500 kHz clock burst */
    Pwm_SetDutyCycle(gLinearCamera.ClkPwmChannel, 0x4000U);

    /* Stop the burst after exactly 129 clocks */
    Gpt_EnableNotification(gLinearCamera.BurstStopGptChannel);
    Gpt_StartTimer(gLinearCamera.BurstStopGptChannel, LINEAR_CAMERA_BURST_STOP_TICKS);
}

/*
 * PWM edge callback:
 * end the SI pulse after the first clock edge.
 */
void CameraClock(void)
{
    if ((gLinearCamera.Status != LINEAR_CAMERA_STATUS_CAPTURING) ||
        (gLinearCamera.Phase != LINEAR_CAMERA_PHASE_SHIFTING))
    {
        return;
    }

    gLinearCameraDbg.ClockEdgeCbCount++;

    if (gLinearCamera.SiPendingLow == TRUE)
    {
        Dio_WriteChannel(gLinearCamera.SiDioChannel, (Dio_LevelType)STD_LOW);
        gLinearCamera.SiPendingLow = FALSE;
        Pwm_DisableNotification(gLinearCamera.ClkPwmChannel);
    }

    LinearCamera_DebugSnapshotState();
}

/*
 * Burst-stop timer callback:
 * end the 129-clock burst.
 *
 * FIX: accept both SHIFTING and WAIT_COMPLETE so DMA-first ordering cannot block stop.
 */
void CameraBurstStop(void)
{
    if (gLinearCamera.Status != LINEAR_CAMERA_STATUS_CAPTURING)
    {
        return;
    }

    if ((gLinearCamera.Phase != LINEAR_CAMERA_PHASE_SHIFTING) &&
        (gLinearCamera.Phase != LINEAR_CAMERA_PHASE_WAIT_COMPLETE))
    {
        return;
    }

    gLinearCameraDbg.BurstStopCbCount++;

    LinearCamera_StopClockOnly();
    gLinearCamera.BurstDone = TRUE;
    gLinearCamera.Phase = LINEAR_CAMERA_PHASE_WAIT_COMPLETE;

    LinearCamera_DebugSnapshotState();
    LinearCamera_TryPublishFrame();
}

/*
 * ADC group notification in DMA mode:
 * expected after DMA major-loop completion.
 *
 * FIX: DMA completion no longer owns the phase transition.
 */
void CameraDmaFinished(void)
{
    if (gLinearCamera.Status != LINEAR_CAMERA_STATUS_CAPTURING)
    {
        return;
    }

    gLinearCameraDbg.DmaDoneCbCount++;

    gLinearCamera.DmaDone = TRUE;

    /* Stop accepting additional ADC triggers once line buffer is full. */
    Adc_DisableHardwareTrigger(gLinearCamera.AdcGroup);

    LinearCamera_DebugSnapshotState();
    LinearCamera_TryPublishFrame();
}

#ifdef __cplusplus
}
#endif
