#pragma once
#ifndef LCDSCREEN_NETVARS_H
#define LCDSCREEN_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <NetVars.h>

void LcdScreen_netvars_nvs_load(void);
void LcdScreen_netvars_nvs_save(void);

void LcdScreen_netvars_append_json(cJSON *root);
bool LcdScreen_netvars_parse_json_dict(cJSON *root);

void LcdScreen_config_parse_json(const char *data);
void LcdScreen_nvs_set_dirty(void);
void LcdScreen_nvs_spin(void);

#ifdef __cplusplus
}
#endif

#endif // LCDSCREEN_NETVARS_H
