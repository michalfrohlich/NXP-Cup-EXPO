#include "pc_web_link.h"

#include <stdbool.h>
#include <stdio.h>
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
#define PC_WEB_HTTP_BODY_MAX (256U)

static const char *TAG = "pc_web_link";
static httpd_handle_t s_httpServer;
static portMUX_TYPE s_statusLock = portMUX_INITIALIZER_UNLOCKED;
static PcWebLink_TuneSubmitFn s_submitTune;
static void *s_submitContext;
static bool s_wifiEnabled;
static bool s_wifiError;
static uint8_t s_wifiClientCount;

static const char s_webPage[] =
    "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
    "<title>Vehicle Tuning</title><style>"
    ":root{color-scheme:dark;--bg:#0c1217;--panel:#151e25;--field:#0f171d;--line:#31414d;"
    "--text:#edf3f6;--muted:#9fb0ba;--green:#38c172;--amber:#f0b429}"
    "*{box-sizing:border-box}body{margin:0;font-family:Arial,sans-serif;background:var(--bg);"
    "color:var(--text);padding:18px}.app{width:min(1080px,100%);margin:auto;display:grid;gap:14px}"
    "header{display:flex;align-items:end;justify-content:space-between;gap:12px}"
    "h1{margin:0;font-size:34px}h2{margin:0 0 10px;font-size:18px}.status{padding:9px 12px;"
    "border:1px solid var(--line);border-radius:6px;background:var(--panel);color:var(--muted)}"
    "form{display:grid;grid-template-columns:1fr 1fr;gap:14px}.group,.submit{border:1px solid "
    "var(--line);border-radius:8px;background:var(--panel);padding:14px}.controller{grid-row:span "
    "2}"
    ".fields{display:grid;gap:8px}.field{display:grid;grid-template-columns:112px 1fr 92px;"
    "align-items:center;gap:10px;padding:9px;background:var(--field);border-radius:6px}"
    ".field label{font-weight:700}.field small{grid-column:2/4;color:var(--muted)}"
    "input[type=range]{width:100%;accent-color:var(--green)}input[type=number]{width:100%;"
    "padding:8px;border:1px solid var(--line);border-radius:5px;background:#080d11;"
    "color:var(--text);font-size:16px;text-align:right}.submit{grid-column:1/3;display:grid;"
    "grid-template-columns:1fr auto;gap:12px;align-items:center}.summary{font-family:Consolas,"
    "monospace;color:#9be8b7;overflow-wrap:anywhere}.actions{display:flex;gap:8px}"
    "button{min-height:40px;padding:0 15px;border:1px solid var(--line);border-radius:6px;"
    "background:#22303a;color:var(--text);font-size:15px;cursor:pointer}.primary{background:"
    "var(--green);border-color:#239a55;color:#06150c;font-weight:700}.note{color:var(--muted);"
    "font-size:13px;margin-top:6px}@media(max-width:760px){header{display:grid}form{"
    "grid-template-columns:1fr}.controller{grid-row:auto}.submit{grid-column:auto;"
    "grid-template-columns:1fr}.field{grid-template-columns:92px 1fr 82px}.actions button{"
    "flex:1}}</style></head><body><main class=\"app\"><header><h1>Vehicle Tuning</h1>"
    "<div id=\"status\" class=\"status\">Ready</div></header><form id=\"tuneForm\">"
    "<section class=\"group controller\"><h2>Steering Controller</h2>"
    "<div id=\"controllerFields\" class=\"fields\"></div></section>"
    "<section class=\"group\"><h2>Steering Output</h2>"
    "<div id=\"steeringFields\" class=\"fields\"></div></section>"
    "<section class=\"group\"><h2>Line Detector</h2>"
    "<div id=\"visionFields\" class=\"fields\"></div></section>"
    "<section class=\"submit\"><div><div id=\"summary\" class=\"summary\"></div>"
    "<div class=\"note\">Values are RAM-only. Success means the S32K validated and stored the "
    "complete snapshot.</div></div><div class=\"actions\"><button type=\"button\" id=\"defaults\">"
    "Defaults</button><button type=\"submit\" class=\"primary\">Apply to S32K</button></div>"
    "</section></form></main><script>"
    "const defs=["
    "{id:'Kp',key:'Kp',label:'KP',min:0,max:99.999,step:.001,val:4.5,g:'controller'},"
    "{id:'Ki',key:'Ki',label:'KI',min:0,max:99.999,step:.001,val:.03,g:'controller'},"
    "{id:'Kd',key:'Kd',label:'KD',min:0,max:99.999,step:.001,val:2,g:'controller'},"
    "{id:'Clamp',key:'steerClamp',label:'Clamp %',min:0,max:100,step:1,val:70,g:'steering'},"
    "{id:'Lpf',key:'steerLpf',label:'LPF alpha',min:0,max:1,step:.001,val:.45,g:'steering'},"
    "{id:'Contrast',key:'minContrast',label:'Min "
    "contrast',min:0,max:4095,step:1,val:650,g:'vision'},"
    "{id:'EdgeHigh',key:'edgeHigh',label:'Edge high %',min:1,max:100,step:1,val:40,g:'vision'},"
    "{id:'EdgeLow',key:'edgeLow',label:'Edge low %',min:1,max:100,step:1,val:55,g:'vision'}];"
    "const statusBox=document.getElementById('status');"
    "function decimals(step){let s=String(step),p=s.indexOf('.');return p<0?0:s.length-p-1}"
    "function normalized(d,v){v=Number(v);if(!Number.isFinite(v))v=d.val;"
    "v=Math.max(d.min,Math.min(d.max,v));return Number(v.toFixed(decimals(d.step)))}"
    "function render(){for(const d of defs){let host=document.getElementById(d.g+'Fields');"
    "host.insertAdjacentHTML('beforeend','<div class=\"field\"><label for=\"'+d.id+'\">'+d.label+"
    "'</label><input id=\"'+d.id+'Range\" type=\"range\" min=\"'+d.min+'\" max=\"'+d.max+"
    "'\" step=\"'+d.step+'\"><input id=\"'+d.id+'\" type=\"number\" min=\"'+d.min+'\" max=\"'+"
    "d.max+'\" step=\"'+d.step+'\" required><small>'+d.min+' .. '+d.max+'</small></div>');"
    "for(const suffix of ['', 'Range'])document.getElementById(d.id+suffix).oninput=e=>setValue(d,"
    "e.target.value);setValue(d,d.val)}}"
    "function setValue(d,v){v=normalized(d,v);document.getElementById(d.id).value=v;"
    "document.getElementById(d.id+'Range').value=v;updateSummary()}"
    "function updateSummary(){let v=Object.fromEntries(defs.map(d=>{let e=document.getElementById("
    "d.id);return[d.id,e?e.value:d.val]}));document.getElementById('summary').textContent='PID '+"
    "v.Kp+' / '+v.Ki+' / '+"
    "v.Kd+' | clamp '+v.Clamp+' | LPF '+v.Lpf+' | vision '+v.Contrast+', '+v.EdgeHigh+'%, '+"
    "v.EdgeLow+'%'}"
    "document.getElementById('defaults').onclick=()=>{defs.forEach(d=>setValue(d,d.val));"
    "statusBox.textContent='Defaults loaded'};"
    "document.getElementById('tuneForm').onsubmit=async e=>{e.preventDefault();"
    "let body=new URLSearchParams();for(const d of defs){let input=document.getElementById(d.id);"
    "if(!input.reportValidity())return;body.append(d.key,normalized(d,input.value))}"
    "statusBox.textContent='Waiting for S32K...';try{let r=await fetch('/tune',{method:'POST',"
    "headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body.toString()});"
    "let text=await r.text();if(!r.ok)throw new Error(text||'Apply failed');"
    "statusBox.textContent=text}catch(err){statusBox.textContent=err.message||'Apply failed'}};"
    "render();</script></body></html>";

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

