#include <string.h>
#include <DualLedTester.h>
#include "DualLedTester_netvars.h"


#include <esp_log.h>
#include <cJSON.h>
#include <nvs.h>

const static char TAG[] = "DualLedTester_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// DualLedTester_dre_t DualLedTester_dre;
extern DualLedTester_dre_t DualLedTester_dre;

const NetVars_desc_t DualLedTester_netvars_desc[] = {
#include "DualLedTester_netvars_fragment.c_"
};

const size_t DualLedTester_netvars_count = sizeof(DualLedTester_netvars_desc) / sizeof(DualLedTester_netvars_desc[0]);

void DualLedTester_netvars_append_json(cJSON *root)
{
    if (DualLedTester_netvars_count > 0)
    {
        NetVars_append_json(DualLedTester_netvars_desc, DualLedTester_netvars_count, root);
    }
}

bool DualLedTester_netvars_parse_json_dict(cJSON *root)
{
    if (DualLedTester_netvars_count > 0)
    {
        return NetVars_parse_json_dict(DualLedTester_netvars_desc, DualLedTester_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void DualLedTester_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("DualLedTester", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        if (DualLedTester_netvars_count > 0)
        {
            NetVars_nvs_load(DualLedTester_netvars_desc, DualLedTester_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void DualLedTester_nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("DualLedTester", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        if (DualLedTester_netvars_count > 0)
        {
            NetVars_nvs_save(DualLedTester_netvars_desc, DualLedTester_netvars_count, my_handle);
        }
        // Close
        nvs_close(my_handle);
    }
}

void DualLedTester_config_parse_json(const char *data)
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
                nvs_cfg_changed = DualLedTester_netvars_parse_json_dict(nvi);
            }
        }
        else
        {
            nvs_cfg_changed = DualLedTester_netvars_parse_json_dict(root);
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            DualLedTester_nvs_cfg_save();
        }
    }
}
