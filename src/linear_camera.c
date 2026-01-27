/*==================================================================================================
*    Copyright 2021-2024 NXP
*
*    NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be
*    used strictly in accordance with the applicable license terms. By expressly
*    accepting such terms or by downloading, installing, activating and/or otherwise
*    using the software, you are agreeing that you have read, and that you agree to
*    comply with and are bound by, such license terms. If you do not agree to be
*    bound by the applicable license terms, then you may not retain, install,
*    activate or otherwise use the software.
==================================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "linear_camera.h"
#include "Adc_Types.h"
#include "pixy2.h"
#include "rgb_led.h"
#include "timebase.h"
/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/
static volatile LinearCamera LinearCameraInstance;

/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/
static Adc_ValueGroupType AdcResultBuffer;

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/
void NewCameraFrame(void){
    Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0x4000);
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
    Pwm_EnableNotification(LinearCameraInstance.ClkPwmChannel, PWM_FALLING_EDGE);
}

void CameraClock(void){
    if(LinearCameraInstance.CurrentIndex < 128U){
        Adc_StartGroupConversion(LinearCameraInstance.InputAdcGroup);
    }
    else{
        Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    }
}

void CameraAdcFinished(void){
    /* ADC is configured for 12-bit mode (0..4095).
     * Convert raw sample to a 0..100 brightness scale.
     */
    uint16 raw12 = (uint16)AdcResultBuffer;              /* 0 .. 4095 */
    uint8  scaled = (uint8)((raw12 * 100U) / 4095U);     /* 0 .. 100 */

    LinearCameraInstance.BufferReference->Values[LinearCameraInstance.CurrentIndex] = scaled;
    LinearCameraInstance.CurrentIndex++;
}


/* === FUNCTION THAT WAS ORIGINALLY USED WITH THE 8-bit RESOLUTION ===
 * void CameraAdcFinished(void){
    LinearCameraInstance.BufferReference->Values[LinearCameraInstance.CurrentIndex] = AdcResultBuffer*25U/64U;
    LinearCameraInstance.CurrentIndex++;
}*/

/* --- safe abort helper (prevents late ISR writes + stops clocking) --- */
static void LinearCamera_AbortCapture(void)
{
    /* Stop sensor clocking first */
    (void)Pwm_SetDutyCycle(LinearCameraInstance.ClkPwmChannel, 0U);
    (void)Pwm_DisableNotification(LinearCameraInstance.ClkPwmChannel);

    /* End shutter */
    (void)Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);

    /* Stop shutter timer notifications and timer */
    (void)Gpt_DisableNotification(LinearCameraInstance.ShutterGptChannel);
    (void)Gpt_StopTimer(LinearCameraInstance.ShutterGptChannel);

    /* Stop ADC group conversion (this API exists in your build) */
    (void)Adc_StopGroupConversion(LinearCameraInstance.InputAdcGroup);
}

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

void LinearCameraInit(Pwm_ChannelType ClkPwmChannel, Gpt_ChannelType ShutterGptChannel, Adc_GroupType InputAdcGroup, Dio_ChannelType ShutterDioChannel){
    LinearCameraInstance.ClkPwmChannel = ClkPwmChannel;
    LinearCameraInstance.ShutterGptChannel = ShutterGptChannel;
    LinearCameraInstance.InputAdcGroup = InputAdcGroup;
    LinearCameraInstance.ShutterDioChannel = ShutterDioChannel;
    LinearCameraInstance.CurrentIndex = 0U;
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_LOW);
    Adc_SetupResultBuffer(LinearCameraInstance.InputAdcGroup , &AdcResultBuffer);
    Adc_EnableGroupNotification(LinearCameraInstance.InputAdcGroup);
}

/* NXP Example code function */
void LinearCameraGetFrame(LinearCameraFrame *Frame){
    LinearCameraInstance.BufferReference = Frame;
    LinearCameraInstance.CurrentIndex = 0U;
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_HIGH);
    Gpt_StartTimer(LinearCameraInstance.ShutterGptChannel, 100U);
    Gpt_EnableNotification(LinearCameraInstance.ShutterGptChannel);
    /* Blocking wait until 128 samples captured by ADC notification */
	while(LinearCameraInstance.CurrentIndex <= 127U)
	{
		;
	}
}
/* Added variable exposure to the NXP Example*/
void LinearCameraGetFrameEx(LinearCameraFrame *Frame, uint32 exposureTicks)
{
    LinearCameraInstance.BufferReference = Frame;
    LinearCameraInstance.CurrentIndex = 0U;

    const uint32 TIMEOUT_MS = 40U;
    uint32 t0 = Timebase_GetMs();

    //clamp min-max
    if (exposureTicks <= 10U)
       {
           exposureTicks = 10U;
       }
    if (exposureTicks >= 80000U)
	   {
		   exposureTicks = 80000U;
	   }


    /* Begin exposure */
    Dio_WriteChannel(LinearCameraInstance.ShutterDioChannel, (Dio_LevelType)STD_HIGH);

    /* End exposure after exposureTicks; NewCameraFrame() will drop SHUTTER and start PWM clocking */
    Gpt_StartTimer(LinearCameraInstance.ShutterGptChannel, exposureTicks);
    Gpt_EnableNotification(LinearCameraInstance.ShutterGptChannel);

    /* Wait for the image */
    while (LinearCameraInstance.CurrentIndex <= 127U)
    {
        uint32 now = Timebase_GetMs();
        if ((now - t0) > TIMEOUT_MS)
        {
            LinearCamera_AbortCapture();
            RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });

            return; /* Exit early: capture failed */
        }
    }

	#if 0
		/* Blocking wait until 128 samples captured by ADC notification */
		while((LinearCameraInstance.CurrentIndex <= 127U) && (safetyCounter < MAX_WAIT))
		{
			safetyCounter++;
		}

		/* If timed out, force the index to 128 to let the rest of the code run */
		if (safetyCounter >= MAX_WAIT)
		{
			LinearCameraInstance.CurrentIndex = 128U;
			RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });  // red on
		}
	#endif
}


#ifdef __cplusplus
}
#endif

/** @} */
