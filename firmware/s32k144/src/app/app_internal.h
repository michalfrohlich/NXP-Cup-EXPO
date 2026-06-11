#ifndef APP_INTERNAL_H
#define APP_INTERNAL_H

#include "app_modes.h"
#include "domain/vehicle_types.h"
#include "app_config.h"
#include "config/actuator_config.h"
#include "config/board_config.h"
#include "board_init.h"
#include <stdio.h>
#include <string.h>

#include "drivers/timebase.h"
#include "drivers/buttons.h"
#include "drivers/onboard_pot.h"
#include "drivers/receiver.h"
#include "drivers/servo.h"
#include "drivers/esc.h"
#include "drivers/linear_camera.h"
#include "drivers/display.h"
#include "drivers/ultrasonic.h"
#include "drivers/teensy_link.h"

#include "services/line_detector.h"
#include "drivers/rgb_led.h"
#include "services/steering_controller.h"
#include "services/steering_smoothing.h"
#include "vision_debug.h"
#include "debug/uart_host_link.h"

typedef enum
{
    APP_BUILD_MODE_LINEAR_CAMERA_TEST = 0,
    APP_BUILD_MODE_NXP_CUP,
    APP_BUILD_MODE_RACE_MODE,
    APP_BUILD_MODE_HONOR_LAP,
    APP_BUILD_MODE_SERVO_RATE_TEST,
    APP_BUILD_MODE_TEENSY_LINK_TEST,
    APP_BUILD_MODE_NXP_CUP_TESTS
} AppBuildMode_t;

AppBuildMode_t App_GetSelectedBuildMode(void);

typedef enum
{
    RUNTIME_TEST_LINEAR_CAMERA = 0,
    RUNTIME_TEST_ESC,
    RUNTIME_TEST_SERVO,
    RUNTIME_TEST_ULTRASONIC,
    RUNTIME_TEST_CAMSERVO,
    RUNTIME_TEST_SIMPLE_DRIVE,
    RUNTIME_TEST_SERIAL_TUNE,
    RUNTIME_TEST_ULTRA_ESC,
    RUNTIME_TEST_RECEIVER,
    RUNTIME_TEST_TEENSY_LINK,
    RUNTIME_TEST_VICTORY_LAP,
    RUNTIME_TEST_COUNT
} RuntimeTestId_t;

typedef enum
{
    SW2_ACTION_NONE = 0,
    SW2_ACTION_CLICK,
    SW2_ACTION_HOLD
} Sw2Action_t;

typedef struct
{
    boolean wasPressed;
    boolean holdHandled;
    uint32 pressedSinceMs;
} Sw2Tracker_t;

typedef struct
{
    uint32 nextRefreshMs;
} ReceiverTestState_t;

typedef enum
{
    ULTRA_TEST_MODE_CLEAR = 0,
    ULTRA_TEST_MODE_STOP_HOLD,
    ULTRA_TEST_MODE_CRAWL,
    ULTRA_TEST_MODE_CUTOFF
} UltrasonicTestMode_t;

typedef struct
{
    uint32 nextTriggerMs;
    uint32 nextDisplayMs;
    uint32 ultraEnableMs;
    uint32 modeUntilMs;
    uint32 slowRangeSinceMs;
    uint32 stopRangeSinceMs;
    float lastDistanceCm;
    boolean haveValidDistance;
    UltrasonicTestMode_t mode;
} UltrasonicTestState_t;

typedef struct
{
    uint32 armDoneMs;
    uint32 nextUltraTrigMs;
    uint32 nextDisplayMs;
    uint32 lastDistanceMs;
    float lastDistanceCm;
    sint32 commandedSpeedPct;
    boolean hasValidDistance;
    UltrasonicTestMode_t mode;
} UltrasonicEscTestState_t;

typedef enum
{
    VICTORY_LAP_PHASE_ARM = 0,
    VICTORY_LAP_PHASE_APPROACH,
    VICTORY_LAP_PHASE_NOTE,
    VICTORY_LAP_PHASE_GAP,
    VICTORY_LAP_PHASE_DONE
} VictoryLapPhase_t;

typedef struct
{
    uint16 hz;
    uint16 durationMs;
    uint16 gapMs;
} VictoryLapNote_t;

