#include "s32k_uart_link.h"

#include <stdbool.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp32_pin_config.h"

#define S32K_UART_LINK_PORT UART_NUM_1
#define S32K_UART_LINK_RX_BUF_BYTES (256)
#define S32K_UART_LINK_TX_BUF_BYTES (256)
#define S32K_UART_LINK_RX_CHUNK_BYTES (32U)
#define S32K_UART_LINK_RX_FRAME_MAX (ESP_S32K_BUTTON_FRAME_LEN)
#define S32K_UART_LINK_READ_TIMEOUT_MS (20U)
#define S32K_UART_LINK_TASK_STACK (4096U)
#define S32K_UART_LINK_TASK_PRIORITY (6U)
#define S32K_UART_LINK_LOG_FIRST_FRAMES (5U)
#define S32K_UART_LINK_LOG_EVERY_FRAMES (10U)
#define S32K_UART_LINK_LOG_FIRST_RX_CHUNKS (3U)
#define S32K_UART_LINK_STATUS_PERIOD_MS (2000U)
#define S32K_UART_LINK_TUNE_LOCK_TIMEOUT_MS (500U)
#define S32K_UART_LINK_TUNE_ACK_ATTEMPT_MS (150U)
#define S32K_UART_LINK_TUNE_SEND_ATTEMPTS (2U)

static const char *TAG = "s32k_uart_link";
static TaskHandle_t s_uartTaskHandle;
static SemaphoreHandle_t s_tuneMutex;
static SemaphoreHandle_t s_tuneResultReady;
static portMUX_TYPE s_statusLock = portMUX_INITIALIZER_UNLOCKED;
static uint8_t s_uartRxFrame[S32K_UART_LINK_RX_FRAME_MAX];
static uint8_t s_uartRxLen;
static bool s_uartRxActive;
static uint32_t s_uartButtonFrames;
static uint32_t s_uartRxBytes;
static uint32_t s_uartRxChunks;
static uint32_t s_uartProtocolErrors;
static uint32_t s_uartAckErrors;
static uint32_t s_tuneResults;
static uint32_t s_tuneRejects;
static uint32_t s_tuneTimeouts;
static uint8_t s_nextTuneSequence;
static uint8_t s_waitingTuneSequence;
static bool s_tuneResultAccepted;
static bool s_waitingForTuneResult;
static EspS32kButtonFrame_t s_lastButtons;

static void status_store_buttons(const EspS32kButtonFrame_t *buttons)
{
    if (buttons == NULL)
    {
        return;
    }

    portENTER_CRITICAL(&s_statusLock);
    s_lastButtons = *buttons;
    portEXIT_CRITICAL(&s_statusLock);
}

static uint32_t status_increment_rx_frames(void)
{
    uint32_t value;

    portENTER_CRITICAL(&s_statusLock);
    s_uartButtonFrames++;
    value = s_uartButtonFrames;
    portEXIT_CRITICAL(&s_statusLock);
    return value;
}

static uint32_t status_increment_protocol_errors(void)
{
    uint32_t value;

    portENTER_CRITICAL(&s_statusLock);
    s_uartProtocolErrors++;
    value = s_uartProtocolErrors;
    portEXIT_CRITICAL(&s_statusLock);
    return value;
}

static uint32_t status_increment_ack_errors(void)
{
    uint32_t value;

    portENTER_CRITICAL(&s_statusLock);
    s_uartAckErrors++;
    value = s_uartAckErrors;
    portEXIT_CRITICAL(&s_statusLock);
    return value;
}

static bool should_log_rx_frame(uint32_t rxFrames)
{
    return (rxFrames <= S32K_UART_LINK_LOG_FIRST_FRAMES) ||
           ((rxFrames % S32K_UART_LINK_LOG_EVERY_FRAMES) == 0U);
}

static TickType_t ticks_at_least_one(uint32_t delayMs)
{
    TickType_t ticks = pdMS_TO_TICKS(delayMs);
    return (ticks > 0U) ? ticks : 1U;
}

static esp_err_t send_ack_frame(const EspS32kAckFrame_t *ack)
{
    uint8_t frame[ESP_S32K_ACK_FRAME_LEN];

    if (!EspS32kAckFrame_Encode(ack, frame, sizeof(frame)))
    {
        return ESP_ERR_INVALID_ARG;
    }

    int written = uart_write_bytes(S32K_UART_LINK_PORT, (const char *)frame, sizeof(frame));

    return (written == (int)sizeof(frame)) ? ESP_OK : ESP_FAIL;
}

static void reset_uart_test_parser(void)
{
    s_uartRxLen = 0U;
    s_uartRxActive = false;
}

