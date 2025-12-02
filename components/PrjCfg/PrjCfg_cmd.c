#include <string.h>

#include <cJSON.h>

#include <nvs_flash.h>
#include <esp_system.h>
#include <esp_log.h>

#include "PrjCfg.h"

#include <OTA.h>

static char TAG[] = "PrjCfg_cmd";

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


void config_parse_json_dict(cJSON *root)
{
    cJSON *nvi = NULL;

    cJSON_ArrayForEach(nvi, root)
    {
        int len = strlen(nvi->string);

        ESP_LOGI(TAG, "string %s len %d", nvi->string, len);

        bool found_key = false;

        if (!found_key && strcmp(nvi->string, "dummy") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            found_key = true;
        }

#if 0
        if (!found_key && strcmp(nvi->string, "rcentr") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.central_role)
            {
                prjcfg.central_role = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
        if (!found_key && strcmp(nvi->string, "rperiph") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.peripheral_role)
            {
                prjcfg.peripheral_role = value;
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
                prjcfg.echo = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
        if (!found_key && strcmp(nvi->string, "otaskip") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.skip_ota)
            {
                prjcfg.skip_ota = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
        if (!found_key && strcmp(nvi->string, "uartbr") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_bridge)
            {
                prjcfg.uart_bridge = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uuport") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            uart_port_t value = (uart_port_t)nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_user_port)
            {
                prjcfg.uart_user_port = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "upport") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            uart_port_t value = (uart_port_t)nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != PrjCfg_dre.uart_periph_port)
            {
                prjcfg.uart_periph_port = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uubaud") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_user_baudrate)
            {
                prjcfg.uart_user_baudrate = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "upbaud") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_periph_baudrate)
            {
                prjcfg.uart_periph_baudrate = value;
                ESP_LOGI(TAG,"Value is stored as %d", prjcfg.uart_periph_baudrate);
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uustopb") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            uart_stop_bits_t value = (uart_stop_bits_t)nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_user_stop_bits)
            {
                prjcfg.uart_user_stop_bits = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
        if (!found_key && strcmp(nvi->string, "upstopb") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = (int)nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_periph_stop_bits)
            {
                prjcfg.uart_periph_stop_bits = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uurx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_user_rx_io_num)
            {
                prjcfg.uart_user_rx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uprx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_periph_rx_io_num)
            {
                prjcfg.uart_periph_rx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uutx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_user_tx_io_num)
            {
                prjcfg.uart_user_tx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uptx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_periph_tx_io_num)
            {
                prjcfg.uart_periph_tx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "blink") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.blink_enabled)
            {
                prjcfg.blink_enabled = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "blinkpin") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.blink_io_num)
            {
                prjcfg.blink_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "blinkval") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.blink_on_value)
            {
                prjcfg.blink_on_value = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }


        if (!found_key && strcmp(nvi->string, "blinkon") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.blink_on_ms)
            {
                prjcfg.blink_on_ms = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "blinkoff") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.blink_off_ms)
            {
                prjcfg.blink_off_ms = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }


        if (!found_key && strcmp(nvi->string, "uhrx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_hover_rx_io_num)
            {
                prjcfg.uart_hover_rx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }


        if (!found_key && strcmp(nvi->string, "hover") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.hover_enabled)
            {
                prjcfg.hover_enabled = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uhport") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            uart_port_t value = (uart_port_t)nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_hover_port)
            {
                prjcfg.uart_hover_port = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }

        if (!found_key && strcmp(nvi->string, "uhtx") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            int value = nvi->valueint;
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            if (value != prjcfg.uart_hover_tx_io_num)
            {
                prjcfg.uart_hover_tx_io_num = value;
                nvs_cfg_changed = true;
            }
            found_key = true;
        }
#endif

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

void append_config_data(cJSON *root)
{
    cJSON_AddBoolToObject(root, "dummy", true);
#if 0
    cJSON_AddBoolToObject(root, "rcentr", prjcfg.central_role);
    cJSON_AddBoolToObject(root, "rperiph", prjcfg.peripheral_role);
    cJSON_AddBoolToObject(root, "echo", prjcfg.echo);
    cJSON_AddBoolToObject(root, "otaskip", prjcfg.skip_ota);
    cJSON_AddBoolToObject(root, "uartbr", prjcfg.uart_bridge);
    cJSON_AddStringToObject(root, "ip", prjcfg.ip_address);

    cJSON_AddNumberToObject(root, "uuport", prjcfg.uart_user_port);
    cJSON_AddNumberToObject(root, "uubaud", prjcfg.uart_user_baudrate);
    cJSON_AddNumberToObject(root, "uustopb", prjcfg.uart_user_stop_bits);
    cJSON_AddNumberToObject(root, "uurx", prjcfg.uart_user_rx_io_num);
    cJSON_AddNumberToObject(root, "uutx", prjcfg.uart_user_tx_io_num);

    cJSON_AddNumberToObject(root, "upport", prjcfg.uart_periph_port);
    cJSON_AddNumberToObject(root, "upbaud", prjcfg.uart_periph_baudrate);
    cJSON_AddNumberToObject(root, "upstopb", prjcfg.uart_periph_stop_bits);
    cJSON_AddNumberToObject(root, "uprx", prjcfg.uart_periph_rx_io_num);
    cJSON_AddNumberToObject(root, "uptx", prjcfg.uart_periph_tx_io_num);
 
    cJSON_AddBoolToObject(root, "blink", prjcfg.blink_enabled);
    cJSON_AddNumberToObject(root, "blinkpin", prjcfg.blink_io_num);
    cJSON_AddBoolToObject(root, "blinkval", prjcfg.blink_on_value);
    cJSON_AddNumberToObject(root, "blinkon", prjcfg.blink_on_ms);
    cJSON_AddNumberToObject(root, "blinkoff", prjcfg.blink_off_ms);

    cJSON_AddBoolToObject(root, "hover", prjcfg.hover_enabled);
    cJSON_AddNumberToObject(root, "uhport", prjcfg.uart_hover_port);
    cJSON_AddNumberToObject(root, "uhrx", prjcfg.uart_hover_rx_io_num);
    cJSON_AddNumberToObject(root, "uhtx", prjcfg.uart_hover_tx_io_num);    
#endif
}


void prjcfg_parse_callback(const char *data, int len)
{
    ESP_LOGI(TAG, "Parsing the CFG payload %d %.*s", len, len, data);
    config_parse_json(data);
}

void prjcfg_req_parse_callback(const char *data, int len)
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

uint32_t counter = 0;
void prjcfg_compose_callback(char *data, int *len)
{
    sprintf((char *)data, "this is a payload (%lu)", counter++);
    *len = strlen((char *)data);
    ESP_LOGI(TAG, "Composing the DATA payload %d %.*s", *len, *len, data);

    cJSON *root = cJSON_CreateObject();
    append_config_data(root);

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