static bool find_form_value(const char *body, const char *key, const char **outStart,
                            size_t *outLength)
{
    size_t keyLen;
    const char *field = body;

    if ((body == NULL) || (key == NULL) || (outStart == NULL) || (outLength == NULL))
    {
        return false;
    }
    keyLen = strlen(key);

    while ((field != NULL) && (*field != '\0'))
    {
        const char *end = strchr(field, '&');
        size_t fieldLen = (end != NULL) ? (size_t)(end - field) : strlen(field);

        if ((fieldLen > (keyLen + 1U)) && (strncmp(field, key, keyLen) == 0) &&
            (field[keyLen] == '='))
        {
            *outStart = &field[keyLen + 1U];
            *outLength = fieldLen - keyLen - 1U;
            return true;
        }

        field = (end != NULL) ? (end + 1) : NULL;
    }

    return false;
}

static bool parse_form_u32(const char *body, const char *key, uint32_t maxValue, uint32_t *outValue)
{
    const char *text;
    size_t length;
    uint32_t value = 0U;

    if ((outValue == NULL) || !find_form_value(body, key, &text, &length) || (length == 0U))
    {
        return false;
    }

    for (size_t index = 0U; index < length; index++)
    {
        if ((text[index] < '0') || (text[index] > '9'))
        {
            return false;
        }
        value = (value * 10U) + (uint32_t)(text[index] - '0');
        if (value > maxValue)
        {
            return false;
        }
    }

    *outValue = value;
    return true;
}

