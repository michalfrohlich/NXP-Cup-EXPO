#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp32_s32k_bridge.h"

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
    ESP_ERROR_CHECK(EspBridge_InitUart());

    if (strlen(ESP32_WIFI_SSID) > 0U)
    {
        esp_err_t ret = EspBridge_InitWifi(ESP32_WIFI_SSID, ESP32_WIFI_PASSWORD);
        if (ret == ESP_OK)
        {
            ESP_ERROR_CHECK(EspBridge_StartHttpServer());
        }
        else
        {
            ESP_LOGW(TAG, "Wi-Fi bridge disabled: %s", esp_err_to_name(ret));
        }
    }
    else
    {
        ESP_LOGW(TAG, "Wi-Fi credentials are not configured; UART bridge is still active");
    }

    while (true)
    {
        EspBridge_ServiceUartTest();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
