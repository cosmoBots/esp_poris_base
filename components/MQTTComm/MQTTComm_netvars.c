#include <string.h>
#include <MQTTComm.h>
#include "MQTTComm_netvars.h"

static netvars_nvs_mgr_t MQTTComm_nvs_mgr = {0};

const NetVars_desc_t MQTTComm_netvars_desc[] = {
#include "MQTTComm_netvars_fragment.c_"
};

const size_t MQTTComm_netvars_count = sizeof(MQTTComm_netvars_desc) / sizeof(MQTTComm_netvars_desc[0]);

void MQTTComm_netvars_append_json(cJSON *root)
{
    if (MQTTComm_netvars_count > 0)
    {
        NetVars_append_json_component("MQTTComm", MQTTComm_netvars_desc, MQTTComm_netvars_count, root);
    }
}

bool MQTTComm_netvars_parse_json_dict(cJSON *root)
{
    if (MQTTComm_netvars_count > 0)
    {
        return NetVars_parse_json_dict(MQTTComm_netvars_desc, MQTTComm_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void MQTTComm_netvars_nvs_load(void)
{
    if (MQTTComm_netvars_count > 0)
    {    
        NetVars_nvs_load_component("MQTTComm", MQTTComm_netvars_desc, MQTTComm_netvars_count);
    }
}

void MQTTComm_netvars_nvs_save(void)
{
    if (MQTTComm_netvars_count > 0)
    {     
        NetVars_nvs_save_component("MQTTComm", MQTTComm_netvars_desc, MQTTComm_netvars_count);
    }
}

void MQTTComm_config_parse_json(const char *data)
{
    if (MQTTComm_netvars_count > 0)
    {  
        bool nvs_cfg_changed = NetVars_parse_json_component_data("MQTTComm", MQTTComm_netvars_desc, MQTTComm_netvars_count, data);
        if (nvs_cfg_changed)
        {
            MQTTComm_nvs_set_dirty();
        }
    }
}

void MQTTComm_nvs_set_dirty(void)
{
    NetVars_nvs_set_dirty(&MQTTComm_nvs_mgr);
}

void MQTTComm_nvs_spin(void)
{
    if (NetVars_nvs_spin(&MQTTComm_nvs_mgr))
    {
        MQTTComm_netvars_nvs_save();
    }
}
