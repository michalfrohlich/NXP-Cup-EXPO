#include <string.h>
#include <stdbool.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp32_display.h"
#include "esp32_s32k_bridge.h"

#define ESP_MAIN_DISPLAY_REFRESH_MS (50U)
#define ESP_MAIN_DISPLAY_SELF_TEST_MS (1500U)

#ifndef ESP32_WIFI_SSID
#define ESP32_WIFI_SSID ""
#endif

#ifndef ESP32_WIFI_PASSWORD
#define ESP32_WIFI_PASSWORD ""
#endif

static const char *TAG = "expo_main";

void app_main(void)
{
    ESP_ERROR_CHECK(EspBridge_InitGpio());
    ESP_ERROR_CHECK(EspBridge_InitI2c());
    (void)EspDisplay_ScanI2c();
    esp_err_t displayRet = EspDisplay_Init();
    const bool displayReady = (displayRet == ESP_OK);
    if (displayReady)
    {
        (void)EspDisplay_ShowSelfTest(0U);
        vTaskDelay(pdMS_TO_TICKS(ESP_MAIN_DISPLAY_SELF_TEST_MS));
    }
    else
    {
        ESP_LOGW(TAG, "Display test skipped: %s", esp_err_to_name(displayRet));
    }

    ESP_ERROR_CHECK(EspBridge_InitUart());
    ESP_ERROR_CHECK(EspBridge_StartUartTask());

    if (strlen(ESP32_WIFI_SSID) > 0U)
    {
        EspBridge_SetWifiStatus(true, false);
        esp_err_t ret = EspBridge_InitWifi(ESP32_WIFI_SSID, ESP32_WIFI_PASSWORD);
        if (ret == ESP_OK)
        {
            EspBridge_SetWifiStatus(true, true);
            ESP_ERROR_CHECK(EspBridge_StartHttpServer());
        }
        else
        {
            EspBridge_SetWifiStatus(true, false);
            ESP_LOGW(TAG, "Wi-Fi bridge disabled: %s", esp_err_to_name(ret));
        }
    }
    else
    {
        EspBridge_SetWifiStatus(false, false);
        ESP_LOGW(TAG, "Wi-Fi credentials are not configured; UART bridge is still active");
    }

    TickType_t lastDisplayRefresh = 0U;
    while (true)
    {
        TickType_t now = xTaskGetTickCount();
        if (displayReady &&
            ((now - lastDisplayRefresh) >= pdMS_TO_TICKS(ESP_MAIN_DISPLAY_REFRESH_MS)))
        {
            EspAppStatus_t status;
            EspBridge_GetStatus(&status);
            (void)EspDisplay_ShowBringup(&status);
            lastDisplayRefresh = now;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
