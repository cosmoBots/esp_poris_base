#include <string.h>
#include <UDPComm.h>
#include "UDPComm_netvars.h"


#include <esp_log.h>
#include <cJSON.h>
#include <nvs.h>

const static char TAG[] = "UDPComm_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// UDPComm_dre_t UDPComm_dre;
extern UDPComm_dre_t UDPComm_dre;

const NetVars_desc_t UDPComm_netvars_desc[] = {
#include "UDPComm_netvars_fragment.c_"
};

const size_t UDPComm_netvars_count = sizeof(UDPComm_netvars_desc) / sizeof(UDPComm_netvars_desc[0]);

void UDPComm_netvars_append_json(cJSON *root)
{
    if (UDPComm_netvars_count > 0)
    {
        cJSON *sub = cJSON_GetObjectItemCaseSensitive(root, "UDPComm");
        if (!sub)
        {
            sub = cJSON_AddObjectToObject(root, "UDPComm");
        }
        if (sub)
        {
            NetVars_append_json(UDPComm_netvars_desc, UDPComm_netvars_count, sub);
        }
    }
}

bool UDPComm_netvars_parse_json_dict(cJSON *root)
{
    if (UDPComm_netvars_count > 0)
    {
        return NetVars_parse_json_dict(UDPComm_netvars_desc, UDPComm_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void UDPComm_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("UDPComm", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        if (UDPComm_netvars_count > 0)
        {
            NetVars_nvs_load(UDPComm_netvars_desc, UDPComm_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void UDPComm_netvars_nvs_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("UDPComm", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        if (UDPComm_netvars_count > 0)
        {
            NetVars_nvs_save(UDPComm_netvars_desc, UDPComm_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void UDPComm_config_parse_json(const char *data)
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
            cJSON *sub = cJSON_GetObjectItemCaseSensitive(nvi, "UDPComm");
            if (sub)
            {
                nvs_cfg_changed = UDPComm_netvars_parse_json_dict(sub);
            }
        }
        }
        else
        {
            cJSON *sub = cJSON_GetObjectItemCaseSensitive(root, "UDPComm");
            if (sub)
            {
                nvs_cfg_changed = UDPComm_netvars_parse_json_dict(sub);
            }
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            UDPComm_netvars_nvs_save();
        }
    }
}
