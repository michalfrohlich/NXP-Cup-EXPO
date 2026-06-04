#ifndef ESP32_DISPLAY_H
#define ESP32_DISPLAY_H

#include "esp_err.h"
#include "esp32_app_status.h"

esp_err_t EspDisplay_Init(void);
bool EspDisplay_IsReady(void);
esp_err_t EspDisplay_Probe(void);
uint8_t EspDisplay_ScanI2c(void);
esp_err_t EspDisplay_ShowSelfTest(uint32_t uptimeMs);
esp_err_t EspDisplay_ShowBringup(const EspAppStatus_t *status);

#endif /* ESP32_DISPLAY_H */
