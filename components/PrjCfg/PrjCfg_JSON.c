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
#if CONFIG_PRJCFG_USE_THREAD
  #include <freertos/semphr.h>
#endif

// END   --- FreeRTOS headers section ---


// BEGIN --- ESP-IDF headers section ---
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_mac.h>
#include <cJSON.h>

// END   --- ESP-IDF headers section ---

// BEGIN --- Project configuration section ---
#include <PrjCfg.h> // Including project configuration module 
// END   --- Project configuration section ---

// BEGIN --- Project configuration section ---

// end   --- Project configuration section ---

// BEGIN --- Self-includes section ---
#include "PrjCfg_nvs.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "PrjCfg_JSON";
// END --- Logging related variables

bool nvs_cfg_changed = false;

void PrjCfg_config_parse_json_dict(cJSON *root)
{
    cJSON *nvi = NULL;

    cJSON_ArrayForEach(nvi, root)
    {
        int len = strlen(nvi->string);

        ESP_LOGI(TAG, "string %s len %d", nvi->string, len);

        bool found_key = false;
#ifdef CONFIG_PORIS_ENABLE_BLECENTRAL
        if (!found_key && strcmp(nvi->string, "rcentr") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.central_role)
            {
                PrjCfg_dre.central_role = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_BLEPERIPHERAL
        if (!found_key && strcmp(nvi->string, "rperiph") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.peripheral_role)
            {
                PrjCfg_dre.peripheral_role = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
        if (!found_key && strcmp(nvi->string, "echo") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.echo)
            {
                PrjCfg_dre.echo = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
        if (!found_key && strcmp(nvi->string, "otaskip") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.skip_ota)
            {
                PrjCfg_dre.skip_ota = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTBRIDGE
        if (!found_key && strcmp(nvi->string, "uartbr") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_bridge)
            {
                PrjCfg_dre.uart_bridge = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTUSER
        if (!found_key && strcmp(nvi->string, "uuport") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            uart_port_t value = (uart_port_t)nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_user_port)
            {
                PrjCfg_dre.uart_user_port = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uubaud") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_user_baudrate)
            {
                PrjCfg_dre.uart_user_baudrate = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uustopb") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            uart_stop_bits_t value = (uart_stop_bits_t)nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_user_stop_bits)
            {
                PrjCfg_dre.uart_user_stop_bits = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uurx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_user_rx_io_num)
            {
                PrjCfg_dre.uart_user_rx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uutx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_user_tx_io_num)
            {
                PrjCfg_dre.uart_user_tx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

#endif
#ifdef CONFIG_PORIS_ENABLE_UARTPERIPH
        if (!found_key && strcmp(nvi->string, "upport") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            uart_port_t value = (uart_port_t)nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_periph_port)
            {
                PrjCfg_dre.uart_periph_port = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "upbaud") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_periph_baudrate)
            {
                PrjCfg_dre.uart_periph_baudrate = value;
                ESP_LOGI(TAG,"Value is stored as %d", PrjCfg_dre.uart_periph_baudrate);
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "upstopb") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = (int)nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_periph_stop_bits)
            {
                PrjCfg_dre.uart_periph_stop_bits = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uprx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_periph_rx_io_num)
            {
                PrjCfg_dre.uart_periph_rx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uptx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_periph_tx_io_num)
            {
                PrjCfg_dre.uart_periph_tx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_BLINK
        if (!found_key && strcmp(nvi->string, "blink") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.blink_enabled)
            {
                PrjCfg_dre.blink_enabled = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "blinkpin") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.blink_io_num)
            {
                PrjCfg_dre.blink_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "blinkval") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.blink_on_value)
            {
                PrjCfg_dre.blink_on_value = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }


        if (!found_key && strcmp(nvi->string, "blinkon") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.blink_on_ms)
            {
                PrjCfg_dre.blink_on_ms = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "blinkoff") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.blink_off_ms)
            {
                PrjCfg_dre.blink_off_ms = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
#endif

#ifdef CONFIG_PORIS_ENABLE_HOVER
        if (!found_key && strcmp(nvi->string, "uhrx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_hover_rx_io_num)
            {
                PrjCfg_dre.uart_hover_rx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }


        if (!found_key && strcmp(nvi->string, "hover") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.hover_enabled)
            {
                PrjCfg_dre.hover_enabled = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uhport") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            uart_port_t value = (uart_port_t)nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_hover_port)
            {
                PrjCfg_dre.uart_hover_port = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uhtx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_hover_tx_io_num)
            {
                PrjCfg_dre.uart_hover_tx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
#endif

    }
}

void PrjCfg_config_parse_json(const char *data)
{
    nvs_cfg_changed = false;

    cJSON *root = cJSON_Parse(data);

    if (root != NULL)
    {
        if (data[0] == '[')
        {
            cJSON *nvi = NULL;
            cJSON_ArrayForEach(nvi, root)
            {
                PrjCfg_config_parse_json_dict(nvi);
            }
        }
        else
        {
            PrjCfg_config_parse_json_dict(root);
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            PrjCfg_nvs_cfg_save();
        }
    }
}

void PrjCfg_append_config_data(cJSON *root)
{
    cJSON_AddStringToObject(root, "ip", PrjCfg_dre.ip_address);
    cJSON_AddStringToObject(root, "id", PrjCfg_dre.unique_id);
#ifdef CONFIG_PORIS_ENABLE_BLECENTRAL
    cJSON_AddBoolToObject(root, "rcentr", PrjCfg_dre.central_role);
#endif
#ifdef CONFIG_PORIS_ENABLE_BLEPERIPHERAL
    cJSON_AddBoolToObject(root, "rperiph", PrjCfg_dre.peripheral_role);
    cJSON_AddBoolToObject(root, "echo", PrjCfg_dre.echo);
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
    cJSON_AddBoolToObject(root, "otaskip", PrjCfg_dre.skip_ota);
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTBRIDGE
    cJSON_AddBoolToObject(root, "uartbr", PrjCfg_dre.uart_bridge);
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTUSER
    cJSON_AddNumberToObject(root, "uuport", PrjCfg_dre.uart_user_port);
    cJSON_AddNumberToObject(root, "uubaud", PrjCfg_dre.uart_user_baudrate);
    cJSON_AddNumberToObject(root, "uustopb", PrjCfg_dre.uart_user_stop_bits);
    cJSON_AddNumberToObject(root, "uurx", PrjCfg_dre.uart_user_rx_io_num);
    cJSON_AddNumberToObject(root, "uutx", PrjCfg_dre.uart_user_tx_io_num);
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTPERIPH
    cJSON_AddNumberToObject(root, "upport", PrjCfg_dre.uart_periph_port);
    cJSON_AddNumberToObject(root, "upbaud", PrjCfg_dre.uart_periph_baudrate);
    cJSON_AddNumberToObject(root, "upstopb", PrjCfg_dre.uart_periph_stop_bits);
    cJSON_AddNumberToObject(root, "uprx", PrjCfg_dre.uart_periph_rx_io_num);
    cJSON_AddNumberToObject(root, "uptx", PrjCfg_dre.uart_periph_tx_io_num);
 #endif
 #ifdef CONFIG_PORIS_ENABLE_BLINK
    cJSON_AddBoolToObject(root, "blink", PrjCfg_dre.blink_enabled);
    cJSON_AddNumberToObject(root, "blinkpin", PrjCfg_dre.blink_io_num);
    cJSON_AddBoolToObject(root, "blinkval", PrjCfg_dre.blink_on_value);
    cJSON_AddNumberToObject(root, "blinkon", PrjCfg_dre.blink_on_ms);
    cJSON_AddNumberToObject(root, "blinkoff", PrjCfg_dre.blink_off_ms);
#endif
#ifdef CONFIG_PORIS_ENABLE_HOVER
    cJSON_AddBoolToObject(root, "hover", PrjCfg_dre.hover_enabled);
    cJSON_AddNumberToObject(root, "uhport", PrjCfg_dre.uart_hover_port);
    cJSON_AddNumberToObject(root, "uhrx", PrjCfg_dre.uart_hover_rx_io_num);
    cJSON_AddNumberToObject(root, "uhtx", PrjCfg_dre.uart_hover_tx_io_num);
#endif
}
