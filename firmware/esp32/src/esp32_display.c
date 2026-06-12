#include "esp32_display.h"

#include <stdio.h>
#include <string.h>

#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "u8g2.h"

#include "esp32_pin_config.h"
#include "esp_s32k_uart_protocol.h"

#define ESP_DISPLAY_I2C_ADDRESS_U8G2 (ESP32_OLED_I2C_ADDRESS << 1U)
#define ESP_DISPLAY_I2C_TIMEOUT_MS (100U)
#define ESP_DISPLAY_SCAN_TIMEOUT_MS (20U)
#define ESP_DISPLAY_TRANSFER_MAX_BYTES (512U)

static const char *TAG = "esp32_display";
static u8g2_t s_u8g2;
static bool s_ready;

esp_err_t EspDisplay_InitBus(void)
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

static uint8_t u8x8_byte_i2c_cb(u8x8_t *u8x8, uint8_t msg, uint8_t argInt, void *argPtr)
{
    static uint8_t transfer[ESP_DISPLAY_TRANSFER_MAX_BYTES];
    static size_t transferLen;

    (void)u8x8;

    switch (msg)
    {
        case U8X8_MSG_BYTE_INIT:
            return 1U;

        case U8X8_MSG_BYTE_START_TRANSFER:
            transferLen = 0U;
            return 1U;

        case U8X8_MSG_BYTE_SEND:
            if ((argPtr == NULL) || ((transferLen + (size_t)argInt) > sizeof(transfer)))
            {
                return 0U;
            }
            (void)memcpy(&transfer[transferLen], argPtr, argInt);
            transferLen += (size_t)argInt;
            return 1U;

        case U8X8_MSG_BYTE_END_TRANSFER:
            if (transferLen == 0U)
            {
                return 1U;
            }
            return (i2c_master_write_to_device((i2c_port_t)ESP32_I2C_PORT, ESP32_OLED_I2C_ADDRESS,
                                               transfer, transferLen,
                                               pdMS_TO_TICKS(ESP_DISPLAY_I2C_TIMEOUT_MS)) == ESP_OK)
                       ? 1U
                       : 0U;

        case U8X8_MSG_BYTE_SET_DC:
            return 1U;

        default:
            return 0U;
    }
}

static uint8_t u8x8_gpio_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t argInt, void *argPtr)
{
    (void)u8x8;
    (void)argPtr;

    switch (msg)
    {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            return 1U;

        case U8X8_MSG_DELAY_MILLI:
            vTaskDelay(pdMS_TO_TICKS(argInt));
            return 1U;

        case U8X8_MSG_DELAY_10MICRO:
            esp_rom_delay_us((uint32_t)argInt * 10U);
            return 1U;

        case U8X8_MSG_DELAY_100NANO:
            __asm__ __volatile__("nop");
            return 1U;

        case U8X8_MSG_DELAY_I2C:
            esp_rom_delay_us(2U);
            return 1U;

        case U8X8_MSG_GPIO_RESET:
            return 1U;

        default:
            return 0U;
    }
}

static void draw_status_line(uint8_t y, const char *label, const char *value)
{
    char line[48];

    (void)snprintf(line, sizeof(line), "%-9s %s", label, value);
    u8g2_DrawStr(&s_u8g2, 0, y, line);
}

static esp_err_t probe_i2c_address(uint8_t address7, TickType_t timeoutTicks)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t ret = i2c_master_start(cmd);
    if (ret == ESP_OK)
    {
        ret = i2c_master_write_byte(cmd, (address7 << 1U) | I2C_MASTER_WRITE, true);
    }
    if (ret == ESP_OK)
    {
        ret = i2c_master_stop(cmd);
    }
    if (ret == ESP_OK)
    {
        ret = i2c_master_cmd_begin((i2c_port_t)ESP32_I2C_PORT, cmd, timeoutTicks);
    }

    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t EspDisplay_Probe(void)
{
    return probe_i2c_address(ESP32_OLED_I2C_ADDRESS, pdMS_TO_TICKS(ESP_DISPLAY_I2C_TIMEOUT_MS));
}

uint8_t EspDisplay_ScanI2c(void)
{
    uint8_t found = 0U;

    ESP_LOGI(TAG, "I2C scan start: sda=%d scl=%d freq=%u", ESP32_I2C_DATA_GPIO, ESP32_I2C_CLK_GPIO,
             (unsigned)ESP32_I2C_FREQ_HZ);

    for (uint8_t address = 0x03U; address <= 0x77U; address++)
    {
        esp_err_t ret = probe_i2c_address(address, pdMS_TO_TICKS(ESP_DISPLAY_SCAN_TIMEOUT_MS));
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "I2C device found: 7bit=0x%02X write8=0x%02X", address,
                     (unsigned)(address << 1U));
            found++;
        }
    }

    if (found == 0U)
    {
        ESP_LOGW(TAG, "I2C scan complete: no devices found");
    }
    else
    {
        ESP_LOGI(TAG, "I2C scan complete: found=%u", (unsigned)found);
    }

    return found;
}

