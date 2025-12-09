#pragma once
#ifndef DUALLED_NETVARS_H
#define DUALLED_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <NetVars.h>

void DualLED_netvars_nvs_load(void);
void DualLED_netvars_nvs_save(void);

void DualLED_netvars_append_json(cJSON *root);
bool DualLED_netvars_parse_json_dict(cJSON *root);

void DualLED_config_parse_json(const char *data);

#ifdef __cplusplus
}
#endif

#endif // DUALLED_NETVARS_H
