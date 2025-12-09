#include <string.h>
#include <OTA.h>
#include "OTA_netvars.h"


#include <esp_log.h>
#include <cJSON.h>
#include <nvs.h>

const static char TAG[] = "OTA_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// OTA_dre_t OTA_dre;
extern OTA_dre_t OTA_dre;

const NetVars_desc_t OTA_netvars_desc[] = {
#include "OTA_netvars_fragment.c_"
};

const size_t OTA_netvars_count = sizeof(OTA_netvars_desc) / sizeof(OTA_netvars_desc[0]);

void OTA_netvars_append_json(cJSON *root)
{
    if (OTA_netvars_count > 0)
    {
        NetVars_append_json(OTA_netvars_desc, OTA_netvars_count, root);
    }
}

bool OTA_netvars_parse_json_dict(cJSON *root)
{
    if (OTA_netvars_count > 0)
    {
        return NetVars_parse_json_dict(OTA_netvars_desc, OTA_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void OTA_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("OTA", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        if (OTA_netvars_count > 0)
        {
            NetVars_nvs_load(OTA_netvars_desc, OTA_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void OTA_nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("OTA", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        if (OTA_netvars_count > 0)
        {
            NetVars_nvs_save(OTA_netvars_desc, OTA_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void OTA_config_parse_json(const char *data)
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
                nvs_cfg_changed = OTA_netvars_parse_json_dict(nvi);
            }
        }
        else
        {
            nvs_cfg_changed = OTA_netvars_parse_json_dict(root);
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            OTA_nvs_cfg_save();
        }
    }
}
