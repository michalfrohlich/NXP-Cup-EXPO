#include "pc_web_link.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"

#define PC_WEB_WIFI_SSID "EXPO_NXP_DATA"
#define PC_WEB_WIFI_PASSWORD "nobodyguessesthispassword1234"
#define PC_WEB_AP_IP "192.168.4.1"
#define PC_WEB_AP_GATEWAY "192.168.4.1"
#define PC_WEB_AP_NETMASK "255.255.255.0"
#define PC_WEB_MAX_CLIENTS (4U)
#define PC_WEB_HTTP_BODY_MAX (96U)

static const char *TAG = "pc_web_link";
static httpd_handle_t s_httpServer;
static portMUX_TYPE s_statusLock = portMUX_INITIALIZER_UNLOCKED;
static PcWebLink_PidSubmitFn s_submitPid;
static void *s_submitContext;
static bool s_wifiEnabled;
static bool s_wifiError;
static uint8_t s_wifiClientCount;

static const char s_webPage[] =
    "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
    "<title>PID Tuner</title><style>"
    ":root{color-scheme:dark;--bg:#101820;--panel:#17212b;--panel2:#1d2a35;"
    "--line:#314454;--text:#eef4f8;--muted:#aab7c2;--accent:#22c55e}"
    "*{box-sizing:border-box}body{margin:0;min-height:100vh;font-family:Arial,sans-serif;"
    "background:var(--bg);color:var(--text);display:flex;align-items:center;"
    "justify-content:center;padding:18px}.app{width:min(920px,100%);display:grid;gap:14px}"
    "header{display:flex;align-items:flex-end;justify-content:space-between;gap:12px}"
    "h1{margin:0;font-size:clamp(30px,6vw,48px);line-height:1}.status{min-width:180px;"
    "padding:8px 10px;border:1px solid var(--line);border-radius:8px;background:var(--panel);"
    "color:var(--muted);font-size:14px;text-align:center}form{display:grid;"
    "grid-template-columns:1.2fr .8fr;gap:14px}.tuners,.side{background:var(--panel);"
    "border:1px solid var(--line);border-radius:8px;padding:14px}.tuners{display:grid;"
    "gap:12px}.tuner{display:grid;grid-template-columns:62px 1fr 70px 36px 36px;"
    "align-items:center;gap:10px;padding:12px;background:var(--panel2);border:1px solid #263746;"
    "border-radius:8px}.tuner label{font-size:18px;font-weight:700}input[type=range]{width:100%;"
    "accent-color:var(--accent)}input[type=number]{width:100%;padding:9px 8px;"
    "border:1px solid var(--line);border-radius:6px;background:#101820;color:var(--text);"
    "font-size:16px;text-align:center}button{min-height:38px;border:1px solid var(--line);"
    "border-radius:6px;background:#223241;color:var(--text);font-size:16px;cursor:pointer}"
    "button:hover{background:#2b4152}button:active{transform:translateY(1px)}.primary{"
    "background:var(--accent);border-color:#16a34a;color:#04130a;font-weight:700}"
    ".side{display:grid;gap:12px;align-content:start}.readout{display:grid;"
    "grid-template-columns:repeat(3,1fr);gap:8px}.tile{padding:10px;border:1px solid var(--line);"
    "border-radius:8px;background:#101820;text-align:center}.tile span{display:block;"
    "color:var(--muted);font-size:13px}.tile strong{font-size:30px}.frame{padding:12px;"
    "border:1px solid var(--line);border-radius:8px;background:#081018;color:#8ef2b3;"
    "font-family:Consolas,monospace;font-size:20px;text-align:center;overflow-wrap:anywhere}"
    ".presets,.actions{display:grid;grid-template-columns:repeat(2,1fr);gap:8px}"
    ".note{color:var(--muted);font-size:13px;line-height:1.4}@media(max-width:760px){"
    "body{align-items:flex-start}header{display:grid}form{grid-template-columns:1fr}"
    ".tuner{grid-template-columns:50px 1fr 66px 34px 34px;gap:8px;padding:10px}"
    ".tile strong{font-size:24px}}</style></head><body><main class=\"app\"><header>"
    "<h1>PID Tuner</h1><div id=\"status\" class=\"status\">Ready</div></header>"
    "<form id=\"pidForm\"><section class=\"tuners\">"
    "<div class=\"tuner\"><label for=\"Kp\">Kp</label><input id=\"KpRange\" type=\"range\" "
    "min=\"0\" max=\"99\" step=\"1\" value=\"0\"><input id=\"Kp\" type=\"number\" min=\"0\" "
    "max=\"99\" step=\"1\" value=\"0\" required><button type=\"button\" class=\"step\" "
    "data-target=\"Kp\" data-step=\"-1\">-</button><button type=\"button\" class=\"step\" "
    "data-target=\"Kp\" data-step=\"1\">+</button></div>"
    "<div class=\"tuner\"><label for=\"Ki\">Ki</label><input id=\"KiRange\" type=\"range\" "
    "min=\"0\" max=\"99\" step=\"1\" value=\"0\"><input id=\"Ki\" type=\"number\" min=\"0\" "
    "max=\"99\" step=\"1\" value=\"0\" required><button type=\"button\" class=\"step\" "
    "data-target=\"Ki\" data-step=\"-1\">-</button><button type=\"button\" class=\"step\" "
    "data-target=\"Ki\" data-step=\"1\">+</button></div>"
    "<div class=\"tuner\"><label for=\"Kd\">Kd</label><input id=\"KdRange\" type=\"range\" "
    "min=\"0\" max=\"99\" step=\"1\" value=\"0\"><input id=\"Kd\" type=\"number\" min=\"0\" "
    "max=\"99\" step=\"1\" value=\"0\" required><button type=\"button\" class=\"step\" "
    "data-target=\"Kd\" data-step=\"-1\">-</button><button type=\"button\" class=\"step\" "
    "data-target=\"Kd\" data-step=\"1\">+</button></div></section><aside class=\"side\">"
    "<div class=\"readout\"><div class=\"tile\"><span>Kp</span><strong id=\"KpOut\">0</strong>"
    "</div><div class=\"tile\"><span>Ki</span><strong id=\"KiOut\">0</strong></div>"
    "<div class=\"tile\"><span>Kd</span><strong id=\"KdOut\">0</strong></div></div>"
    "<div id=\"frame\" class=\"frame\">#P00I00D00_</div><div class=\"presets\">"
    "<button type=\"button\" data-preset=\"0,0,0\">Zero</button>"
    "<button type=\"button\" data-preset=\"10,0,0\">P only</button>"
    "<button type=\"button\" data-preset=\"20,5,0\">Balanced</button>"
    "<button type=\"button\" data-preset=\"35,8,4\">Responsive</button></div>"
    "<div class=\"actions\"><button type=\"button\" id=\"resetBtn\">Reset</button>"
    "<button type=\"submit\" class=\"primary\">Send PID</button></div>"
    "<div class=\"note\">Success confirms UART transmission only. The S32K does not yet apply "
    "these values.</div></aside></form></main><script>"
    "const gains=['Kp','Ki','Kd'],statusBox=document.getElementById('status');"
    "function clamp(v){v=parseInt(v,10);return Number.isNaN(v)?0:Math.max(0,Math.min(99,v))}"
    "function pad(v){return String(clamp(v)).padStart(2,'0')}"
    "function updateFrame(){document.getElementById('frame').textContent='#P'+"
    "pad(document.getElementById('Kp').value)+'I'+pad(document.getElementById('Ki').value)+"
    "'D'+pad(document.getElementById('Kd').value)+'_'}"
    "function setGain(id,v){v=clamp(v);document.getElementById(id).value=v;"
    "document.getElementById(id+'Range').value=v;document.getElementById(id+'Out').textContent=v;"
    "updateFrame()}gains.forEach(id=>{document.getElementById(id).oninput=e=>setGain(id,e.target."
    "value);"
    "document.getElementById(id+'Range').oninput=e=>setGain(id,e.target.value)});"
    "document.querySelectorAll('.step').forEach(b=>b.onclick=()=>setGain(b.dataset.target,"
    "clamp(document.getElementById(b.dataset.target).value)+parseInt(b.dataset.step,10)));"
    "document.querySelectorAll('[data-preset]').forEach(b=>b.onclick=()=>{b.dataset.preset.split(',"
    "')"
    ".forEach((v,i)=>setGain(gains[i],v));statusBox.textContent=b.textContent});"
    "document.getElementById('resetBtn').onclick=()=>{gains.forEach(id=>setGain(id,0));"
    "statusBox.textContent='Reset'};document.getElementById('pidForm').onsubmit=async e=>{"
    "e.preventDefault();let body=new URLSearchParams();"
    "gains.forEach(id=>body.append(id,clamp(document.getElementById(id).value)));"
    "statusBox.textContent='Sending to UART...';try{let r=await fetch('/pid',{method:'POST',"
    "headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body.toString()});"
    "let t=await r.text();if(!r.ok)throw new Error(t||'Send failed');"
    "statusBox.textContent='Sent to UART'}catch(err){statusBox.textContent=err.message||'Send "
    "failed'}};"
    "gains.forEach(id=>setGain(id,0));</script></body></html>";

