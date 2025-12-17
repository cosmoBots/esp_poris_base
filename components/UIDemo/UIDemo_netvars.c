#include <string.h>
#include <UIDemo.h>
#include "UIDemo_netvars.h"

static netvars_nvs_mgr_t UIDemo_nvs_mgr = {0};

const NetVars_desc_t UIDemo_netvars_desc[] = {
#include "UIDemo_netvars_fragment.c_"
};

const size_t UIDemo_netvars_count = sizeof(UIDemo_netvars_desc) / sizeof(UIDemo_netvars_desc[0]);

void UIDemo_netvars_append_json(cJSON *root)
{
    if (UIDemo_netvars_count > 0)
    {
        NetVars_append_json_component("UIDemo", UIDemo_netvars_desc, UIDemo_netvars_count, root);
    }
}

bool UIDemo_netvars_parse_json_dict(cJSON *root)
{
    if (UIDemo_netvars_count > 0)
    {
        return NetVars_parse_json_dict(UIDemo_netvars_desc, UIDemo_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void UIDemo_netvars_nvs_load(void)
{
    if (UIDemo_netvars_count > 0)
    {
        NetVars_nvs_load_component("UIDemo", UIDemo_netvars_desc, UIDemo_netvars_count);
    }
}

void UIDemo_netvars_nvs_save(void)
{
    if (UIDemo_netvars_count > 0)
    {
        NetVars_nvs_save_component("UIDemo", UIDemo_netvars_desc, UIDemo_netvars_count);
    }
}

void UIDemo_config_parse_json(const char *data)
{
    if (UIDemo_netvars_count > 0)
    {
        bool nvs_cfg_changed = NetVars_parse_json_component_data("UIDemo", UIDemo_netvars_desc, UIDemo_netvars_count, data);
        if (nvs_cfg_changed)
        {
            UIDemo_nvs_set_dirty();
        }
    }
}

void UIDemo_nvs_set_dirty(void)
{
    NetVars_nvs_set_dirty(&UIDemo_nvs_mgr);
}

void UIDemo_nvs_spin(void)
{
    if (NetVars_nvs_spin(&UIDemo_nvs_mgr))
    {
        UIDemo_netvars_nvs_save();
    }
}
