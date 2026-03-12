
/*EXPO_03_Nxp_Cup_project - main file */
#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
 *                                        INCLUDE FILES -TODO: need to cleanup and sort
==================================================================================================*/
#include "display.h"
#include "receiver.h"
#include "CDD_I2c.h"
#include "esc.h"
#include "servo.h"
#include "linear_camera.h"
#include "camera_emulator.h" //for testing
#include "main_functions.h"
#include "hbridge.h"
#include "Mcal.h"
#include "pixy2.h"
#include <stdio.h>
#include <string.h>
#include "onboard_pot.h"
#include "buttons.h"
#include "ultrasonic.h"
#include "timebase.h"
#include "services/vision_linear_v2.h" //most complex version of the algorithm
#include "app/vision_debug.h"
#include "rgb_led.h"

#include "S32K144.h"
/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/
#define TESTS_ENABLE     1 /* All macros are here - used for testing */
	/* Test mode flags – enable only one at a time (disable if TESTS_ENABLE = 0 */
	#define DISPLAY_TEST      0   /* Camera signal simulation + display + buttons + potentiometer demo */
	#define ULTRASONIC_500MS_SAMPLE    0 //working example of using ultrasonic

	#define RAW_CAMERA_TEST  0   /* Raw linear camera capture + OLED bar graph */
	#define SIMPLIFIED_VISION_TEST 0 /* Gemini simplified vision module test */
	#define EXPOSURE_SCAN_TEST 0 /* Vision linear with adjustable exposure test*/
	#define VISION_V2_SPLIT_TEST 0 /* Vision linear v2 - complex algorithm module*/
	#define VISION_V2_DEBUG 0 //full vision debug module used
	#define VISION_REFACTOR_DEBUG1 1 //after changing to raw 12-bit values and non-blocking driver

#define CAR_MAIN         0   /* Line-following car main code that will run the whole car*/

/*==================================================================================================
 *                 SYSTEM INITIALIZATION - shared initialization for both test and car mains
==================================================================================================*/
static void System_Init(void)
{
	/*Initialize RTD drivers with the compiled configurations*/
	DriversInit();

	/*Initialize driver for timers used for timings in the main loop*/
	Timebase_Init();

	/*Initialize driver for the onboard potentiometer used for user input*/
	OnboardPot_Init();

	/* Initialize ultrasonic driver (TRIG low, ICU notification, internal state) */
	Ultrasonic_Init();     /* uses Dio + Icu, so it must come after DriversInit */

	/* Buttons driver:
	     *  - Uses Dio pins configured in the RTD (PTC12, PTC13).
	     *  - Has no explicit init function; its state is zero-initialized at startup.
	     *  - You MUST call Buttons_Update() periodically in the main loop
	     *    before using Buttons_IsPressed() / Buttons_WasPressed().
	     */

	/*Initialize Esc driver*/
	/*First parameter: The Pwm Channel that was configured in Peripherals tool for the Esc*/
	/*Next parameters: Amount of Pwm ticks needed to achieve 1ms, 1.5ms and 2ms long signals, standard for controlling an esc.*/
	EscInit(0U, 1638U, 2457U, 3276U);

	/*Initialize Servo driver*/
	/*First parameter: The Pwm Channel that was configured in Peripherals tool for the Servo*/
	/*Next parameters: Amount of Pwm ticks needed to achieve maximum desired left turn, right turn and middle position for your servo.*/
	ServoInit(1U, 3300U, 1700U, 2500U);

	/*Initialize Hbridge driver*/
	/*First two parameters: The Pwm Channels that were configured in Peripherals tool for the motors' speeds*/
	/*Next parameters: The Pcr of the pins used for the motors' directions*/
	//HbridgeInit(2U, 3U, 32U, 33U, 6U, 64U);

	/*Initialize display driver*/
	/*The display driver is for a 0.96", 128x32 OLED monochrome using a SSD1306 chip, like the one in the MR-CANHUBK344 kit*/
	/*Parameter: Should the driver rotate the fontmap before printing. You should leave this to 1U unless you have a special use case*/
	DisplayInit(0U, STD_ON);

	/*Initialize linear camera driver*/
	/*First parameter: The Pwm channel that was configured in Peripherals tool for the clock sent to the camera*/
	/*Second parameter: The Gpt channel that was configured in Peripherals tool for determining the length of the shutter signal*/
	/*Third parameter: The Adc group that was configured in Peripherals tool for sampling the analog signal output of the camera*/
	/*Fourth parameter: The Pcr of the pin used for sending the shutter signal to the camera*/
	LinearCameraInit(4U, 1U, 0U, 97U);

	/*Initialize Pixy2 camera driver (optional)*/
	/*Pixy2Init(0x54, 0U);*/
	/*Pixy2Test();*/

	RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });  // red on
	for (volatile uint32 i=0; i<700000u; i++) { }                      // crude delay
	RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=true, .b=false });  // green on
	for (volatile uint32 i=0; i<600000u; i++) { }
	RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=true });  // blue on
	for (volatile uint32 i=0; i<500000u; i++) { }
	RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=false }); //LED off
}