static void set_error_status(bool error)
{
    portENTER_CRITICAL(&s_statusLock);
    s_wifiError = error;
    portEXIT_CRITICAL(&s_statusLock);
}

static void wifi_event_handler(void *arg, esp_event_base_t eventBase, int32_t eventId,
                               void *eventData)
{
    (void)arg;

    if ((eventBase == WIFI_EVENT) && (eventId == WIFI_EVENT_AP_START))
    {
        portENTER_CRITICAL(&s_statusLock);
        s_wifiEnabled = true;
        s_wifiError = false;
        portEXIT_CRITICAL(&s_statusLock);
        ESP_LOGI(TAG, "Wi-Fi AP started: SSID=%s address=http://%s/", PC_WEB_WIFI_SSID,
                 PC_WEB_AP_IP);
    }
    else if ((eventBase == WIFI_EVENT) && (eventId == WIFI_EVENT_AP_STACONNECTED))
    {
        const wifi_event_ap_staconnected_t *event = (const wifi_event_ap_staconnected_t *)eventData;
        portENTER_CRITICAL(&s_statusLock);
        if (s_wifiClientCount < PC_WEB_MAX_CLIENTS)
        {
            s_wifiClientCount++;
        }
        portEXIT_CRITICAL(&s_statusLock);
        ESP_LOGI(TAG, "PC client connected, aid=%u", (unsigned)event->aid);
    }
    else if ((eventBase == WIFI_EVENT) && (eventId == WIFI_EVENT_AP_STADISCONNECTED))
    {
        const wifi_event_ap_stadisconnected_t *event =
            (const wifi_event_ap_stadisconnected_t *)eventData;
        portENTER_CRITICAL(&s_statusLock);
        if (s_wifiClientCount > 0U)
        {
            s_wifiClientCount--;
        }
        portEXIT_CRITICAL(&s_statusLock);
        ESP_LOGI(TAG, "PC client disconnected, aid=%u", (unsigned)event->aid);
    }
}

