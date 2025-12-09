#pragma once
#ifndef RELAYS_NETVARS_H
#define RELAYS_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <NetVars.h>

void Relays_netvars_nvs_load(void);
void Relays_netvars_nvs_save(void);

void Relays_netvars_append_json(cJSON *root);
bool Relays_netvars_parse_json_dict(cJSON *root);

void Relays_config_parse_json(const char *data);

void Relays_nvs_set_dirty(void);
void Relays_nvs_spin(void);

#ifdef __cplusplus
}
#endif

#endif // RELAYS_NETVARS_H
