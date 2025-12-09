#pragma once
#ifndef RELAYSTEST_NETVARS_H
#define RELAYSTEST_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <NetVars.h>

void RelaysTest_netvars_nvs_load(void);
void RelaysTest_netvars_nvs_save(void);

void RelaysTest_netvars_append_json(cJSON *root);
bool RelaysTest_netvars_parse_json_dict(cJSON *root);

void RelaysTest_config_parse_json(const char *data);

void RelaysTest_nvs_set_dirty(void);
void RelaysTest_nvs_spin(void);

#ifdef __cplusplus
}
#endif

#endif // RELAYSTEST_NETVARS_H