static bool parse_form_u8(const char *body, const char *key, uint8_t *outValue)
{
    const size_t keyLen = strlen(key);
    const char *field = body;

    while ((field != NULL) && (*field != '\0'))
    {
        const char *end = strchr(field, '&');
        size_t fieldLen = (end != NULL) ? (size_t)(end - field) : strlen(field);

        if ((fieldLen > (keyLen + 1U)) && (strncmp(field, key, keyLen) == 0) &&
            (field[keyLen] == '='))
        {
            char *valueEnd;
            unsigned long value = strtoul(&field[keyLen + 1U], &valueEnd, 10);
            const char *expectedEnd = field + fieldLen;

            if ((valueEnd == &field[keyLen + 1U]) || (valueEnd != expectedEnd) ||
                (value > ESP_S32K_PID_VALUE_MAX))
            {
                return false;
            }

            *outValue = (uint8_t)value;
            return true;
        }

        field = (end != NULL) ? (end + 1) : NULL;
    }

    return false;
}

static bool parse_pid_form(const char *body, EspS32kPidFrame_t *outPid)
{
    return (body != NULL) && (outPid != NULL) && parse_form_u8(body, "Kp", &outPid->proportional) &&
           parse_form_u8(body, "Ki", &outPid->integral) &&
           parse_form_u8(body, "Kd", &outPid->derivative);
}

static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, s_webPage, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t pid_post_handler(httpd_req_t *req)
{
    char body[PC_WEB_HTTP_BODY_MAX + 1U];
    size_t received = 0U;
    EspS32kPidFrame_t pid;

    if ((req == NULL) || (req->content_len == 0U) || (req->content_len > PC_WEB_HTTP_BODY_MAX))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid request body");
        return ESP_FAIL;
    }

    while (received < req->content_len)
    {
        int chunk = httpd_req_recv(req, &body[received], req->content_len - received);
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

    if ((s_submitPid == NULL) || (s_submitPid(&pid, s_submitContext) != ESP_OK))
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "UART transmission failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "PID written to UART: P=%u I=%u D=%u", (unsigned)pid.proportional,
             (unsigned)pid.integral, (unsigned)pid.derivative);
    httpd_resp_set_type(req, "text/plain");
    return httpd_resp_sendstr(req, "OK");
}

