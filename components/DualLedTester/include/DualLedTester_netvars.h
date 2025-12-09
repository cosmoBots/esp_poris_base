#pragma once
#ifndef DUALLEDTESTER_NETVARS_H
#define DUALLEDTESTER_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <NetVars.h>

void DualLedTester_netvars_nvs_load(void);
void DualLedTester_netvars_nvs_save(void);

void DualLedTester_netvars_append_json(cJSON *root);
bool DualLedTester_netvars_parse_json_dict(cJSON *root);

void DualLedTester_config_parse_json(const char *data);

#ifdef __cplusplus
}
#endif

#endif // DUALLEDTESTER_NETVARS_H
