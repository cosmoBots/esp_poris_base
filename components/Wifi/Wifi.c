// BEGIN --- Standard C headers section ---
#include <stdio.h>
#include <string.h>

// END   --- Standard C headers section ---

// BEGIN --- SDK config section---
#include <sdkconfig.h>
// END   --- SDK config section---

// BEGIN --- FreeRTOS headers section ---
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

// END   --- FreeRTOS headers section ---


// BEGIN --- ESP-IDF headers section ---
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_log.h>

// END   --- ESP-IDF headers section ---

// BEGIN --- Project configuration section ---
#include <PrjCfg.h> // Including project configuration module 
// END   --- Project configuration section ---

// BEGIN --- Project configuration section ---

// end   --- Project configuration section ---

// BEGIN --- Self-includes section ---
#include "Wifi.h"
#include "WifiSecrets.h"
// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "Wifi";
// END --- Logging related variables

// BEGIN --- Internal variables (DRE)
Wifi_dre_t Wifi_dre = {
    .enabled = true,
    .last_return_code = Wifi_ret_ok
};
// END   --- Internal variables (DRE)

// BEGIN --- Functional variables and handlers

// You can activate the FORCE to use data from WifiSecrets.h
#ifdef WIFI_CONN_FORCE
#undef CONFIG_WIFI_CONN_SSID
#define CONFIG_WIFI_CONN_SSID WIFI_CONN_FORCE_SSID
#undef CONFIG_WIFI_CONN_PASSWORD
#define CONFIG_WIFI_CONN_PASSWORD WIFI_CONN_FORCE_PASSWORD
#endif

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group;

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi desconectado, reintentando...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "IP obtenida: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// END   --- Functional variables and handlers


// BEGIN ------------------ Public API (COMMON + SPIN)------------------

Wifi_return_code_t Wifi_setup(void)
{
    // Init liviano; no arranca tarea.
    ESP_LOGD(TAG, "setup()");
    Wifi_dre.last_return_code = Wifi_ret_ok;

        ESP_LOGI(TAG, "Inicializando WiFi...");

    // Inicializa NVS (necesario para WiFi y RF Calibration)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            }
        }
    };

    strncpy((char *) wifi_config.sta.ssid,
            CONFIG_WIFI_CONN_SSID, sizeof(wifi_config.sta.ssid));

    strncpy((char *) wifi_config.sta.password,
            CONFIG_WIFI_CONN_PASSWORD, sizeof(wifi_config.sta.password));

    ESP_LOGI(TAG, "Conectando a SSID: %s", CONFIG_WIFI_CONN_SSID);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Bloquea hasta obtener IP
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT,
                                           pdFALSE, pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi conectado correctamente");
    }

    return Wifi_ret_ok;
}

Wifi_return_code_t Wifi_spin(void)
{
    bool en = Wifi_dre.enabled;
    if (!en) return Wifi_ret_ok;

    ESP_LOGI(TAG, "Hello world!");
    vTaskDelay(pdMS_TO_TICKS(120));
    return Wifi_ret_ok;
}

Wifi_return_code_t Wifi_enable(void)
{
    Wifi_dre.enabled = true;
    Wifi_dre.last_return_code = Wifi_ret_ok;
    return Wifi_ret_ok;
}

Wifi_return_code_t Wifi_disable(void)
{
    Wifi_dre.enabled = false;
    Wifi_dre.last_return_code = Wifi_ret_ok;
    return Wifi_ret_ok;
}

// BEGIN ------------------ Public API (COMMON)------------------
