#include <string.h>
#include <Measurement.h>
#include "Measurement_netvars.h"

static netvars_nvs_mgr_t Measurement_nvs_mgr = {0};

const NetVars_desc_t Measurement_netvars_desc[] = {
#include "Measurement_netvars_fragment.c_"
};

const size_t Measurement_netvars_count = sizeof(Measurement_netvars_desc) / sizeof(Measurement_netvars_desc[0]);

void Measurement_netvars_append_json(cJSON *root)
{
    if (Measurement_netvars_count > 0)
    {
        NetVars_append_json_component("Measurement", Measurement_netvars_desc, Measurement_netvars_count, root);
    }
}

void Measurement_netvars_nvs_load(void)
{
    if (Measurement_netvars_count > 0)
    {
        NetVars_nvs_load_component("Measurement", Measurement_netvars_desc, Measurement_netvars_count);
    }
}

void Measurement_netvars_nvs_save(void)
{
    if (Measurement_netvars_count > 0)
    {
        NetVars_nvs_save_component("Measurement", Measurement_netvars_desc, Measurement_netvars_count);
    }
}

void Measurement_config_parse_json(const char *data)
{
    if (Measurement_netvars_count > 0)
    {
        bool nvs_cfg_changed = NetVars_parse_json_component_data("Measurement", Measurement_netvars_desc, Measurement_netvars_count, data);
        if (nvs_cfg_changed)
        {
            Measurement_nvs_set_dirty();
        }
    }
}

void Measurement_nvs_set_dirty(void)
{
    NetVars_nvs_set_dirty(&Measurement_nvs_mgr);
}

void Measurement_nvs_spin(void)
{
    if (NetVars_nvs_spin(&Measurement_nvs_mgr))
    {
        Measurement_netvars_nvs_save();
    }
}
