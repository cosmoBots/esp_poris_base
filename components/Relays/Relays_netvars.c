#include <string.h>
#include <Relays.h>
#include "Relays_netvars.h"


#include <esp_log.h>
#include <cJSON.h>
#include <nvs.h>

const static char TAG[] = "Relays_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// Relays_dre_t Relays_dre;
extern Relays_dre_t Relays_dre;

const NetVars_desc_t Relays_netvars_desc[] = {
#include "Relays_netvars_fragment.c_"
};

const size_t Relays_netvars_count = sizeof(Relays_netvars_desc) / sizeof(Relays_netvars_desc[0]);

void Relays_netvars_append_json(cJSON *root)
{
    if (Relays_netvars_count > 0)
    {
        cJSON *sub = cJSON_GetObjectItemCaseSensitive(root, "Relays");
        if (!sub)
        {
            sub = cJSON_AddObjectToObject(root, "Relays");
        }
        if (sub)
        {
            NetVars_append_json(Relays_netvars_desc, Relays_netvars_count, sub);
        }
    }
}

bool Relays_netvars_parse_json_dict(cJSON *root)
{
    if (Relays_netvars_count > 0)
    {
        return NetVars_parse_json_dict(Relays_netvars_desc, Relays_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void Relays_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("Relays", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        if (Relays_netvars_count > 0)
        {
            NetVars_nvs_load(Relays_netvars_desc, Relays_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void Relays_nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("Relays", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        if (Relays_netvars_count > 0)
        {
            NetVars_nvs_save(Relays_netvars_desc, Relays_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void Relays_config_parse_json(const char *data)
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
            cJSON *sub = cJSON_GetObjectItemCaseSensitive(nvi, "Relays");
            if (sub)
            {
                nvs_cfg_changed = Relays_netvars_parse_json_dict(sub);
            }
        }
        }
        else
        {
            cJSON *sub = cJSON_GetObjectItemCaseSensitive(root, "Relays");
            if (sub)
            {
                nvs_cfg_changed = Relays_netvars_parse_json_dict(sub);
            }
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            Relays_nvs_cfg_save();
        }
    }
}