static void handle_button_frame(const EspS32kButtonFrame_t *buttons)
{
    EspS32kAckFrame_t ack;
    const bool testButtonPressed = buttons->sw2Pressed || buttons->sw3Pressed;

    status_store_buttons(buttons);
    (void)gpio_set_level(ESP32_STATUS_LED_GPIO, testButtonPressed ? 1 : 0);

    ack = *buttons;
    if (send_ack_frame(&ack) == ESP_OK)
    {
        uint32_t rxFrames = status_increment_rx_frames();
        if (should_log_rx_frame(rxFrames))
        {
            EspAppStatus_t status;
            S32kUartLink_GetStatus(&status);
            ESP_LOGI(TAG, "S32K buttons seq=%u t=%u sw2=%u sw3=%u swpcb=%u rx=%lu err=%lu",
                     (unsigned)buttons->sequence, (unsigned)buttons->timestampMs,
                     buttons->sw2Pressed ? 1U : 0U, buttons->sw3Pressed ? 1U : 0U,
                     buttons->swPcbOn ? 1U : 0U, (unsigned long)rxFrames,
                     (unsigned long)status.protocolErrors);
        }
    }
    else
    {
        uint32_t ackErrors = status_increment_ack_errors();
        ESP_LOGE(TAG, "UART ACK send failed ackerr=%lu", (unsigned long)ackErrors);
    }
}

static void handle_tune_result(const EspS32kTuneResultFrame_t *result)
{
    bool matched = false;

    portENTER_CRITICAL(&s_statusLock);
    if (s_waitingForTuneResult && (result->sequence == s_waitingTuneSequence))
    {
        s_tuneResultAccepted = result->accepted;
        if (result->accepted)
        {
            s_tuneResults++;
        }
        else
        {
            s_tuneRejects++;
        }
        matched = true;
    }
    portEXIT_CRITICAL(&s_statusLock);

    if (matched)
    {
        (void)xSemaphoreGive(s_tuneResultReady);
        ESP_LOGI(TAG, "S32K tune result seq=%u accepted=%u", (unsigned)result->sequence,
                 result->accepted ? 1U : 0U);
    }
}

static void consume_uart_test_byte(uint8_t byte)
{
    EspS32kButtonFrame_t decodedButtons;
    EspS32kTuneResultFrame_t decodedResult;

    if (byte == (uint8_t)ESP_S32K_FRAME_START)
    {
        s_uartRxFrame[0] = byte;
        s_uartRxLen = 1U;
        s_uartRxActive = true;
        return;
    }

    if (!s_uartRxActive)
    {
        return;
    }

    if (s_uartRxLen >= S32K_UART_LINK_RX_FRAME_MAX)
    {
        (void)status_increment_protocol_errors();
        reset_uart_test_parser();
        return;
    }

    s_uartRxFrame[s_uartRxLen] = byte;
    s_uartRxLen++;

    if (byte != (uint8_t)ESP_S32K_FRAME_END)
    {
        return;
    }

    if ((s_uartRxLen == ESP_S32K_BUTTON_FRAME_LEN) &&
        EspS32kButtonFrame_Decode(s_uartRxFrame, s_uartRxLen, &decodedButtons))
    {
        handle_button_frame(&decodedButtons);
    }
    else if ((s_uartRxLen == ESP_S32K_TUNE_RESULT_FRAME_LEN) &&
             EspS32kTuneResultFrame_Decode(s_uartRxFrame, s_uartRxLen, &decodedResult))
    {
        handle_tune_result(&decodedResult);
    }
    else
    {
        uint32_t protocolErrors = status_increment_protocol_errors();
        ESP_LOGW(TAG, "Bad S32K UART frame len=%u err=%u", (unsigned)s_uartRxLen,
                 (unsigned)protocolErrors);
    }

    reset_uart_test_parser();
}

