#pragma once
#ifndef MEASUREMENT_NETVARS_H
#define MEASUREMENT_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <NetVars.h>

void Measurement_netvars_nvs_load(void);
void Measurement_netvars_nvs_save(void);

void Measurement_netvars_append_json(cJSON *root);
bool Measurement_netvars_parse_json_dict(cJSON *root);

void Measurement_config_parse_json(const char *data);

void Measurement_nvs_set_dirty(void);
void Measurement_nvs_spin(void);

#ifdef __cplusplus
}
#endif

#endif // MEASUREMENT_NETVARS_H
