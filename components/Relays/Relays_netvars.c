#include <string.h>
#include <Relays.h>
#include "Relays_netvars.h"

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// Relays_dre_t Relays_dre;
extern Relays_dre_t Relays_dre;
static netvars_nvs_mgr_t Relays_nvs_mgr = {0};

const NetVars_desc_t Relays_netvars_desc[] = {
#include "Relays_netvars_fragment.c_"
};

const size_t Relays_netvars_count = sizeof(Relays_netvars_desc) / sizeof(Relays_netvars_desc[0]);

void Relays_netvars_append_json(cJSON *root)
{
    if (Relays_netvars_count > 0)
    {
        NetVars_append_json_component("Relays", Relays_netvars_desc, Relays_netvars_count, root);
    }
}

bool Relays_netvars_parse_json_dict(cJSON *root)
{
    if (Relays_netvars_count > 0)
    {
        return NetVars_parse_json_dict(Relays_netvars_desc, Relays_netvars_count, root);
    }
    else
    {
        return false;
    }
}

void Relays_netvars_nvs_load(void)
{
    if (Relays_netvars_count > 0)
    {
        NetVars_nvs_load_component("Relays", Relays_netvars_desc, Relays_netvars_count);
    }
}

void Relays_netvars_nvs_save(void)
{
    if (Relays_netvars_count > 0)
    {
        NetVars_nvs_save_component("Relays", Relays_netvars_desc, Relays_netvars_count);
    }
}

void Relays_config_parse_json(const char *data)
{
    if (Relays_netvars_count > 0)
    {
        bool nvs_cfg_changed = NetVars_parse_json_component_data("Relays", Relays_netvars_desc, Relays_netvars_count, data);
        if (nvs_cfg_changed)
        {
            Relays_nvs_set_dirty();
        }
    }
}

void Relays_nvs_set_dirty(void)
{
    NetVars_nvs_set_dirty(&Relays_nvs_mgr);
}

void Relays_nvs_spin(void)
{
    if (NetVars_nvs_spin(&Relays_nvs_mgr))
    {
        Relays_netvars_nvs_save();
    }
}
