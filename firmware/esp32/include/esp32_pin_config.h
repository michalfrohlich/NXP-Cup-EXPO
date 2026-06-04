#pragma once

/*
  ESP32 shield pin plan.
  Power pins are connector positions, not GPIO numbers.
*/

#define ESP32_UART_RX_GPIO 16
#define ESP32_UART_TX_GPIO 17
#define ESP32_STATUS_LED_GPIO 2
#define ESP32_BUTTON1_GPIO 18
#define ESP32_BUTTON2_GPIO 19
#define ESP32_I2C_DATA_GPIO 21
#define ESP32_I2C_CLK_GPIO 22

#define ESP32_I2C_PORT 0
#define ESP32_I2C_FREQ_HZ 800000
#define ESP32_OLED_I2C_ADDRESS 0x3C

#define ESP32_HEADER_VIN_PIN 1
#define ESP32_HEADER_GND_PIN 2

#define ESP32_SCREEN_POWER "3V3"
#define ESP32_SCREEN_GROUND "GND"
