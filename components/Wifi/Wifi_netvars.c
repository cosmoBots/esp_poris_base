#include <string.h>
#include <Wifi.h>
#include "Wifi_netvars.h"


#include <esp_log.h>
#include <cJSON.h>
#include <nvs.h>

const static char TAG[] = "Wifi_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// Wifi_dre_t Wifi_dre;
extern Wifi_dre_t Wifi_dre;

const NetVars_desc_t Wifi_netvars_desc[] = {
#include "Wifi_netvars_fragment.c_"
};

const size_t Wifi_netvars_count = sizeof(Wifi_netvars_desc) / sizeof(Wifi_netvars_desc[0]);

void Wifi_netvars_append_json(cJSON *root)
{
    if (Wifi_netvars_count > 0)
    {
        cJSON *sub = cJSON_GetObjectItemCaseSensitive(root, "Wifi");
        if (!sub)
        {
            sub = cJSON_AddObjectToObject(root, "Wifi");
        }
        if (sub)
        {
            NetVars_append_json(Wifi_netvars_desc, Wifi_netvars_count, sub);
        }
    }
}

bool Wifi_netvars_parse_json_dict(cJSON *root)
{
    if (Wifi_netvars_count > 0)
    {
        return NetVars_parse_json_dict(Wifi_netvars_desc, Wifi_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void Wifi_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("Wifi", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        if (Wifi_netvars_count > 0)
        {
            NetVars_nvs_load(Wifi_netvars_desc, Wifi_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void Wifi_netvars_nvs_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("Wifi", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        if (Wifi_netvars_count > 0)
        {
            NetVars_nvs_save(Wifi_netvars_desc, Wifi_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void Wifi_config_parse_json(const char *data)
{
    bool nvs_cfg_changed = false;

    cJSON *root = cJSON_Parse(data);

    if (root != NULL)
    {
        if (data[0] == '[')
        {
            cJSON *nvi = NULL;
            cJSON_ArrayForEach(nvi, root)
        {
            cJSON *sub = cJSON_GetObjectItemCaseSensitive(nvi, "Wifi");
            if (sub)
            {
                nvs_cfg_changed = Wifi_netvars_parse_json_dict(sub);
            }
        }
        }
        else
        {
            cJSON *sub = cJSON_GetObjectItemCaseSensitive(root, "Wifi");
            if (sub)
            {
                nvs_cfg_changed = Wifi_netvars_parse_json_dict(sub);
            }
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            Wifi_netvars_nvs_save();
        }
    }
}
