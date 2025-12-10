#include <string.h>
#include <LcdScreen.h>
#include "LcdScreen_netvars.h"

static netvars_nvs_mgr_t LcdScreen_nvs_mgr = {0};

const NetVars_desc_t LcdScreen_netvars_desc[] = {
#include "LcdScreen_netvars_fragment.c_"
};

const size_t LcdScreen_netvars_count = sizeof(LcdScreen_netvars_desc) / sizeof(LcdScreen_netvars_desc[0]);

void LcdScreen_netvars_append_json(cJSON *root)
{
    if (LcdScreen_netvars_count > 0)
    {
        NetVars_append_json_component("LcdScreen", LcdScreen_netvars_desc, LcdScreen_netvars_count, root);
    }
}

bool LcdScreen_netvars_parse_json_dict(cJSON *root)
{
    if (LcdScreen_netvars_count > 0)
    {
        return NetVars_parse_json_dict(LcdScreen_netvars_desc, LcdScreen_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void LcdScreen_netvars_nvs_load(void)
{
    if (LcdScreen_netvars_count > 0)
    {
        NetVars_nvs_load_component("LcdScreen", LcdScreen_netvars_desc, LcdScreen_netvars_count);
    }
}

void LcdScreen_netvars_nvs_save(void)
{
    if (LcdScreen_netvars_count > 0)
    {
        NetVars_nvs_save_component("LcdScreen", LcdScreen_netvars_desc, LcdScreen_netvars_count);
    }
}

void LcdScreen_config_parse_json(const char *data)
{
    if (LcdScreen_netvars_count > 0)
    {
        bool nvs_cfg_changed = NetVars_parse_json_component_data("LcdScreen", LcdScreen_netvars_desc, LcdScreen_netvars_count, data);
        if (nvs_cfg_changed)
        {
            LcdScreen_nvs_set_dirty();
        }
    }
}

void LcdScreen_nvs_set_dirty(void)
{
    NetVars_nvs_set_dirty(&LcdScreen_nvs_mgr);
}

void LcdScreen_nvs_spin(void)
{
    if (NetVars_nvs_spin(&LcdScreen_nvs_mgr))
    {
        LcdScreen_netvars_nvs_save();
    }
}
