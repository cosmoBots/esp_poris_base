// Standard C
#include <stdio.h>
#include <string.h>

// FreeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ESP components
#include <esp_log.h>
#include <driver/gpio.h>
#include <nvs_flash.h>
#include <cJSON.h>

#include <esp_mac.h>

#include "PrjCfg.h"

static char TAG[] = "PrjCfg";

prjcfg_t prjcfg;

void nvs_cfg_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("cfg", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        PrjCfg_netvars_nvs_load(my_handle);
        // Close
        nvs_close(my_handle);
    }
}

void nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("cfg", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {

        PrjCfg_netvars_nvs_save(my_handle);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        // Close
        nvs_commit(my_handle);
        nvs_close(my_handle);
    }
}

bool nvs_cfg_changed = false;

void config_parse_json(const char *data)
{
    nvs_cfg_changed = false;

    cJSON *root = cJSON_Parse(data);
    if (root == NULL)
    {
        return;
    }

    if (data[0] == '[')
    {
        // Caso: payload es un array de objetos JSON
        cJSON *nvi = NULL;
        cJSON_ArrayForEach(nvi, root)
        {
            bool changed = false;
            PrjCfg_netvars_parse_json_dict(nvi, &changed);
            if (changed)
            {
                nvs_cfg_changed = true;
            }
        }
    }
    else
    {
        // Caso: payload es un único objeto JSON
        PrjCfg_netvars_parse_json_dict(root, &nvs_cfg_changed);
    }

    cJSON_Delete(root);

    if (nvs_cfg_changed)
    {
        nvs_cfg_save();  // dentro abrirá NVS y llamará a PrjCfg_netvars_nvs_save(h)
    }
}


void append_config_data(cJSON *root)
{
    PrjCfg_netvars_append_json(root);
}

static void get_identifiers(void)
{
    esp_read_mac(prjcfg.eth_mac, ESP_MAC_BASE);
    sprintf(prjcfg.unique_id, "%02X%02X%02X%02X%02X%02X", prjcfg.eth_mac[0], prjcfg.eth_mac[1], prjcfg.eth_mac[2], prjcfg.eth_mac[3], prjcfg.eth_mac[4], prjcfg.eth_mac[5]);
}


uint32_t counter = 0;


static bool nvs_cfg_changed = false;

void nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("cfg", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        int8_t aux;

#if 0        
        ESP_LOGI(TAG, "Writing central_role %d in NVS ... ", prjcfg.central_role);
        err = nvs_set_u8(my_handle, "rcentr", prjcfg.central_role);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing peripheral_role %d in NVS ... ", prjcfg.peripheral_role);
        err = nvs_set_u8(my_handle, "rperiph", prjcfg.peripheral_role);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing echo parameter %d in NVS ... ", prjcfg.echo);
        err = nvs_set_u8(my_handle, "echo", prjcfg.echo);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_user_port parameter %d in NVS ... ", prjcfg.uart_user_port);
        err = nvs_set_u8(my_handle, "uuport", prjcfg.uart_user_port);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_periph_port parameter %d in NVS ... ", prjcfg.uart_periph_port);
        err = nvs_set_u8(my_handle, "upport", prjcfg.uart_periph_port);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_user_baudrate parameter %d in NVS ... ", prjcfg.uart_user_baudrate);
        err = nvs_set_i32(my_handle, "uubaud", prjcfg.uart_user_baudrate);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_periph_baudrate parameter %d in NVS ... ", prjcfg.uart_periph_baudrate);
        err = nvs_set_i32(my_handle, "upbaud", prjcfg.uart_periph_baudrate);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_user_stop_bits parameter %d in NVS ... ", prjcfg.uart_user_stop_bits);
        err = nvs_set_u8(my_handle, "uustopb", prjcfg.uart_user_stop_bits);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_periph_stop_bits parameter %d in NVS ... ", prjcfg.uart_periph_stop_bits);
        err = nvs_set_u8(my_handle, "upstopb", prjcfg.uart_periph_stop_bits);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_user_rx_io_num parameter %d in NVS ... ", prjcfg.uart_user_rx_io_num);
        aux = (int8_t)prjcfg.uart_user_rx_io_num;
        err = nvs_set_i8(my_handle, "uurx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_periph_rx_io_num parameter %d in NVS ... ", prjcfg.uart_periph_rx_io_num);
        aux = (int8_t)prjcfg.uart_periph_rx_io_num;
        err = nvs_set_i8(my_handle, "uprx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_user_tx_io_num parameter %d in NVS ... ", prjcfg.uart_user_tx_io_num);
        aux = (gpio_num_t)prjcfg.uart_user_tx_io_num;
        err = nvs_set_i8(my_handle, "uutx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_periph_tx_io_num parameter %d in NVS ... ", prjcfg.uart_periph_tx_io_num);
        aux = (gpio_num_t)prjcfg.uart_periph_tx_io_num;
        err = nvs_set_i8(my_handle, "uptx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing skip_ota %d in NVS ... ", prjcfg.skip_ota);
        err = nvs_set_u8(my_handle, "otaskip", prjcfg.skip_ota);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_bridge %d in NVS ... ", prjcfg.uart_bridge);
        err = nvs_set_u8(my_handle, "uartbr", prjcfg.uart_bridge);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing blink %d in NVS ... ", prjcfg.blink_enabled);
        err = nvs_set_u8(my_handle, "blink", prjcfg.blink_enabled);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing blinkpin parameter %d in NVS ... ", prjcfg.blink_io_num);
        aux = (gpio_num_t)prjcfg.blink_io_num;
        err = nvs_set_i8(my_handle, "blinkpin", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }


        ESP_LOGI(TAG, "Writing blinkval %d in NVS ... ", prjcfg.blink_on_value);
        err = nvs_set_u8(my_handle, "blinkval", prjcfg.blink_on_value);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing blinkon parameter %lu in NVS ... ", prjcfg.blink_on_ms);
        err = nvs_set_u32(my_handle, "blinkon", prjcfg.blink_on_ms);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }


        ESP_LOGI(TAG, "Writing blinkoff parameter %lu in NVS ... ", prjcfg.blink_off_ms);
        err = nvs_set_u32(my_handle, "blinkoff", prjcfg.blink_off_ms);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing hover %d in NVS ... ", prjcfg.hover_enabled);
        err = nvs_set_u8(my_handle, "hover", prjcfg.hover_enabled);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_hover_port parameter %d in NVS ... ", prjcfg.uart_hover_port);
        err = nvs_set_u8(my_handle, "uhport", prjcfg.uart_hover_port);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }


        ESP_LOGI(TAG, "Writing uart_hover_tx_io_num parameter %d in NVS ... ", prjcfg.uart_hover_tx_io_num);
        aux = (gpio_num_t)prjcfg.uart_hover_tx_io_num;
        err = nvs_set_i8(my_handle, "uhtx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_hover_rx_io_num parameter %d in NVS ... ", prjcfg.uart_hover_rx_io_num);
        aux = (gpio_num_t)prjcfg.uart_hover_rx_io_num;
        err = nvs_set_i8(my_handle, "uhrx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }
#endif
        // Close
        nvs_close(my_handle);
    }
}

void config_parse_json(const char *data)
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
                config_parse_json_dict(nvi);
            }
        }
        else
        {
            config_parse_json_dict(root);
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            nvs_cfg_save();
        }
    }
}


void main_parse_callback(const char *data, int len)
{
    ESP_LOGI(TAG, "Parsing the CFG payload %d %.*s", len, len, data);
    config_parse_json(data);
}

void main_req_parse_callback(const char *data, int len)
{
    ESP_LOGI(TAG, "Parsing the REQ payload %d %.*s", len, len, data);
    if (len == 1)
    {
        if (data[0] == 'r')
        {
            esp_restart();
        }
        if (data[0] == 'u')
        {
            OTA_enable();
            OTA_start();
        }
    }
}


void append_main_data(cJSON *root)
{
    cJSON_AddBoolToObject(root, "dummy", true);
}

void main_compose_callback(char *data, int *len)
{
    sprintf((char *)data, "this is a payload (%lu)", counter++);
    *len = strlen((char *)data);
    ESP_LOGI(TAG, "Composing the DATA payload %d %.*s", *len, *len, data);

    cJSON *root = cJSON_CreateObject();
    append_main_data(root);

    char *cPayload = cJSON_PrintUnformatted(root);
    if (cPayload != NULL)
    {
        size_t length = strlen(cPayload);
        *len = (int)length;
        ESP_LOGI(TAG, "LEN %d", length);
        ESP_LOGI(TAG, "Payload %s", cPayload);
        strncpy(data, cPayload, length);
        free(cPayload);
    }
    else
    {
        data[0] = 'a';
        data[1] = '\0';
        *len = 1;
    }
    cJSON_Delete(root);
}