static bool parse_form_milli(const char *body, const char *key, uint32_t maxValue,
                             uint32_t *outValue)
{
    const char *text;
    size_t length;
    uint32_t whole = 0U;
    uint32_t fraction = 0U;
    uint32_t fractionScale = 1U;
    bool seenDot = false;
    bool seenDigit = false;

    if ((outValue == NULL) || !find_form_value(body, key, &text, &length) || (length == 0U))
    {
        return false;
    }

    for (size_t index = 0U; index < length; index++)
    {
        const char ch = text[index];
        if ((ch >= '0') && (ch <= '9'))
        {
            const uint32_t digit = (uint32_t)(ch - '0');
            seenDigit = true;
            if (seenDot)
            {
                if (fractionScale >= 1000U)
                {
                    return false;
                }
                fraction = (fraction * 10U) + digit;
                fractionScale *= 10U;
            }
            else
            {
                const uint32_t maxWhole = maxValue / 1000U;
                if ((whole > (maxWhole / 10U)) ||
                    ((whole == (maxWhole / 10U)) && (digit > (maxWhole % 10U))))
                {
                    return false;
                }
                whole = (whole * 10U) + digit;
            }
        }
        else if ((ch == '.') && !seenDot)
        {
            seenDot = true;
        }
        else
        {
            return false;
        }
    }

    if (!seenDigit)
    {
        return false;
    }

    while (fractionScale < 1000U)
    {
        fraction *= 10U;
        fractionScale *= 10U;
    }

    *outValue = (whole * 1000U) + fraction;
    return (*outValue <= maxValue);
}

static bool parse_tune_form(const char *body, EspS32kTuneFrame_t *outTune)
{
    uint32_t steerClamp;
    uint32_t steerLpf;
    uint32_t minContrast;
    uint32_t edgeHigh;
    uint32_t edgeLow;

    if ((body == NULL) || (outTune == NULL) ||
        !parse_form_milli(body, "Kp", ESP_S32K_TUNE_GAIN_MILLI_MAX, &outTune->proportionalMilli) ||
        !parse_form_milli(body, "Ki", ESP_S32K_TUNE_GAIN_MILLI_MAX, &outTune->integralMilli) ||
        !parse_form_milli(body, "Kd", ESP_S32K_TUNE_GAIN_MILLI_MAX, &outTune->derivativeMilli) ||
        !parse_form_u32(body, "steerClamp", ESP_S32K_TUNE_STEER_CLAMP_MAX, &steerClamp) ||
        !parse_form_milli(body, "steerLpf", ESP_S32K_TUNE_LPF_MILLI_MAX, &steerLpf) ||
        !parse_form_u32(body, "minContrast", ESP_S32K_TUNE_MIN_CONTRAST_MAX, &minContrast) ||
        !parse_form_u32(body, "edgeHigh", ESP_S32K_TUNE_EDGE_PCT_MAX, &edgeHigh) ||
        !parse_form_u32(body, "edgeLow", ESP_S32K_TUNE_EDGE_PCT_MAX, &edgeLow) ||
        (edgeHigh < ESP_S32K_TUNE_EDGE_PCT_MIN) || (edgeLow < ESP_S32K_TUNE_EDGE_PCT_MIN))
    {
        return false;
    }

    outTune->sequence = 0U;
    outTune->steerClamp = (uint8_t)steerClamp;
    outTune->steerLpfMilli = (uint16_t)steerLpf;
    outTune->minContrast = (uint16_t)minContrast;
    outTune->edgeHighPct = (uint8_t)edgeHigh;
    outTune->edgeLowPct = (uint8_t)edgeLow;
    return true;
}

static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, s_webPage, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t tune_post_handler(httpd_req_t *req)
{
    char body[PC_WEB_HTTP_BODY_MAX + 1U];
    char response[32];
    size_t received = 0U;
    EspS32kTuneFrame_t tune;

    if (req == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if ((req->content_len == 0U) || (req->content_len > PC_WEB_HTTP_BODY_MAX))
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

    if (!parse_tune_form(body, &tune))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid tune values or missing field");
        return ESP_FAIL;
    }

    if (s_submitTune == NULL)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "tune link unavailable");
        return ESP_FAIL;
    }

    esp_err_t result = s_submitTune(&tune, s_submitContext);
    if (result == ESP_ERR_TIMEOUT)
    {
        httpd_resp_set_status(req, "504 Gateway Timeout");
        return httpd_resp_sendstr(req, "S32K response timeout");
    }
    if (result == ESP_ERR_INVALID_RESPONSE)
    {
        httpd_resp_set_status(req, "422 Unprocessable Entity");
        return httpd_resp_sendstr(req, "S32K rejected values");
    }
    if (result != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "UART transmission failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Tune stored by S32K: seq=%u P=%lu I=%lu D=%lu", (unsigned)tune.sequence,
             (unsigned long)tune.proportionalMilli, (unsigned long)tune.integralMilli,
             (unsigned long)tune.derivativeMilli);
    (void)snprintf(response, sizeof(response), "Applied by S32K (seq %02u)",
                   (unsigned)tune.sequence);
    httpd_resp_set_type(req, "text/plain");
    return httpd_resp_sendstr(req, response);
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
    const httpd_uri_t tuneUri = {
        .uri = "/tune",
        .method = HTTP_POST,
        .handler = tune_post_handler,
        .user_ctx = NULL,
    };

    ret = httpd_register_uri_handler(s_httpServer, &rootUri);
    if (ret == ESP_OK)
    {
        ret = httpd_register_uri_handler(s_httpServer, &tuneUri);
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

esp_err_t PcWebLink_Init(PcWebLink_TuneSubmitFn submitTune, void *context)
{
    if (submitTune == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    s_submitTune = submitTune;
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