/*==================================================================================================
 *                                 MAIN CODE FOR THE CAR
==================================================================================================*/

#if CAR_MAIN && !TESTS_ENABLE

/* ----- Top-level application states ----- */
typedef enum
{
    APP_STATE_INIT = 0,   /* One-time init + possible calibration */
    APP_STATE_IDLE,       /* Car stopped, tuning / menu */
    APP_STATE_RUN         /* Car driving on the track */
} AppState_t;

/* Global application state */
static AppState_t gAppState = APP_STATE_INIT;

/* Latched RUN/STOP request from SW3 (edge-triggered) */
static boolean gRunStopRequest = FALSE;

/* ---------------- Vision / Camera runtime ---------------- */
static LinearCameraFrame         gCamFrame;
static VisionLinear_ResultType   gVision;

static uint32 gFrameCount = 0u;

/* Tunables for MVP scheduling (based on blocking camera capture) */
#define CAM_EVERY_N_LOOPS     2u   /* 2 * 5ms = 10ms capture period */
#define DISP_EVERY_N_LOOPS    2u   /* 2 * 5ms = 10ms display text update (can slow later) */

/*------------------------------------------------------------------------------------------------
 *                                   MAIN
-------------------------------------------------------------------------------------------------*/

int main(void)
{
    uint32 lastLoopMs  = 0u;  /* Time of last 5 ms loop execution */
    uint32 loopCounter = 0u;  /* Counts 5 ms ticks; used for "every Nth loop" scheduling */

    /* One-time system / drivers init */
    System_Init();

    /* Base timestamp for 5 ms scheduling */
    lastLoopMs = Timebase_GetMs();

    /* ---------------- MAIN CYCLIC LOOP ----------------
     *  - Executes the state machine every 5 ms.
     *  - Inside that 5 ms slot we can further schedule slower tasks
     *    (10 ms, 20 ms, 50 ms...) using loopCounter.
     */
    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();

        /* Wait until at least 5 ms elapsed since last loop. */
        if ((uint32)(nowMs - lastLoopMs) < 5u)
        {
            continue; //Not time yet → skip this iteration
        }

        /* 5 ms elapsed → start a new logical loop tick */
        lastLoopMs = nowMs;
        loopCounter++;

        /* --------- Common periodic input sampling (every 5 ms) --------- */

        /* Debounce all buttons and update edge detection.
         * Must be called once per logical loop.
         */
        Buttons_Update();

        /* Detect RUN/STOP button (SW3) edge once per loop.
         *   - This latches a request handled by the state machine.
         *   - Using a latched flag avoids missing presses when
         *     state logic takes more than one loop to react.
         */
        if (Buttons_WasPressed(BUTTON_ID_SW3) == TRUE)
        {
            gRunStopRequest = TRUE;
        }

/* =============================================================
*  TOP-LEVEL STATE MACHINE: INIT → IDLE ↔ RUN
* ============================================================= */
        switch (gAppState)
        {
            case APP_STATE_INIT:
                /* INIT state (first few 5 ms ticks after boot)
                 * --------------------------------------------
                 * Intended later:
                 *   - One-time calibrations (e.g. baseline for
                 *     linear camera, ESC arming sequence, etc.).
                 *   - Display splash / "Initializing" message.
                 *   - Any checks that need the timebase running.
                 *
                 * MVP now:
                 *   - Directly jump to IDLE on the first loop tick.
                 */

            	/* INIT screen */
				DisplayClear();
				DisplayText(0U, "INIT", 4U, 0U);
				DisplayText(0U, "Initializing...", 14U, 2U);
				DisplayRefresh();

                gAppState = APP_STATE_IDLE;
                break;

            case APP_STATE_IDLE:
                /* IDLE state (car is fully stopped)
                 * ---------------------------------
                 * Intended later:
                 *   - Force safe outputs every loop (throttle = 0,
                 *     steering = center).
                 *   - Run simple menu / tuning UI on the display.
                 *   - Read onboard potentiometer for base speed or
                 *     Kp adjustment.
                 *
                 * Example of multi-rate tasks in IDLE:
                 *   - Every 20 ms (4 × 5 ms) update display text:
                 *       if ((loopCounter % 4u) == 0u) { update IDLE UI }
                 *   - Every 100 ms (20 × 5 ms) blink an LED:
                 *       if ((loopCounter % 20u) == 0u) { toggle LED }
                 */

            	/* IDLE screen */
            	            DisplayClear();
            	            DisplayText(0U, "IDLE", 4U, 0U);
            	            DisplayText(0U, "SW3 = RUN/STOP", 14U, 2U);
            	            DisplayRefresh();

                /* MVP transition: start driving when SW3 pressed */
                if (gRunStopRequest == TRUE)
                {
                    gRunStopRequest = FALSE;

                    /* TODO (later): reset controllers, filters, and any
                     * RUN sub-state in the steering module before
                     * entering APP_STATE_RUN.
                     */

                    gAppState = APP_STATE_RUN;
                }
                break;



            case APP_STATE_RUN:
                /* RUN state (car driving on the track)
                 * ------------------------------------
                 * The detailed behaviour is inside the steering/vision
                 * modules; main.c only orchestrates timing.
                 *
                 * Typical scheduling inside RUN:
                 *   - Every 5 ms (every loop):
                 *       * Use the latest vision result to compute
                 *         steering & throttle (fast control loop).
                 *   - Every 10 ms (2 × 5 ms):
                 *       if ((loopCounter % 2u) == 0u)
                 *       {
                 *           // Update debug text on display
                 *       }
                 *   - Every 50 ms (10 × 5 ms):
                 *       if ((loopCounter % 10u) == 0u)
                 *       {
                 *           // Trigger new ultrasonic measurement
                 *       }
                 *   - Every 100 ms (20 × 5 ms):
                 *       if ((loopCounter % 20u) == 0u)
                 *       {
                 *           // Process slower house-keeping tasks
                 *       }
                 *
                 * All of that logic will live in the dedicated
                 * service modules; here we only provide comments
                 * and the timing hooks.
                 */

                /* TODO (later):
                 *   - Call vision + steering controller modules here.
                 *   - Apply ESC and servo outputs.
                 *   - Integrate ultrasonic-based braking.
                 *   - Log / show key values on OLED.
                 */

            	/* ===== Timing hooks + MVP camera/vision/debug ===== */

				/* Every 5 ms (every loop):
				 * - Fast control loop should use the latest gVision result.
				 *   (steering module will own the detailed behaviour later)
				 */
				/* TODO (later): SteeringController_Update5ms(&gVision, ...); */


                /* RUN → IDLE: Stop driving when SW3 is pressed again. --------------------SAFETY STOP--------------- */
                if (gRunStopRequest == TRUE)
                {
                    gRunStopRequest = FALSE;

                    /* TODO (later): smooth ramp-down of throttle and
                     * optional \"RUN_BRAKE\" behaviour inside steering
                     * module before fully stopping.
                     */

                    gAppState = APP_STATE_IDLE;
                }
                break;

            default:
                /* Should not happen; fall back to a safe state. */
                gAppState = APP_STATE_IDLE;
                break;
        }
    }
}
#endif /* CAR_MAIN && !TESTS_ENABLE */


