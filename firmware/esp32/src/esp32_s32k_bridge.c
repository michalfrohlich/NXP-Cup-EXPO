#include "esp32_s32k_bridge.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "esp32_pin_config.h"

#define ESP_BRIDGE_UART_PORT UART_NUM_1
#define ESP_BRIDGE_UART_RX_BUF_BYTES (256)
#define ESP_BRIDGE_UART_TX_BUF_BYTES (256)
#define ESP_BRIDGE_UART_RX_CHUNK_BYTES (32U)
#define ESP_BRIDGE_UART_RX_FRAME_MAX (ESP_S32K_BUTTON_FRAME_LEN)
#define ESP_BRIDGE_UART_READ_TIMEOUT_MS (10U)
#define ESP_BRIDGE_UART_IDLE_DELAY_MS (10U)
#define ESP_BRIDGE_UART_TASK_STACK (4096U)
#define ESP_BRIDGE_UART_TASK_PRIORITY (6U)
#define ESP_BRIDGE_HTTP_BODY_MAX (96U)
#define ESP_BRIDGE_WIFI_TIMEOUT_MS (10000U)
#define ESP_BRIDGE_LOG_FIRST_FRAMES (5U)
#define ESP_BRIDGE_LOG_EVERY_FRAMES (10U)

static const char *TAG = "esp_s32k_bridge";
static EventGroupHandle_t s_wifiEventGroup;
static httpd_handle_t s_httpServer;
static TaskHandle_t s_uartTaskHandle;
static portMUX_TYPE s_statusLock = portMUX_INITIALIZER_UNLOCKED;
static uint8_t s_uartRxFrame[ESP_BRIDGE_UART_RX_FRAME_MAX];
static uint8_t s_uartRxLen;
static bool s_uartRxActive;
static uint32_t s_uartButtonFrames;
static uint32_t s_uartProtocolErrors;
static uint32_t s_uartAckErrors;
static EspS32kButtonFrame_t s_lastButtons;
static bool s_wifiEnabled;
static bool s_wifiConnected;

#define WIFI_CONNECTED_BIT BIT0

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
    return (rxFrames <= ESP_BRIDGE_LOG_FIRST_FRAMES) ||
           ((rxFrames % ESP_BRIDGE_LOG_EVERY_FRAMES) == 0U);
}

static TickType_t ticks_at_least_one(uint32_t delayMs)
{
    TickType_t ticks = pdMS_TO_TICKS(delayMs);
    return (ticks > 0U) ? ticks : 1U;
}

static bool parse_u8_form_field(const char *body, const char *key, uint8_t *outValue)
{
    const char *start;
    char *end;
    unsigned long parsed;

    if ((body == NULL) || (key == NULL) || (outValue == NULL))
    {
        return false;
    }

    start = strstr(body, key);
    if (start == NULL)
    {
        return false;
    }

    start += strlen(key);
    if (*start != '=')
    {
        return false;
    }
    start++;

    parsed = strtoul(start, &end, 10);
    if ((end == start) || (parsed > ESP_S32K_PID_VALUE_MAX))
    {
        return false;
    }

    if ((*end != '\0') && (*end != '&'))
    {
        return false;
    }

    *outValue = (uint8_t)parsed;
    return true;
}

static bool parse_pid_form(const char *body, EspS32kPidFrame_t *pid)
{
    if ((body == NULL) || (pid == NULL))
    {
        return false;
    }

    return ((parse_u8_form_field(body, "Kp", &pid->proportional) ||
             parse_u8_form_field(body, "P", &pid->proportional)) &&
            (parse_u8_form_field(body, "Ki", &pid->integral) ||
             parse_u8_form_field(body, "I", &pid->integral)) &&
            (parse_u8_form_field(body, "Kd", &pid->derivative) ||
             parse_u8_form_field(body, "D", &pid->derivative)));
}

