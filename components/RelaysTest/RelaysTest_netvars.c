#include <string.h>
#include <RelaysTest.h>
#include "RelaysTest_netvars.h"

static netvars_nvs_mgr_t RelaysTest_nvs_mgr = {0};

const NetVars_desc_t RelaysTest_netvars_desc[] = {
#include "RelaysTest_netvars_fragment.c_"
};

const size_t RelaysTest_netvars_count = sizeof(RelaysTest_netvars_desc) / sizeof(RelaysTest_netvars_desc[0]);

void RelaysTest_netvars_append_json(cJSON *root)
{
    if (RelaysTest_netvars_count > 0)
    {
        NetVars_append_json_component("RelaysTest", RelaysTest_netvars_desc, RelaysTest_netvars_count, root);
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
    if (RelaysTest_netvars_count > 0)
    {
        NetVars_nvs_load_component("RelaysTest", RelaysTest_netvars_desc, RelaysTest_netvars_count);
    }
}

void RelaysTest_netvars_nvs_save(void)
{
    if (RelaysTest_netvars_count > 0)
    {
        NetVars_nvs_save_component("RelaysTest", RelaysTest_netvars_desc, RelaysTest_netvars_count);
    }
}

void RelaysTest_config_parse_json(const char *data)
{
    if (RelaysTest_netvars_count > 0)
    {
        bool nvs_cfg_changed = NetVars_parse_json_component_data("RelaysTest", RelaysTest_netvars_desc, RelaysTest_netvars_count, data);
        if (nvs_cfg_changed)
        {
            RelaysTest_nvs_set_dirty();
        }
    }
}

void RelaysTest_nvs_set_dirty(void)
{
    NetVars_nvs_set_dirty(&RelaysTest_nvs_mgr);
}

void RelaysTest_nvs_spin(void)
{
    if (NetVars_nvs_spin(&RelaysTest_nvs_mgr))
    {
        RelaysTest_netvars_nvs_save();
    }
}
