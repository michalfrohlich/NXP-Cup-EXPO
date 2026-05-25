#pragma once
/*
  board_config.h
  ==============
  S32K board routing aliases for this car build.

  Constants in this file may depend on generated RTD/MCAL symbols.
*/

#include "Std_Types.h"
#include "Adc_CfgDefines.h"
#include "Dio_Cfg.h"
#include "Gpt.h"
#include "Icu_Cfg.h"
#include "Pwm_Cfg.h"

/* =========================================================
   Pot board routing and input calibration
========================================================= */
#define ONBOARD_POT_ADC_GROUP             AdcGroup_pot
#define POT_LEFT_RAW                      0
#define POT_CENTER_RAW                    128
#define POT_RIGHT_RAW                     255

/* =========================================================
   Buttons / LEDs board routing
========================================================= */
#define RGB_LED_DIO_CH_R                  (DioConf_DioChannel_DioChannel_LED_R)
#define RGB_LED_DIO_CH_G                  (DioConf_DioChannel_DioChannel_LED_G)
#define RGB_LED_DIO_CH_B                  (DioConf_DioChannel_DioChannel_LED_B)

/* =========================================================
   Timebase board routing
========================================================= */
#define MS_TIMER_CHANNEL                  ((Gpt_ChannelType)2u)
#define US_TIMER_CHANNEL                  ((Gpt_ChannelType)3u)

/* =========================================================
   Display board routing
========================================================= */
#define DISPLAY_I2C_ADDRESS               (0x3CU)

/* =========================================================
   Serial debug board routing
========================================================= */
#define UART_HOST_UART_CHANNEL         (0U)

/* =========================================================
   Servo board routing
========================================================= */
#define SERVO_PWM_CH                      Servo_Pwm

/* =========================================================
   ESC board routing
========================================================= */
#define ESC_PWM_CH                        Esc_Pwm
#define ESC_SECOND_PWM_CH                 Esc2_Pwm
/* Generated second ESC output: FTM3 CH2 on PTB10. */

/* =========================================================
   Linear camera board routing
========================================================= */
#define CAM_CLK_PWM_CH                    LinearCamera_Clk
#define CAM_SHUTTER_GPT_CH                1U
#define CAM_ADC_GROUP                     0U
#define CAM_SHUTTER_PCR                   97U

/* =========================================================
   Ultrasonic board routing
========================================================= */
#define ULTRA_DIO_TRIG_CHANNEL            (DioConf_DioChannel_PTE15_UltraTrig)
#define ULTRA_ICU_ECHO_CHANNEL            (IcuConf_IcuChannel_Ultrasonic_Echo)