static esp_err_t init_gpio(void)
{
    gpio_config_t outputConfig = {
        .pin_bit_mask = (1ULL << ESP32_STATUS_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t ret = gpio_config(&outputConfig);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = gpio_set_level(ESP32_STATUS_LED_GPIO, 0);
    if (ret != ESP_OK)
    {
        return ret;
    }

    gpio_config_t inputConfig = {
        .pin_bit_mask = (1ULL << ESP32_BUTTON1_GPIO) | (1ULL << ESP32_BUTTON2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ret = gpio_config(&inputConfig);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "GPIO ready: status=%d buttons=%d/%d", ESP32_STATUS_LED_GPIO,
                 ESP32_BUTTON1_GPIO, ESP32_BUTTON2_GPIO);
    }

    return ret;
}

esp_err_t S32kUartLink_Init(void)
{
    s_tuneMutex = xSemaphoreCreateMutex();
    s_tuneResultReady = xSemaphoreCreateBinary();
    if ((s_tuneMutex == NULL) || (s_tuneResultReady == NULL))
    {
        if (s_tuneMutex != NULL)
        {
            vSemaphoreDelete(s_tuneMutex);
            s_tuneMutex = NULL;
        }
        if (s_tuneResultReady != NULL)
        {
            vSemaphoreDelete(s_tuneResultReady);
            s_tuneResultReady = NULL;
        }
        return ESP_ERR_NO_MEM;
    }

    esp_err_t ret = init_gpio();
    if (ret != ESP_OK)
    {
        return ret;
    }

    const uart_config_t config = {
        .baud_rate = ESP_S32K_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ret = uart_driver_install(S32K_UART_LINK_PORT, S32K_UART_LINK_RX_BUF_BYTES,
                              S32K_UART_LINK_TX_BUF_BYTES, 0, NULL, 0);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = uart_param_config(S32K_UART_LINK_PORT, &config);
    if (ret != ESP_OK)
    {
        (void)uart_driver_delete(S32K_UART_LINK_PORT);
        return ret;
    }

    ret = uart_set_pin(S32K_UART_LINK_PORT, ESP32_UART_TX_GPIO, ESP32_UART_RX_GPIO,
                       UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK)
    {
        (void)uart_driver_delete(S32K_UART_LINK_PORT);
        return ret;
    }

    uint32_t actualBaud = 0U;
    (void)uart_get_baudrate(S32K_UART_LINK_PORT, &actualBaud);
    ESP_LOGI(TAG, "UART ready: port=%d tx=%d rx=%d requested=%u actual=%lu", S32K_UART_LINK_PORT,
             ESP32_UART_TX_GPIO, ESP32_UART_RX_GPIO, (unsigned)ESP_S32K_UART_BAUD_RATE,
             (unsigned long)actualBaud);

    reset_uart_test_parser();
    portENTER_CRITICAL(&s_statusLock);
    s_uartButtonFrames = 0U;
    s_uartRxBytes = 0U;
    s_uartRxChunks = 0U;
    s_uartProtocolErrors = 0U;
    s_uartAckErrors = 0U;
    s_tuneResults = 0U;
    s_tuneRejects = 0U;
    s_tuneTimeouts = 0U;
    s_nextTuneSequence = 0U;
    s_waitingTuneSequence = 0U;
    s_tuneResultAccepted = false;
    s_waitingForTuneResult = false;
    (void)memset(&s_lastButtons, 0, sizeof(s_lastButtons));
    portEXIT_CRITICAL(&s_statusLock);

    return ESP_OK;
}

esp_err_t S32kUartLink_SendTuneAndWait(EspS32kTuneFrame_t *tune)
{
    uint8_t frame[ESP_S32K_TUNE_FRAME_LEN];
    bool accepted;
    esp_err_t result = ESP_FAIL;
    uint8_t attempt;

    if ((tune == NULL) || (s_tuneMutex == NULL) || (s_tuneResultReady == NULL))
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(s_tuneMutex, ticks_at_least_one(S32K_UART_LINK_TUNE_LOCK_TIMEOUT_MS)) !=
        pdTRUE)
    {
        return ESP_ERR_TIMEOUT;
    }

    tune->sequence = s_nextTuneSequence;
    s_nextTuneSequence = (uint8_t)((s_nextTuneSequence + 1U) % (ESP_S32K_BUTTON_SEQ_MAX + 1U));
    while (xSemaphoreTake(s_tuneResultReady, 0U) == pdTRUE)
    {
    }

    portENTER_CRITICAL(&s_statusLock);
    s_waitingTuneSequence = tune->sequence;
    s_tuneResultAccepted = false;
    s_waitingForTuneResult = true;
    portEXIT_CRITICAL(&s_statusLock);

    if (!EspS32kTuneFrame_Encode(tune, frame, sizeof(frame)))
    {
        result = ESP_ERR_INVALID_ARG;
        goto done;
    }

    for (attempt = 0U; attempt < S32K_UART_LINK_TUNE_SEND_ATTEMPTS; attempt++)
    {
        int written = uart_write_bytes(S32K_UART_LINK_PORT, (const char *)frame, sizeof(frame));
        if (written != (int)sizeof(frame))
        {
            result = ESP_FAIL;
            goto done;
        }

        if (xSemaphoreTake(s_tuneResultReady,
                           ticks_at_least_one(S32K_UART_LINK_TUNE_ACK_ATTEMPT_MS)) == pdTRUE)
        {
            portENTER_CRITICAL(&s_statusLock);
            accepted = s_tuneResultAccepted;
            portEXIT_CRITICAL(&s_statusLock);
            result = accepted ? ESP_OK : ESP_ERR_INVALID_RESPONSE;
            goto done;
        }

        ESP_LOGW(TAG, "Tune response timeout seq=%u attempt=%u", (unsigned)tune->sequence,
                 (unsigned)(attempt + 1U));
    }

    portENTER_CRITICAL(&s_statusLock);
    s_tuneTimeouts++;
    portEXIT_CRITICAL(&s_statusLock);
    result = ESP_ERR_TIMEOUT;

done:
    portENTER_CRITICAL(&s_statusLock);
    s_waitingForTuneResult = false;
    portEXIT_CRITICAL(&s_statusLock);
    (void)xSemaphoreGive(s_tuneMutex);
    return result;
}

esp_err_t S32kUartLink_Read(uint8_t *data, size_t maxLength, size_t *outRead)
{
    if (outRead != NULL)
    {
        *outRead = 0U;
    }

    if ((data == NULL) || (maxLength == 0U))
    {
        return ESP_ERR_INVALID_ARG;
    }

    int read = uart_read_bytes(S32K_UART_LINK_PORT, data, maxLength,
                               ticks_at_least_one(S32K_UART_LINK_READ_TIMEOUT_MS));
    if (read < 0)
    {
        return ESP_FAIL;
    }

    if (outRead != NULL)
    {
        *outRead = (size_t)read;
    }

    return ESP_OK;
}

void S32kUartLink_Service(void)
{
    uint8_t bytes[S32K_UART_LINK_RX_CHUNK_BYTES];
    size_t read = 0U;

    if (S32kUartLink_Read(bytes, sizeof(bytes), &read) != ESP_OK)
    {
        return;
    }

    if (read > 0U)
    {
        uint32_t chunk;
        uint32_t total;

        portENTER_CRITICAL(&s_statusLock);
        s_uartRxBytes += (uint32_t)read;
        s_uartRxChunks++;
        chunk = s_uartRxChunks;
        total = s_uartRxBytes;
        portEXIT_CRITICAL(&s_statusLock);

        if (chunk <= S32K_UART_LINK_LOG_FIRST_RX_CHUNKS)
        {
            ESP_LOGI(TAG, "Raw UART RX chunk=%lu bytes=%u total=%lu", (unsigned long)chunk,
                     (unsigned)read, (unsigned long)total);
            ESP_LOG_BUFFER_HEXDUMP(TAG, bytes, read, ESP_LOG_INFO);
        }
    }

    for (size_t i = 0U; i < read; i++)
    {
        consume_uart_test_byte(bytes[i]);
    }
}

static void uart_task(void *arg)
{
    TickType_t nextStatusTick =
        xTaskGetTickCount() + ticks_at_least_one(S32K_UART_LINK_STATUS_PERIOD_MS);

    (void)arg;

    while (true)
    {
        S32kUartLink_Service();

        TickType_t now = xTaskGetTickCount();
        if ((int32_t)(now - nextStatusTick) >= 0)
        {
            uint32_t bytes;
            uint32_t frames;
            uint32_t errors;

            portENTER_CRITICAL(&s_statusLock);
            bytes = s_uartRxBytes;
            frames = s_uartButtonFrames;
            errors = s_uartProtocolErrors;
            portEXIT_CRITICAL(&s_statusLock);

            ESP_LOGI(TAG, "UART status: raw=%lu frames=%lu protoerr=%lu", (unsigned long)bytes,
                     (unsigned long)frames, (unsigned long)errors);
            nextStatusTick = now + ticks_at_least_one(S32K_UART_LINK_STATUS_PERIOD_MS);
        }
    }
}

esp_err_t S32kUartLink_StartTask(void)
{
    if (s_uartTaskHandle != NULL)
    {
        return ESP_OK;
    }

    BaseType_t created = xTaskCreate(uart_task, "s32k_uart", S32K_UART_LINK_TASK_STACK, NULL,
                                     S32K_UART_LINK_TASK_PRIORITY, &s_uartTaskHandle);
    if (created != pdPASS)
    {
        s_uartTaskHandle = NULL;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "UART service task started");
    return ESP_OK;
}

void S32kUartLink_GetStatus(EspAppStatus_t *outStatus)
{
    if (outStatus == NULL)
    {
        return;
    }

    portENTER_CRITICAL(&s_statusLock);
    outStatus->sw2Pressed = s_lastButtons.sw2Pressed;
    outStatus->sw3Pressed = s_lastButtons.sw3Pressed;
    outStatus->swPcbOn = s_lastButtons.swPcbOn;
    outStatus->sequence = s_lastButtons.sequence;
    outStatus->timestampMs = s_lastButtons.timestampMs;
    outStatus->rxFrames = s_uartButtonFrames;
    outStatus->protocolErrors = s_uartProtocolErrors;
    outStatus->ackErrors = s_uartAckErrors;
    outStatus->tuneResults = s_tuneResults;
    outStatus->tuneRejects = s_tuneRejects;
    outStatus->tuneTimeouts = s_tuneTimeouts;
    portEXIT_CRITICAL(&s_statusLock);
}
