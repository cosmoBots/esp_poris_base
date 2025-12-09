#include <string.h>
#include <Provisioning.h>
#include "Provisioning_netvars.h"


#include <esp_log.h>
#include <cJSON.h>
#include <nvs.h>

const static char TAG[] = "Provisioning_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// Provisioning_dre_t Provisioning_dre;
extern Provisioning_dre_t Provisioning_dre;

const NetVars_desc_t Provisioning_netvars_desc[] = {
#include "Provisioning_netvars_fragment.c_"
};

const size_t Provisioning_netvars_count = sizeof(Provisioning_netvars_desc) / sizeof(Provisioning_netvars_desc[0]);

void Provisioning_netvars_append_json(cJSON *root)
{
    if (Provisioning_netvars_count > 0)
    {
        cJSON *sub = cJSON_GetObjectItemCaseSensitive(root, "Provisioning");
        if (!sub)
        {
            sub = cJSON_AddObjectToObject(root, "Provisioning");
        }
        if (sub)
        {
            NetVars_append_json(Provisioning_netvars_desc, Provisioning_netvars_count, sub);
        }
    }
}

bool Provisioning_netvars_parse_json_dict(cJSON *root)
{
    if (Provisioning_netvars_count > 0)
    {
        return NetVars_parse_json_dict(Provisioning_netvars_desc, Provisioning_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void Provisioning_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("Provisioning", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        if (Provisioning_netvars_count > 0)
        {
            NetVars_nvs_load(Provisioning_netvars_desc, Provisioning_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void Provisioning_netvars_nvs_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("Provisioning", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        if (Provisioning_netvars_count > 0)
        {
            NetVars_nvs_save(Provisioning_netvars_desc, Provisioning_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void Provisioning_config_parse_json(const char *data)
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
            cJSON *sub = cJSON_GetObjectItemCaseSensitive(nvi, "Provisioning");
            if (sub)
            {
                nvs_cfg_changed = Provisioning_netvars_parse_json_dict(sub);
            }
        }
        }
        else
        {
            cJSON *sub = cJSON_GetObjectItemCaseSensitive(root, "Provisioning");
            if (sub)
            {
                nvs_cfg_changed = Provisioning_netvars_parse_json_dict(sub);
            }
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            Provisioning_netvars_nvs_save();
        }
    }
}