typedef struct
{
    VictoryLapPhase_t phase;
    uint32 armDoneMs;
    uint32 nextUltraTrigMs;
    uint32 nextDisplayMs;
    uint32 noteUntilMs;
    uint32 lastDistanceMs;
    float lastDistanceCm;
    sint32 commandedSpeedPct;
    uint8 noteIndex;
    uint16 currentNoteHz;
    boolean hasValidDistance;
} VictoryLapTestState_t;

typedef struct
{
    boolean configured;
    boolean useSmoothing;
    uint32 nextDisplayMs;
    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;
} ServoTestState_t;

typedef enum
{
    SERVO_RATE_TEST_FREQ_10HZ = 0,
    SERVO_RATE_TEST_FREQ_50HZ,
    SERVO_RATE_TEST_FREQ_100HZ,
    SERVO_RATE_TEST_FREQ_250HZ,
    SERVO_RATE_TEST_FREQ_COUNT
} ServoRateTestFreqId_t;

typedef enum
{
    SERVO_RATE_TEST_MOTION_POT = 0,
    SERVO_RATE_TEST_MOTION_FINE_SWEEP,
    SERVO_RATE_TEST_MOTION_COARSE_SWEEP,
    SERVO_RATE_TEST_MOTION_COUNT
} ServoRateTestMotionId_t;

typedef struct
{
    ServoRateTestFreqId_t freqId;
    ServoRateTestMotionId_t motionId;
    uint32 nextButtonsMs;
    uint32 nextCommandMs;
    uint32 nextDisplayMs;
    sint16 command;
    sint16 sweepDirection;
    uint8 lastPotLevel;
    uint32 startMs;
    ServoDebugSnapshot servoDbg;
    boolean noPwmCallbackFault;
    boolean bufferedLagFault;
} ServoRateTestState_t;

typedef struct
{
    VisionDebug_State_t vdbg;
    SteeringControllerState_t ctrl;
    Sw2Tracker_t sw2Tracker;
    LinearCameraFrame processedFrame;
    VisionOutput_t result;
    LineDetector_DebugOut_t dbg;
    uint16 filteredBuf[VISION_LINEAR_BUFFER_SIZE];
    sint16 gradientBuf[VISION_LINEAR_BUFFER_SIZE];
    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;
    uint32 nextUiMs;
    uint32 nextSteerMs;
    uint32 nextDisplayMs;
    uint32 nextStreamMs;
    uint16 streamSeq;
    uint16 streamDropCount;
    boolean haveValidVision;
    boolean displayDirty;
    boolean paused;
    boolean servoEnabled;
    uint8 steeringBaseSpeed;
} LinearCameraTestState_t;

typedef enum
{
    ESC_TEST_STATE_DISARMED = 0,
    ESC_TEST_STATE_ARMING,
    ESC_TEST_STATE_ARMED_NEUTRAL,
    ESC_TEST_STATE_RUNNING,
    ESC_TEST_STATE_ESTOP
} EscTestStateId_t;

typedef enum
{
    ESC_TEST_MODE_BOTH = 0,
    ESC_TEST_MODE_ESC1,
    ESC_TEST_MODE_ESC2,
    ESC_TEST_MODE_DIFF,
    ESC_TEST_MODE_COUNT
} EscTestModeId_t;

typedef struct
{
    EscTestStateId_t state;
    EscTestModeId_t mode;
    Sw2Tracker_t sw2Tracker;
    Sw2Tracker_t sw3Tracker;
    uint32 armDoneMs;
    uint32 nextDisplayMs;
    sint16 commandPct;
    sint16 primaryCmdPct;
    sint16 secondaryCmdPct;
    uint8 lastPotLevel;
    boolean reverse;
    boolean requirePotZero;
} EscManualTestState_t;

typedef struct
{
    float kp;
    float kd;
    float ki;
    float iTermClamp;
    float steerLpfAlpha;
    sint16 steerClamp;
    sint16 steerRateMax;
    uint8 baseSpeedPct;
} CamTuneProfile_t;

typedef enum
{
    NXP_CUP_PROFILE_SUPERFAST = 0,
    NXP_CUP_PROFILE_5050,
    NXP_CUP_PROFILE_SLOW,
    NXP_CUP_PROFILE_COUNT
} NxpCupProfileId_t;

typedef struct
{
    VisionDebug_State_t vdbg;
    SteeringControllerState_t ctrl;
    LinearCameraFrame processedFrame;
    VisionOutput_t result;
    LineDetector_DebugOut_t dbg;
    uint16 filteredBuf[VISION_LINEAR_BUFFER_SIZE];
    sint16 gradientBuf[VISION_LINEAR_BUFFER_SIZE];
    boolean haveValidVision;
    uint32 lastFrameMs;
    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;
    uint32 nextControlMs;
    uint32 nextSteerMs;
    uint32 nextDisplayMs;
    boolean displayDirty;
    CamTuneProfile_t activeTune;
} CamServoState_t;