static esp_err_t send_ack_frame(const EspS32kAckFrame_t *ack)
{
    uint8_t frame[ESP_S32K_ACK_FRAME_LEN];

    if (!EspS32kAckFrame_Encode(ack, frame, sizeof(frame)))
    {
        return ESP_ERR_INVALID_ARG;
    }

    int written = uart_write_bytes(ESP_BRIDGE_UART_PORT, (const char *)frame, sizeof(frame));

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
    const bool anyPressed = buttons->sw2Pressed || buttons->sw3Pressed || buttons->swPcbOn;

    status_store_buttons(buttons);
    (void)gpio_set_level(ESP32_STATUS_LED_GPIO, anyPressed ? 1 : 0);

    ack = *buttons;
    if (send_ack_frame(&ack) == ESP_OK)
    {
        uint32_t rxFrames = status_increment_rx_frames();
        if (should_log_rx_frame(rxFrames))
        {
            EspAppStatus_t status;
            EspBridge_GetStatus(&status);
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

static void consume_uart_test_byte(uint8_t byte)
{
    EspS32kButtonFrame_t decoded;

    if (byte == (uint8_t)ESP_S32K_PID_FRAME_START)
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

    if (s_uartRxLen >= ESP_BRIDGE_UART_RX_FRAME_MAX)
    {
        (void)status_increment_protocol_errors();
        reset_uart_test_parser();
        return;
    }

    s_uartRxFrame[s_uartRxLen] = byte;
    s_uartRxLen++;

    if (byte != (uint8_t)ESP_S32K_PID_FRAME_END)
    {
        return;
    }

    if ((s_uartRxLen == ESP_S32K_BUTTON_FRAME_LEN) &&
        EspS32kButtonFrame_Decode(s_uartRxFrame, s_uartRxLen, &decoded))
    {
        handle_button_frame(&decoded);
    }
    else
    {
        uint32_t protocolErrors = status_increment_protocol_errors();
        ESP_LOGW(TAG, "Bad S32K UART frame len=%u err=%u", (unsigned)s_uartRxLen,
                 (unsigned)protocolErrors);
    }

    reset_uart_test_parser();
}

static void wifi_event_handler(void *arg, esp_event_base_t eventBase, int32_t eventId,
                               void *eventData)
{
    (void)arg;

    if ((eventBase == WIFI_EVENT) && (eventId == WIFI_EVENT_STA_START))
    {
        (void)esp_wifi_connect();
    }
    else if ((eventBase == WIFI_EVENT) && (eventId == WIFI_EVENT_STA_DISCONNECTED))
    {
        ESP_LOGW(TAG, "Wi-Fi disconnected, retrying");
        (void)esp_wifi_connect();
    }
    else if ((eventBase == IP_EVENT) && (eventId == IP_EVENT_STA_GOT_IP))
    {
        const ip_event_got_ip_t *event = (const ip_event_got_ip_t *)eventData;
        ESP_LOGI(TAG, "Wi-Fi IP: " IPSTR, IP2STR(&event->ip_info.ip));
        (void)xEventGroupSetBits(s_wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

static esp_err_t pid_post_handler(httpd_req_t *req)
{
    char body[ESP_BRIDGE_HTTP_BODY_MAX + 1U];
    size_t received = 0U;
    EspS32kPidFrame_t pid;
    size_t written = 0U;

    if ((req == NULL) || (req->content_len > ESP_BRIDGE_HTTP_BODY_MAX))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "body too large");
        return ESP_FAIL;
    }

    while (received < req->content_len)
    {
        int chunk = httpd_req_recv(req, &body[received], (size_t)req->content_len - received);
        if (chunk <= 0)
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "receive failed");
            return ESP_FAIL;
        }
        received += (size_t)chunk;
    }
    body[received] = '\0';

    if (!parse_pid_form(body, &pid))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "expected Kp=0..99&Ki=0..99&Kd=0..99");
        return ESP_FAIL;
    }

    esp_err_t ret = EspBridge_SendPidFrame(&pid, &written);
    if ((ret != ESP_OK) || (written != ESP_S32K_PID_FRAME_LEN))
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "uart send failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "PID sent: P=%u I=%u D=%u", (unsigned)pid.proportional, (unsigned)pid.integral,
             (unsigned)pid.derivative);

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "OK\n");
    return ESP_OK;
}

