#include <string.h>
#include <DualLED.h>
#include "DualLED_netvars.h"


// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// DualLED_dre_t DualLED_dre;
extern DualLED_dre_t DualLED_dre;
static netvars_nvs_mgr_t DualLED_nvs_mgr = {0};

const NetVars_desc_t DualLED_netvars_desc[] = {
#include "DualLED_netvars_fragment.c_"
};

const size_t DualLED_netvars_count = sizeof(DualLED_netvars_desc) / sizeof(DualLED_netvars_desc[0]);

void DualLED_netvars_append_json(cJSON *root)
{
    if (DualLED_netvars_count > 0)
    {
        NetVars_append_json_component("DualLED", DualLED_netvars_desc, DualLED_netvars_count, root);
    }
}

bool DualLED_netvars_parse_json_dict(cJSON *root)
{
    if (DualLED_netvars_count > 0)
    {
        return NetVars_parse_json_dict(DualLED_netvars_desc, DualLED_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void DualLED_netvars_nvs_load(void)
{
    if (DualLED_netvars_count > 0)
    {
        NetVars_nvs_load_component("DualLED", DualLED_netvars_desc, DualLED_netvars_count);
    }
}

void DualLED_netvars_nvs_save(void)
{
    if (DualLED_netvars_count > 0)
    {
        NetVars_nvs_save_component("DualLED", DualLED_netvars_desc, DualLED_netvars_count);
    }
}

void DualLED_config_parse_json(const char *data)
{
    if (DualLED_netvars_count > 0)
    {    
        bool nvs_cfg_changed = NetVars_parse_json_component_data("DualLED", DualLED_netvars_desc, DualLED_netvars_count, data);
        if (nvs_cfg_changed)
        {
            DualLED_nvs_set_dirty();
        }
    }
}

void DualLED_nvs_set_dirty(void)
{
    NetVars_nvs_set_dirty(&DualLED_nvs_mgr);
}

void DualLED_nvs_spin(void)
{
    if (NetVars_nvs_spin(&DualLED_nvs_mgr))
    {
        DualLED_netvars_nvs_save();
    }
}