typedef enum
{
    NXP_CUP_ULTRA_CLEAR = 0,
    NXP_CUP_ULTRA_STOP_HOLD,
    NXP_CUP_ULTRA_CRAWL,
    NXP_CUP_ULTRA_CUTOFF
} NxpCupUltraMode_t;

typedef struct
{
    uint32 nextTriggerMs;
    uint32 ultraEnableMs;
    uint32 modeUntilMs;
    uint32 lastDistanceMs;
    float lastDistanceCm;
    boolean haveValidDistance;
    NxpCupUltraMode_t mode;
} NxpCupUltraState_t;

typedef enum
{
    NXP_CUP_STATE_MENU = 0,
    NXP_CUP_STATE_READY,
    NXP_CUP_STATE_ESC_REARM,
    NXP_CUP_STATE_RUN
} NxpCupModeState_t;

typedef struct
{
    NxpCupModeState_t state;
    NxpCupProfileId_t profileId;
    NxpCupUltraState_t ultraSt;
    CamServoState_t camSt;
    sint32 autoSpeedPct;
    uint32 nextAutoSpeedMs;
    uint32 nextDisplayMs;
    uint32 escRearmDoneMs;
    boolean cameraStarted;
} NxpCupState_t;

typedef struct
{
    uint32 armDoneMs;
    sint32 autoSpeedPct;
    uint32 nextAutoSpeedMs;
    boolean runEnabled;
    CamServoState_t camSt;
} SimpleDriveState_t;

typedef struct
{
    uint8 selectedIndex;
    uint8 topIndex;
    uint32 lastPotMs;
    boolean testActive;
    RuntimeTestId_t activeTest;
} TestsMenuState_t;

typedef struct
{
    uint32 nextUltraTrigMs;
    uint32 nextDisplayMs;
    uint32 lastDistanceMs;
    float lastDistanceCm;
    sint32 commandedSpeedPct;
    boolean hasValidDistance;
} HonorLapState_t;

typedef enum
{
    RACE_PHASE_ESC_ARM = 0,
    RACE_PHASE_SPEED_RUN,
    RACE_PHASE_HONOR_RUN,
    RACE_PHASE_STOPPED,
    RACE_PHASE_FAULT
} RacePhase_t;

typedef struct
{
    RacePhase_t phase;
    SteeringControllerState_t ctrl;
    LinearCameraFrame processedFrame;
    VisionOutput_t result;
    uint32 nextControlMs;
    uint32 nextSpeedRampMs;
    uint32 nextUltraTrigMs;
    uint32 nextDisplayMs;
    uint32 armDoneMs;
    uint32 lostSinceMs;
    uint32 lastDistanceMs;
    float lastDistanceCm;
    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;
    sint32 targetSpeedPct;
    sint32 currentSpeedPct;
    uint8 finishStreak;
    boolean haveValidVision;
    boolean haveValidDistance;
    boolean ultraError;
    boolean displayInitialized;
    boolean displayWasOn;
} RaceModeState_t;

typedef enum
{
    SERIAL_TUNE_SCREEN_WAIT = 0,
    SERIAL_TUNE_SCREEN_MENU,
    SERIAL_TUNE_SCREEN_SERVO_MENU,
    SERIAL_TUNE_SCREEN_EDIT_KP,
    SERIAL_TUNE_SCREEN_EDIT_KI,
    SERIAL_TUNE_SCREEN_EDIT_KD,
    SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP,
    SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF
} SerialTuneScreen_t;

typedef struct
{
    SerialTuneScreen_t screen;
    float kp;
    float ki;
    float kd;
    sint16 servoClamp;
    float servoLpfAlpha;
    char inputBuf[SERIAL_TUNE_INPUT_MAX_LEN + 1U];
    uint8 inputLen;
    boolean connected;
} SerialTuneState_t;

typedef struct
{
    CamTuneProfile_t profile;
    boolean initialized;
} RuntimeTuneState_t;

