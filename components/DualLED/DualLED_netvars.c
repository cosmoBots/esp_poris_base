#include <string.h>
#include <DualLED.h>
#include "DualLED_netvars.h"


#include <esp_log.h>
#include <cJSON.h>
#include <nvs.h>

const static char TAG[] = "DualLED_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// DualLED_dre_t DualLED_dre;
extern DualLED_dre_t DualLED_dre;

const NetVars_desc_t DualLED_netvars_desc[] = {
#include "DualLED_netvars_fragment.c_"
};

const size_t DualLED_netvars_count = sizeof(DualLED_netvars_desc) / sizeof(DualLED_netvars_desc[0]);

void DualLED_netvars_append_json(cJSON *root)
{
    if (DualLED_netvars_count > 0)
    {
        NetVars_append_json(DualLED_netvars_desc, DualLED_netvars_count, root);
    }
}

bool DualLED_netvars_parse_json_dict(cJSON *root)
{
    if (DualLED_netvars_count > 0)
    {
        return NetVars_parse_json_dict(DualLED_netvars_desc, DualLED_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void DualLED_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("DualLED", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        if (DualLED_netvars_count > 0)
        {
            NetVars_nvs_load(DualLED_netvars_desc, DualLED_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void DualLED_nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("DualLED", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        if (DualLED_netvars_count > 0)
        {
            NetVars_nvs_save(DualLED_netvars_desc, DualLED_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void DualLED_config_parse_json(const char *data)
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
                nvs_cfg_changed = DualLED_netvars_parse_json_dict(nvi);
            }
        }
        else
        {
            nvs_cfg_changed = DualLED_netvars_parse_json_dict(root);
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            DualLED_nvs_cfg_save();
        }
    }
}