/*==================================================================================================
 *                                       TESTS - must be enabled by the macros above
==================================================================================================*/
#if TESTS_ENABLE

#if VISION_REFACTOR_DEBUG1

int main(void)
{
    System_Init();
    VisionLinear_InitV2();

    VisionDebug_State_t vdbg;
    VisionDebug_Init(&vdbg, 3200U);

    static LinearCameraFrame frame;
    static LinearCameraFrame processedFrame;
    VisionLinear_ResultType result;

    VisionLinear_DebugOut_t dbg;
    static uint16 smoothBuf[VISION_LINEAR_BUFFER_SIZE];

    const uint32 TEST_SHUTTER_FREQUENCY = 100U;

    const uint32 LOOP_PERIOD_MS    = 5U;
    const uint32 DISPLAY_PERIOD_MS = 20U;
    const uint32 DISPLAY_TICKS     = (DISPLAY_PERIOD_MS / LOOP_PERIOD_MS);

    uint32 nextTickMs = Timebase_GetMs();
    uint32 tickCount  = 0U;

    /* NEW: track if we ever processed a frame */
    boolean haveValidVision = FALSE;

    LinearCameraSetShutterFrequencyTicks(TEST_SHUTTER_FREQUENCY);
    (void)LinearCameraStartStream(&frame);

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();

        if ((uint32)(nowMs - nextTickMs) < LOOP_PERIOD_MS)
        {
            continue;
        }

        nextTickMs += LOOP_PERIOD_MS;
        tickCount++;

        /* --- 5ms tasks start --- */

        Buttons_Update();

        {
            boolean screenTogglePressed = Buttons_WasPressed(BUTTON_ID_SW2);
            boolean modeNextPressed     = Buttons_WasPressed(BUTTON_ID_SW3);

            VisionDebug_OnTick(&vdbg, screenTogglePressed, modeNextPressed);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });

        /* ---- PROCESS FRAME WHEN READY ---- */

        {
            const LinearCameraFrame *latestFrame = (const LinearCameraFrame*)0;

            if (LinearCameraGetLatestFrame(&latestFrame) == TRUE)
            {
                dbg.mask      = (uint32)VLIN_DBG_NONE;
                dbg.smoothOut = (uint16*)0;

                if (VisionDebug_WantsVisionDebugData(&vdbg) == TRUE)
                {
                    VisionDebug_PrepareVisionDbg(&vdbg, &dbg, smoothBuf);
                }

                RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=true });

                (void)memcpy(processedFrame.Values, latestFrame->Values, sizeof(processedFrame.Values));
                VisionLinear_ProcessFrameEx(processedFrame.Values, &result, &dbg);

                haveValidVision = TRUE;
            }
        }

        /* ---- DISPLAY EVERY 20ms USING LAST RESULT ---- */

        if ((DISPLAY_TICKS != 0U) &&
            ((tickCount % DISPLAY_TICKS) == 0U) &&
            (haveValidVision == TRUE))
        {
            const uint16 *pSmooth =
                (dbg.smoothOut != (uint16*)0) ? smoothBuf : (const uint16*)0;

            const VisionLinear_DebugOut_t *pDbg =
                (dbg.mask != (uint32)VLIN_DBG_NONE) ? &dbg : (const VisionLinear_DebugOut_t*)0;

            VisionDebug_Draw(&vdbg, processedFrame.Values, pSmooth, &result, pDbg);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=false });

        /* --- 5ms tasks end --- */
    }
}