extern ReceiverTestState_t g_receiverTest;
extern UltrasonicTestState_t g_ultrasonicTest;
extern UltrasonicEscTestState_t g_ultrasonicEscTest;
extern VictoryLapTestState_t g_victoryLapTest;
extern ServoTestState_t g_servoTest;
extern ServoRateTestState_t g_servoRateTest;
extern EscManualTestState_t g_escTest;
extern LinearCameraTestState_t g_linearCameraTest;
extern SimpleDriveState_t g_simpleDriveTest;
extern TestsMenuState_t g_testsMenu;
extern HonorLapState_t g_honorLap;
extern NxpCupState_t g_nxpCup;
extern RaceModeState_t g_raceMode;
extern SerialTuneState_t g_serialTune;
extern RuntimeTuneState_t g_runtimeTune;

extern const CamTuneProfile_t g_nxpCupProfiles[NXP_CUP_PROFILE_COUNT];
extern const char g_testsMenuItems[RUNTIME_TEST_COUNT][17];
extern const uint16 g_servoRateTestFreqHz[SERVO_RATE_TEST_FREQ_COUNT];
extern const uint16 g_servoRateTestPeriodMs[SERVO_RATE_TEST_FREQ_COUNT];
extern const char g_servoRateTestMotionText[SERVO_RATE_TEST_MOTION_COUNT][12];

boolean time_reached(uint32 nowMs, uint32 dueMs);
uint16 VisionDebug_WhiteMaxFromPot(uint8 potLevel);
Sw2Action_t Sw2Tracker_Update(Sw2Tracker_t *st, uint32 nowMs);
Sw2Action_t ButtonTracker_Update(Sw2Tracker_t *st, ButtonId_t id, uint32 nowMs);

void StatusLed_Blue(void);
void StatusLed_Green(void);
void StatusLed_Cyan(void);
void StatusLed_Yellow(void);
void StatusLed_Red(void);
void StatusLed_Off(void);

void Esc_StopNeutral(void);
void Esc_SetLogicalSpeed(int primaryLogicalCmd, int secondaryLogicalCmd);
void busy_delay(volatile uint32 ticks);
void display_power_stabilize_delay(void);
void App_InitRuntimeCore(void);
void App_InitRuntimeCommon(void);
void DisplayTextPadded(uint16 displayLine, const char *text);
void RuntimeTune_InitDefaults(void);

void serial_tune_test_enter(uint32 nowMs);
void serial_tune_test_update(void);
void serial_tune_test_exit(void);

void receiver_test_enter(uint32 nowMs);
void receiver_test_update(uint32 nowMs);
void receiver_test_exit(void);
void teensy_link_test_enter(uint32 nowMs);
void teensy_link_test_update(uint32 nowMs, boolean sw2Pressed);
void teensy_link_test_exit(void);
void ultrasonic_test_enter(uint32 nowMs);
void ultrasonic_test_update(uint32 nowMs, boolean sw2Pressed);
void ultrasonic_test_exit(void);
void ultrasonic_esc_test_enter(uint32 nowMs);
void ultrasonic_esc_test_update(uint32 nowMs);
void ultrasonic_esc_test_exit(void);
void victory_lap_test_enter(uint32 nowMs);
void victory_lap_test_update(uint32 nowMs);
void victory_lap_test_exit(void);
void servo_test_enter(uint32 nowMs);
void servo_test_update(uint32 nowMs, boolean sw2Pressed, uint8 potLevel);
void servo_test_exit(void);
void esc_test_enter(uint32 nowMs);
void esc_test_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed, uint8 potLevel);
void esc_test_exit(void);
void linear_camera_test_enter_common(uint32 nowMs, boolean servoEnabled);
void linear_camera_test_enter(uint32 nowMs);
void linear_camera_test_update(uint32 nowMs, boolean modeNextPressed, uint8 potLevel);
void linear_camera_test_exit(void);
void camservo_enter_with_profile(CamServoState_t *st, uint32 nowMs, const CamTuneProfile_t *profile);
void camservo_enter(CamServoState_t *st, uint32 nowMs);
void camservo_update(CamServoState_t *st, uint32 nowMs, boolean sw2);
void camservo_exit(void);
void simple_drive_test_enter(uint32 nowMs);
void simple_drive_test_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed);
void simple_drive_test_exit(void);

void mode_linear_camera_test(void);
void mode_nxp_cup_tests(void);
void mode_nxp_cup(void);
void mode_honor_lap(void);
void mode_race_mode(void);
void mode_servo_rate_test(void);
void mode_teensy_link_test(void);

sint32 honor_speed_from_distance(boolean hasValidDistance, float distanceCm);

#endif /* APP_INTERNAL_H */
