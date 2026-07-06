#include <stdbool.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp32_display.h"
#include "pc_web_link.h"
#include "s32k_uart_link.h"

#define ESP_MAIN_DISPLAY_REFRESH_MS (50U)
#define ESP_MAIN_DISPLAY_SELF_TEST_MS (1500U)
#define ESP_MAIN_DISPLAY_RETRY_MS (1000U)

static const char *TAG = "expo_main";

static TickType_t ticks_at_least_one(uint32_t delayMs)
{
    TickType_t ticks = pdMS_TO_TICKS(delayMs);
    return (ticks > 0U) ? ticks : 1U;
}

static esp_err_t submit_tune_to_s32k(EspS32kTuneFrame_t *tune, void *context)
{
    (void)context;
    return S32kUartLink_SendTuneAndWait(tune);
}

static esp_err_t submit_drive_command_to_s32k(EspS32kDriveCommandFrame_t *command, void *context)
{
    (void)context;
    return S32kUartLink_SendDriveCommandAndWait(command);
}

void app_main(void)
{
    ESP_ERROR_CHECK(EspDisplay_InitBus());
    (void)EspDisplay_ScanI2c();
    esp_err_t displayRet = EspDisplay_Init();
    bool displayReady = (displayRet == ESP_OK);
    TickType_t nextDisplayRetry =
        xTaskGetTickCount() + ticks_at_least_one(ESP_MAIN_DISPLAY_RETRY_MS);

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

    esp_err_t webRet = PcWebLink_Init(submit_tune_to_s32k, submit_drive_command_to_s32k, NULL);
    if (webRet != ESP_OK)
    {
        ESP_LOGE(TAG, "PC web link unavailable; UART bridge remains active: %s",
                 esp_err_to_name(webRet));
    }

    TickType_t lastDisplayRefresh = 0U;
    while (true)
    {
        TickType_t now = xTaskGetTickCount();
        if ((!displayReady) && ((int32_t)(now - nextDisplayRetry) >= 0))
        {
            displayRet = EspDisplay_Init();
            displayReady = (displayRet == ESP_OK);
            if (displayReady)
            {
                ESP_LOGI(TAG, "Display initialized after retry");
                (void)EspDisplay_ShowSelfTest((uint32_t)(now * portTICK_PERIOD_MS));
                vTaskDelay(pdMS_TO_TICKS(ESP_MAIN_DISPLAY_SELF_TEST_MS));
                lastDisplayRefresh = xTaskGetTickCount();
            }
            else
            {
                ESP_LOGW(TAG, "Display retry failed: %s", esp_err_to_name(displayRet));
                nextDisplayRetry = now + ticks_at_least_one(ESP_MAIN_DISPLAY_RETRY_MS);
            }
        }

        if (displayReady &&
            ((now - lastDisplayRefresh) >= pdMS_TO_TICKS(ESP_MAIN_DISPLAY_REFRESH_MS)))
        {
            EspAppStatus_t status = {0};
            S32kUartLink_GetStatus(&status);
            PcWebLink_GetStatus(&status);
            (void)EspDisplay_ShowBringup(&status);
            lastDisplayRefresh = now;
        }
        vTaskDelay(ticks_at_least_one(5U));
    }
}
