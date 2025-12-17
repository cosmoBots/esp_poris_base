#pragma once
#ifndef UIDEMO_NETVARS_H
#define UIDEMO_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <NetVars.h>

void UIDemo_netvars_nvs_load(void);
void UIDemo_netvars_nvs_save(void);

void UIDemo_netvars_append_json(cJSON *root);
bool UIDemo_netvars_parse_json_dict(cJSON *root);

void UIDemo_config_parse_json(const char *data);
void UIDemo_nvs_set_dirty(void);
void UIDemo_nvs_spin(void);

#ifdef __cplusplus
}
#endif

#endif // UIDEMO_NETVARS_H
