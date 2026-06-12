#include <stdbool.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp32_display.h"
#include "pc_web_link.h"
#include "s32k_uart_link.h"

#define ESP_MAIN_DISPLAY_REFRESH_MS (50U)
#define ESP_MAIN_DISPLAY_SELF_TEST_MS (1500U)

static const char *TAG = "expo_main";

static esp_err_t submit_pid_to_s32k(const EspS32kPidFrame_t *pid, void *context)
{
    (void)context;
    return S32kUartLink_SendPid(pid);
}

void app_main(void)
{
    ESP_ERROR_CHECK(EspDisplay_InitBus());
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

    ESP_ERROR_CHECK(S32kUartLink_Init());
    ESP_ERROR_CHECK(S32kUartLink_StartTask());

    esp_err_t webRet = PcWebLink_Init(submit_pid_to_s32k, NULL);
    if (webRet != ESP_OK)
    {
        ESP_LOGE(TAG, "PC web link unavailable; UART bridge remains active: %s",
                 esp_err_to_name(webRet));
    }

    TickType_t lastDisplayRefresh = 0U;
    while (true)
    {
        TickType_t now = xTaskGetTickCount();
        if (displayReady &&
            ((now - lastDisplayRefresh) >= pdMS_TO_TICKS(ESP_MAIN_DISPLAY_REFRESH_MS)))
        {
            EspAppStatus_t status = {0};
            S32kUartLink_GetStatus(&status);
            PcWebLink_GetStatus(&status);
            (void)EspDisplay_ShowBringup(&status);
            lastDisplayRefresh = now;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