#endif

#if VISION_V2_DEBUG

#include "app/vision_debug.h"

int main(void)
{
    System_Init();
    VisionLinear_InitV2();

    /* --- Vision debug UI state --- */
    VisionDebug_State_t vdbg;
    VisionDebug_Init(&vdbg, 80U); //set the max graph value here

    /* --- Runtime data --- */
    LinearCameraFrame frame;
    VisionLinear_ResultType result;

    VisionLinear_DebugOut_t dbg;
    static uint8 smoothBuf[VISION_LINEAR_BUFFER_SIZE];

    /* --- Fixed test settings --- */
    const uint32 TEST_EXPOSURE = 100U;

    const uint32 LOOP_PERIOD_MS    = 5U;
    const uint32 DISPLAY_PERIOD_MS = 20U;
    const uint32 DISPLAY_TICKS     = (DISPLAY_PERIOD_MS / LOOP_PERIOD_MS); /* 4 */

    uint32 nextTickMs = Timebase_GetMs();
    uint32 tickCount  = 0U;

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();

        /* Phase-locked 5ms scheduler */
        if ((uint32)(nowMs - nextTickMs) < LOOP_PERIOD_MS)
        {
            continue;
        }

        nextTickMs += LOOP_PERIOD_MS;
        tickCount++;

        /* --- 5ms tasks start --- */

        /* Check buttons */
        Buttons_Update();

        /* Button mapping:
           - SW2 toggles MAIN <-> DEBUG
           - SW3 cycles DEBUG modes (FIXED <-> AUTO) for now
        */
        boolean screenTogglePressed = Buttons_WasPressed(BUTTON_ID_SW2);
        boolean modeNextPressed     = Buttons_WasPressed(BUTTON_ID_SW3);

        VisionDebug_OnTick(&vdbg, screenTogglePressed, modeNextPressed);

        /* Scope-friendly markers - for debugging */
        RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });

        /* Capture (blocking) */
        LinearCameraGetFrameEx(&frame, TEST_EXPOSURE);

        /* Decide if we will draw this frame (20ms cadence) */
        boolean doDisplay = ((DISPLAY_TICKS != 0U) && ((tickCount % DISPLAY_TICKS) == 0U));

        /* Default: debug export OFF for this 5ms frame */
        dbg.mask     = 0u;
        dbg.smoothOut = (uint8*)0;

        /* Enable debug export only on frames we will display AND only if UI wants it */
        if ((doDisplay == TRUE) && (VisionDebug_WantsVisionDebugData(&vdbg) == TRUE))
        {
            VisionDebug_PrepareVisionDbg(&vdbg, &dbg, smoothBuf);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=true }); //img processed debug light

        /* Always run the same processing function */
        VisionLinear_ProcessFrameEx(frame.Values, &result, &dbg);

        /* Display refresh (every 20ms) */
        if (doDisplay == TRUE)
        {
            const uint8 *pSmooth = (dbg.smoothOut != (uint8*)0) ? smoothBuf : (const uint8*)0;
            const VisionLinear_DebugOut_t *pDbg = (dbg.mask != 0u) ? &dbg : (const VisionLinear_DebugOut_t*)0;

            VisionDebug_Draw(&vdbg, frame.Values, pSmooth, &result, pDbg);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=false });
        /* --- 5ms tasks end --- */
    }
}

