#include <string.h>
#include <Measurement.h>
#include "Measurement_netvars.h"


#include <esp_log.h>

const static char TAG[] = "Measurement_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// MEASUREMENT_t MEASUREMENT;
extern Measurement_dre_t Measurement_dre;

const NetVars_desc_t Measurement_netvars_desc[] = {
#include "Measurement_netvars_fragment.c_"
};

const size_t Measurement_netvars_count = sizeof(Measurement_netvars_desc) / sizeof(Measurement_netvars_desc[0]);

void Measurement_netvars_append_json(cJSON *root)
{
    NetVars_append_json(Measurement_netvars_desc, Measurement_netvars_count, root);
}

bool Measurement_netvars_parse_json_dict(cJSON *root)
{
    return NetVars_parse_json_dict(Measurement_netvars_desc, Measurement_netvars_count, root);
}

void Measurement_netvars_nvs_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("Measurement", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the load
        NetVars_nvs_load(Measurement_netvars_desc, Measurement_netvars_count, my_handle);
        // Close
        nvs_close(my_handle);
    }
}

void Measurement_nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("Measurement", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        // Implement the save
        NetVars_nvs_save(Measurement_netvars_desc, Measurement_netvars_count, my_handle);
        // Close
        nvs_close(my_handle);
    }
}

void Measurement_config_parse_json(const char *data)
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
                nvs_cfg_changed = Measurement_netvars_parse_json_dict(nvi);
            }
        }
        else
        {
            nvs_cfg_changed = Measurement_netvars_parse_json_dict(root);
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            Measurement_nvs_cfg_save();
        }
    }
}