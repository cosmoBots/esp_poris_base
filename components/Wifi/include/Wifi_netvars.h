#pragma once
#ifndef WIFI_NETVARS_H
#define WIFI_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <NetVars.h>

void Wifi_netvars_nvs_load(void);
void Wifi_netvars_nvs_save(void);

void Wifi_netvars_append_json(cJSON *root);
bool Wifi_netvars_parse_json_dict(cJSON *root);

void Wifi_config_parse_json(const char *data);

#ifdef __cplusplus
}
#endif

#endif // WIFI_NETVARS_H
