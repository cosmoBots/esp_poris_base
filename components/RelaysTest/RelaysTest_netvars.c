#include <string.h>
#include <RelaysTest.h>
#include "RelaysTest_netvars.h"


#include <esp_log.h>
#include <cJSON.h>
#include <nvs.h>

const static char TAG[] = "RelaysTest_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// RelaysTest_dre_t RelaysTest_dre;
extern RelaysTest_dre_t RelaysTest_dre;

const NetVars_desc_t RelaysTest_netvars_desc[] = {
#include "RelaysTest_netvars_fragment.c_"
};

const size_t RelaysTest_netvars_count = sizeof(RelaysTest_netvars_desc) / sizeof(RelaysTest_netvars_desc[0]);

void RelaysTest_netvars_append_json(cJSON *root)
{
    if (RelaysTest_netvars_count > 0)
    {
        cJSON *sub = cJSON_GetObjectItemCaseSensitive(root, "RelaysTest");
        if (!sub)
        {
            sub = cJSON_AddObjectToObject(root, "RelaysTest");
        }
        if (sub)
        {
            NetVars_append_json(RelaysTest_netvars_desc, RelaysTest_netvars_count, sub);
        }
    }
}

bool RelaysTest_netvars_parse_json_dict(cJSON *root)
{
    if (RelaysTest_netvars_count > 0)
    {
        return NetVars_parse_json_dict(RelaysTest_netvars_desc, RelaysTest_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void RelaysTest_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("RelaysTest", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        if (RelaysTest_netvars_count > 0)
        {
            NetVars_nvs_load(RelaysTest_netvars_desc, RelaysTest_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void RelaysTest_nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("RelaysTest", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        if (RelaysTest_netvars_count > 0)
        {
            NetVars_nvs_save(RelaysTest_netvars_desc, RelaysTest_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void RelaysTest_config_parse_json(const char *data)
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
            cJSON *sub = cJSON_GetObjectItemCaseSensitive(nvi, "RelaysTest");
            if (sub)
            {
                nvs_cfg_changed = RelaysTest_netvars_parse_json_dict(sub);
            }
        }
        }
        else
        {
            cJSON *sub = cJSON_GetObjectItemCaseSensitive(root, "RelaysTest");
            if (sub)
            {
                nvs_cfg_changed = RelaysTest_netvars_parse_json_dict(sub);
            }
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            RelaysTest_nvs_cfg_save();
        }
    }
}
