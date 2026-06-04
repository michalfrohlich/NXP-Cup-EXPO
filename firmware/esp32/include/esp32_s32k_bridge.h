#ifndef ESP32_S32K_BRIDGE_H
#define ESP32_S32K_BRIDGE_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp32_app_status.h"
#include "esp_s32k_uart_protocol.h"

esp_err_t EspBridge_InitGpio(void);
esp_err_t EspBridge_InitI2c(void);
esp_err_t EspBridge_InitUart(void);
esp_err_t EspBridge_InitWifi(const char *ssid, const char *password);
esp_err_t EspBridge_StartHttpServer(void);
esp_err_t EspBridge_StartUartTask(void);

esp_err_t EspBridge_SendPidFrame(const EspS32kPidFrame_t *pid, size_t *outWritten);
esp_err_t EspBridge_ReadUart(uint8_t *data, size_t maxLength, size_t *outRead);
void EspBridge_ServiceUartTest(void);
void EspBridge_GetStatus(EspAppStatus_t *outStatus);
void EspBridge_SetWifiStatus(bool enabled, bool connected);

#endif /* ESP32_S32K_BRIDGE_H */
