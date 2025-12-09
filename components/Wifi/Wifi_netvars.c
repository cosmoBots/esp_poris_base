#include <string.h>
#include <Wifi.h>
#include "Wifi_netvars.h"

static netvars_nvs_mgr_t Wifi_nvs_mgr = {0};

const NetVars_desc_t Wifi_netvars_desc[] = {
#include "Wifi_netvars_fragment.c_"
};

const size_t Wifi_netvars_count = sizeof(Wifi_netvars_desc) / sizeof(Wifi_netvars_desc[0]);

void Wifi_netvars_append_json(cJSON *root)
{
    if (Wifi_netvars_count > 0)
    {
        NetVars_append_json_component("Wifi", Wifi_netvars_desc, Wifi_netvars_count, root);
    }
}

bool Wifi_netvars_parse_json_dict(cJSON *root)
{
    if (Wifi_netvars_count > 0)
    {
        return NetVars_parse_json_dict(Wifi_netvars_desc, Wifi_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void Wifi_netvars_nvs_load(void)
{
    if (Wifi_netvars_count > 0)
    {
        NetVars_nvs_load_component("Wifi", Wifi_netvars_desc, Wifi_netvars_count);
    }
}

void Wifi_netvars_nvs_save(void)
{
    if (Wifi_netvars_count > 0)
    {
        NetVars_nvs_save_component("Wifi", Wifi_netvars_desc, Wifi_netvars_count);
    }
}

void Wifi_config_parse_json(const char *data)
{
    if (Wifi_netvars_count > 0)
    {
        bool nvs_cfg_changed = NetVars_parse_json_component_data("Wifi", Wifi_netvars_desc, Wifi_netvars_count, data);
        if (nvs_cfg_changed)
        {
            Wifi_nvs_set_dirty();
        }
    }
}

void Wifi_nvs_set_dirty(void)
{
    NetVars_nvs_set_dirty(&Wifi_nvs_mgr);
}

void Wifi_nvs_spin(void)
{
    if (NetVars_nvs_spin(&Wifi_nvs_mgr))
    {
        Wifi_netvars_nvs_save();
    }
}
