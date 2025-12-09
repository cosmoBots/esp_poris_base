#include <string.h>
#include <UDPComm.h>
#include "UDPComm_netvars.h"

static netvars_nvs_mgr_t UDPComm_nvs_mgr = {0};

const NetVars_desc_t UDPComm_netvars_desc[] = {
#include "UDPComm_netvars_fragment.c_"
};

const size_t UDPComm_netvars_count = sizeof(UDPComm_netvars_desc) / sizeof(UDPComm_netvars_desc[0]);

void UDPComm_netvars_append_json(cJSON *root)
{
    if (UDPComm_netvars_count > 0)
    {
        NetVars_append_json_component("UDPComm", UDPComm_netvars_desc, UDPComm_netvars_count, root);
    }
}

bool UDPComm_netvars_parse_json_dict(cJSON *root)
{
    if (UDPComm_netvars_count > 0)
    {
        return NetVars_parse_json_dict(UDPComm_netvars_desc, UDPComm_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void UDPComm_netvars_nvs_load(void)
{
    if (UDPComm_netvars_count > 0)
    {    
        NetVars_nvs_load_component("UDPComm", UDPComm_netvars_desc, UDPComm_netvars_count);
    }
}

void UDPComm_netvars_nvs_save(void)
{
    if (UDPComm_netvars_count > 0)
    {        
        NetVars_nvs_save_component("UDPComm", UDPComm_netvars_desc, UDPComm_netvars_count);
    }
}

void UDPComm_config_parse_json(const char *data)
{
    if (UDPComm_netvars_count > 0)
    {         
        bool nvs_cfg_changed = NetVars_parse_json_component_data("UDPComm", UDPComm_netvars_desc, UDPComm_netvars_count, data);
        if (nvs_cfg_changed)
        {
            UDPComm_nvs_set_dirty();
        }
    }
}

void UDPComm_nvs_set_dirty(void)
{
    NetVars_nvs_set_dirty(&UDPComm_nvs_mgr);
}

void UDPComm_nvs_spin(void)
{
    if (NetVars_nvs_spin(&UDPComm_nvs_mgr))
    {
        UDPComm_netvars_nvs_save();
    }
}