#endif /* VISION_V2_SPLIT_TEST */

/*==================================================================================================
 *                                       VISION LINEAR V2 TESTING
==================================================================================================*/
#if VISION_V2_SPLIT_TEST
int main(void)
{
    /* 1. Initialization */
    System_Init();
    VisionLinear_InitV2(); /* Initializes the internal center-tracking state */

    LinearCameraFrame frame;
    VisionLinear_ResultType result;
    char msg[17];

    /* Using a fixed exposure for stability during algo testing */
    const uint32 TEST_EXPOSURE = 100U;

    for (;;)
    {
        /* 2. Capture and Process */
    	RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });
        LinearCameraGetFrameEx(&frame, TEST_EXPOSURE);
        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=true, .b=false });
        VisionLinear_ProcessFrame(frame.Values, &result);

        DisplayClear();

        /* --- Line 0: Status String --- */
        /* Status: 0=LOST, 1=BOTH, 2=LEFT, 3=RIGHT */
        const char* statusStr;
        switch(result.Status) {
            case VISION_LINEAR_TRACK_BOTH:  statusStr = "BOTH";  break;
            case VISION_LINEAR_TRACK_LEFT:  statusStr = "L-ONLY"; break;
            case VISION_LINEAR_TRACK_RIGHT: statusStr = "R-ONLY"; break;
            default:                        statusStr = "LOST";   break;
        }
        (void)snprintf(msg, sizeof(msg), "S:%-6s C:%3u%%", statusStr, result.Confidence);
        DisplayText(0U, msg, 16U, 0U);

        /* --- Line 1: Steering Error --- */
        sint16 errPct = (sint16)(result.Error * 100.0f);
        (void)snprintf(msg, sizeof(msg), "Steer Err: %+3d", errPct);
        DisplayText(1U, msg, 16U, 0U);

        /* --- Line 2-3: The Visual Graph --- */
        DisplayGraph(2U, frame.Values, 128U, 2U);

        /* --- DRAW ACTUAL LINE MARKERS --- */
        /* We use the specific indices provided by the new struct.
         * We place the markers on the very bottom row (Line 3).
         * Screen is 128px wide, text columns are ~6px wide.
         */
        if (result.LeftLineIdx != VISION_LINEAR_INVALID_IDX)
        {
            /* Draw 'L' under the left line position */
            DisplayText(3U, "L", 1U, result.LeftLineIdx / 6U);
        }

        if (result.RightLineIdx != VISION_LINEAR_INVALID_IDX)
        {
            /* Draw 'R' under the right line position */
            DisplayText(3U, "R", 1U, result.RightLineIdx / 6U);
        }

        DisplayRefresh();
    }
    return 0;
}
#endif

