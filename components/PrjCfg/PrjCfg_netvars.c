#include <string.h>
#include <PrjCfg.h>
#include "PrjCfg_netvars.h"


#include <esp_log.h>

const static char TAG[] = "PrjCfg_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// prjcfg_t prjcfg;
extern PrjCfg_dre_t PrjCfg_dre;

const NetVars_desc_t PrjCfg_netvars_desc[] = {
#include "PrjCfg_netvars_fragment.c_"
};

const size_t PrjCfg_netvars_count = sizeof(PrjCfg_netvars_desc) / sizeof(PrjCfg_netvars_desc[0]);

void PrjCfg_netvars_append_json(cJSON *root)
{
    NetVars_append_json(PrjCfg_netvars_desc, PrjCfg_netvars_count, root);
}

bool PrjCfg_netvars_parse_json_dict(cJSON *root)
{
    return NetVars_parse_json_dict(PrjCfg_netvars_desc, PrjCfg_netvars_count, root);
}

void PrjCfg_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("PrjCfg", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        NetVars_nvs_load(PrjCfg_netvars_desc, PrjCfg_netvars_count, my_handle);
        // Close
        nvs_close(my_handle);
    }
}

void PrjCfg_nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("PrjCfg", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        NetVars_nvs_save(PrjCfg_netvars_desc, PrjCfg_netvars_count, my_handle);
        // Close
        nvs_close(my_handle);
    }
}

void PrjCfg_config_parse_json(const char *data)
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
                nvs_cfg_changed = PrjCfg_netvars_parse_json_dict(nvi);
            }
        }
        else
        {
            nvs_cfg_changed = PrjCfg_netvars_parse_json_dict(root);
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            PrjCfg_nvs_cfg_save();
        }
    }
}