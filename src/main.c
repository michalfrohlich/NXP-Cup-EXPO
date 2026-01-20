
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
#include "camera_emulator.h" //for testng
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
#include "vision_linear.h"

#include "S32K144.h"
/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/
#define TESTS_ENABLE     0 /* All macros are here - used for testing */
	/* Test mode flags – enable only one at a time (disable if TESTS_ENABLE = 0 */
	#define DISPLAY_TEST      0   /* Camera signal simulation + display + buttons + potentiometer demo */
	#define ULTRASONIC_500MS_SAMPLE    0 //working example of using ultrasonic

#define CAR_MAIN         1   /* Line-following car main code that will run the whole car*/

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
	HbridgeInit(2U, 3U, 32U, 33U, 6U, 64U);

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

	/* Force PTD0 (BLUE LED) OFF to indicate end of initialization */
	/* Set PTD0 as GPIO output, HIGH → LED off (active-low) */
	IP_PORTD->PCR[0] = PORT_PCR_MUX(1);      /* MUX = 1 → GPIO */
	IP_PTD->PDDR     |= (1UL << 0);          /* PTD0 as output */
	IP_PTD->PSOR     =  (1UL << 0);          /* set PTD0 high → LED off */
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

        /* Wait until at least 5 ms elapsed since last loop.
         * Using unsigned subtraction makes this safe even if the
         * millisecond counter wraps around.
         */
        if ((uint32)(nowMs - lastLoopMs) < 5u)
        {
            /* Not time yet → skip this iteration. */
            continue;
        }

        /* 5 ms elapsed → start a new logical loop tick */
        lastLoopMs = nowMs;
        loopCounter++;   /* 1 increment = 5 ms */

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

                /* RUN → IDLE: Stop driving when SW3 is pressed again. */
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
int main(void)
{
	System_Init(); //initialize everything
/*==================================================================================================
*                                  ULTRASONIC TEST
==================================================================================================*/

#if ULTRASONIC_500MS_SAMPLE

    /* 16-char fixed lines (padded) — safe for your DisplayText() */
    const char L0[] = "ULTRA 500ms     ";  /* 16 */
    const char L1[] = "S:     I:      ";  /* 16 */
    const char L2[] = "cm:    x:      ";  /* 16 */
    const char L3[] = "t:     a:      ";  /* 16 */

    uint32 lastTrigMs     = Timebase_GetMs();
    float  lastDistanceCm = 0.0f;

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

#endif /* ULTRASONIC_500MS_TEST */

/*==================================================================================================
 *                                       DISPLAY TEST
==================================================================================================*/

#if DISPLAY_TEST

	uint8 SimPixels[CAMERA_EMU_NUM_PIXELS];
	uint8 GraphValues[CAMERA_EMU_NUM_PIXELS];
	static uint32 EmuFrameCount = 0U; //number of times the graph was updated

	CameraEmulator_Init();

	DisplayClear();
	DisplayText(0U, "HELLO", 5U, 0U);
	DisplayRefresh();

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
#endif /* DISPLAY_TEST */

} /*main function end */

#endif /* TESTS_ENABLE */
