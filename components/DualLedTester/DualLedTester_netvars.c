#include <string.h>
#include <DualLedTester.h>
#include "DualLedTester_netvars.h"

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// DualLedTester_dre_t DualLedTester_dre;
extern DualLedTester_dre_t DualLedTester_dre;
static netvars_nvs_mgr_t DualLedTester_nvs_mgr = {0};

const NetVars_desc_t DualLedTester_netvars_desc[] = {
#include "DualLedTester_netvars_fragment.c_"
};

const size_t DualLedTester_netvars_count = sizeof(DualLedTester_netvars_desc) / sizeof(DualLedTester_netvars_desc[0]);

void DualLedTester_netvars_append_json(cJSON *root)
{
    if (DualLedTester_netvars_count > 0)
    {
        NetVars_append_json_component("DualLedTester", DualLedTester_netvars_desc, DualLedTester_netvars_count, root);
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
    if (DualLedTester_netvars_count > 0)
    {
        NetVars_nvs_load_component("DualLedTester", DualLedTester_netvars_desc, DualLedTester_netvars_count);
    }
}

void DualLedTester_netvars_nvs_save(void)
{
    if (DualLedTester_netvars_count > 0)
    {
        NetVars_nvs_save_component("DualLedTester", DualLedTester_netvars_desc, DualLedTester_netvars_count);
    }
}

void DualLedTester_config_parse_json(const char *data)
{
    if (DualLedTester_netvars_count > 0)
    {
        bool nvs_cfg_changed = NetVars_parse_json_component_data("DualLedTester", DualLedTester_netvars_desc, DualLedTester_netvars_count, data);
        if (nvs_cfg_changed)
        {
            DualLedTester_nvs_set_dirty();
        }
    }
}

void DualLedTester_nvs_set_dirty(void)
{
    NetVars_nvs_set_dirty(&DualLedTester_nvs_mgr);
}

void DualLedTester_nvs_spin(void)
{
    if (NetVars_nvs_spin(&DualLedTester_nvs_mgr))
    {
        DualLedTester_netvars_nvs_save();
    }
}