/*==================================================================================================
 *                                       EXPOSURE ADJUSTMENT TESTING
==================================================================================================*/
#if EXPOSURE_SCAN_TEST
    int main(void)
    		{
    		/* Data containers */
    		LinearCameraFrame frame;
    		VisionResultType visionResult;
			char debugLine[17]; /* 16 chars + null terminator */

    		/* Initialize System */
    		System_Init();
    		VisionLinear_Init();

    		DisplayClear();
    		DisplayText(0U, "MVP VISION", 10U, 0U);
    		DisplayRefresh();

    		for (;;)
    		    {
    		        /* 1. Read Pot (0...255) */
    				uint16 potRaw = OnboardPot_ReadLevelFiltered();

    		        /* 2. Map to 10 - 130k range */
    		        uint32 shutterHighTimeTicks = (uint32)(potRaw * potRaw) + 10U;

    		        /* 3. Capture */
    		        LinearCameraGetFrameEx(&frame, shutterHighTimeTicks);
    		        //LinearCameraGetFrame(&frame);

    		        /* 4. Vision Process */
    		        Vision_Process(frame.Values, &visionResult); //vison_linear (v1)

    		        /* 5. Display */
    		        DisplayClear();

    		        /* Format Line 0: Exposure only */
    		        /* Using %u because shutterHighTimeTicks is uint32 (small enough for %u) */
    		        memset(debugLine, 0, sizeof(debugLine));
    		        snprintf(debugLine, sizeof(debugLine), "Exp: %u", (unsigned int)shutterHighTimeTicks);
    		        DisplayText(0U, debugLine, 16U, 0U);

    		        /* Format Line 1: Left and Right Indices */
    		        memset(debugLine, 0, sizeof(debugLine));
    		        snprintf(debugLine, sizeof(debugLine), "L:%3u R:%3u",
    		                 (unsigned int)visionResult.LeftEdgeIdx,
    		                 (unsigned int)visionResult.RightEdgeIdx);
    		        DisplayText(1U, debugLine, 16U, 0U);

    		        /* Line 2-3: Graph */
    		        DisplayGraph(2U, frame.Values, 128U, 2U);

    		        DisplayRefresh();
    		    }
	}