esp_err_t EspBridge_InitGpio(void)
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

esp_err_t EspBridge_InitI2c(void)
{
    const i2c_port_t port = (i2c_port_t)ESP32_I2C_PORT;
    const i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = ESP32_I2C_DATA_GPIO,
        .scl_io_num = ESP32_I2C_CLK_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = ESP32_I2C_FREQ_HZ,
        .clk_flags = 0,
    };

    esp_err_t ret = i2c_param_config(port, &config);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "I2C ready: SDA=%d SCL=%d", ESP32_I2C_DATA_GPIO, ESP32_I2C_CLK_GPIO);
    }

    return ret;
}

esp_err_t EspBridge_InitUart(void)
{
    const uart_config_t config = {
        .baud_rate = ESP_S32K_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t ret = uart_driver_install(ESP_BRIDGE_UART_PORT, ESP_BRIDGE_UART_RX_BUF_BYTES,
                                        ESP_BRIDGE_UART_TX_BUF_BYTES, 0, NULL, 0);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = uart_param_config(ESP_BRIDGE_UART_PORT, &config);
    if (ret != ESP_OK)
    {
        (void)uart_driver_delete(ESP_BRIDGE_UART_PORT);
        return ret;
    }

    ret = uart_set_pin(ESP_BRIDGE_UART_PORT, ESP32_UART_TX_GPIO, ESP32_UART_RX_GPIO,
                       UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK)
    {
        (void)uart_driver_delete(ESP_BRIDGE_UART_PORT);
        return ret;
    }

    ESP_LOGI(TAG, "UART ready: port=%d tx=%d rx=%d baud=%u", ESP_BRIDGE_UART_PORT,
             ESP32_UART_TX_GPIO, ESP32_UART_RX_GPIO, (unsigned)ESP_S32K_UART_BAUD_RATE);

    reset_uart_test_parser();
    portENTER_CRITICAL(&s_statusLock);
    s_uartButtonFrames = 0U;
    s_uartProtocolErrors = 0U;
    s_uartAckErrors = 0U;
    (void)memset(&s_lastButtons, 0, sizeof(s_lastButtons));
    portEXIT_CRITICAL(&s_statusLock);

    return ESP_OK;
}

esp_err_t EspBridge_SendPidFrame(const EspS32kPidFrame_t *pid, size_t *outWritten)
{
    uint8_t frame[ESP_S32K_PID_FRAME_LEN];

    if (outWritten != NULL)
    {
        *outWritten = 0U;
    }

    if (!EspS32kPidFrame_Encode(pid, frame, sizeof(frame)))
    {
        return ESP_ERR_INVALID_ARG;
    }

    int written = uart_write_bytes(ESP_BRIDGE_UART_PORT, (const char *)frame, sizeof(frame));
    if (written < 0)
    {
        return ESP_FAIL;
    }

    if (outWritten != NULL)
    {
        *outWritten = (size_t)written;
    }

    return (written == (int)sizeof(frame)) ? ESP_OK : ESP_FAIL;
}

esp_err_t EspBridge_ReadUart(uint8_t *data, size_t maxLength, size_t *outRead)
{
    if (outRead != NULL)
    {
        *outRead = 0U;
    }

    if ((data == NULL) || (maxLength == 0U))
    {
        return ESP_ERR_INVALID_ARG;
    }

    int read = uart_read_bytes(ESP_BRIDGE_UART_PORT, data, maxLength,
                               ticks_at_least_one(ESP_BRIDGE_UART_READ_TIMEOUT_MS));
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

void EspBridge_ServiceUartTest(void)
{
    uint8_t bytes[ESP_BRIDGE_UART_RX_CHUNK_BYTES];
    size_t read = 0U;

    if (EspBridge_ReadUart(bytes, sizeof(bytes), &read) != ESP_OK)
    {
        return;
    }

    for (size_t i = 0U; i < read; i++)
    {
        consume_uart_test_byte(bytes[i]);
    }
}

static void uart_task(void *arg)
{
    (void)arg;

    while (true)
    {
        EspBridge_ServiceUartTest();
        vTaskDelay(ticks_at_least_one(ESP_BRIDGE_UART_IDLE_DELAY_MS));
    }
}

esp_err_t EspBridge_StartUartTask(void)
{
    if (s_uartTaskHandle != NULL)
    {
        return ESP_OK;
    }

    BaseType_t created = xTaskCreate(uart_task, "s32k_uart", ESP_BRIDGE_UART_TASK_STACK, NULL,
                                     ESP_BRIDGE_UART_TASK_PRIORITY, &s_uartTaskHandle);
    if (created != pdPASS)
    {
        s_uartTaskHandle = NULL;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "UART service task started");
    return ESP_OK;
}

void EspBridge_GetStatus(EspAppStatus_t *outStatus)
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
    outStatus->wifiEnabled = s_wifiEnabled;
    outStatus->wifiConnected = s_wifiConnected;
    portEXIT_CRITICAL(&s_statusLock);
}

void EspBridge_SetWifiStatus(bool enabled, bool connected)
{
    portENTER_CRITICAL(&s_statusLock);
    s_wifiEnabled = enabled;
    s_wifiConnected = connected;
    portEXIT_CRITICAL(&s_statusLock);
}

esp_err_t EspBridge_InitWifi(const char *ssid, const char *password)
{
    if ((ssid == NULL) || (ssid[0] == '\0'))
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = nvs_flash_init();
    if ((ret == ESP_ERR_NVS_NO_FREE_PAGES) || (ret == ESP_ERR_NVS_NEW_VERSION_FOUND))
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_netif_init();
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_event_loop_create_default();
    if ((ret != ESP_OK) && (ret != ESP_ERR_INVALID_STATE))
    {
        return ret;
    }

    s_wifiEventGroup = xEventGroupCreate();
    if (s_wifiEventGroup == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    (void)esp_netif_create_default_wifi_sta();

    wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&initConfig);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler,
                                              NULL, NULL);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler,
                                              NULL, NULL);
    if (ret != ESP_OK)
    {
        return ret;
    }

    wifi_config_t wifiConfig = {0};
    (void)strncpy((char *)wifiConfig.sta.ssid, ssid, sizeof(wifiConfig.sta.ssid) - 1U);
    if (password != NULL)
    {
        (void)strncpy((char *)wifiConfig.sta.password, password,
                      sizeof(wifiConfig.sta.password) - 1U);
    }

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_wifi_start();
    if (ret != ESP_OK)
    {
        return ret;
    }

    ESP_LOGI(TAG, "Connecting to Wi-Fi SSID %s", ssid);
    EventBits_t bits = xEventGroupWaitBits(s_wifiEventGroup, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE,
                                           pdMS_TO_TICKS(ESP_BRIDGE_WIFI_TIMEOUT_MS));

    return ((bits & WIFI_CONNECTED_BIT) != 0U) ? ESP_OK : ESP_ERR_TIMEOUT;
}

esp_err_t EspBridge_StartHttpServer(void)
{
    if (s_httpServer != NULL)
    {
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    esp_err_t ret = httpd_start(&s_httpServer, &config);
    if (ret != ESP_OK)
    {
        return ret;
    }

    const httpd_uri_t pidUri = {
        .uri = "/pid",
        .method = HTTP_POST,
        .handler = pid_post_handler,
        .user_ctx = NULL,
    };

    ret = httpd_register_uri_handler(s_httpServer, &pidUri);
    if (ret != ESP_OK)
    {
        (void)httpd_stop(s_httpServer);
        s_httpServer = NULL;
        return ret;
    }

    ESP_LOGI(TAG, "HTTP endpoint ready: POST /pid with Kp=10&Ki=5&Kd=1");
    return ESP_OK;
}