static esp_err_t configure_ap_address(esp_netif_t *netif)
{
    esp_netif_ip_info_t address = {0};
    esp_err_t ret = esp_netif_str_to_ip4(PC_WEB_AP_IP, &address.ip);
    if (ret == ESP_OK)
    {
        ret = esp_netif_str_to_ip4(PC_WEB_AP_GATEWAY, &address.gw);
    }
    if (ret == ESP_OK)
    {
        ret = esp_netif_str_to_ip4(PC_WEB_AP_NETMASK, &address.netmask);
    }
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_netif_dhcps_stop(netif);
    if ((ret != ESP_OK) && (ret != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED))
    {
        return ret;
    }

    ret = esp_netif_set_ip_info(netif, &address);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_netif_dhcps_start(netif);
    return ((ret == ESP_OK) || (ret == ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED)) ? ESP_OK : ret;
}

static esp_err_t start_http_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    esp_err_t ret = httpd_start(&s_httpServer, &config);
    if (ret != ESP_OK)
    {
        return ret;
    }

    const httpd_uri_t rootUri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler,
        .user_ctx = NULL,
    };
    const httpd_uri_t pidUri = {
        .uri = "/pid",
        .method = HTTP_POST,
        .handler = pid_post_handler,
        .user_ctx = NULL,
    };

    ret = httpd_register_uri_handler(s_httpServer, &rootUri);
    if (ret == ESP_OK)
    {
        ret = httpd_register_uri_handler(s_httpServer, &pidUri);
    }
    if (ret != ESP_OK)
    {
        (void)httpd_stop(s_httpServer);
        s_httpServer = NULL;
        return ret;
    }

    ESP_LOGI(TAG, "Web UI ready: http://%s/", PC_WEB_AP_IP);
    return ESP_OK;
}

esp_err_t PcWebLink_Init(PcWebLink_PidSubmitFn submitPid, void *context)
{
    if (submitPid == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    s_submitPid = submitPid;
    s_submitContext = context;
    portENTER_CRITICAL(&s_statusLock);
    s_wifiEnabled = false;
    s_wifiError = false;
    s_wifiClientCount = 0U;
    portEXIT_CRITICAL(&s_statusLock);

    esp_err_t ret = nvs_flash_init();
    if ((ret == ESP_ERR_NVS_NO_FREE_PAGES) || (ret == ESP_ERR_NVS_NEW_VERSION_FOUND))
    {
        ret = nvs_flash_erase();
        if (ret == ESP_OK)
        {
            ret = nvs_flash_init();
        }
    }
    if (ret != ESP_OK)
    {
        set_error_status(true);
        return ret;
    }

    ret = esp_netif_init();
    if ((ret != ESP_OK) && (ret != ESP_ERR_INVALID_STATE))
    {
        set_error_status(true);
        return ret;
    }

    ret = esp_event_loop_create_default();
    if ((ret != ESP_OK) && (ret != ESP_ERR_INVALID_STATE))
    {
        set_error_status(true);
        return ret;
    }

    esp_netif_t *netif = esp_netif_create_default_wifi_ap();
    if (netif == NULL)
    {
        set_error_status(true);
        return ESP_FAIL;
    }

    ret = configure_ap_address(netif);
    if (ret != ESP_OK)
    {
        set_error_status(true);
        return ret;
    }

    wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&initConfig);
    if (ret != ESP_OK)
    {
        set_error_status(true);
        return ret;
    }

    ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler,
                                              NULL, NULL);
    if (ret != ESP_OK)
    {
        set_error_status(true);
        return ret;
    }

    wifi_config_t config = {0};
    (void)strncpy((char *)config.ap.ssid, PC_WEB_WIFI_SSID, sizeof(config.ap.ssid) - 1U);
    (void)strncpy((char *)config.ap.password, PC_WEB_WIFI_PASSWORD,
                  sizeof(config.ap.password) - 1U);
    config.ap.ssid_len = (uint8_t)strlen(PC_WEB_WIFI_SSID);
    config.ap.channel = 1U;
    config.ap.max_connection = PC_WEB_MAX_CLIENTS;
    config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ret = esp_wifi_set_mode(WIFI_MODE_AP);
    if (ret == ESP_OK)
    {
        ret = esp_wifi_set_config(WIFI_IF_AP, &config);
    }
    if (ret == ESP_OK)
    {
        ret = esp_wifi_start();
    }
    if (ret == ESP_OK)
    {
        ret = start_http_server();
    }
    if (ret != ESP_OK)
    {
        set_error_status(true);
        return ret;
    }

    return ESP_OK;
}

void PcWebLink_GetStatus(EspAppStatus_t *outStatus)
{
    if (outStatus == NULL)
    {
        return;
    }

    portENTER_CRITICAL(&s_statusLock);
    outStatus->wifiEnabled = s_wifiEnabled;
    outStatus->wifiConnected = (s_wifiClientCount > 0U);
    outStatus->wifiError = s_wifiError;
    outStatus->wifiClientCount = s_wifiClientCount;
    portEXIT_CRITICAL(&s_statusLock);
}