#endif

	/*==================================================================================================
	*                                  SIMPLIFIED VISION MODULE TEST
	==================================================================================================*/

	#if SIMPLIFIED_VISION_TEST
		int main(void)
		{
		/* Data containers */
		LinearCameraFrame frame;
		VisionResultType visionResult; /* Uses the new simplified struct */
		uint32 frameCount = 0u;
		uint32 lastUpdateMs = 0u;

		/* Initialize System */
		System_Init();

		DisplayClear();
		DisplayText(0U, "MVP VISION", 10U, 0U);
		DisplayRefresh();

		for (;;)
		{
			uint32 nowMs = Timebase_GetMs();


			/* Run loop at ~20ms (50Hz) to make display readable */
			if ((uint32)(nowMs - lastUpdateMs) < 20u)
			{
				continue;
			}
			lastUpdateMs = nowMs;

			/* 1. Capture Frame (Blocking) */
			LinearCameraGetFrame(&frame);
			frameCount++;

			/* 2. Process Data (The Brain) */
			Vision_Process(frame.Values, &visionResult);

			/* 3. Visualization */
			DisplayClear();

			/* --- Line 0: Status & Error --- */
			/* Ex: "OK   Err:-0.45" */
			char line0[16];
			const char *statusStr = "???";

			switch(visionResult.Status)
			{
				case VISION_STATUS_OK:          statusStr = "OK "; break;
				case VISION_STATUS_CROSSING:    statusStr = "CRS"; break;
				case VISION_STATUS_LOST:        statusStr = "LST"; break;
				case VISION_STATUS_OUT_OF_LANE: statusStr = "OUT"; break;
				default:                        statusStr = "ERR"; break;
			}

			/* Float formatting manually if standard library is heavy,
			   but snprintf usually works on S32K */
			int errInt = (int)(visionResult.Error * 100.0f);
			(void)snprintf(line0, sizeof(line0), "%s Err:%+d.%02d",
						   statusStr, errInt / 100, (errInt < 0 ? -errInt : errInt) % 100);

			DisplayText(0U, line0, 16U, 0U);

			/* --- Line 1: Debug Details --- */
			/* Ex: "L:10 R:118 T:55" (Left, Right, Threshold) */
			char line1[16];
			(void)snprintf(line1, sizeof(line1), "L:%3u R:%3u T:%2u",
						   visionResult.LeftEdgeIdx,
						   visionResult.RightEdgeIdx,
						   visionResult.ThresholdVal);
			DisplayText(1U, line1, 16U, 0U);

			/* --- Line 2 & 3: Camera Graph --- */
			/* To make it easier to see what the car is thinking,
			   we will mark the detected edges in the visual graph
			   by forcing those specific pixels to 0 (Black notch)
			   so you can see if they align with the real lines. */

			if (visionResult.LeftEdgeIdx < 128)
			{
				 frame.Values[visionResult.LeftEdgeIdx] = 0U;
			}
			if (visionResult.RightEdgeIdx < 128)
			{
				 frame.Values[visionResult.RightEdgeIdx] = 0U;
			}

			/* Draw graph on lower half (Row 2, spanning 2 rows) */
			DisplayBarGraph(2U, frame.Values, 128U, 2U);

			DisplayRefresh();
		}
	}

	#endif /* SIMPLIFIED_VISION_TEST */

		/*==================================================================================================
		 *                                  RAW CAMERA TEST
		 *  - Captures a real frame from the linear camera driver (blocking capture)
		 *  - Displays raw pixel values (already scaled 0..100 by driver) as a bar graph on the OLED
		 *  - Shows frame count
		 *==================================================================================================*/
		#if RAW_CAMERA_TEST

		int main(void)
		{
		    LinearCameraFrame frame;
		    uint32 frameCount = 0u;
		    uint32 lastFrameMs = Timebase_GetMs();

		    System_Init(); // initialize everything

		    DisplayClear();
		    DisplayText(0U, "RAW CAMERA", 10U, 0U);
		    DisplayRefresh();

		    for (;;)
		    {
		        uint32 nowMs = Timebase_GetMs();

		        /* Conservative capture rate: 20ms */
		        if ((uint32)(nowMs - lastFrameMs) < 20u)
		        {
		            continue;
		        }
		        lastFrameMs = nowMs;

		        /* ---- 1) Capture frame (BLOCKING) ---- */
		        LinearCameraGetFrame(&frame);
		        frameCount++;

		        /* ---- 2) Draw ---- */
		        DisplayClear();

		        /* Bar graph in lower 3 pages */
		        DisplayBarGraph(1U, frame.Values, 128u, 3U);

		        /* Header: frame count */
		        {
		            char line0[16];
		            (void)snprintf(line0, sizeof(line0),
		                           "F:%05lu",
		                           (unsigned long)(frameCount % 100000ul));
		            DisplayText(0U, line0, (uint8)strlen(line0), 0U);
		        }

		        DisplayRefresh();
		    }
		}

		#endif /* RAW_CAMERA_TEST */

	#if ULTRASONIC_500MS_SAMPLE
		int main(void)
		{
		/* 16-char fixed lines (padded) — safe for your DisplayText() */
		const char L0[] = "ULTRA 500ms     ";  /* 16 */
		const char L1[] = "S:     I:      ";  /* 16 */
		const char L2[] = "cm:    x:      ";  /* 16 */
		const char L3[] = "t:     a:      ";  /* 16 */

		uint32 lastTrigMs     = Timebase_GetMs();
		float  lastDistanceCm = 0.0f;

		System_Init(); //initialize everything

		for (;;)
		{
			uint32 nowMs = Timebase_GetMs();

			/* Trigger every 500ms, but do not interrupt a BUSY measurement */
			if ((uint32)(nowMs - lastTrigMs) >= 500u)
			{
				if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
				{
					lastTrigMs = nowMs;
					Ultrasonic_StartMeasurement();
				}
			}

			/* Let the driver progress/timeout */
			Ultrasonic_Task();

			/* Read status for display */
			Ultrasonic_StatusType st = Ultrasonic_GetStatus();

			/* consume valid and set invalid to max+1 */
			float d;
			if (Ultrasonic_GetDistanceCm(&d) == TRUE)
			{
				lastDistanceCm = d;
			}
			else if (st == ULTRA_STATUS_ERROR)
			{
				lastDistanceCm = ULTRA_MAX_DISTANCE_CM + 1.0f;  /* “no obstacle” sentinel */
			}

			/* Debug getters */
			uint32 irqCnt = Ultrasonic_GetIrqCount();
			uint32 ticks  = Ultrasonic_GetLastHighTicks();
			uint16 idx    = Ultrasonic_GetTsIndex();

			/* Age since last trigger (ms) */
			uint32 ageMs = (uint32)(nowMs - lastTrigMs);

			/* ----- OLED ----- */
			DisplayClear();

			DisplayText(0U, L0, 16U, 0U);

			/* Line 1: status + IRQ count */
			DisplayText(1U, L1, 16U, 0U);
			if (st == ULTRA_STATUS_NEW_DATA)      { DisplayText(1U, "NEW  ", 5U, 2U); }
			else if (st == ULTRA_STATUS_BUSY)    { DisplayText(1U, "BUSY ", 5U, 2U); }
			else if (st == ULTRA_STATUS_ERROR)   { DisplayText(1U, "ERR  ", 5U, 2U); }
			else                                 { DisplayText(1U, "IDLE ", 5U, 2U); }
			DisplayValue(1U, (int)irqCnt, 6U, 10U);   /* I: at col 10..15 */

			/* Line 2: distance + timestamp index */
			DisplayText(2U, L2, 16U, 0U);
			{
				int distInt = (int)(lastDistanceCm + 0.5f);
				DisplayValue(2U, distInt, 4U, 3U);     /* cm at col 3..6 */
			}
			DisplayValue(2U, (int)idx, 6U, 10U);       /* x: at col 10..15 */

			/* Line 3: ticks + age */
			DisplayText(3U, L3, 16U, 0U);
			DisplayValue(3U, (int)ticks, 5U, 2U);      /* t: at col 2..6 */
			DisplayValue(3U, (int)ageMs, 6U, 10U);     /* a: at col 10..15 */

			DisplayRefresh();
		}
		}
	#endif /* ULTRASONIC_500MS_TEST */

	/*==================================================================================================
	 *                                       DISPLAY TEST
	==================================================================================================*/

	#if DISPLAY_TEST
		int main(void)
		{
		uint8 SimPixels[CAMERA_EMU_NUM_PIXELS];
		uint8 GraphValues[CAMERA_EMU_NUM_PIXELS];
		static uint32 EmuFrameCount = 0U; //number of times the graph was updated

		CameraEmulator_Init();

		DisplayClear();
		DisplayText(0U, "HELLO", 5U, 0U);
		DisplayRefresh();

		System_Init(); //initialize everything

		for (;;) //loop forever
		{

			/* Update debounced button  */
			Buttons_Update();

			/* ----- Capture new frame and process it ----- (for now it is simulated */
			if (g_EmuNewFrameFlag)
			{
				g_EmuNewFrameFlag = FALSE;

				/* 0) Read potentiometer -> 0..255 "brightness" */

				uint8 baseLevelPot = OnboardPot_ReadLevelFiltered(); //Read pot normally
				uint8 baseLevel = ReadBaselineWithButton(baseLevelPot); //Override with button test (read with a different function

				/* 1) Get emulated camera frame */
				CameraEmulator_GetFrame(SimPixels, baseLevel);
				EmuFrameCount++;

				/* 2) Scale 0..255 -> 0..100 for bar height */
				for (uint16 i = 0U; i < CAMERA_EMU_NUM_PIXELS; i++)
				{
					GraphValues[i] = (uint8)(((uint16)SimPixels[i] * 100U) / 255U);
				}

				/* 3) Clear entire display buffer */
				DisplayClear();

				/* 4) Draw bar graph in lower 3 pages (rows 8–31):
				 *    - DisplayLine = 1   → start at second page (row 8)
				 *    - LinesSpan   = 3   → use 3 pages (24 px high)
				 *      Bars will internally use only 2/3 of that height, with a ref line at the top.
				 */
				DisplayBarGraph(1U, GraphValues, CAMERA_EMU_NUM_PIXELS, 3U);

				/* 5) Prepare debug text in the top-left corner (page 0, row 0–7).
				 *    We keep the number reasonably small: modulo 100000 → max 5 digits. */
				/* Left side: read counter "R:xxxxx" */
				char debugLineLeft[16];
				uint32 displayCount = EmuFrameCount % 100000UL;
				(void)snprintf(debugLineLeft, sizeof(debugLineLeft), "R:%5lu", (unsigned long)displayCount);

				/* Right side: baseLevel "B:xxx" */
				/* The display is 128 px wide. Each character is 6 px (5 px glyph + 1 px spacing).
				 * "B:255" is 5 characters → 5 * 6 = 30 px.
				 * 128 - 30 ≈ 98 px → as column index ~98 / 6 ≈ 16 characters from left.
				 */
				char debugLineRight[16];
				(void)snprintf(debugLineRight, sizeof(debugLineRight), "B:%3u", baseLevel);

				/* Draw both texts on page 0.
				 * left text at column = 0 chars
				 * right text at column = 16 chars  (→ top-right area)
				 */
				DisplayText(0U, debugLineLeft, 7U, 0U);   /* top-left  */
				DisplayText(0U, debugLineRight, 5U, 11U); /* top-right */

				/* 6) Send everything to the OLED */
				DisplayRefresh();
			}
		}
		}
	#endif /* DISPLAY_TEST */

#endif /* TESTS_ENABLE */