esp_err_t EspDisplay_Init(void)
{
    esp_err_t ret = EspDisplay_Probe();
    if (ret != ESP_OK)
    {
        s_ready = false;
        ESP_LOGE(TAG, "OLED not detected: addr=0x%02X sda=%d scl=%d err=%s", ESP32_OLED_I2C_ADDRESS,
                 ESP32_I2C_DATA_GPIO, ESP32_I2C_CLK_GPIO, esp_err_to_name(ret));
        return ret;
    }

    u8g2_Setup_sh1122_i2c_256x64_f(&s_u8g2, U8G2_R0, u8x8_byte_i2c_cb, u8x8_gpio_delay_cb);
    u8x8_SetI2CAddress(&s_u8g2.u8x8, ESP_DISPLAY_I2C_ADDRESS_U8G2);

    u8g2_InitDisplay(&s_u8g2);
    u8g2_SetPowerSave(&s_u8g2, 0U);
    u8g2_SetContrast(&s_u8g2, 180U);

    s_ready = true;
    ESP_LOGI(TAG, "SH1122 OLED ready: addr=0x%02X sda=%d scl=%d", ESP32_OLED_I2C_ADDRESS,
             ESP32_I2C_DATA_GPIO, ESP32_I2C_CLK_GPIO);
    return ESP_OK;
}

bool EspDisplay_IsReady(void)
{
    return s_ready;
}

esp_err_t EspDisplay_ShowSelfTest(uint32_t uptimeMs)
{
    char value[40];

    if (!s_ready)
    {
        return ESP_ERR_INVALID_STATE;
    }

    u8g2_ClearBuffer(&s_u8g2);
    u8g2_SetFont(&s_u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&s_u8g2, 0, 8, "ESP32 DISPLAY SELF TEST");
    u8g2_DrawHLine(&s_u8g2, 0, 11, 256);
    u8g2_DrawFrame(&s_u8g2, 0, 14, 62, 48);
    u8g2_DrawBox(&s_u8g2, 6, 20, 12, 12);
    u8g2_DrawFrame(&s_u8g2, 24, 20, 12, 12);
    u8g2_DrawCircle(&s_u8g2, 48, 26, 7, U8G2_DRAW_ALL);
    u8g2_DrawLine(&s_u8g2, 6, 52, 56, 38);

    (void)snprintf(value, sizeof(value), "addr=0x%02X", ESP32_OLED_I2C_ADDRESS);
    draw_status_line(25, "I2C", value);
    (void)snprintf(value, sizeof(value), "sda=%d scl=%d %ukHz", ESP32_I2C_DATA_GPIO,
                   ESP32_I2C_CLK_GPIO, (unsigned)(ESP32_I2C_FREQ_HZ / 1000U));
    draw_status_line(37, "PINS", value);
    (void)snprintf(value, sizeof(value), "uptime=%lu ms", (unsigned long)uptimeMs);
    draw_status_line(49, "TEST", value);
    draw_status_line(61, "STATE", "probe ok");
    u8g2_SendBuffer(&s_u8g2);

    return ESP_OK;
}

esp_err_t EspDisplay_ShowBringup(const EspAppStatus_t *status)
{
    char value[64];

    if (!s_ready)
    {
        return ESP_ERR_INVALID_STATE;
    }
    if (status == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    u8g2_ClearBuffer(&s_u8g2);
    u8g2_SetFont(&s_u8g2, u8g2_font_5x8_tf);
    u8g2_DrawStr(&s_u8g2, 0, 7, "ESP UART DECODE  #Bsw2sw3pcbQseqTtime_");
    u8g2_DrawHLine(&s_u8g2, 0, 10, 256);

    (void)snprintf(value, sizeof(value), "#B%u%u%uQ%02uT%04u_", status->sw2Pressed ? 1U : 0U,
                   status->sw3Pressed ? 1U : 0U, status->swPcbOn ? 1U : 0U,
                   (unsigned)status->sequence, (unsigned)status->timestampMs);
    draw_status_line(19, "FRAME", value);

    (void)snprintf(value, sizeof(value), "sw2=%u sw3=%u pcb=%u", status->sw2Pressed ? 1U : 0U,
                   status->sw3Pressed ? 1U : 0U, status->swPcbOn ? 1U : 0U);
    draw_status_line(28, "BUTTONS", value);

    (void)snprintf(value, sizeof(value), "seq=%02u timestamp=%04u", (unsigned)status->sequence,
                   (unsigned)status->timestampMs);
    draw_status_line(37, "FIELDS", value);

    (void)snprintf(value, sizeof(value), "rx=%lu protoerr=%lu ackerr=%lu",
                   (unsigned long)status->rxFrames, (unsigned long)status->protocolErrors,
                   (unsigned long)status->ackErrors);
    draw_status_line(46, "COUNTS", value);

    (void)snprintf(value, sizeof(value), "baud=%u i2c=%ukHz", (unsigned)ESP_S32K_UART_BAUD_RATE,
                   (unsigned)(ESP32_I2C_FREQ_HZ / 1000U));
    draw_status_line(55, "LINK", value);

    if (status->wifiError)
    {
        (void)snprintf(value, sizeof(value), "wifi:err");
    }
    else if (status->wifiConnected)
    {
        (void)snprintf(value, sizeof(value), "wifi:pc clients=%u",
                       (unsigned)status->wifiClientCount);
    }
    else if (status->wifiEnabled)
    {
        (void)snprintf(value, sizeof(value), "wifi:ap 192.168.4.1");
    }
    else
    {
        (void)snprintf(value, sizeof(value), "wifi:init");
    }
    draw_status_line(63, "NET", value);
    u8g2_SendBuffer(&s_u8g2);

    return ESP_OK;
}
