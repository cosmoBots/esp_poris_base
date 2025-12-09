#include <string.h>
#include <MQTTComm.h>
#include "MQTTComm_netvars.h"


#include <esp_log.h>
#include <cJSON.h>
#include <nvs.h>

const static char TAG[] = "MQTTComm_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// MQTTComm_dre_t MQTTComm_dre;
extern MQTTComm_dre_t MQTTComm_dre;

const NetVars_desc_t MQTTComm_netvars_desc[] = {
#include "MQTTComm_netvars_fragment.c_"
};

const size_t MQTTComm_netvars_count = sizeof(MQTTComm_netvars_desc) / sizeof(MQTTComm_netvars_desc[0]);

void MQTTComm_netvars_append_json(cJSON *root)
{
    if (MQTTComm_netvars_count > 0)
    {
        NetVars_append_json(MQTTComm_netvars_desc, MQTTComm_netvars_count, root);
    }
}

bool MQTTComm_netvars_parse_json_dict(cJSON *root)
{
    if (MQTTComm_netvars_count > 0)
    {
        return NetVars_parse_json_dict(MQTTComm_netvars_desc, MQTTComm_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void MQTTComm_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("MQTTComm", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        if (MQTTComm_netvars_count > 0)
        {
            NetVars_nvs_load(MQTTComm_netvars_desc, MQTTComm_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void MQTTComm_nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("MQTTComm", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        if (MQTTComm_netvars_count > 0)
        {
            NetVars_nvs_save(MQTTComm_netvars_desc, MQTTComm_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void MQTTComm_config_parse_json(const char *data)
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
                nvs_cfg_changed = MQTTComm_netvars_parse_json_dict(nvi);
            }
        }
        else
        {
            nvs_cfg_changed = MQTTComm_netvars_parse_json_dict(root);
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            MQTTComm_nvs_cfg_save();
        }
    }
}
